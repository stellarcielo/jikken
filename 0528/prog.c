#include <stdio.h>
#include <stdlib.h>

#include <pigpiod_if2.h>

#define HIGH 1
#define LOW 0

#define LEDPIN 17

#define LOOPCOUNT 1000

int main(int argc, char *argv[]){
	
	if(argc != 2){
		fprintf(stderr, "Usage: %s <Level> <Control value>\n", argv[0]);
		fprintf(stderr, "Level: light level 0~9\nControl value: control value 0~9");
		exit(1);
	}
	int level = atoi(argv[1]);
	//int ctrl = atoi(argv[2]);

	if(0 > level && level < 9){
		fprintf(stderr, "Level is 0~9.");
		exit(1);
	}
	//if(0 > ctrl && ctrl < 9){
	//	fprintf(stderr, "Control value is 0~9.");
	//	exit(1);
	//}

	int pd, t;

	if ((pd = pigpio_start(NULL, NULL)) < 0){
		printf("pigpio connection failed.\n");
		printf("pigpio check start.\n");
		exit(EXIT_FAILURE);
	}
	
	set_mode(pd, LEDPIN, PI_OUTPUT);

	t = 0;

	while(t < LOOPCOUNT){
		if(level != 0)gpio_write(pd, LEDPIN, HIGH);
		time_sleep(level * 0.001);
		if(level != 9)gpio_write(pd, LEDPIN, LOW);
		time_sleep(0.01 - level * 0.001);
		t++;
	}
	
	pigpio_stop(pd);

	return 0;
}
