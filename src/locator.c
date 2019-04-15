/* file: locator.c */
//hardware stuff
#define FRAMES     128 /**< Number of frames to read in. Frames contain both left- and right-channel samples and are read into the PCM buffer */
#define SAMPLERATE 44100 /**< Audio sampling frequency (Hz) */
#define TOP1       0 /**< Enumeration for top microphone buffer. (Multiplexer select = 0)*/
#define TOP2       1 /**< Enumeration for top microphone buffer. (Multiplexer select = 1)*/
#define LEFT       2 /**< Enumeration for left microphone buffer. */
#define RIGHT      3 /**< Enumeration for right microphone buffer. */

#define MUXSE      21 /**< wiringPi pin connected to multiplexer. */
#define LASER      22 /**< wiringPi pin connected to LED. */
#define ELEVATION  23 /**< wiringPi pin connected to top servo. */
#define AZIMUTH    26 /**< wiringPi pin connected to bottom servo. */
#define BASE_CLK   19200000 /**< Raspberry Pi base clock frequency (Hz). */
#define PWM_FREQ   50  /**< Desired PWM frequency (Hz). */
#define PWMRNG     2000 /**< Range of PWM period. */
#define DELAY      500  /**< Motor delay (milliseconds). */
#define AZ_MAX     90  /**<Maximum azimuthal angle (degrees). Used in limiting range of motion to prevent damage.*/
#define AZ_MIN    -90  /**<Minimum azimuthal angle (degrees). Used in limiting range of motion to prevent damage.*/
#define EL_MAX     70 /**<Maximum elevation angle (degrees). Used in limiting range of motion to prevent damage.*/
#define EL_MIN    -35 /**<Minimum elevation angle (degrees). Used in limiting range of motion to prevent damage.*/
#define DELAY      500  /**<Servo delay (milliseconds). Used in preventing jitter.*/

//physics stuff
#define MICDIST    10.385 /**<Distance between pairs of mics (centimetres).*/
#define VSOUND     34600 /**<Speed of sound in air (centimetres/second).*/
#define MICRO      0.000001 /**<Order of magnitude conversion.*/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <wiringPi.h>
#include <alsa/asoundlib.h>

FILE *out;
int debug = 0;
snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames = FRAMES;
unsigned int sample_rate;
int sel; // mux
double mic_x = (double)MICDIST/2; // x coord of mic relative to other mic

char *buffer;   // main ALSA PCM buffer

// buffers from microphones
signed int *top1; /**<Buffer for top mic. This buffer, along with left is read into when the multiplexer select is 0. */
signed int *top2; /**<Buffer for top mic. This buffer, along with right is read into when the multiplexer select is 1. */
signed int *left; /**<Buffer for left mic. This buffer is read into when the multiplexer select is 0. */
signed int *right; /**<Buffer for right mic. This buffer is read into when the multiplexer select is 1. */

//buffer sizes
int size;
int size_mic;

// keep track of positions of both servos
int az_curr;
int el_curr;

// for sound level
signed int threshold = 600;

// zero crossings
int zeroTop1; /**<Index of zero crossing in top1 mic buffer. */
int zeroLeft; /**<Index of zero crossing in left mic buffer. */
int zeroTop2; /**<Index of zero crossing in top2 mic buffer. */
int zeroRight;/**<Index of zero crossing in right mic buffer. */

// delays
float delTopLeft; /**<Time delay of zero crossing between top and left mics (seconds).*/
float delTopRight; /**<Time delay of zero crossing between top and right mics (seconds).*/ 
float delLeftRight; /**<Time delay of zero crossing between left and right mics (seconds).*/

// functions
//DEBUG
/**
   * A function that takes an integer buffer and renders its contents to sdout for debugging.
   * @param buf A pointer to a signed integer.
   * @param size  The number of elements in the array.
   */
void printbuf(signed int *buf, int size);

/**
   * A function that takes an integer buffer and renders its contents to the specified file descriptor for debugging.
   * @param file  A pointer to a file descriptor.
   * @param buf A pointer to a signed integer.
   * @param size  The number of elements in the array.
   */
void printBufToFile(FILE *file, signed int *buf, int size);

//SERVOS AND LASER

/**
   * Sets up PWM control for the servo motors.
   */
void pwmSetup();

/**
   * Turns on the front LED.
   */
void laserOn();

/**
   * Turns off the front LED.
   */
void laserOff();

/**
   * Stops movement of both servos.
   */
void stopMotors();

/**
   * Brings both servos to their 0 position, i.e. facing straight ahead
   */
void zeroMotors();

/**
   * Turns a given motor by the specified angle in degrees. Prevents movements of the servos at their angular limits so as to avoid damage
   * @param angle The angle by which the motor should turn. 
   * @param motor An integer that distiguishes between the azimuthal or elevation servo.
   * @see AZIMUTH
   * @see ELEVATION
   */
void turnMotorBy(int angle, int motor); // 

/**
   * Moves servos in response to timing difference data.
   * @param delTL Time difference between sound reaching top and left mics (seconds) 
   * @param delTR Time difference between sound reaching top and right mics (seconds)
   * @param delLR Time difference between sound reaching left and right mics (seconds)
   */
void updatePosition(float delTL, float delTR, float delLR); // sets servos and LED

void zeroAzimuth();
void zeroElevation();
int  getPwmClk(int pwmRange);
int  getPwmValue(int angle);
void turnMotorTo(int angle, int motor);
void stopMotor(int motor);

//MICS
//int tobuf is the character code for the different buffers

/**
   * Splits data in the PCM buffer so that the correct channel is sent to the destination buffer.
   * @param tobuf Character code for destination buffer
   * @param frombuf Pointer to PCM buffer
   * @param sizefrom Size of PCM buffer
   * @see top1
   * @see top2
   * @see left
   * @see right
   */
void fillbuf(int tobuf, char *frombuf, int sizefrom); 

/**
   * Initialises the ALSA PCM interface.
   * @param device Name of the I2S device
   * @param rate Audio sampling rate (Hz)
   * @return Error code (0 on success)
   */
int setupdevice(char *device, unsigned int rate);

/**
   * Allocates memory for PCM and mic data buffers.
   * @return Error code (0 on success)
   */ 
int setupmicbuffers();

/**
   * Frees memory allocated to PCM and mic data buffers.
   */ 
void freebuffers();

/**
   * Updates top1, left, top2 and right buffers.
   */ 
void readMics(); 

//DATA MANIPULATION

/**
   * Converts a 16bit big endian number to an integer.
   * @param msb Most significant byte of the 16bit number.
   * @param lsb Least significant byte of the 16bit number.
   * @return Signed integer. 
   */ 
int convertValue(char msb, char lsb); 

/**
   * Finds index of a data buffer where the sound pressure level crosses the x-axis, if such a crossing exists.
   * @param buffer Audio data buffer.
   * @param zeroCrossing Pointer to an integer where the zero crossing index will be stored if found. Do not use if findZero() returns error.
   * @return Error code (0 on success). 
   * @see zeroTop1
   * @see zeroLeft
   * @see zeroTop2
   * @see zeroRight

   */ 
int findZero(signed int *buffer, int *zeroCrossing); 

/**
   * Updates delay variables, which keep track of the timing difference between mics
   * @return Error code (0 on success). 
   * @see delTopLeft
   * @see delTopRight
   * @see delLeftRight
   */ 
int calcDelays();

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



int main (int argc, char *argv[])
{
	if(argc == 3){
		debug = 1;
		char *filename = argv[2];
		out = fopen(filename, "w+");
	}

	char *dev = argv[1];
	if (wiringPiSetup() == -1) exit(1);

	pinMode(MUXSE, OUTPUT);
	pinMode(LASER, OUTPUT);
	pinMode(AZIMUTH, PWM_OUTPUT);
    pinMode(ELEVATION, PWM_OUTPUT);
	pwmSetup();
	setupdevice(dev, SAMPLERATE);
	setupmicbuffers(); //sets size_mic

	laserOn();
	zeroMotors();
	delay(DELAY);
/*	updatePosition(0, 200*MICRO, -297*MICRO);*/
/*	delay(DELAY);*/
	stopMotors();

	int loops = 0;
    for (int i = 0; i < loops; i++) //change to while(1) for real use
    {
        // 1) read data from microphone
        readMics();

        if(debug){
	        printf("top1:\n");
		    printbuf(top1, size_mic);
		    printf("left:\n");
		    printbuf(left, size_mic);
		    printf("top2:\n");
		    printbuf(top2, size_mic);
		    printf("right:\n");
		    printbuf(right, size_mic);
		    printBufToFile(out, top1, size_mic);
		    printBufToFile(out, left, size_mic);
		    printBufToFile(out, top2, size_mic);
		    printBufToFile(out, right, size_mic);
        }
        // 2) if we can find 0 crossings, calculate angles, else goto 1)
        if (!findZero(top1, &zeroTop1)){
            continue;
        }else if(!findZero(left, &zeroLeft)){
            continue;
        }else if(!findZero(top2, &zeroTop2)){
            continue;
        }else if(!findZero(right, &zeroRight)){
            continue;
        }

        // 3) calculate delays
        if (!calcDelays()){
            continue;
        }

        // 4) update position and turn on LED
        //updatePosition(delTopLeft, delTopRight, delLeftRight);
        updatePosition(0, delTopRight, delLeftRight);

        // delay for a bit to prevent jittering
        delay(DELAY);
    }
    freebuffers();
    if(debug) fclose(out);
}


int convertValue(char msb, char lsb){
	int usingned =  (msb | lsb << 8);
	return -(usingned & 0x8000) | (usingned & 0x7fff);
}

void printbuf(signed int *buf, int size){
	for (int i = 0; i<size; i++){
		printf("[%d], ", buf[i]);
	}
	printf("\n\n");
}

void printBufToFile(FILE *fd, signed int *buf, int size){
	fprintf(fd, "[");
	for (int i = 0; i<size; i++){
		fprintf(fd, "%d, ", buf[i]);
	}
	fprintf(fd, "]\n");
}

int setupdevice(char *device, unsigned int rate){
	int err;
    sample_rate = rate;


    printf("req period size: %d\n", frames);
	printf("req sample rate: %d\n", sample_rate);
    /* Open PCM device for recording (capture). */
    err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err)
    {
        fprintf(stderr, "Unable to open PCM device: %s\n", snd_strerror(err));
        return err;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* ### Set the desired hardware parameters. ### */

    /* Interleaved mode */
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err<0)
    {
        fprintf(stderr, "Error setting interleaved mode: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Signed 16-bit big-endian format */
    err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_BE);
    if (err<0)
    {
        fprintf(stderr, "Error setting format: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Two channels (stereo) */
    err = snd_pcm_hw_params_set_channels(handle, params, 2);
    if (err<0)
    {
        fprintf(stderr, "Error setting channels: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* 44100 bits/second sampling rate (CD quality) */
    err = snd_pcm_hw_params_set_rate_near(handle, params, &sample_rate, NULL);
    if (err<0)
    {
        fprintf(stderr, "Error setting sampling rate (%d): %s\n", sample_rate, snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Set period size*/
    err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, NULL);
    if (err<0)
    {
        fprintf(stderr, "Error setting period size: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    unsigned int periodtime;
    snd_pcm_hw_params_get_period_time(params, &periodtime, NULL);
    printf("period time: %dus\n", periodtime);

    printf("actual period size: %d\n", frames);
	printf("actual sample rate: %d\n", sample_rate);

    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(handle, params);
    if (err<0)
    {
        fprintf(stderr, "Unable to set HW parameters: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

}

int setupmicbuffers(){
	int err;

    size = frames * 2 * 2; /* 2 bytes/sample, 2 channels */
    size_mic = frames; // one int per frame received
    int size_mic_bytes = size_mic *  sizeof(signed int);

    printf("main_buffer_size: %d bytes (%d samples)\n", size, size/2);
    printf("mic_buffers_size: %d bytes (%d ints)\n", size_mic_bytes, size_mic);
    buffer = (char *) malloc(size);
    if (!buffer)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }

    top1 = (signed int *) malloc(size_mic_bytes);
    if (!top1)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }

    top2 = (signed int *) malloc(size_mic_bytes);
    if (!top2)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }

    left = (signed int *) malloc(size_mic_bytes);
    if (!left)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }

    right = (signed int *) malloc(size_mic_bytes);
    if (!right)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }


    return 0;
}

void freebuffers(){
	free(buffer);
    free(top1);
    free(top2);
	free(left);
	free(right);
}

//(char code for sub buffer to fill, main buffer, size of main buffer)
void fillbuf(int tobuf, char *frombuf, int sizefrom){
	switch(tobuf){
		char lsb;
		char msb;
		int  value;
		case TOP1:
			for(int i = 0; i < sizefrom; i+=4){
				lsb   = frombuf[i];
				msb   = frombuf[i+1];
				value = convertValue(msb, lsb);
				top1[i/4] = value;
			}
			break;

		case TOP2:
			for(int i = 0; i < sizefrom; i+=4){
				lsb   = frombuf[i];
				msb   = frombuf[i+1];
				value = convertValue(msb, lsb);
				top2[i/4] = value;
			}
			break;

		case LEFT:
			for(int i = 2; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				value = convertValue(msb, lsb);
				left[((i+2)/4) - 1] = value;
			}
			break;

		case RIGHT:
			for(int i = 2; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				value = convertValue(msb, lsb);
				right[((i+2)/4) - 1] = value;
			}
			break;

		default:
			printf("Unknown buffer: %c\n", tobuf);
	}
}


//fills micT0, micT1, micB0 and micB1 buffers
void readMics()
{
	int err;
    sel = 1;
	digitalWrite(MUXSE, sel);
	err = snd_pcm_readi(handle, buffer, frames);
	if (debug) printf("mux: %d, read %d frames to buffer\n", sel, err);
    fillbuf(TOP1, buffer, size);
    fillbuf(LEFT, buffer, size);

	sel = 0;
	digitalWrite(MUXSE, sel);
	err = snd_pcm_readi(handle, buffer, frames);
	if (debug) printf("mux: %d, read %d frames to buffer\n", sel, err);
	fillbuf(TOP2, buffer, size);
	fillbuf(RIGHT, buffer, size);

    if (err == -EPIPE) fprintf(stderr, "Overrun occurred: %d\n", err);
    if (err < 0) err = snd_pcm_recover(handle, err, 0);
    // Still an error, need to exit.
    if (err < 0)
    {
        fprintf(stderr, "Error occured while recording: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        freebuffers();
    }
}


int findZero(int *buffer, int *zeroCrossing)
{
    // find the beginning of the sound
    int i = 0;
    while(buffer[i] < threshold && i < FRAMES){
        i++;
    }
    // find first zero crossing
    while(buffer[i]>0 && i < FRAMES){
        i++;
    }

    if(i==FRAMES){
        return 0;
    }


    *zeroCrossing = i;
    

    return 1;
}

int calcDelays(){
    // given the spacing between the mics, max delay can be speed_sound x distance
    float maxDelay = MICDIST/VSOUND;
    float stepSize = 1.0/SAMPLERATE;

    // option 2: we dont care that the zero crossings come from the same frame --maxDelay is enough
    delTopLeft = (zeroTop1 - zeroLeft) * stepSize;    
    if (delTopLeft > maxDelay || delTopLeft < -maxDelay) {
    	return 0;
    }
	
	delTopRight = (zeroTop2 - zeroRight) * stepSize;
    if (delTopRight > maxDelay || delTopRight < -maxDelay) {
    	return 0;
    }

    // left right delay is just the difference between delTopLeft and delTopRight
    delLeftRight = delTopLeft - delTopRight;    
    if(delLeftRight > maxDelay || delLeftRight < -maxDelay) {
    	return 0;
    }
    
    printf("topleft indices: %d, %d\n", zeroTop1, zeroLeft);
    printf("topright indices: %d, %d\n", zeroTop2, zeroRight);
    
    printf("delayTL: %.6f\n", delTopLeft);
    printf("delayTR: %.6f\n", delTopRight);
    printf("delayLR: %.6f\n", delLeftRight);

    return 1;
}

/*double getMinX(double l, double ){*/
/*	double exp1 = pow(l, 2)*(pow(l, 2) - 4*pow(mic_x));*/
/*	double exp2 = 4*(4*pow(mic_x, 2) - pow(l, 2));*/
/*	return sqrt(-exp1/exp2);*/
/*}*/

double hyperbola(double l, double x){
	double exp1 = (pow(l, 2)/4) - pow(mic_x, 2);
	double exp2 = ((4*pow(mic_x, 2))/pow(l, 2)) - 1;
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

// args -- (delay between top mic and left, delay between top mic and right, delay between left mic and right) in us
void updatePosition(float delTL, float delTR, float delLR){
    int azimuth_angle   = round(getDevFromNormal(delLR));
    int elevation_angle = round(getDevFromNormal(delTR));
    turnMotorBy(azimuth_angle, AZIMUTH);
    turnMotorBy(elevation_angle, ELEVATION);
    laserOn();
    printf("azimuth deviation: %d degrees\n", azimuth_angle);
    printf("elevation deviation: %d degrees\n", elevation_angle);
}

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

void zeroMotors(){
	zeroAzimuth();
	zeroElevation();
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

void laserOn(){
	digitalWrite(LASER, 1);
}

void laserOff(){
	digitalWrite(LASER, 0);
}
