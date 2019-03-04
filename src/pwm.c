#include <wiringPi.h>
//#include <softPwm.h>
//#include <pigpio.h>
#include <stdio.h>
#include <signal.h>

#define PWM0 23 //wiringpi
#define PWM1 1 
#define PWM_PIN 31 //physical
#define BASE_CLK 19200000
#define PWM_PERIOD 50
#define PWMRNG 200  // range of pwm values that can be written 

int getPwmClk(int pwmRange){
    return BASE_CLK/(pwmRange * PWM_PERIOD);
}

float getConvFactor(){
    // For a pwm range of 100 -- 0 degrees -> 1; 180 degrees -> 13
    float pwmr_float = (float)PWMRNG;
    return ((pwmr_float/100) * 13)/180; 

}

int getPwmValue(int angle){  // from 0 -180
    // 1 represents the 0 degree position -- from experiment
    if(angle == 0) return 1;
    
    //13 represents the 180 position -- from experiment
    float conv_factor = getConvFactor();
    int toWrite = (int)(conv_factor * angle);
    return toWrite; 
}

void turnMotor(int angle, int pin){
    int toWrite = getPwmValue(angle);
    pwmWrite(pin, toWrite);
    printf("angle: %d - pwmVal: %d\n", angle,  toWrite);
}

void stopMotor(int pin){
    pwmWrite(pin, 0);
}

void pwmSetup(){
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(PWMRNG);
    int clk = getPwmClk(PWMRNG);
    pwmSetClock(clk);  
    printf("pwm_range: %d\npwm_clk: %d\n", PWMRNG, clk);
}

int main(){
    wiringPiSetup();
    
    
    //pinMode (PWM0, PWM_OUTPUT);
    pinMode (PWM1, PWM_OUTPUT);
    pwmSetup();

    turnMotor(0, PWM1);
    delay(1000);
    
    turnMotor(90, PWM1);
    delay(1000);
    
    turnMotor(180, PWM1);
    delay(1000);
    
    stopMotor(PWM1);
}
    /*
    gpioCfgClock(5,1,0);
    gpioInitialise();
    gpioHardwarePWM(PWM, 50, 7);
    */

