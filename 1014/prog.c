#include <stdio.h>
#include <stdlib.h>
#include <pigpiod_if2.h>

#define PWMPIN 18

#define SERVOCENTER 1450
#define SERVOMAX 2400
#define SERVOMIN 500
#define PWMFREQ 50

int main() {
	int pd, angle, qflag, duty;

	pd = pigpio_start(NULL, NULL);
	if (pd < 0) {
		printf("pigpio connection failed.\ncheck pigpiod.\n");
		exit(1);
	}

	printf("\n");

	qflag = 0;
	while(qflag == 0) {
		printf("Input angle (leftside:-90 ~ 0 ~ +90:right side) :");
		scanf("%d", &angle);
		if ((angle >= -90) && (angle <= 90)) {
			duty = (SERVOMIN + (SERVOMAX - SERVOMIN) * (angle + 90)  / 180) * PWMFREQ;
			printf("duty = %d\n", duty);
			hardware_PWM(pd, PWMPIN, PWMFREQ, duty);
		} else {
			qflag = 1;
		}
	}

	duty = (SERVOMIN + (SERVOMAX - SERVOMIN) * (0 + 90)  / 180) * PWMFREQ;
	hardware_PWM(pd, PWMPIN, PWMFREQ, duty);
	time_sleep(0.5);
	hardware_PWM(pd, PWMPIN, PWMFREQ, 0);

	pigpio_stop(pd);
	return 0;
}
