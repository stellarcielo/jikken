#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <pigpiod_if2.h>

#define HIGH 1
#define LOW 0

#define LOOPCOUNT 10

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

struct threadArgs{
	int pd;
	int inPin;
	int *pin;
};

void *countUp(void *args);

int main(){
	int pd;
	pthread_t thread;
	struct threadArgs *args;

	if ((pd = pigpio_start(NULL, NULL)) < 0){
		printf("pigpio connection failed.\n");
		printf("pigpio check start.\n");
		exit(EXIT_FAILURE);
	}

	int pin[7] = {4, 5, 6, 13, 16, 17, 19};
	int inPin = 20;	

	for(int i = 0; i < 7; i++)set_mode(pd, pin[i], PI_OUTPUT);
	set_mode(pd, inPin, PI_INPUT);

	args -> pd = pd;
	args -> inPin = inPin;
	args -> pin = pin;

	if ((pthread_create(&thread, NULL, countUp, (void *)args)) != 0){
		fprintf(stderr, "thread create failed.\n");
		exit(1);
	}

	char buf[10] = {0};

	printf("if you stop it, type 'q'.\n");

	while(1){
		scanf("%s", buf);
		if(buf[0] == 'q'){
			break;
		}
	}

	if((pthread_detach(thread)) != 0){
		fprintf(stderr, "pthread detach failed.\n");
		exit(1);
	}

	for(int i = 0; i < 7; i++) gpio_write(pd, pin[i], LOW);

	pigpio_stop(pd);

	return 0;
}

void *countUp(void *args){
	
	struct threadArgs *th_args = (struct threadArgs *)args;

	int num = 0;
	int hl_old = 0, hl = 0;

	while(1){
		hl = gpio_read(th_args -> pd, th_args -> inPin);
		if (hl != hl_old && hl == 0 && hl_old == 0){
			num++;
			hl_old = hl;
		}

		for(int i = 0; i < 7; i++){
			gpio_write(th_args -> pd, th_args -> pin[i],ledhl[num][i]);
		}

		time_sleep(0.1);
	}

}
