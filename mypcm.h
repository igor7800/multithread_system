#include <alsa/asoundlib.h>

#define PCM_DEVICE "plughw:0,0"
 
 
/**
 * Open the PCM device in playback mode 
 * and writes errror if device 
 * @param **pcm_handle handle to pcm
 * @param *card audio card to play to
 * @param stream Wanted stream
 * @param mode Open mode
 */
void open_pcm (snd_pcm_t **pcm_handle, 
		  char *card,
		  snd_pcm_stream_t stream, 
		  int mode)
{
    unsigned int pcm;
    pcm = snd_pcm_open(pcm_handle,card, stream, 0);
    if (pcm < 0)
    { 
	printf("ERROR: Can't open \"%s\" PCM device. %s\n",
	       card, snd_strerror(pcm));
	exit(1);
    }  

}


/**
 * Restrict a configuration space to contain only
 * one access type 
 * and writes an error if interleaved mode can't be set
 * @param **pcm_handle handle to pcm
 * @param *params configuration space
 */
void set_access (snd_pcm_t *pcm_handle, 
		    snd_pcm_hw_params_t *params)
{
    unsigned int pcm;
    pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
				       SND_PCM_ACCESS_RW_INTERLEAVED);
    if (pcm < 0)
    { 
	printf("ERROR: Can't set interleaved mode. %s\n", 
	       snd_strerror(pcm));
	exit(1);
    }
}

/**
 * Restrict a configuration space to contain only one format 
 * and writes an error if format can't be set
 * @param **pcm_handle handle to pcm
 * @param *params configuration space
 */
void set_format (snd_pcm_t *pcm_handle,
		    snd_pcm_hw_params_t *params)
{
    unsigned int pcm;
    pcm = snd_pcm_hw_params_set_format(pcm_handle, params,
				       SND_PCM_FORMAT_S16_LE);
    if (pcm < 0)
    { 
	printf("ERROR: Can't set format. %s\n",
	       snd_strerror(pcm));
	exit(1);
    }
}


/**
 * Restrict a configuration space to one channels count 
 * and writes an error if channels number can't be set
 * @param **pcm_handle handle to pcm
 * @param *params configuration space
 * @param channels channels count
 */
void set_channels(snd_pcm_t *pcm_handle,
		     snd_pcm_hw_params_t *params,
		     int channels)
{
    unsigned int pcm;
    pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
    if (pcm < 0)
    { 
	printf("ERROR: Can't set channels number. %s\n", 
	       snd_strerror(pcm));
	exit(1);
    }
}


/**
 * Restrict a configuration space to contatin only one rate 
 * and writes an error if rate can't be set
 * @param **pcm_handle handle to pcm
 * @param *params configuration space
 * @param rate approximate rate
 */
void set_rate(snd_pcm_t *pcm_handle,
		 snd_pcm_hw_params_t *params,
		 int rate)
{
    unsigned int pcm;
    pcm = snd_pcm_hw_params_set_rate(pcm_handle, params, rate, 0);
    if (pcm < 0) 
    {
	printf("ERROR: Can't set rate. %s\n", 
	       snd_strerror(pcm));
	exit(1);
    }
}


/**
 * Set hardware parameters
 * and writes an errors if parameters can't be set
 * @param **pcm_handle handle to pcm
 * @param *params configuration space
 * @param channels channels count
 * @param rate approximate rate
 */
void set_params(snd_pcm_t *pcm_handle, 
		   snd_pcm_hw_params_t *params,
		   int channels,
		   int rate)
{
    set_access(pcm_handle,params);
    set_format(pcm_handle,params);
    set_channels(pcm_handle,params,channels);
    set_rate(pcm_handle,params,rate);
}


/**
 * Write hardware parameters
 * and writes an error if hardware parameters can't be set
 * @param *pcm_handle handle to the pcm
 * @param *params configuration space
 */
void write_params(snd_pcm_t *pcm_handle, 
		     snd_pcm_hw_params_t *params)
{
    unsigned int pcm;
    pcm = snd_pcm_hw_params(pcm_handle, params);
    if (pcm < 0)
    {
	printf("ERROR: Can't set harware parameters. %s\n",
	       snd_strerror(pcm));
	exit(1);
    }
}
/**
 * Prepair audio interface for use
 * and writes an error if it can't be prepaired
 * @param *pcm_handle handle to the pcm
 */
void prepair_interface(snd_pcm_t *pcm_handle)
{
    unsigned int pcm;
    pcm = snd_pcm_prepare (pcm_handle);
    if (pcm < 0)
    {
	fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
		 snd_strerror (pcm));
	exit (1);
    }
}


/**
 * Extracts period time from a configuration space
 * and writes an error if period time can't be extracted
 * @param *params configuration space
 * @return tmp approximate period duration
 */
unsigned int get_period_time(snd_pcm_hw_params_t *params)
{
    unsigned int tmp,pcm;
    pcm = snd_pcm_hw_params_get_period_time(params, &tmp, NULL);  
    if (pcm < 0)
    {
	printf("ERROR: Can't extract maximum period time.%s\n",
	       snd_strerror(pcm));
	exit(1); 
    }
    return tmp;
}

/**
 * Write contents of a buffer to the PCM
 * write error when unable to write to PCM device 
 * @param **pcm_handle handle to playback
 * @param frames period size
 * @param *buffer pointer to buffer
 * @param buff_size size of the buffer
 */
void play(snd_pcm_t *pcm_handle,  
	     float *buff,
	     int buffer_size)		  
{
    unsigned int pcm;
    pcm = snd_pcm_writei(pcm_handle, buff, buffer_size);
    if (pcm == -EPIPE)
    {
	printf("ERROR: an underrun occured\n");
	snd_pcm_prepare(pcm_handle);
    }
    else if (pcm < 0) 
    {
	printf("ERROR. Can't write to PCM device. %s\n",
	       snd_strerror(pcm));
	exit(1);
    }

}


/**
 * Reading from audio card to the buffer
 * write errors too
 * @param **pcm_handle handle to playback
 * @param frames period size
 * @param period length of period
 * @param *buffer pointer to buffer
 * @param buff_size size of the buffer
 */
void record(snd_pcm_t *pcm_handle,  
	       float *buff,	  
	       snd_pcm_uframes_t frames) 
		  
{
    unsigned int pcm;
    pcm = snd_pcm_readi (pcm_handle, buff, frames);
    if (pcm != frames) 
    {
	fprintf (stderr, "ERROR: read from audio interface failed (%s)\n",
		 snd_strerror (pcm));
	exit (1);
    }
    if (pcm == -EBADFD)
    {
	fprintf (stderr, "ERROR: PCM is not in the right state (%s)\n",
		 snd_strerror (pcm));
	exit (1);
    }
    if (pcm == -EPIPE)
    {
	fprintf (stderr, "ERROR: an overrun occured  (%s)\n",
		 snd_strerror (pcm));
	exit (1);
    }
    if (pcm == -ESTRPIPE)
    {
	fprintf (stderr, "ERROR: a suspend event occured (%s)\n",
		 snd_strerror (pcm));
	exit (1);
    }
}
