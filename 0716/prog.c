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
	{1, 1, 1, 1, 0, 1, 1}
};
int stop_flag = 0;

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

	args = (struct threadArgs *)malloc(sizeof(struct threadArgs));

	if ((pd = pigpio_start(NULL, NULL)) < 0){
		printf("pigpio connection failed.\n");
		printf("pigpio check start.\n");
		exit(EXIT_FAILURE);
	}

	int pin[7] = {4, 5, 6, 13, 16, 17, 19};
	int inPin = 20;	

	for(int i = 0; i < 7; i++)set_mode(pd, pin[i], PI_OUTPUT);
	set_mode(pd, inPin, PI_INPUT);
	set_pull_up_down(pd, inPin, PI_PUD_UP);

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
			stop_flag = 1;
			break;
		}
	}

	if((pthread_join(thread, NULL)) != 0){
		fprintf(stderr, "pthread detach failed.\n");
		exit(1);
	}

	for(int i = 0; i < 7; i++) gpio_write(pd, pin[i], LOW);

	pigpio_stop(pd);

	free(args);

	return 0;
}

void *countUp(void *args){
	struct threadArgs *th_args = (struct threadArgs *) args;

	int num = 0;
	int hl_old = 1, hl = 1;

	while(!stop_flag){
		hl = gpio_read(th_args -> pd, th_args -> inPin);
		if (hl == 0 && hl_old == 1){
			num = (num + 1) %10;

			for(int i = 0; i < 7; i++){
				gpio_write(th_args -> pd, th_args -> pin[i],ledhl[num][i]);
			}
		}
		hl_old = hl;

		time_sleep(0.02);
	}

}
