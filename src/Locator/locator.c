#define MUXSE      21
#define LASER      22
#define FRAMES     64
#define SAMPLERATE 44000
#define CSVDIR     "../../csv"

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>


snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames = FRAMES; 
unsigned int sample_rate;
int sel;

char *buffer;   // main ALSA PCM buffer

// buffers from microphones
signed int *micM0buf;
signed int *micM1buf;
signed int *mic0buf;
signed int *mic1buf;

//buffer sizes
int size;
int size_mic;


// functions

void printbuf(int *buf, int size); // for debugging

void fillbuf(char tobuf, char *frombuf, int sizefrom);

int setupdevice(char *device, unsigned int rate); // returns 0 on success

int setupmicbuffers(); // 0 on success

void freebuffers();

void readMics(); // updates micM0, micM1, mic1 and mic0 buffers

int convertValue(char msb, char lsb); // returns signed int value of 16bit BE

int *normalize(int *buffer); // returns normalized buffer

int findZero(int *buffer); // returns 0 on success

float calcAngle(int *top_buf, int *side_buf); // returns angle in degrees

void updatePosition(float del1, float del2, float del3); // sets servos and LED


int main (int argc, char *argv[])
{
	int loops = 400;
	if (wiringPiSetup() == -1) exit(1);
	pinMode(MUXSE, OUTPUT);
	pinMode(LASER, OUTPUT);
	digitalWrite(LASER, 0);
	
	char * dev = argv[1];
	setupdevice(dev, SAMPLERATE);
	setupmicbuffers(); //sets size_mic
    for (int i = 0; i < loops; i++) //change to while(1) for real use 
    {
        // 1) read data from microphone
        readMics();
        printf("micM0:\n");
        printbuf(micM0buf, size_mic);
        printf("mic0:\n");
        printbuf(mic0buf, size_mic);
        printf("micM1:\n");
        printbuf(micM1buf, size_mic);
        printf("mic1:\n");
        printbuf(mic1buf, size_mic);
        

        // 2) normalize data around 0
        int *top1Norm = normalize(micM0buf);
        int *leftNorm = normalize(mic0buf);
        int *top2Norm = normalize(micM1buf);
        int *rightNorm = normalize(mic1buf);

        // 3) if we can find 0 crossings, calculate angles, else goto 1)
        if (!findZero(top1Norm)){
            continue;
        }else if(!findZero(leftNorm)){
            continue;
        }else if(!findZero(top2Norm)){
            continue;
        }else if(!findZero(rightNorm)){
            continue;
        }

        // 4) calculate angles
        float delTopLeft = calcAngle(top1Norm, leftNorm);
        float delTopRight = calcAngle(top2Norm, rightNorm);
        float delLeftRight = calcAngle(leftNorm, rightNorm);

        // 5) update position and turn on LED
        updatePosition(delTopLeft, delTopRight, delLeftRight);

        // delay for a bit to prevent jittering
        // delay(1000);
        
        

    }
    freebuffers();
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
	/* Use a buffer large enough to hold one period */
	
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
    
    micM0buf = (signed int *) malloc(size_mic_bytes);
    if (!micM0buf)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }
    
    micM1buf = (signed int *) malloc(size_mic_bytes);
    if (!micM1buf)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }
    
    mic0buf = (signed int *) malloc(size_mic_bytes);
    if (!mic0buf)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }
    
    mic1buf = (signed int *) malloc(size_mic_bytes);
    if (!mic1buf)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }
    
    
    return 0;
}

void freebuffers(){
	free(buffer); 
    free(micM0buf);
    free(micM1buf);
	free(mic0buf);
	free(mic1buf);
}

//(char code for sub buffer to fill, main buffer, size of main buffer)
void fillbuf(char tobuf, char *frombuf, int sizefrom){
	switch(tobuf){
		char lsb;
		char msb;
		int  value;
		case 'a':
			for(int i = 0; i < sizefrom; i+=4){
				lsb   = frombuf[i];
				msb   = frombuf[i+1];
				value = convertValue(msb, lsb);
				micM0buf[i/4] = value;
			}
			break;
			
		case 'b':
			for(int i = 0; i < sizefrom; i+=4){
				lsb   = frombuf[i];
				msb   = frombuf[i+1];
				value = convertValue(msb, lsb);
				micM1buf[i/4] = value;
			}
			break;
			
		case '0':
			for(int i = 2; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				value = convertValue(msb, lsb);
				mic0buf[((i+2)/4) - 1] = value;
			}
			break;
			
		case '1':
			for(int i = 2; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				value = convertValue(msb, lsb);
				mic1buf[((i+2)/4) - 1] = value;
			}
			break;
			
		default:
			printf("Unknown buffer: %c\n", tobuf);
	}
}


//fills micM0, micM1, mic0 and mic1 buffers
void readMics()
{
	int err;
    sel = 1;
	digitalWrite(MUXSE, sel);
	err = snd_pcm_readi(handle, buffer, frames);
	printf("mux: %d, read %d frames to buffer\n", sel, err);
    fillbuf('b', buffer, size);
    fillbuf('1', buffer, size); 
/*    printf("MAIN MIC:\n");*/
/*    printbuffer(micM1buf, size_mic, 1);*/
/*    printf("MIC 1:\n");*/
/*	printbuffer(mic1buf, size_mic, 1);*/

	sel = 0;
	digitalWrite(MUXSE, sel);
	err = snd_pcm_readi(handle, buffer, frames);
	printf("mux: %d, read %d frames to buffer\n", sel, err);
	fillbuf('a', buffer, size);
	fillbuf('0', buffer, size);
/*	printf("MAIN MIC:\n");*/
/*    printbuffer(micM0buf, size_mic, 1); */
/*	printf("MIC 0:\n");*/
/*    printbuffer(mic0buf, size_mic, 1);*/
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

int *normalize(int *buffer)
{
    //todo Roni
    static int *normalized;
    return normalized;
}

int findZero(int *buffer)
{
    //todo Roni
    return 0;
}

float calcAngle(int *top_buf, int *side_buf)
{
    // todo Roni
    float angle;
    return angle;
}

// args -- (delay between top mic and left, delay between top mic and right, delay between left mic and right) in us
void updatePosition(float delay1, float delay2, float delay3)
{
    // todo Try Kavi
}
