using namespace std;
#include <algorithm>
#include <iostream>
// #define SAMPLES 128

int main(){

    // buffer with adudio data
    float samples[] = {1, 2, 3, 4, 5, 6, 7, 8};

    int SAMPLES = sizeof(samples)/sizeof(*samples);

    // ok we hvae the samples, get the mean (avg)
    float meanval = 0;
    for (int i=0; i<SAMPLES; i++) {
      meanval += samples[i];
    }
    meanval /= SAMPLES;

    cout << meanval << "\n";

    // subtract it from all sapmles to get a 'normalized' output
    for (int i=0; i<SAMPLES; i++) {
      samples[i] -= meanval;
      //Serial.println(samples[i]);
    }

    // find the 'peak to peak' max
    float maxsample, minsample;
    minsample = 100000;
    maxsample = -100000;
    for (int i=0; i<SAMPLES; i++) {
      minsample = min(minsample, (float) samples[i]);
      maxsample = max(maxsample, (float) samples[i]);
    }

    cout<<maxsample << " " << minsample;

    return 0;

}
