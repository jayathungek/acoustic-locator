#define MUXSE  21

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
snd_pcm_uframes_t frames = 64; 
char *buffer; 
char *micMbuf;
char *mic0buf;
char *mic1buf;

int setupdevice(char *device, unsigned int rate){
	int err;
    sample_rate = rate;
    
    printf("requested period size: %d\n", frames);
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
   
    
    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(handle, params);
    if (err<0)
    {
        fprintf(stderr, "Unable to set HW parameters: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
	printf("actual period size: %d\n", frames);
	printf("actual sample rate: %d\n", sample_rate);
}

void printbuffer(char *buffer, int size, int channels){
	//loop through L/R samples (pairs of shorts) in char buffer
	if(channels == 2){
		for (int i = 0; i<size; i+=4){
			char lsb = buffer[i];
			char msb = buffer[i+1];
			int lchannel = msb | lsb << 8;
			
			lsb = buffer[i+2];
			msb = buffer[i+3];
			int rchannel = msb | lsb << 8;
		
			printf("[%04x | %04x], ", lchannel, rchannel );
		}
	}else if(channels == 1){
		for (int i = 0; i<size; i+=2){
			char lsb = buffer[i];
			char msb = buffer[i+1];
			int channel = msb | lsb << 8;		
			printf("[%04x], ", channel);
		}
	}else{
		printf("%d channels not supported", channels);
	}
	
	printf("\n");
}

int setupbuffers(){
	int err;
	/* Use a buffer large enough to hold one period */
	
    size = frames * 2 * 2; /* 2 bytes/sample, 2 channels */
    size_mic = size / 2;
    printf("main_buffer_size: %d\n", size);
    printf("mic_buffers_size: %d\n", size_mic);
    buffer = (char *) malloc(size);
    if (!buffer)
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

int main(int argc, char *argv[]){
	char * dev = argv[1];
	setupdevice(dev, 48000);
	setupbuffers();
	
	int totalFrames = 0;
	int loops = 4;
	int err;
/*	int i = 0; i<loops; i++*/
/*	;;*/
	for(int i = 0; i<loops; i++){
		err = snd_pcm_readi(handle, buffer, frames); 
        totalFrames += err;
        if (err == -EPIPE) fprintf(stderr, "Overrun occurred: %d\n", err);
        if (err < 0) err = snd_pcm_recover(handle, err, 0);
        // Still an error, need to exit.
        if (err < 0)
        {
            fprintf(stderr, "Error occured while recording: %s\n", snd_strerror(err));
            snd_pcm_close(handle);
            free(buffer); 
            free(micMbuf);
			free(mic0buf);
			free(mic1buf); 
            return err;
        }
        printbuffer(buffer, size, 2);
	}
	printf("read %d frames total\n", totalFrames);
	snd_pcm_close(handle);
	free(buffer); 
	free(micMbuf);
	free(mic0buf);
	free(mic1buf); 
	
}
