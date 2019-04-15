#include "utils.h"
#include <math.h>
#include <stdio.h>

int convertValue(char msb, char lsb){
	int usingned =  (lsb | msb << 8);
	return -(usingned & 0x8000) | (usingned & 0x7fff);
}

double hyperbola(double l, double x){
	double exp1 = (pow(l, 2)/4) - pow(MIC_X, 2);
	double exp2 = ((4*pow(MIC_X, 2))/pow(l, 2)) - 1;
	double exp3 = pow(x, 2) * exp2;
	return sqrt(exp1 + exp3);

}

float getDevFromNormal(float delta){
	// all distances/coords in cm
	// l - extra distance sound has to travel to further mic
	if(delta == 0) return 0;
	double l  = (double)VSOUND * delta;
	double x1 = 20; //arbitrary, but must be above the parabola minimum
	double x2 = 40; // x2 > x1
	double y1 = hyperbola(l, x1);
	double y2 = hyperbola(l, x2);

	double alpha_prime = atan((y2 - y1)/(x2 - x1));
	if (delta > 0){
		return (M_PI_2 - alpha_prime)*(180/M_PI);
	}else{
		return -(M_PI_2 - alpha_prime)*(180/M_PI);
	}

}


int findZero(int *buffer, int size, int *zeroCrossing, int threshold)
{
    // find the beginning of the sound
    int i = 0;
    while(buffer[i] < threshold && i < size){
        i++;
    }
    // find first zero crossing
    while(buffer[i]>0 && i < size){
        i++;
    }

    if(i==size){
        return 0;
    }


    *zeroCrossing = i;
    

    return 1;
}

int calcDelays(int zeroTop1, int zeroLeft, int zeroTop2, int zeroRight, float *delTopLeft, float *delTopRight, float *delLeftRight, int samplerate){
    // given the spacing between the mics, max delay can be speed_sound x distance
    float maxDelay = MICDIST/VSOUND;
    float stepSize = 1.0/samplerate;

    // option 2: we dont care that the zero crossings come from the same frame --maxDelay is enough
    *delTopLeft = (zeroTop1 - zeroLeft) * stepSize;    
    if (*delTopLeft > maxDelay || *delTopLeft < -maxDelay) {
    	return 0;
    }
	
	*delTopRight = (zeroTop2 - zeroRight) * stepSize;
    if (*delTopRight > maxDelay || *delTopRight < -maxDelay) {
    	return 0;
    }

    // left right delay is just the difference between delTopLeft and delTopRight
    *delLeftRight = *delTopLeft - *delTopRight;    
    if(*delLeftRight > maxDelay || *delLeftRight < -maxDelay) {
    	return 0;
    }
    
    printf("topleft indices: %d, %d\n", zeroTop1, zeroLeft);
    printf("topright indices: %d, %d\n", zeroTop2, zeroRight);
    
    printf("delayTL: %.6f\n", *delTopLeft);
    printf("delayTR: %.6f\n", *delTopRight);
    printf("delayLR: %.6f\n", *delLeftRight);

    return 1;
}
