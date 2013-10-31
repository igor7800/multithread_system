#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>

#define BUF_SIZE 4410
#define NUM_OF_FILTERS 13


//-----Shared variables-----
sem_t SHARED_VARS;
float coefficients[NUM_OF_FILTERS];
float mic_input[BUF_SIZE];
float ref_input[BUF_SIZE];



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
    for (; ;)
    { 
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
	
        //sem_wait (&SHARED_VARS);
        //playback(output_buf)
	//sem_post (&SHARED_VARS);	
    }
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
    float ref_buffer[BUF_SIZE];

   for (; ;)
   { 
       sem_wait (&SHARED_VARS); 	
       read_mic_input("sin_1000Hz.txt");
       sem_post (&SHARED_VARS);
	
       sem_wait (&SHARED_VARS);
       lp_filter(ref_input,ref_buffer);
       sem_post (&SHARED_VARS);	
       
       sem_wait (&SHARED_VARS);
       ref_rms = calculate_RMS (ref_input);
       printf ("%f\n",ref_rms);		
       sem_post (&SHARED_VARS); 
       
       sem_wait (&SHARED_VARS);
       mic_rms = calculate_RMS (mic_input);
       printf ("%f\n",mic_rms);		
       sem_post (&SHARED_VARS);
       
       sem_wait (&SHARED_VARS);
       coefficients[1]=ref_rms-mic_rms;
       printf ("%f\n",coefficients[1]);
       sem_post (&SHARED_VARS);
   }
}



int main (int argc, char *argv[]) 
{
    int id_number[2];
    pthread_t t_id[2];
    
    id_number[0] = 1;
    id_number[1] = 2;

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

    



    


