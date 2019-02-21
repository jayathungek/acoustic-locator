#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include <wiringPi.h>

#define WS 8000 // Word Select frequency
#define SF 64   // Scaling Factor to get clock frequency
#define MF 0.5  // MUX scaling factor

int main(){
    wiringPiSetupGpio();
    pinMode(7, GPIO_CLOCK); 
    pinMode(29, GPIO_CLOCK);
    pinMode(31, GPIO_CLOCK);
    
    gpioClockSet (7, WS) ; // Word select set on pin 7
    gpioClockSet (29, SF*WS) ; // Clock set on pin 29
    gpioClockSet (31, WS*MF) ; //MUX set on pin 31
    while(1){
    }
    return 0;
}
