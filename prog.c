#include <stdio.h>
#include <stdlib.h>

# include <pigpiod_if2.h>

#define HIGH 1
#define LOW 0

#define LEDPIN 17

#define LOOPCOUNT 10

int main(){
	int pd, t;

	if ((pd = pigpio_start(NULL, NULL)) < 0){
		printf("pigpio connection failed.\n");
		printf("pigpio check start.\n");
		exit(EXIT_FAILURE);
	}
	
	set_mode(pd, LEDPIN, PI_OUTPUT);

	t = 0;

	while(t < LOOPCOUNT){
		gpio_write(pd, LEDPIN, HIGH);
		time_sleep(0.5);
		gpio_write(pd, LEDPIN, LOW);
		time_sleep(0.5);
		t++;
	}
	
	pigpio_stop(pd);

	return 0;
}
