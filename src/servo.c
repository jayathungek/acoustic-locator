#include "servo.h"
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
//Function definitions


int getPwmClk(int pwmRange){
    return BASE_CLK/(pwmRange * PWM_FREQ);
}


int getPwmValue(int angle){  // from -90 to +90
    if (angle > 90){
    	printf("err: Max angle is 90\n");
    	exit(-1);
    }

    if (angle < -90){
    	printf("err: Min angle is -90\n");
    	exit(-1);
    }

    return 160 - angle;

}

void turnMotorTo(int angle, int motor){
    int toWrite = getPwmValue(angle);
    pwmWrite(motor, toWrite);
    printf("angle: %d - pwmVal: %d\n", angle,  toWrite);
}

void stopMotor(int pin){
    pwmWrite(pin, 0);
}

void stopMotors(){
	stopMotor(AZIMUTH);
	stopMotor(ELEVATION);
}

void turnMotorBy(int angle, int motor, int *curr){
	int new_angle;
	switch(motor){
    	case AZIMUTH:
    		new_angle = *curr + angle;
    		if(new_angle > AZ_MIN && new_angle < AZ_MAX){
    			*curr = new_angle;
    			turnMotorTo(*curr, AZIMUTH);
    		}
    		break;

    	case ELEVATION:
    		new_angle = *curr + angle;
    		if(new_angle > EL_MIN && new_angle < EL_MAX){
    			*curr = new_angle;
    			turnMotorTo(*curr, ELEVATION);
    		}
    		break;
    	default:
    		printf("Motor not found: %d\n", motor);
    }
}

void zeroAzimuth(int *curr){
	turnMotorTo(0, AZIMUTH);
	*curr = 0;
}

void zeroElevation(int *curr){
	turnMotorTo(0, ELEVATION);
	*curr = 0;
}

void zeroMotors(int *az_curr, int *el_curr){
	zeroAzimuth(az_curr);
	zeroElevation(el_curr);
}

void pwmSetup(){
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(PWMRNG);
    int clk = getPwmClk(PWMRNG);
    pwmSetClock(clk);
    printf("pwm_range: %d\npwm_clk: %d\n", PWMRNG, clk);
}

void laserOn(){
	digitalWrite(LASER, 1);
}

void laserOff(){
	digitalWrite(LASER, 0);
}
