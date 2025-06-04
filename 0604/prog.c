#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <pigpiod_if2.h>

#define HIGH 1
#define LOW 0

#define LEDPIN 17

int pd;
int level;

void *pwm_write(void *p);

int main(int argc, char *argv[]){
	
	if(argc != 2){
		fprintf(stderr, "Usage: %s <Level> <Control value>\n", argv[0]);
		fprintf(stderr, "Level: light level 0~9\n");
		exit(1);
	}
	level = atoi(argv[1]);

	if(0 > level && level > 9){
		fprintf(stderr, "Level is 0~9.");
		exit(1);
	}

	if ((pd = pigpio_start(NULL, NULL)) < 0){
		printf("pigpio connection failed.\n");
		printf("pigpio check start.\n");
		exit(EXIT_FAILURE);
	}
	
	set_mode(pd, LEDPIN, PI_OUTPUT);

	pthread_t thread;

	if((pthread_create(&thread, NULL, pwm_write, NULL)) != 0){
		fprintf(stderr, "Failed create pthread.");
		exit(1);
	}

	char data[10];
	printf("If you exit it, type 'q'.\n");
	while(data[0] != 'q'){
		fgets(data, 10, stdin);
	}

	if((pthread_detach(thread)) != 0){
		fprintf(stderr, "pthread join failed.");	
		exit(1);
	}

	gpio_write(pd, LEDPIN, LOW);
	
	pigpio_stop(pd);

	return 0;
}

void *pwm_write(void *p){
	while(1){
		if(level != 0)gpio_write(pd, LEDPIN, HIGH);
		time_sleep(level * 0.001);
		if(level != 9)gpio_write(pd, LEDPIN, LOW);
		time_sleep(0.01 - level * 0.001);
	}
}
