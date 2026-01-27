#include <stdio.h>
#include <stdlib.h>
#include <pigpiod_if2.h>

#define PWMPIN 18

#define SERVOCENTER 1450
#define SERVOMAX 2400
#define SERVOMIN 500
#define PWMFREQ 50

int main(int argc, char *argv[]) {
	int pd, angle, qflag, duty;

	if (argc != 2){
		angle = 0;
	}

	pd = pigpio_start(NULL, NULL);
	if (pd < 0) {
		fprintf(stderr, "pigpio connection failed.\ncheck pigpiod.\n");
		exit(1);
	}

	angle = atoi(argv[1]);

	if (-90 <= angle && angle <= 90){
		duty = (SERVOMIN + (SERVOMAX - SERVOMIN) * (angle + 90)  / 180) * PWMFREQ;
		hardware_PWM(pd, PWMPIN, PWMFREQ, duty);
	}

	pigpio_stop(pd);
	return 0;
}
