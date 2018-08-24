#include <iostream>
#include <assert.h>
#include "vocoder/common.h"
#include "vocoder/world.h"
#include "vocoder/synthesis.h"

using namespace std;


void merge_file(const char *plf0, const char *pbap, const char *pmgc, const char *pout, const int &num_spectrum)
{
    /**
     * when in 16k, mgc=60, lf0=1, bap=1
     * when in 24k, mgc=60, lf0=1, bap=3
     * when in 48k, mgc=60, lf0=1, bap=5
     */
#ifdef K16000
    const int num_bap = 1;
#else
    #ifdef K24000
        const int num_bap = 3;
    #else
        #ifdef K48000
            const int num_bap = 5;
        #else
            const int num_bap = 1;
        #endif
    #endif
#endif
    FILE *fp_lf0 = fopen(plf0, "rb");
    FILE *fp_bap = fopen(pbap, "rb");
    FILE *fp_mgc = fopen(pmgc, "rb");
    fseek(fp_lf0, 0, SEEK_END);
    fseek(fp_bap, 0, SEEK_END);
    fseek(fp_mgc, 0, SEEK_END);
    unsigned int len_lf0 = ftell(fp_lf0);
    unsigned int len_bap = ftell(fp_bap);
    unsigned int len_mgc = ftell(fp_mgc);
    assert(len_bap == len_lf0*num_bap);
    assert(len_mgc == len_lf0*num_spectrum);
    rewind(fp_lf0);
    rewind(fp_bap);
    rewind(fp_mgc);

    int num_frame = len_lf0/(sizeof(float));

    float flt_out;
    FILE *fp_out = fopen(pout, "wb+");
    for(int i = 0; i < num_frame; i++)
    {
        for(int j = 0; j < num_spectrum; j++)
        {
            fread(&flt_out, sizeof(float), 1, fp_mgc);  //mgc
            fwrite(&flt_out, sizeof(float), 1, fp_out);
        }
        fread(&flt_out, sizeof(float), 1, fp_lf0);  //lf0
        fwrite(&flt_out, sizeof(float), 1, fp_out);

        for(int j = 0; j < num_bap; j++)
        {
            fread(&flt_out, sizeof(float), 1, fp_bap);  //bap
            fwrite(&flt_out, sizeof(float), 1, fp_out);
        }
    }
    fclose(fp_out);

    fclose(fp_lf0);
    fclose(fp_bap);
    fclose(fp_mgc);
}

void reorder_bap(const char *pbap, const int &num_bap, const char *pout)
{
    FILE *fp_bap = fopen(pbap, "rb");
    fseek(fp_bap, 0, SEEK_END);
    unsigned int len_bap = ftell(fp_bap);
    rewind(fp_bap);
    int num_frame = len_bap/(sizeof(float))/num_bap;

    float flt_out;
    FILE *fp_out = fopen(pout, "wb+");
    for(int i = 0; i < num_frame; i++)
    {
        for(int j = 0; j < num_bap; j++)
        {
            if(j == 0)
            {
                fread(&flt_out, sizeof(float), 1, fp_bap);  //bap
                flt_out = flt_out * 1.3;
                fwrite(&flt_out, sizeof(float), 1, fp_out);
            }
        }
    }
    fclose(fp_out);
    fclose(fp_bap);
}

void synth_test(const int &offline, const int &num_spectrum, const int &num_skip, const double &world_alpha, const char *pinput, const char *poutput)
{
    assert(offline == 1 || offline == 0);

    /**
     * when in 16k, mgc=60, lf0=1, bap=1
     * when in 24k, mgc=60, lf0=1, bap=3
     * when in 48k, mgc=60, lf0=1, bap=5
     */
#ifdef K16000
    const int num_bap = 1;
    const int sample_rate = 16000;
#else
    #ifdef K24000
        const int num_bap = 3;
        const int sample_rate = 24000;
    #else
        #ifdef K48000
            const int num_bap = 5;
            const int sample_rate = 48000;
        #else
            const int num_bap = 1;
            const int sample_rate = 8000;
        #endif
    #endif
#endif

    int num_column = num_spectrum+1+num_bap;

    FILE *fp_all = fopen(pinput, "rb");
    fseek(fp_all, 0, SEEK_END);
    unsigned int len_all = ftell(fp_all);
    assert(len_all % (sizeof(float)*num_column) == 0);
    rewind(fp_all);

    int num_frame = len_all/(sizeof(float)*num_column);  // calculate frame number

    float **world_input_param = (float**)malloc(num_frame*sizeof(float*));
    for(int i = 0; i < num_frame; i++)
    {
        world_input_param[i] = (float*)malloc(num_column*sizeof(float));
        fread(world_input_param[i], sizeof(float), num_spectrum, fp_all);  //mgc
        fread(world_input_param[i]+num_spectrum, sizeof(float), 1, fp_all);  //lf0
        fread(world_input_param[i]+num_spectrum+1, sizeof(float), num_bap, fp_all);  //bap
    }
    fclose(fp_all);

    int y_length = (int)((num_frame-num_skip*2)*5.0/1000*sample_rate);
    double *y = (double *)malloc(sizeof(double)*y_length);

    HTS_World_Synthesize(world_input_param,
                         world_alpha,
                         num_spectrum,
                         num_frame,
                         0+num_skip,
                         num_frame-num_skip,
                         num_skip,
                         offline,
                         y_length,
                         y);

    for(int i = 0; i < num_frame; i++)
    {
        free(world_input_param[i]);
    }
    free(world_input_param);

    //wirte pcm file
    FILE *fout = fopen(poutput, "wb+");
    for(int i = 0; i < y_length; i++)
    {
        short tmp_signal = (short)(MyMaxInt(-32768, MyMinInt(32767, (int)(y[i]*32767))));
        fwrite(&tmp_signal, sizeof(short), 1, fout);
    }
    fclose(fout);
    free(y);
}


int main()
{
    // synthesize audio from file
#ifdef K16000
    //16K
    merge_file("f0mgcbap/16K_19.lf0", "f0mgcbap/16K_19.bap", "f0mgcbap/16K_19.mgc", "f0mgcbap/16K_19.params", 19);
    synth_test(1, 19, 5, 0.42, "f0mgcbap/16K_19.params", "16K_19.pcm");
    merge_file("f0mgcbap/16K_60.lf0", "f0mgcbap/16K_60.bap", "f0mgcbap/16K_60.mgc", "f0mgcbap/16K_60.params", 60);
    synth_test(0, 60, 0, 0.58, "f0mgcbap/16K_60.params",  "16K_60.pcm");
#else
    #ifdef K24000
        //24K
        merge_file("f0mgcbap/24K_60.lf0", "f0mgcbap/24K_60.bap", "f0mgcbap/24K_60.mgc", "f0mgcbap/24K_60.params", 60);
        synth_test(0, 60, 0, 0.63, "f0mgcbap/24K_60.params", "24K_60.pcm");
    #else
        #ifdef K48000
            merge_file("f0mgcbap/48K_60.lf0", "f0mgcbap/48K_60.bap", "f0mgcbap/48K_60.mgc", "f0mgcbap/48K_60.params", 60);
            synth_test(0, 60, 0, 0.77, "f0mgcbap/48K_60.params", "48K_60.pcm");
        #else
            merge_file("f0mgcbap/8K_60.lf0", "f0mgcbap/8K_60.bap", "f0mgcbap/8K_60.mgc", "f0mgcbap/8K_60.params", 60);
            synth_test(0, 60, 0, 0.58, "f0mgcbap/8K_60.params", "8K_60.pcm");
        #endif
    #endif
#endif

    return 0;
}

