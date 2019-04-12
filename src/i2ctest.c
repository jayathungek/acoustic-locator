#include <stdio.h>
#include <wiringPiI2C.h>
#include <time.h>

int main(){
    int tempsensor = wiringPiI2CSetup(0x27);
    printf("Sensor handle: %d\n", tempsensor);
    while(1){
      	int b1 = wiringPiI2CReadReg8(tempsensor, 0x00); // first byte
        int b2 = wiringPiI2CReadReg8(tempsensor, 0x10);
	/*
        int b2 = wiringPiI2CReadReg8(tempsensor, 0x00);
    	int b3 = wiringPiI2CReadReg8(tempsensor, 0x00);
	int b4 = wiringPiI2CReadReg8(tempsensor, 0x00);

 	int temphigh = b3 & 0xff; //all 8 bits of 3rd byte required
	int templow = b4 >> 2; // rshift to get rid of last 2 bits in 4th byte
    	int temp = temphigh | templow;  //combine to get 14-bit temperature
         */
        int tempb1 =( b1 & 0x00ff);
        int tempb2 = b2 & 0x00ff;
    	printf("b1: %x, b2: %x, Tempb1: %x, Tempb2:  %x\r", b1, b2, tempb1, tempb2);
        usleep(10000);
    }
    return 0;
}
