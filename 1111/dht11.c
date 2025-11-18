#include <stdio.h>
#include <stdlib.h>
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
	int *dPin;
	int *pin;
	float hum;
	float temp;
} threadArgs;
int stop_flag = 0;

int bit_trans(tdata *td, int ofs);
void edge_detection(int pd, unsigned int gpio, unsigned int level, unsigned int tick, void* td);
void *displayDigit(void *args);

int ledhl[10][7] =  {
	{0,0,0,0,0,0,1},	//0
	{1,0,0,1,1,1,1},	//1
	{0,0,1,0,0,1,0},	//2
	{0,0,0,0,1,1,0},	//3
	{1,0,0,1,1,0,0},	//4
	{0,1,0,0,1,0,0},	//5
	{0,1,0,0,0,0,0},	//6
	{0,0,0,1,1,0,1},	//7
	{0,0,0,0,0,0,0},	//8
	{0,0,0,0,1,0,0}};	//9

int main() {
	int pd, qflag, cid;
	int hu, hl, tu, tl, cb;
	float temp, humi;
	tdata td;
	char ans;
	pthread_t thread = 0;

	if ((pd = pigpio_start(NULL, NULL)) < 0) {
		fprintf(stderr, "pigpiod connection failed.\n");
		fprintf(stderr, "check start pigpiod.\n");
		exit(1);
	}

	threadArgs *args = malloc(sizeof(threadArgs));

	set_pull_up_down(pd, DHT11PIN, PI_PUD_OFF);
	gpio_write(pd, DHT11PIN, LOW);
	set_mode(pd, DHT11PIN, PI_OUTPUT);
	time_sleep(0.020);
	set_mode(pd, DHT11PIN, PI_INPUT);
	printf("sensor init...\n");
	time_sleep(2.0);

	int pin[8] = {19,20,21,22,23,24,25,26}; //include DP.
	int dPin[4] = {4,5,6,12};

	args->pd = pd;
	args->dPin = dPin;
	args->pin = pin;

	for (int i = 0; i < 8; i++) set_mode(pd, pin[i], PI_OUTPUT);
	for (int i = 0; i < 4; i++) set_mode(pd, dPin[i], PI_OUTPUT);

	qflag = 0;
	while (qflag == 0) {
		printf("Read Data from DHT11 / quit [y/q]:");
		scanf(" %c", &ans);
		if (ans != 'q') {
			stop_flag = 1;
			if (thread) pthread_join(thread, NULL);

			td.p = 0;
			if ((cid = callback_ex(pd, DHT11PIN, FALLING_EDGE, edge_detection, &td)) < 0) {
				fprintf(stderr, "failed callback_ex()\n");
				qflag = 1;
			} else {
				set_mode(pd, DHT11PIN, PI_OUTPUT);
				time_sleep(0.020);
				set_mode(pd, DHT11PIN, PI_INPUT);
				time_sleep(0.010);

				if (callback_cancel(cid) != 0) fprintf(stderr, "failed callback_cancel\n");

				printf("falling times: %d\n", td.p);
				if(td.p != DETECTUM) {
					fprintf(stderr, "failed reading data.\n");
				} else {
					hu = bit_trans(&td, HUMIUPPER);
					hl = bit_trans(&td, HUMILOWER);
					tu = bit_trans(&td, TEMPUPPER);
					tl = bit_trans(&td, TEMPLOWER);
					cb = bit_trans(&td, CHECKBYTE);
					if (((hu + hl + tu + tl) & 0xFF) != cb) {
						printf("%d %d\n", hu + hl + tu + tl, cb);
						fprintf(stderr, "Checkbyte error.\n");
					} else {
						humi = hu + hl * 0.1;
						temp = tu + (tl & 0x7F) * 0.1;

						if(tl & 0x80) temp = -temp;

						printf("HUM: %3.1f%, TEMP:%2.1fC\n", humi, temp);
						args->hum = humi;
						args->temp = temp;

						stop_flag = 0;

						pthread_create(&thread, NULL, displayDigit, (void *)args);
					}
				}
				printf("wait 2 second...\n");
				time_sleep(2.0);
			}
		} else {
			qflag = 1;
			stop_flag = 1;
		}
	}

	if (thread) pthread_join(thread, NULL);
	pigpio_stop(pd);
	return 0;
}

int bit_trans(tdata *td, int ofs) {
	int b,i,t,d,p;
	d = 0;
	for (i = 0; i < 8; i++) {
		p = ofs * 8 + 3 + i;
		t = td->timedata[p] - td->timedata[p - 1];
		if (t < DETECTIONTH) b = LOW;
		else b = HIGH;
		d = (d << 1) | b;
	}
	return d;
}

void edge_detection(int pd, unsigned int gpio, unsigned int level, unsigned int tick, void* td){
	tdata *tdp;
	tdp = (tdata *)td;

	if (tdp -> p < MAXEDGECOUNT) {
		tdp -> timedata[tdp -> p++] = tick;
	}
}

void *displayDigit(void *args) {
	threadArgs *a = (threadArgs *) args;

	int pd = a->pd;
	int *pin = a->pin;
	int *dPin = a->pin;

	int hum1 = ((int)a->hum) / 10;
	int hum2 = ((int)a->hum) % 10;

	int temp1 = ((int)a->temp) / 10;
	int temp2 = ((int)a->temp) % 10;

	while (!stop_flag) {
		for (int k = 0;k < 300; k++) {
			int digits[4] = {hum1, hum2, -1, -1};

			for (int i = 0; i < 4; i++) {
				for (int d = 0; d < 4; d++) {
					gpio_write(pd, pin[d], (d == i) ? 1 : 0);

					if (digits[i] >= 0) {
						for (int s = 0; s < 7; s++) {
							gpio_write(pd, pin[s], ledhl[digits[i]][s]);
						}
					} else {
						for (int s = 0; s < 7; s++) {
							gpio_write(pd, pin[s], 1);
						}
					}
					time_sleep(0.002);
				}
			}
		}

		for (int k = 0; k < 300; k++) {
			int digits[4] = {-1, -1, temp1, temp2};

			for (int i = 0; i < 4; i++) {
				for (int d = 0; d < 4; d++) {
					gpio_write(pd, dPin[d], (d == i) ? 1 : 0);

					if (digits[i] >= 0) {
						for (int s = 0; s < 7; s++) {
							gpio_write(pd, pin[s], ledhl[digits[i]][s]);
						}
					} else {
						for (int s = 0; s < 7; s++) {
							gpio_write(pd, pin[s], 1);
						}
					}
					time_sleep(0.002);
				}
			}
		}
	}

	return NULL;
}

			
