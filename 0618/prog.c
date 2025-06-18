#include <stdio.h>
#include <stdlib.h>

# include <pigpiod_if2.h>

#define HIGH 1
#define LOW 0

#define LOOPCOUNT 10

int main(){
	int pd, num;

	if ((pd = pigpio_start(NULL, NULL)) < 0){
		printf("pigpio connection failed.\n");
		printf("pigpio check start.\n");
		exit(EXIT_FAILURE);
	}

	scanf("%d", &num);

	num = num % 10;

	int pin[7] = {4, 5, 6, 13, 16, 17, 19};
	
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
	time_sleep(10);

	for (int i = 0; i < 7; i++){
		gpio_write(pd, pin[i], LOW);
	}
	
	pigpio_stop(pd);

	return 0;
}
