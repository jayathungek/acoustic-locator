#include <stdio.h>
#include <wiringPiI2C.h>
#include <time.h>

int main(){
    int tempsensor = wiringPiI2CSetup(0x27);

    while(1){
    	int b1 = wiringPiI2CReadReg8(tempsensor, 0x00); // first byte
	int b2 = wiringPiI2CReadReg8(tempsensor, 0x00);
    	int b3 = wiringPiI2CReadReg8(tempsensor, 0x00);
	int b4 = wiringPiI2CReadReg8(tempsensor, 0x00);

 	int temphigh = b3 & 0xff; //all 8 bits of 3rd byte required
	int templow = b4 >> 2; // rshift to get rid of last 2 bits in 4th byte
    	int temp = temphigh | templow;  //combine to get 14-bit temperature
    	printf("Temp: %d\r", temp);
        usleep(5000);
    }
    return 0;
}
