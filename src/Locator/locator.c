//hardware stuff
#define MUXSE      21
#define FRAMES     64
#define SAMPLERATE 44000
#define TOP1       0
#define TOP2       1
#define LEFT       2
#define RIGHT      3

#define LASER      22
#define ELEVATION  23   //top servo
#define AZIMUTH    26   //bottom servo
#define BASE_CLK   19200000
#define PWM_FREQ   50   //Hz
#define PWMRNG     2000  // range of pwm values that can be written
#define DELAY      500   // motor delay, ms 
#define AZ_MAX     90    // Limit range of motion to prevent damage - values
#define AZ_MIN    -90    // derived from experiment
#define EL_MAX     70
#define EL_MIN    -35
#define DELAY      500   // servo delay, ms

//physics stuff
#define MICDIST    10.0 // cm
#define VSOUND     34600 // cm/s
#define MICRO      0.000001

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <wiringPi.h>
#include <alsa/asoundlib.h>


snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames = FRAMES; 
unsigned int sample_rate;
int sel; // mux
double mic_x = (double)MICDIST/2; // x coord of mic relative to other mic

char *buffer;   // main ALSA PCM buffer

// buffers from microphones
signed int *top1;
signed int *top2;
signed int *left;
signed int *right;

//buffer sizes
int size;
int size_mic;

// keep track of positions of both servos
int az_curr;
int el_curr;


// functions
//DEBUG
void printbuf(int *buf, int size); 

//SERVOS AND LED
int  getPwmClk(int pwmRange);
int  getPwmValue(int angle);
void turnMotorTo(int angle, int motor);
void turnMotorBy(int angle, int motor); // prevents movement if at limits
void stopMotor(int motor);
void stopMotors();
void zeroMotors();

void zeroAzimuth();
void zeroElevation();
void pwmSetup();
void updatePosition(float delayTL, float delTR, float delayLR); // sets servos and LED

//MICS
//int tobuf is the character code for the different buffers 
void fillbuf(int tobuf, char *frombuf, int sizefrom); // splits the raw data buffer so that the correct channel is sent to tobuf
int setupdevice(char *device, unsigned int rate); // returns 0 on success
int setupmicbuffers(); // 0 on success
void freebuffers();
void readMics(); // updates micT0, micT1, mic1 and mic0 buffers

//DATA MANIPULATION
int convertValue(char msb, char lsb); // returns signed int value of 16bit BE
int *normalize(int *buffer); // returns normalized buffer
int findZero(int *buffer); // returns 0 on success
float calcAngle(int *top_buf, int *side_buf); // returns angle in degrees
double hyperbola(double l, double x); // hyperbola that represents possible mic locations
//delta in microseconds
float getDevFromNormal(float delta); // uses time diff delta to calculate the angular offset of the noise from the normal line
/*double getMinX(double l); // hyperbola not valid for all x, returns minimum safe x*/




int main (int argc, char *argv[])
{
	
	
	char * dev = argv[1];
	if (wiringPiSetup() == -1) exit(1);
	
	pinMode(MUXSE, OUTPUT);
	pinMode(LASER, OUTPUT);
	pinMode (AZIMUTH, PWM_OUTPUT);
    pinMode (ELEVATION, PWM_OUTPUT);
	pwmSetup();
	setupdevice(dev, SAMPLERATE);
	setupmicbuffers(); //sets size_mic
	
	
	digitalWrite(LASER, 0);
	
	zeroMotors();
	delay(DELAY);
	int loops = 0;
    for (int i = 0; i < loops; i++) //change to while(1) for real use 
    {
        // 1) read data from microphone
        readMics();
        printf("top1:\n");
        printbuf(top1, size_mic);
        printf("left:\n");
        printbuf(left, size_mic);
        printf("top2:\n");
        printbuf(top2, size_mic);
        printf("right:\n");
        printbuf(right, size_mic);
        

        // 2) normalize data around 0
        int *top1Norm = normalize(top1);
        int *leftNorm = normalize(left);
        int *top2Norm = normalize(top2);
        int *rightNorm = normalize(right);

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
	printf("mux: %d, read %d frames to buffer\n", sel, err);
    fillbuf(TOP1, buffer, size);
    fillbuf(LEFT, buffer, size);

	sel = 0;
	digitalWrite(MUXSE, sel);
	err = snd_pcm_readi(handle, buffer, frames);
	printf("mux: %d, read %d frames to buffer\n", sel, err);
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
	double l  = (double)VSOUND * delta * MICRO;
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
void updatePosition(float delayTL, float delayTR, float delayLR){
    int azimuth_angle   = round(getDevFromNormal(delayLR));
    int elevation_angle = round(getDevFromNormal(delayTR));
    turnMotorBy(azimuth_angle, AZIMUTH);
    turnMotorBy(elevation_angle, ELEVATION);
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
