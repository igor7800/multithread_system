#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>
#include "mypcm.h"

#define BUF_SIZE 128
#define NUM_OF_FILTERS 13
#define CHANNELS 2
#define RATE 44100


//-----Shared variables-----
sem_t SHARED_VARS;
float coefficients[NUM_OF_FILTERS];
float mic_input[BUF_SIZE];
float ref_input[BUF_SIZE];


snd_pcm_t *capture_handle;
snd_pcm_t *playback_handle;
snd_pcm_hw_params_t *capture_params;
snd_pcm_hw_params_t *playback_params;  



/**
 * Reads from file and puts data in shared variable ref_input
 * @param *filename name of the file
 */
void read_ref_input (const char *filename) 
{
    int i = 0;
    float buf;
    FILE *fp; 
    fp = fopen(filename, "r");
    
    while (fscanf(fp, "%e", &buf) != EOF) 
    {  
        ref_input[i] = buf;
        i++;        
    } 
    fclose(fp);
}


/**
 * Reads from file and puts data in shared variable mic_input
 * @param *filename name of the file
 */
void read_mic_input (const char *filename) 
{
    int i = 0;
    float buf;
    FILE *fp; 
    fp = fopen(filename, "r");
    
    while (fscanf(fp, "%e", &buf) != EOF) 
    {  
        mic_input[i] = buf;
        i++;        
    } 
    fclose(fp);
}


/**
 * Function to calculate RMS value.
 * @param buf pointer to data_buffer. 
 * @return float containing RMS value.
 */
float calculate_RMS (float *buf) 
{
    float squared_sum;
    float  data_RMS;
    int i;
    
    for ( i = 0 ; i < sizeof(buf) ; i++ )
	squared_sum += buf[i] * buf[i];
    data_RMS = squared_sum / sizeof(buf);
    data_RMS = sqrt(data_RMS);
    return data_RMS;
}



/**
 * Function to apply lp-filter to a buffer.
 * @param *input_buf input to the filter
 * @param *output_buf output from the filter
 */
//fixme: Need a gain factor from shared variables
void lp_filter (float *input_buf,float *output_buf) {
    int i;
    float a = 0.8825;
    float b = 0.1250;
    float old_output = 0;
    for (i = 0; i < sizeof(input_buf); ++i)
    {
	output_buf[i] = a * old_output + b * input_buf[i];
	old_output = output_buf[i];
    } 
}


/**
 * Thread to playback audio and apply the output filter, by use of semaphor.
 * @param input ID of thread used in test.
 * @return void
 */
void player_filter (void * input) 
{
    int i;
    float output_buf[BUF_SIZE];
    float coeff[NUM_OF_FILTERS];

    sem_wait (&SHARED_VARS);
    read_ref_input("sin_1000Hz.txt");
    sem_post (&SHARED_VARS);
	
    sem_wait (&SHARED_VARS);
    for (i = 0; i <NUM_OF_FILTERS; ++i)
	coeff[i] = coefficients[i];	
    sem_post (&SHARED_VARS);
	
    sem_wait (&SHARED_VARS);
    lp_filter(ref_input,output_buf);
    sem_post (&SHARED_VARS);
	
    sem_wait (&SHARED_VARS);
    //fixme: must be a real  output_buffer from filter
    play(playback_handle,mic_input,BUF_SIZE); 
    sem_post (&SHARED_VARS);	

}
 


/**
 * Thread to compare the mic- and ref-signals, and write result in semaphor.
 * @param input ID of thread used in test.
 * @return void
 */
void controller (void * input)
{
    float ref_rms;
    float mic_rms;
    float filtered_mic_input[BUF_SIZE];
    float filtered_ref_input[BUF_SIZE];

    sem_wait (&SHARED_VARS); 	
    record(capture_handle,mic_input,BUF_SIZE);
    sem_post (&SHARED_VARS);
	
    sem_wait (&SHARED_VARS);
    lp_filter(mic_input,filtered_mic_input);
    sem_post (&SHARED_VARS);	
              
    sem_wait (&SHARED_VARS);
    lp_filter(ref_input,filtered_ref_input);
    sem_post (&SHARED_VARS);	
       
    sem_wait (&SHARED_VARS);
    ref_rms = calculate_RMS (filtered_ref_input);
    sem_post (&SHARED_VARS); 
       
    sem_wait (&SHARED_VARS);
    mic_rms = calculate_RMS (filtered_mic_input);
    sem_post (&SHARED_VARS);
       
    sem_wait (&SHARED_VARS);
    coefficients[1]=1/(ref_rms-mic_rms);
    sem_post (&SHARED_VARS);
       
}



int main (int argc, char *argv[]) 
{
  int id_number[2];
  pthread_t t_id[2];
  id_number[0] = 1;
  id_number[1] = 2;
  //---------------------------------------------------
  //-------ALSA setup----------------------------------
  //---------------------------------------------------
  open_pcm(&capture_handle,PCM_DEVICE,SND_PCM_STREAM_CAPTURE,0); 
  open_pcm(&playback_handle,PCM_DEVICE,SND_PCM_STREAM_PLAYBACK,0);
  //-----------------------------------------------------
  snd_pcm_hw_params_malloc (&playback_params);
  snd_pcm_hw_params_malloc (&capture_params);
  //----------------------------------------------------
  snd_pcm_hw_params_any (playback_handle, playback_params);
  snd_pcm_hw_params_any (capture_handle, capture_params);
  //----------------------------------------------------
  set_params(playback_handle,playback_params,CHANNELS,RATE);
  set_params(capture_handle,capture_params,CHANNELS,RATE);
  //----------------------------------------------------
  write_params(playback_handle,playback_params);    
  write_params(capture_handle,capture_params);
  //----------------------------------------------------
  prepair_interface(capture_handle);
  prepair_interface(playback_handle);
  //----------------------------------------------------
  snd_pcm_hw_params_free (playback_params);
  snd_pcm_hw_params_free (capture_params);
    
  //----------THREADS----------------------------------
  for (; ;)
  {
    sem_init(&SHARED_VARS, 0, 1);
    pthread_create(&t_id[0], NULL, (void *) &player_filter, (void *) &id_number[0]);
    pthread_create(&t_id[1], NULL, (void *) &controller   , (void *) &id_number[1]);
	
    pthread_join(t_id[0], NULL);
    pthread_join(t_id[1], NULL);
  }
    return 0;
} 

    



    


