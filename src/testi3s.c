#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

int i,buff_i,j;
short * sampleLP;
short * sampleRP;
int err;
char *buffer;
int buffer_frames = 128;
unsigned int rate = 48000;
snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *hw_params;
snd_pcm_sframes_t avail_cap;

snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

int setup_device(char* device){
	if ((err = snd_pcm_open (&capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
             device,
             snd_strerror (err));
    exit (1);
  }

  fprintf(stderr, "audio interface opened\n");
		   
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stderr, "hw_params allocated\n");
			 
  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stderr, "hw_params initialized\n");
	
  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stderr, "hw_params access set\n");
	
  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stderr, "hw_params format set\n");
	
  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (err));
    exit (1);
  }
	
  fprintf(stderr, "hw_params rate set\n");

  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params,2)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stderr, "hw_params channels set\n");
	
  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stderr, "hw_params set\n");
	
  snd_pcm_hw_params_free (hw_params);

  fprintf(stderr, "hw_params freed\n");
	
  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stderr, "audio interface prepared\n");
  return err;
}

void prepare_buffer(){
	buffer = malloc(128 * snd_pcm_format_width(format) / 8 * 2);
	fprintf(stderr, "buffer allocated\n");
}


int get_period_time(){
	unsigned int v;
	if((err = snd_pcm_hw_params_get_period_time(hw_params, &v, NULL)) < 0){
		fprintf (stderr, "cannot get period time (%s)\n", snd_strerror(err));
		exit(1);
	}
	return v;
}




void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file)
{
    unsigned buf;
    while(num_bytes>0)
    {   buf = word & 0xff;
        fwrite(&buf, 1,1, wav_file);
        num_bytes--;
    word >>= 8;
    }
}
 
/* information about the WAV file format from
    http://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 */
 
void write_wav_header(char * filename, unsigned long num_samples, short int * data, int s_rate)
{
    FILE* wav_file;
    unsigned int sample_rate;
    unsigned int num_channels;
    unsigned int bytes_per_sample;
    unsigned int byte_rate;
    unsigned long i;    /* counter for samples */
 
    num_channels = 2;   /* stereo */
    bytes_per_sample = 2;
 
    if (s_rate<=0) sample_rate = 44100;
    else sample_rate = (unsigned int) s_rate;
 
    byte_rate = sample_rate*num_channels*bytes_per_sample;
 
    wav_file = fopen(filename, "w");
    //assert(wav_file);   /* make sure it opened */
 
    /* write RIFF header */
    fwrite("RIFF", 1, 4, wav_file);
    write_little_endian(36 + bytes_per_sample* num_samples*num_channels, 4, wav_file);
    fwrite("WAVE", 1, 4, wav_file);
 
    /* write fmt  subchunk */
    fwrite("fmt ", 1, 4, wav_file);
    write_little_endian(16, 4, wav_file);   /* SubChunk1Size is 16 */
    write_little_endian(1, 2, wav_file);    /* PCM is format 1 */
    write_little_endian(num_channels, 2, wav_file);
    write_little_endian(sample_rate, 4, wav_file);
    write_little_endian(byte_rate, 4, wav_file);
    write_little_endian(num_channels*bytes_per_sample, 2, wav_file);  /* block align */
    write_little_endian(8*bytes_per_sample, 2, wav_file);  /* bits/sample */
 
    /* write data subchunk */
    fwrite("data", 1, 4, wav_file);
    write_little_endian(bytes_per_sample* num_samples*num_channels, 4, wav_file);
    for (i=0; i< num_samples; i++)
    {   write_little_endian((unsigned int)(data[i]),bytes_per_sample, wav_file);
    }
 
    fclose(wav_file);
}

int main (int argc, char *argv[]){

	setup_device(argv[1]);
	prepare_buffer();
	unsigned int period = get_period_time();
	printf("period time: %dus\n", period);
	long rec_time = strtol(argv[2], NULL, 10);
	//i dont know why, but this factor of 2 seems to make it work
	long loops = (rec_time*2*1000000)/period; 
	
  for (i = 0; i < loops; ++i) {
    avail_cap = snd_pcm_avail ( capture_handle  );
    
    //fprintf (stderr,"snd_pcm_avail: %d ", (avail_cap=snd_pcm_avail_update( capture_handle  ))  );

    if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
      fprintf (stderr, "read from audio interface failed (%s)\n", snd_strerror (err));
      exit (1);
    }

/*    for (j = 0; j < 128; j+=2) {*/
/*      */
/*      sampleLP = ( short * )(&buffer[j]   );*/
/*      sampleRP = ( short * )(&buffer[j+1] );*/
/*      printf( "f(%d,%d)\n", ( int ) (sampleLP[0]) ,  ( int )( sampleRP[0] ) );*/
/* */
/*      //printf( "f(%d,%d,0,0)\n", buffer[j], buffer[j+1]);*/
/* */
/* */
/*    }  */
  }

  free(buffer);

  fprintf(stderr, "buffer freed\n");
	
  snd_pcm_close (capture_handle);
  fprintf(stderr, "audio interface closed\n");

  exit (0);
}


