#include <iostream>
#include <assert.h>
#include "../../vocoder/common.h"
#include "../../vocoder/world.h"
#include "interface.h"

using namespace std;


int add(int a, int b)
{
	return a + b;
}

void synthesizeFromParamsFile(const int &offline,
                              const int &num_spectrum,
                              const int &num_skip,
                              const double &world_alpha,
                              const char *pinput,
                              const char *poutput)
{
    assert(offline == 1 || offline == 0);
    int num_column = num_spectrum+1+1;  //mgc+lf0+bap

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
        fread(world_input_param[i]+num_spectrum+1, sizeof(float), 1, fp_all);  //bap
    }
    fclose(fp_all);

    int y_length = (int)((num_frame-num_skip*2)*5.0/1000*16000);
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
