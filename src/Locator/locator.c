#include <stdlib.h>
#include <math.h>

#define FRAMELENGTH 64
#define FRAMES 1000
#define SAMPLES FRAMES*FRAMELENGTH
#define SAMPLERATE 44100
#define SOUNDSPEED 300
#define MICSEPARATION 0.010385
// for sound level
const int threshold = 10000;

// buffers from microphone
int top1[SAMPLES], left[SAMPLES], top2[SAMPLES], right[SAMPLES];

// zero crossings
int zeroTop1, zeroLeft, zeroTop2, zeroRight;

// delays
float delTopLeft, delTopRight, delLeftRight;


// functions
void readMic(int frameSize);

int *normalize(int *buffer); // returns normalized buffer

int findZero(int *buffer, int *zeroCrossing); // returns 0 on success

int calcDelays(); // returns angle in degrees

void updatePosition(float del1, float del2, float del3); // sets servos and LED


int main ()
{
    while (1)
    {
        // 1) read data from microphone
        readMic(FRAMELENGTH);

        // 2) normalize data around 0
        // int *top1Norm = normalize(top1);
        // int *leftNorm = normalize(left);
        // int *top2Norm = normalize(top2);
        // int *rightNorm = normalize(right);

        // 3) if we can find 0 crossings, calculate angles, else goto 1)
        if (!findZero(top1, &zeroTop1)){
            continue;
        }else if(!findZero(left, &zeroLeft)){
            continue;
        }else if(!findZero(top2, &zeroTop2)){
            continue;
        }else if(!findZero(right, &zeroRight)){
            continue;
        }

        // 4) calculate delays
        if (!calcDelays()){
            continue;
        }

        // 5) update position and turn on LED
        updatePosition(delTopLeft, delTopRight, delLeftRight);

        // delay for a bit to prevent jittering
        // delay(1000);

    }
}




void readMic(int frameSize)
{
    // todo Kavi
}

int *normalize(int *buffer)
{

    static int *normalized;

    // allocate mempory
    normalized = (int *) malloc(SAMPLES);   //todo -- free when finished

    // ok we hvae the samples, get the mean (avg)
    float meanval = 0;
    for (int i=0; i<SAMPLES; i++) {
      meanval += buffer[i];
    }
    meanval /= SAMPLES;

    // subtract it from all sapmles to get a 'normalized' output
    for (int i=0; i<SAMPLES; i++) {
      buffer[i] -= meanval;
    }


    return normalized;
}

int findZero(int *buffer, int *zeroCrossing)
{
    // find the beginning of the sound
    int i = 0;
    while(buffer[i] < threshold && i < SAMPLES - 1){
        i++;
    }

    // find first zero crossing
    while(buffer[i]>0 && i < SAMPLES){
        i++;
    }

    if(i==SAMPLES){
        return 0;
    }

    *zeroCrossing = i;
    return 1;
}

int calcDelays()
{
    // given the spacing between the mics, max delay can be speed_sound x distance
    float maxDelay = SOUNDSPEED * MICSEPARATION;
    float stepSize = 1/SAMPLERATE;

    // comment 1 option out
    //  option 1: for accuracy, we want both zero crossings to come from the same frame
    // if ((zeroTop1 - zeroLeft) * stepSize > maxDelay || (zeroTop1 - zeroLeft) * stepSize < -maxDelay || zeroTop1 / FRAMELENGTH != zeroLeft / FRAMELENGTH){
    //     return 0;
    // }
    //
    // delTopLeft = zeroTop1 - zeroLeft;
    //
    // if ((zeroTop2 - zeroRight) * stepSize > maxDelay || (zeroTop2 - zeroRight) * stepSize < -maxDelay || zeroTop2 / FRAMELENGTH != zeroRight / FRAMELENGTH){
    //     return 0;
    // }
    //
    // delTopRight = zeroTop2 - zeroRight;
    //
    // if ((zeroLeft - zeroRight) * stepSize > maxDelay || (zeroLeft - zeroRight) * stepSize < -maxDelay){
    //     return 0;
    // }
    //
    // delLeftRight = zeroLeft - zeroRight;


    // option 2: we dont care that the zero crossings come from the same frame --maxDelay is enough
    if ((zeroTop1 - zeroLeft) * stepSize > maxDelay || (zeroTop1 - zeroLeft) * stepSize < -maxDelay){
        return 0;
    }

    delTopLeft = zeroTop1 - zeroLeft;

    if ((zeroTop2 - zeroRight) * stepSize > maxDelay || (zeroTop2 - zeroRight) * stepSize < -maxDelay){
        return 0;
    }

    delTopRight = zeroTop2 - zeroRight;

    if ((zeroLeft - zeroRight) * stepSize > maxDelay || (zeroLeft - zeroRight) * stepSize < -maxDelay){
        return 0;
    }

    delLeftRight = zeroLeft - zeroRight;

    return 1;
}

// args -- (delay between top mic and left, delay between top mic and right, delay between left mic and right) in us
void updatePosition(float delay1, float delay2, float delay3)
{
    // todo Try Kavi
}
