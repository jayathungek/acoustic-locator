#define MUXSE  21
#define FRAMES 64

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

int size;
int size_mic;
unsigned int sample_rate;
snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames = FRAMES; 
char *buffer0;
char *buffer1; 
char *micMbuf;
char *mic0buf;
char *mic1buf;

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
    /* Signed 16-bit little-endian format */
    err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
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

void printbuffer(char *buf, int size, int channels){
	//loop through L/R samples (pairs of shorts) in char buffer
	if(channels == 2){
		for (int i = 0; i<size; i+=4){
			char lsb = buf[i];
			char msb = buf[i+1];
			int lchannel = msb | lsb << 8;
			
			lsb = buf[i+2];
			msb = buf[i+3];
			int rchannel = msb | lsb << 8;
		
			printf("[%04x | %04x], ", lchannel, rchannel );
		}
	}else if(channels == 1){
		for (int i = 0; i<size; i+=2){
			char lsb = buf[i];
			char msb = buf[i+1];
			int channel = msb | lsb << 8;		
			printf("[%04x], ", channel);
		}
	}else{
		printf("%d channels not supported", channels);
	}
	
	printf("\n\n");
}

int setupbuffers(){
	int err;
	/* Use a buffer large enough to hold one period */
	
    size = frames * 2 * 2; /* 2 bytes/sample, 2 channels */
    size_mic = size / 2;
    printf("main_buffer_size: %d\n", size);
    printf("mic_buffers_size: %d\n", size_mic);
    buffer0 = (char *) malloc(size);
    if (!buffer0)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    } 
    
    buffer1 = (char *) malloc(size);
    if (!buffer1)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    } 
    
    micMbuf = (char *) malloc(size_mic);
    if (!micMbuf)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }
    
    mic0buf = (char *) malloc(size_mic);
    if (!mic0buf)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }
    
    mic1buf = (char *) malloc(size_mic);
    if (!mic1buf)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }
    
    
    return 0;
}

void freebuffers(){
	free(buffer0);
	free(buffer1); 
    free(micMbuf);
	free(mic0buf);
	free(mic1buf); 
}

//(char code for sub buffer to fill, main buffer, size of main buffer)
void fillbuf(char tobuf, char *frombuf, int sizefrom){
	switch(tobuf){
		char lsb;
		char msb;
		case 'm':
			for(int i = 0; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				micMbuf[i/2] = lsb;
				micMbuf[(i/2) + 1] = msb;
			}
			break;
			
		case '0':
			for(int i = 2; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				mic0buf[(i/2) - 1] = lsb;
				mic0buf[(i/2)] = msb;
			}
			break;
			
		case '1':
			for(int i = 2; i < sizefrom; i+=4){
				lsb = frombuf[i];
				msb = frombuf[i+1];
				mic1buf[(i/2) - 1] = lsb;
				mic1buf[(i/2)] = msb;
			}
			break;
			
		default:
			printf("Unknown buffer: %c\n", tobuf);
	}
}

int main(int argc, char *argv[]){
	if (wiringPiSetup() == -1) exit(1);
	pinMode(MUXSE, OUTPUT);
	
	char * dev = argv[1];
	setupdevice(dev, 44000);
	setupbuffers();
	
	int totalFrames = 0;
	int loops = 4;
	int err;
	int sel;
/*	int i = 0; i<loops; i++*/
/*	;;*/
	
	for(int i = 0; i<loops; i++){
		sel = 1;
		digitalWrite(MUXSE, sel);
		err = snd_pcm_readi(handle, buffer0, frames);
		totalFrames += err;
		printf("mux: %d, read %d frames from buffer\n", sel, totalFrames);
		totalFrames = 0;
/*		printbuffer(buffer0, size, 2);*/
        fillbuf('m', buffer0, size);
        fillbuf('1', buffer0, size); 
        printf("MAIN MIC:\n");
        printbuffer(micMbuf, size_mic, 1);
        printf("MIC 1:\n");
        printbuffer(mic1buf, size_mic, 1);
        
		sel = 0;
		digitalWrite(MUXSE, sel);
		err = snd_pcm_readi(handle, buffer0, frames);
		totalFrames += err;
		printf("mux: %d, read %d frames from buffer\n", sel, totalFrames);
		totalFrames = 0;
/*		printbuffer(buffer0, size, 2); */
		fillbuf('0', buffer0, size); 
		printf("MIC 0:\n");
        printbuffer(mic0buf, size_mic, 1);
        
/*        if (err == -EPIPE) fprintf(stderr, "Overrun occurred: %d\n", err);*/
/*        if (err < 0) err = snd_pcm_recover(handle, err, 0);*/
/*        // Still an error, need to exit.*/
/*        if (err < 0)*/
/*        {*/
/*            fprintf(stderr, "Error occured while recording: %s\n", snd_strerror(err));*/
/*            snd_pcm_close(handle);*/
/*            freebuffers();*/
/*            return err;*/
/*        }*/
        
	}
	snd_pcm_close(handle);
	freebuffers();
	
}
