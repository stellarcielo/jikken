#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <pigpiod_if2.h>

#define HIGH 1
#define LOW 0

void signalHandler(int signalType);

int pd;

int pin[7] = {4, 5, 6, 13, 16, 17, 19};

int main(int argc, char *argv[]){
	int num;

	if (argc != 2){
		fprintf(stderr, "Usage: %s <NUMBER>", argv[0]);
	}

	if ((pd = pigpio_start(NULL, NULL)) < 0){
		printf("pigpio connection failed.\n");
		printf("pigpio check start.\n");
		exit(EXIT_FAILURE);
	}

	struct sigaction handler;

	handler.sa_handler = signalHandler;

	if ((sigfillset(&handler.sa_mask)) < 0){
		fprintf(stderr, "sigfillset() failed");
		exit(1);
	}

	handler.sa_flags = 0;

	if((sigaction(SIGINT, &handler, 0)) < 0){
		fprintf(stderr, "sigaction() failed");
		exit(1);
	}

	num = atoi(argv[1]);

	num = num % 10;

	for(int i = 0; i < 7; i++)set_mode(pd, pin[i], PI_OUTPUT);

	int ledhl[10][7] = {
		{1, 1, 1, 1, 1, 1, 0} ,
		{0, 1, 1, 0, 0, 0, 0} ,
		{1, 1, 0, 1, 1, 0, 1} ,
		{1, 1, 1, 1, 0, 0, 1} ,
		{0, 1, 1, 0, 0, 1, 1} ,
		{1, 0, 1, 1, 0, 1, 1} ,
		{1, 0, 1, 1, 1, 1, 1} ,
		{1, 1, 1, 0, 0, 1, 0} ,
		{1, 1, 1, 1, 1, 1, 1} ,
		{1, 1, 1, 0, 0, 1, 1}
	};

	for (int i = 0; i < 7; i++){
		gpio_write(pd, pin[i], ledhl[num][i]);
	}
	while(1){
	}
}

void signalHandler(int signalType){
	for (int i = 0; i < 7; i++){
		gpio_write(pd, pin[i], LOW);
	}

	pigpio_stop(pd);
	exit(0);
}
