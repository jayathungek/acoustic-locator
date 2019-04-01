#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define BUFLEN 30 


short buf[BUFLEN];
float bufnorm[BUFLEN];
snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *hw_params;
unsigned int rate = 48000;

void printbuf(short buf[], int len){
	for (int i=0; i<len; i++){
		printf("%d, ", buf[i]);
	}
	printf("\n");
}

void printfloatbuf(float buf[], int len, int channels){
	for (int i=0; i<len; i++){
		if(channels == 1){
			if(i%2 == 0) printf("%.2f, ", buf[i]);
		}else if (channels == 2){
			printf("%.2f, ", buf[i]);
		}else{
			exit(-1);
		}
		
	}
	printf("\n");
}

short bufavg(short buf[], int len){
	short total = 0;
	for (int i = 0; i<len; i++){
		total += buf[i];
	}
	return total/BUFLEN;
}

short getmaxabs(short buf[], int len){
	short max = 0;
	for(int i = 0; i < len; i++){
		if(abs(buf[i]) > max){
			max = abs(buf[i]);
		}
	}
	return max;
}

float* getnorm(short buf[], int len){
	short max = getmaxabs(buf, len);
	for(int i = 0; i < len; i++){
		if(max){
			bufnorm[i] = ((float)buf[i])/max;
		}
	}
	return bufnorm;
}

void setparams(char device[], unsigned int chan){
	int i;
	int err;
	
	printf("channels: %d\n", chan);
	

	if ((err = snd_pcm_open (&capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 device,
			 snd_strerror (err));
		exit (1);
	}
	   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
			 
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S32_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, chan)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
}

int main (int argc, char *argv[])
{
	int readerr;
	int channels = strtol(argv[2], NULL, 10);
	setparams(argv[1], channels);

	while (1) {
		if ((readerr = snd_pcm_readi (capture_handle, buf, BUFLEN)) != BUFLEN) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
				 snd_strerror (readerr));
			exit (1);
		}
/*		printf("buffer average: %d\n", bufavg(buf, BUFLEN));*/
		printfloatbuf(getnorm(buf, BUFLEN), BUFLEN, channels);
	}

	snd_pcm_close (capture_handle);
	exit (0);
}
 
/*  long loops;*/
/*  int rc;*/
/*  int size;*/
/*  snd_pcm_t *handle;*/
/*  snd_pcm_hw_params_t *params;*/
/*  unsigned int val;*/
/*  int dir;*/
/*  snd_pcm_uframes_t frames;*/
/*  char *buffer;*/

/*   Open PCM device for recording (capture). */
/*  rc = snd_pcm_open(&handle, "plughw:1",*/
/*                    SND_PCM_STREAM_CAPTURE, 0);*/
/*  if (rc < 0) {*/
/*    fprintf(stderr,*/
/*            "unable to open pcm device: %s\n",*/
/*            snd_strerror(rc));*/
/*    exit(1);*/
/*  }*/

/*   Allocate a hardware parameters object. */
/*  snd_pcm_hw_params_alloca(&params);*/

/*   Fill it in with default values. */
/*  snd_pcm_hw_params_any(handle, params);*/

/*   Set the desired hardware parameters. */
/* */
/*   Interleaved mode */

/*  if(rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED) <  0){*/
/*    fprintf(stderr,*/
/*            "err set_access_first: %s\n",*/
/*            snd_strerror(rc));*/
/*  }*/

/*   Signed 32-bit little-endian format */
/*  if(rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE) < 0){*/
/*    fprintf(stderr,*/
/*            "err set_format: %s\n",*/
/*            snd_strerror(rc));*/
/*  }*/
/*  */

/*   Two channels (stereo) */
/*  if(rc = snd_pcm_hw_params_set_channels(handle, params, 2) < 0){*/
/*    fprintf(stderr,*/
/*            "err set_channels_min: %s\n",*/
/*            snd_strerror(rc));*/
/*  }*/

/*   48000 bits/second sampling rate */
/*  val = 48000;*/
/*  if(rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir) < 0){*/
/*    fprintf(stderr,*/
/*            "err set_rate_near: %s\n",*/
/*            snd_strerror(rc));*/
/*  }*/

/*  */


/*   Write the parameters to the driver */
/*  rc = snd_pcm_hw_params(handle, params);*/
/*  if (rc < 0) {*/
/*    fprintf(stderr,*/
/*            "unable to set hw parameters: %s\n",*/
/*            snd_strerror(rc));*/
/*    exit(1);*/
/*  }*/
/*  frames = 32;*/
/*   Use a buffer large enough to hold one period */
/*  snd_pcm_hw_params_get_period_size(params,*/
/*                                      &frames, &dir);*/
/*  size = frames * 4;  2 bytes/sample, 2 channels */
/*  buffer = (char *) malloc(size);*/

/*   We want to loop for 5 seconds */
/*  snd_pcm_hw_params_get_period_time(params,*/
/*                                         &val, &dir);*/
/*  loops = 5000000 / val;*/

/*  while (loops > 0) {*/
/*    loops--;*/
/*    rc = snd_pcm_readi(handle, buffer, frames);*/
/*    if (rc == -EPIPE) {*/
/*       EPIPE means overrun */
/*      fprintf(stderr, "overrun occurred\n");*/
/*      snd_pcm_prepare(handle);*/
/*    } else if (rc < 0) {*/
/*      fprintf(stderr,*/
/*              "error from read: %s\n",*/
/*              snd_strerror(rc));*/
/*    } else if (rc != (int)frames) {*/
/*      fprintf(stderr, "short read, read %d frames\n", rc);*/
/*    }*/
/*    rc = write(1, buffer, size);*/
/*    if (rc != size)*/
/*      fprintf(stderr,*/
/*              "short write: wrote %d bytes\n", rc);*/
/*  }*/

/*  snd_pcm_drain(handle);*/
/*  snd_pcm_close(handle);*/
/*  free(buffer); */
