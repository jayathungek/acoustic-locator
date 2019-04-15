/* file: locator.c */

//hardware stuff
#define DEVICE     "plughw:1" /**< Name of the ALSA PCM device. */
#define FRAMES     128 /**< Number of frames to read in. Frames contain both left- and right-channel samples and are read into the PCM buffer */
#define THRESHOLD  600 /**< Minimum detectable sound pressure level. */
#define SAMPLERATE 44100 /**< Audio sampling frequency (Hz) */
#define TOP1       0 /**< Enumeration for top microphone buffer. (Multiplexer select = 0)*/
#define TOP2       1 /**< Enumeration for top microphone buffer. (Multiplexer select = 1)*/
#define LEFT       2 /**< Enumeration for left microphone buffer. */
#define RIGHT      3 /**< Enumeration for right microphone buffer. */

#define MUXSE      21 /**< wiringPi pin connected to multiplexer. */


#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <wiringPi.h>
#include <alsa/asoundlib.h>

#include "servo.h"
#include "utils.h"


FILE *out;
int debug = 0;
snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames = FRAMES;
unsigned int sample_rate;
int sel; // mux


char *buffer;   // main ALSA PCM buffer

// buffers from microphones
signed int *top1; /**<Buffer for top mic. This buffer, along with left is read into when the multiplexer select is 0. */
signed int *top2; /**<Buffer for top mic. This buffer, along with right is read into when the multiplexer select is 1. */
signed int *left; /**<Buffer for left mic. This buffer is read into when the multiplexer select is 0. */
signed int *right; /**<Buffer for right mic. This buffer is read into when the multiplexer select is 1. */

//Buffer sizes
int pcm_bufsize = 0;
int mic_bufsize = 0;

// keep track of positions of both servos
int az_curr = 0;
int el_curr = 0;


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
   * @param buf A pointer to the buffer.
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



//MICS
//int tobuf is the character code for the different buffers

/**
   * Splits data in the PCM buffer so that the correct channel is sent to the destination buffer.
   * @param tobufCode Character code for destination buffer
   * @param tobuf Pointer to destination buffer
   * @param frombuf Pointer to PCM buffer
   * @param sizefrom Size of PCM buffer
   * @see top1
   * @see top2
   * @see left
   * @see right
   */
void fillbuf(int tobufCode, int *tobuf, char *frombuf, int sizefrom); 

/**
   * Initialises the ALSA PCM interface.
   * @param device Name of the I2S device
   * @param rate Audio sampling rate (Hz)
   * @return Error code (0 on success)
   */
int setupdevice(char *device, unsigned int rate);

/**
   * Allocates memory for PCM and mic data buffers.
   * @ param frames The number of frames in the PCM buffer
   * @ param pcm_size A pointer to the integer that contains the pcm buffer size
   * @ param mic_size A pointer to the integer that contains the pcm buffer size
   * @ return Error code (0 on success)
   */ 
int setupmicbuffers(int frames, int*pcm_size, int *mic_size);

/**
   * Frees memory allocated to PCM and mic data buffers.
   */ 
void freebuffers();

/**
   * Updates top1, left, top2 and right buffers.
   */ 
void readMics(); 
/**
   * Moves servos in response to timing difference data.
   * @param delTL Time difference between sound reaching top and left mics (seconds) 
   * @param delTR Time difference between sound reaching top and right mics (seconds)
   * @param delLR Time difference between sound reaching left and right mics (seconds)
   */
void updatePosition(float delTL, float delTR, float delLR); // sets servos and LED

int main (int argc, char *argv[])
{
	if(argc == 2){
		debug = 1;
		char *filename = argv[2];
		out = fopen(filename, "w+");
	} 
	if (wiringPiSetup() == -1) exit(1);
    
    char *dev = (char *)DEVICE;
	pinMode(MUXSE, OUTPUT);
	pinMode(LASER, OUTPUT);
	pinMode(AZIMUTH, PWM_OUTPUT);
    pinMode(ELEVATION, PWM_OUTPUT);
	pwmSetup();
	setupdevice(dev, SAMPLERATE);
	setupmicbuffers(FRAMES, &pcm_bufsize, &mic_bufsize);
	laserOn();
	zeroMotors(&az_curr, &el_curr);
	delay(DELAY); 
	stopMotors();

	int loops = 10000;
    for (int i = 0; i < loops; i++) //change to while(1) for real use
    {
    	stopMotors();
        // 1) read data from microphone
        readMics(top1, left, top2, right);

        if(debug){
	        printf("top1:\n");
		    printbuf(top1, mic_bufsize);
		    printf("left:\n");
		    printbuf(left, mic_bufsize);
		    printf("top2:\n");
		    printbuf(top2, mic_bufsize);
		    printf("right:\n");
		    printbuf(right, mic_bufsize);
		    printBufToFile(out, top1, mic_bufsize);
		    printBufToFile(out, left, mic_bufsize);
		    printBufToFile(out, top2, mic_bufsize);
		    printBufToFile(out, right, mic_bufsize);
        }
        // 2) if we can find 0 crossings, calculate angles, else goto 1)
        if (!findZero(top1, FRAMES, &zeroTop1, THRESHOLD)){
            continue;
        }else if(!findZero(left, FRAMES, &zeroLeft, THRESHOLD)){
            continue;
        }else if(!findZero(top2, FRAMES, &zeroTop2, THRESHOLD)){
            continue;
        }else if(!findZero(right, FRAMES, &zeroRight, THRESHOLD)){
            continue;
        }

        // 3) calculate delays
        if (!calcDelays(zeroTop1, zeroLeft, zeroTop2, zeroRight, &delTopLeft, &delTopRight, &delLeftRight, SAMPLERATE)){
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

int setupmicbuffers(int frames, int *pcm_size, int *mic_size){
	int err;

    *pcm_size = frames * 2 * 2; /* 2 bytes/sample, 2 channels */
    *mic_size = frames; // one int per frame received
    int size_mic_bytes = frames *  sizeof(signed int); 

    printf("main_buffer_size: %d bytes (%d samples)\n", pcm_bufsize, pcm_bufsize/2);
    printf("mic_buffers_size: %d bytes (%d ints)\n", size_mic_bytes, frames);
    buffer = (char *) malloc(*pcm_size);
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
void fillbuf(int tobufCode, int *tobuf, char *frombuf, int sizefrom){
	switch(tobufCode){
		char lsb;
		char msb;
		int  value;
		case TOP1:
			for(int i = 0; i < sizefrom; i+=4){
				lsb   = frombuf[i];
				msb   = frombuf[i+1];
				value = convertValue(msb, lsb);
				tobuf[i/4] = value;
			}
			break;

		case TOP2:
			for(int i = 0; i < sizefrom; i+=4){
				lsb   = frombuf[i];
				msb   = frombuf[i+1];
				value = convertValue(msb, lsb);
				tobuf[i/4] = value;
			}
			break;

		case LEFT:
			for(int i = 2; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				value = convertValue(msb, lsb);
				tobuf[((i+2)/4) - 1] = value;
			}
			break;

		case RIGHT:
			for(int i = 2; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				value = convertValue(msb, lsb);
				tobuf[((i+2)/4) - 1] = value;
			}
			break;

		default:
			printf("Unknown buffer: %c\n", tobuf);
	}
}


//fills top1, left, top2 and right buffers
void readMics(int *top1, int *left, int *top2, int *right)
{
	int err;
    sel = 1;
	digitalWrite(MUXSE, sel);
	err = snd_pcm_readi(handle, buffer, frames);
	if (debug) printf("mux: %d, read %d frames to buffer\n", sel, err);
    fillbuf(TOP1, top1, buffer, pcm_bufsize);
    fillbuf(LEFT, left, buffer, pcm_bufsize);

	sel = 0;
	digitalWrite(MUXSE, sel);
	err = snd_pcm_readi(handle, buffer, frames);
	if (debug) printf("mux: %d, read %d frames to buffer\n", sel, err);
	fillbuf(TOP2, top2, buffer, pcm_bufsize);
	fillbuf(RIGHT, right, buffer, pcm_bufsize);

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


// args -- (delay between top mic and left, delay between top mic and right, delay between left mic and right) in us
void updatePosition(float delTL, float delTR, float delLR){
    int azimuth_angle   = round(getDevFromNormal(delLR));
    int elevation_angle = round(getDevFromNormal(delTR));
    turnMotorBy(azimuth_angle, AZIMUTH, &az_curr);
    turnMotorBy(elevation_angle, ELEVATION, &el_curr);
    laserOn();
    printf("azimuth deviation: %d degrees\n", azimuth_angle);
    printf("elevation deviation: %d degrees\n", elevation_angle);
}


