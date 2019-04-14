#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h> 
#include <signal.h>

#define LASER      22
#define ELEVATION  23   //top
#define AZIMUTH    26   //bottom
#define BASE_CLK   19200000
#define PWM_FREQ   50   //Hz
#define PWMRNG     2000  // range of pwm values that can be written
#define DELAY      500   // motor delay, ms 

#define AZ_MAX     90
#define AZ_MIN    -90
#define EL_MAX     70
#define EL_MIN    -35

int az_curr;
int el_curr;

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

void turnMotorBy(int angle, int motor){
	int new_angle;
	switch(motor){
    	case AZIMUTH:
    		new_angle = az_curr + angle;
    		if(new_angle > AZ_MIN && new_angle < AZ_MAX){
    			az_curr = new_angle;
    			turnMotorTo(az_curr, AZIMUTH);
    		}
    		break;
    	
    	case ELEVATION:
    		new_angle = el_curr + angle;
    		if(new_angle > EL_MIN && new_angle < EL_MAX){
    			el_curr = new_angle;
    			turnMotorTo(el_curr, ELEVATION);
    		}
    		break;
    	default:
    		printf("Motor not found: %d\n", motor);
    }
}

void zeroAzimuth(){
	turnMotorTo(0, AZIMUTH);
	az_curr = 0;
}

void zeroElevation(){
	turnMotorTo(0, ELEVATION);
	el_curr = 0;
}

void pwmSetup(){
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(PWMRNG);
    int clk = getPwmClk(PWMRNG);
    pwmSetClock(clk);
    az_curr = 0;
    el_curr = 0;
    printf("pwm_range: %d\npwm_clk: %d\n", PWMRNG, clk);
}

int main(int argc, char *argv[]){
    wiringPiSetup();

    int val = strtol(argv[1], NULL, 10);
    pinMode (AZIMUTH, PWM_OUTPUT);
    pinMode (ELEVATION, PWM_OUTPUT);
    pwmSetup();
    
    zeroAzimuth();
    zeroElevation();
	delay(DELAY);

	turnMotorBy(45, AZIMUTH);
	turnMotorBy(45, ELEVATION);
	delay(DELAY);
	turnMotorBy(15, AZIMUTH);
	turnMotorBy(15, ELEVATION);
    delay(DELAY);
    turnMotorBy(-15, AZIMUTH);
	turnMotorBy(-15, ELEVATION);
	delay(DELAY);
	turnMotorBy(-45, AZIMUTH);
	turnMotorBy(-45, ELEVATION);
    delay(DELAY);
    return 0;
    
}
    /*
    gpioCfgClock(5,1,0);
    gpioInitialise();
    gpioHardwarePWM(PWM, 50, 7);
    turnMotorTo(0, PWM0);
    delay(1000);
    
    turnMotorTo(90, PWM0);
    delay(1000);
    
    turnMotorTo(180, PWM0);
    delay(1000);
    
    stopMotor(PWM0);
    
    turnMotorTo(0, PWM1);
    delay(1000);
    
    turnMotorTo(90, PWM1);
    delay(1000);
    
    turnMotorTo(180, PWM1);
    delay(1000);
    
    stopMotor(PWM1);
    */

