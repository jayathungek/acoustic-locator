#ifndef UTILS_H
#define UTILS_H

//physics stuff
#define MICDIST    10.385 /**<Distance between pairs of mics (centimetres).*/
#define VSOUND     34600 /**<Speed of sound in air (centimetres/second).*/
#define MICRO      0.000001 /**<Order of magnitude conversion.*/
#define MIC_X      MICDIST/2 // x coord of mic relative to other mic
/**
   * Converts a 16bit big endian number to an integer.
   * @param msb Most significant byte of the 16bit number.
   * @param lsb Least significant byte of the 16bit number.
   * @return Signed integer. 
   */ 
int convertValue(char msb, char lsb); 

/**
   *A mathematical function that represents possible sound source locations for a particular timing difference.
   * @param l Extra distance travelled by sound from closer to further mic
   * @param x Arbitrary x-coordinate greater than mic's x coordinate 
   * @return Possible Y-coordinate of sound source. 
   */ 
double hyperbola(double l, double x);
//delta in microseconds

/**
   * Uses time difference between to calculate the angular offset of the noise from the normal line, i.e. line that bisects the line that joins the mics.
   * @param delta Time difference (microseconds)
   * @return Angular offset from normal (degrees). 
   * @see hyperbola()
   */ 
float getDevFromNormal(float delta); 

/**
   * Finds index of a data buffer where the sound pressure level crosses the x-axis, if such a crossing exists.
   * @param buffer Audio data buffer.
   * @param size Size of the data biffer
   * @param zeroCrossing Pointer to an integer where the zero crossing index will be stored if found. Do not use if findZero() returns error.
   * @return Error code (0 on success). 
   * @param threshold Minimum sound pressure level for detection
   * @see zeroTop1
   * @see zeroLeft
   * @see zeroTop2
   * @see zeroRight

   */ 
int findZero(signed int *buffer, int size, int *zeroCrossing, int threshold); 

/**
   * Updates delay variables, which keep track of the timing difference between mics
   * @return Error code (0 on success). 
   * @param delTopLeft Pointer to a float where the delay between top and left mics will be stored if found. Do not use if CalcDelays() returns error.
   * @param delTopRight Pointer to a float where the delay between top and right mics will be stored if found. Do not use if CalcDelays() returns error.
   * @param delLeftRight Pointer to a float where the delay between left and right mics will be stored if found. Do not use if CalcDelays() returns error.
   */ 
int calcDelays(int zeroTop1, int zeroLeft, int zeroTop2, int zeroRight, float *delTopLeft, float *delTopRight, float *delLeftRight, int samplerate);

#endif 
