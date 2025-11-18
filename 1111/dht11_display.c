#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pigpiod_if2.h>
#include <pthread.h>

#define DHT11PIN 27

#define HUMIUPPER 0
#define HUMILOWER 1
#define TEMPUPPER 2
#define TEMPLOWER 3
#define CHECKBYTE 4

#define MAXEDGECOUNT 100
#define DETECTUM 43
#define DETECTIONTH 100

#define HIGH 1
#define LOW 0

typedef struct {
    unsigned int timedata[MAXEDGECOUNT];
    int p;
} tdata;

typedef struct {
    int pd;
    int dPin[4];
    int pin[8];
    float hum;
    float temp;
    pthread_mutex_t lock;
} threadArgs;

int stop_flag = 0;

int bit_trans(tdata *td, int ofs);
void edge_detection(int pd, unsigned int gpio, unsigned int level, unsigned int tick, void* td);
void *displayDigit(void *args);

/* 7seg a〜g の点灯パターン（0/1 はセグメント ON/OFF、回路に応じて反転させてください） */
int ledhl[10][7] = {
    {0,0,0,0,0,0,1}, //0
    {1,0,0,1,1,1,1}, //1
    {0,0,1,0,0,1,0}, //2
    {0,0,0,0,1,1,0}, //3
    {1,0,0,1,1,0,0}, //4
    {0,1,0,0,1,0,0}, //5
    {0,1,0,0,0,0,0}, //6
    {0,0,0,1,1,0,1}, //7
    {0,0,0,0,0,0,0}, //8
    {0,0,0,0,1,0,0}  //9
};

int main() {
    int pd, qflag = 0, cid;
    int hu, hl, tu, tl, cb;
    float temp, humi;
    tdata td;
    char ans;
    pthread_t thread = 0;

    if ((pd = pigpio_start(NULL, NULL)) < 0) {
        fprintf(stderr, "pigpiod connection failed.\n");
        return 1;
    }

    threadArgs *args = malloc(sizeof(threadArgs));
    if (!args) {
        fprintf(stderr, "malloc failed\n");
        pigpio_stop(pd);
        return 1;
    }
    pthread_mutex_init(&args->lock, NULL);

    /* ピン定義（必要ならここを変更） */
    int pin_tmp[8]  = {19,20,21,22,23,24,25,26}; // a b c d e f g dp
    int dPin_tmp[4] = {4,5,6,12};                // digit select

    /* args にコピー（stack 参照を避ける） */
    for (int i=0;i<8;i++) args->pin[i] = pin_tmp[i];
    for (int i=0;i<4;i++) args->dPin[i] = dPin_tmp[i];
    args->pd = pd;
    args->hum = 0.0f;
    args->temp = 0.0f;

    /* DHT11 初期化 */
    set_pull_up_down(pd, DHT11PIN, PI_PUD_OFF);
    gpio_write(pd, DHT11PIN, LOW);
    set_mode(pd, DHT11PIN, PI_OUTPUT);
    time_sleep(0.020);
    set_mode(pd, DHT11PIN, PI_INPUT);
    printf("sensor init...\n");
    time_sleep(2.0);

    /* 7seg GPIO 出力設定 */
    for (int i = 0; i < 8; i++) set_mode(pd, args->pin[i], PI_OUTPUT);
    for (int i = 0; i < 4; i++) set_mode(pd, args->dPin[i], PI_OUTPUT);

    while (!qflag) {
        printf("Read Data from DHT11 / quit [y/q]: ");
        if (scanf(" %c", &ans) != 1) break;

        if (ans == 'q') {
            qflag = 1;
            stop_flag = 1;
            break;
        }

        /* 表示スレッドを停止して結合 */
        stop_flag = 1;
        if (thread) pthread_join(thread, NULL);

        td.p = 0;
        cid = callback_ex(pd, DHT11PIN, FALLING_EDGE, edge_detection, &td);
        if (cid < 0) {
            fprintf(stderr, "failed callback_ex()\n");
            break;
        }

        /* DHT11 に start signal を送る（0.02s） */
        set_mode(pd, DHT11PIN, PI_OUTPUT);
        gpio_write(pd, DHT11PIN, LOW);
        time_sleep(0.020);
        set_mode(pd, DHT11PIN, PI_INPUT);
        time_sleep(0.010);

        if (callback_cancel(cid) != 0) fprintf(stderr, "failed callback_cancel\n");

        printf("falling times: %d\n", td.p);
        if (td.p != DETECTUM) {
            fprintf(stderr, "failed reading data.\n");
            continue;
        }

        hu = bit_trans(&td, HUMIUPPER);
        hl = bit_trans(&td, HUMILOWER);
        tu = bit_trans(&td, TEMPUPPER);
        tl = bit_trans(&td, TEMPLOWER);
        cb = bit_trans(&td, CHECKBYTE);

        if (((hu + hl + tu + tl) & 0xFF) != cb) {
            printf("Checkbyte error: sum=%d cb=%d\n", hu + hl + tu + tl, cb);
            continue;
        }

        humi = hu + hl * 0.1f;
        temp = tu + (tl & 0x7F) * 0.1f;
        if (tl & 0x80) temp = -temp;

        printf("HUM: %3.1f%%, TEMP: %2.1fC\n", humi, temp);

        /* スレッド引数更新（排他） */
        pthread_mutex_lock(&args->lock);
        args->hum = humi;
        args->temp = temp;
        pthread_mutex_unlock(&args->lock);

        stop_flag = 0;
        if (pthread_create(&thread, NULL, displayDigit, (void *)args) != 0) {
            fprintf(stderr, "thread create failed.\n");
            stop_flag = 1;
        }

        printf("wait 2 second...\n");
        time_sleep(2.0);
    }

    /* 終了処理 */
    stop_flag = 1;
    if (thread) pthread_join(thread, NULL);
    pthread_mutex_destroy(&args->lock);
    free(args);
    pigpio_stop(pd);
    return 0;
}

int bit_trans(tdata *td, int ofs) {
    int b,i,t,d,p;
    d = 0;
    for (i = 0; i < 8; i++) {
        p = ofs * 8 + 3 + i;
        t = td->timedata[p] - td->timedata[p - 1];
        b = (t < DETECTIONTH) ? LOW : HIGH;
        d = (d << 1) | b;
    }
    return d;
}

void edge_detection(int pd, unsigned int gpio, unsigned int level, unsigned int tick, void* td){
    tdata *tdp = (tdata *)td;
    if (tdp->p < MAXEDGECOUNT) {
        tdp->timedata[tdp->p++] = tick;
    }
}

void *displayDigit(void *args) {
    threadArgs *a = (threadArgs *) args;
    int pd = a->pd;

    while (!stop_flag) {
        float humv, tempv;
        pthread_mutex_lock(&a->lock);
        humv = a->hum;
        tempv = a->temp;
        pthread_mutex_unlock(&a->lock);

        int hum1 = ((int)humv) / 10;
        int hum2 = ((int)humv) % 10;
        int temp1 = ((int)tempv) / 10;
        int temp2 = ((int)tempv) % 10;

        /* 湿度表示（左2桁） */
        for (int cycle = 0; cycle < 300 && !stop_flag; cycle++) {
            int digits[4] = { hum1, hum2, -1, -1 }; // -1 は表示オフ
            for (int i = 0; i < 4; i++) {
                /* まず桁選択を行う（digitピンを ON にして他を OFF） */
                for (int d = 0; d < 4; d++) {
                    gpio_write(pd, a->dPin[d], (d == i) ? 1 : 0);
                }
                /* セグメント設定 */
                if (digits[i] >= 0) {
                    for (int s = 0; s < 7; s++) {
                        gpio_write(pd, a->pin[s], ledhl[digits[i]][s]);
                    }
                } else {
                    for (int s = 0; s < 7; s++) {
                        /* 消灯（回路に合わせて1/0を調整） */
                        gpio_write(pd, a->pin[s], 1);
                    }
                }
                time_sleep(0.002);
            }
        }

        /* 温度表示（右2桁） */
        for (int cycle = 0; cycle < 300 && !stop_flag; cycle++) {
            int digits[4] = { -1, -1, temp1, temp2 };
            for (int i = 0; i < 4; i++) {
                for (int d = 0; d < 4; d++) {
                    gpio_write(pd, a->dPin[d], (d == i) ? 1 : 0);
                }
                if (digits[i] >= 0) {
                    for (int s = 0; s < 7; s++) {
                        gpio_write(pd, a->pin[s], ledhl[digits[i]][s]);
                    }
                } else {
                    for (int s = 0; s < 7; s++) {
                        gpio_write(pd, a->pin[s], 1);
                    }
                }
                time_sleep(0.002);
            }
        }
    }

    /* 全桁消灯して終了 */
    for (int d = 0; d < 4; d++) gpio_write(pd, a->dPin[d], 0);
    for (int s = 0; s < 7; s++) gpio_write(pd, a->pin[s], 1);

    return NULL;
}
