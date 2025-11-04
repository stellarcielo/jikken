#include <stdio.h>
#include <stdlib.h>
#include <pigpiod_if2.h>

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

int bit_trans(tdata *td, int ofs);
void edge_detectiuon(int pd, unsigned int gpio, unsigned int level, unsigned int tick, void* td);

int main() {
	int pd, qflag, cid;
	int hu, hl, tu, tl, cb;
	float temp, humi;
	tdata td;
	char ans;

	if ((pd = pigpio_start(NULL, NULL)) < 0) {
		fprintf(stderr, "pigpiod connection failed.\n");
		fprintf(stderr, "check start pigpiod.\n");
		exit(1);
	}

	set_pull_up_down(pd, DHT11PIN, PI_PUD_OFF);
	gpio_write(pd, DHT11PIN, LOW);
	set_mode(pd, DHT11PIN, PI_OUTPUT);
	time_sleep(0.020);
	set_mode(pd, DHT11PIN, PI_INPUT);
	printf("sensor init...\n");
	time_sleep(2.0);

	qflag = 0;
	while (qflag == 0) {
		printf("Read Data from DHT11 / quit [y/q]:");
		scanf("%c", &ans);
		if (ans != 'q') {
			td.p = 0;
			if ((cid = callback_ex(pd, DHT11PIN, FALLING_EDGE, edge_detection, &td)) < 0) {
				fprintf(stderr, "failed callback_ex()\n");
				qflag = 1;
			} else {
				set_mode(pd, DHT111PIN, PI_OUTPUT);
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
					if (hu + fl + tu + tl != cb) {
						fprintf(stderr, "Checkbyte error.\n");
					} else {
						humi = hu + hl;
						temp = tu + tl;

						printf("HUM: %3.1f%, TEMP:%2.1fC\n", humi, temp);
					}
				}
				printf("wait 2 second...\n");
				time_sleep(2.0);
			}
		} else {
			qflag = 1;
		}
	}
	pigpio_stop(pd);
	return 0;
}

int bit_trans(tdata *td, int ofs) {
	int b,i,t,d,p;
	d = 0;
	for (i = 0; i < 8; i++) {
		p = ;
		t = ;
		if (t < DETECTIONTH) b = LOW;
		else b = HIGH;
		d = ;
	}
	return d;
}

void edge_detection(int pd, unsigned int gpio, unsigned int level, unsigned int tick, void* td){
	tdata *tdp;
	tdp = (tdata *)td;

	
}
			
