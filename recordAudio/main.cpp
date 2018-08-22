#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>

snd_pcm_t* initRecorder()
{
    int channels = 1;
    unsigned int sample_rate = 16000;
    snd_pcm_format_t pcm_format = SND_PCM_FORMAT_S16_LE;
    snd_pcm_t *capture_handle = NULL;
    snd_pcm_hw_params_t *hw_params = NULL;

    int ret = snd_pcm_open(&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0);
    if(ret < 0)
    {
        printf("cannot open audio device %s (%s)\n", "default", snd_strerror(ret));
        return NULL;
    }

    ret = snd_pcm_hw_params_malloc(&hw_params);
    if(ret < 0)
    {
        printf("cannot allocate hardware parameter structure (%s)\n", snd_strerror(ret));
        snd_pcm_close(capture_handle);
        return NULL;
    }

    ret = snd_pcm_hw_params_any(capture_handle, hw_params);
    if(ret < 0)
    {
        printf("cannot initialize hardware parameter structure (%s)\n", snd_strerror(ret));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return NULL;
    }

    ret = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if(ret < 0)
    {
        printf("cannot set access type (%s)\n", snd_strerror(ret));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return NULL;
    }

    ret = snd_pcm_hw_params_set_format(capture_handle, hw_params, pcm_format);
    if(ret < 0)
    {
        printf("cannot set sample format (%s)\n", snd_strerror(ret));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return NULL;
    }

    ret = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &sample_rate, 0);
    if(ret < 0)
    {
        printf("cannot set sample rate (%s)\n", snd_strerror(ret));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return NULL;
    }

    ret = snd_pcm_hw_params_set_channels(capture_handle, hw_params, channels);
    if(ret < 0)
    {
        printf("cannot set channel count (%s)\n", snd_strerror(ret));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return NULL;
    }

    ret = snd_pcm_hw_params(capture_handle, hw_params);
    if(ret < 0)
    {
        printf("cannot set parameters (%s)\n", snd_strerror(ret));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return NULL;
    }

    ret = snd_pcm_prepare(capture_handle);
    if(ret < 0)
    {
        printf("cannot prepare audio interface for use (%s)\n", snd_strerror(ret));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        return NULL;
    }

    snd_pcm_hw_params_free(hw_params);
    return capture_handle;
}


int main(int argc, char **argv)
{
    snd_pcm_t *handle = initRecorder();
    if(handle == NULL)
        return -1;
    printf("init finish\n");

    int nframes = 512;
    int buff_size = nframes*2;
    char *pcm_buff = (char*)malloc(buff_size*sizeof(char));
    FILE *fp = fopen("record.pcm", "wb");

    for(int i = 0; i < 100; i++)
    {
        int ret = snd_pcm_readi(handle, pcm_buff, nframes);
        if(ret != nframes)
        {
            printf("read from audio interface failed (%s)\n", snd_strerror(ret));
            break;
        }
        fwrite(pcm_buff, 1, buff_size, fp);
        printf("read %d done\n", i);
    }

    fclose(fp);
    free(pcm_buff);
    snd_pcm_close(handle);
    printf("record finish\n");

    return 0;
}

