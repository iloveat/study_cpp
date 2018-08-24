#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "consts.h"
#include "matlabfuns.h"
#include "sptk.h"
#include "synthesis.h"
#include "world.h"

#define PI2     (6.28318530717958647692)
#define LSMALL  (-0.5e+10)
#define LOG2    (0.693147180559945)

#define HTRS_RH_PARA_TIMES      (3)
#define WL_REAR_DEAL_RH_PARA    (0.7)
#define	HTRS_RH_PARA_MIN_DIFF   (0.052)
#define	LZERO                   (-1.0e+10)
#define PI                      (3.14159265358979323846)

/* EM_lspcheck: rearrange LSPs to enhance formants     */
/* Added by naxy 2010-9-17					           */
/* Reference:								           */
/*   "USTC Blizzard Challenge 2006 Report"             */
/* Notice:                                             */
/*   This is not like lspcheck in SPTK. It is actually */
/*   a formant enhancement function.                   */
static void EM_lspcheck(double *lsp, double *d, double *nlsp, const int m)
{
    double dd1, dd2;

    for(int i = 0; i < m -1; i++)
    {
        d[i] = WL_REAR_DEAL_RH_PARA *(lsp[i+1]-lsp[i]);
    }

    nlsp[0] = lsp[0];
    for(int i = 1; i < m -1; i++)
    {
        dd1 = d[i-1] * d[i-1];
        dd2 = d[i] * d[i];
        nlsp[i] = lsp[i-1] + d[i-1] + (lsp[i+1] - lsp[i-1] - d[i] - d[i-1])*dd1/(dd1+dd2);
    }
    nlsp[m-1] = lsp[m-1];
}

static void EM_lspcheck_modify(double *nlsp, const int m)
{
    double f1, f2;

    for(int i = 0; i < m -1; i++)
    {
        f1 = nlsp[i+1]-nlsp[i];
        if(f1 < HTRS_RH_PARA_MIN_DIFF)
        {
            f2 = (HTRS_RH_PARA_MIN_DIFF-f1)/2;
            nlsp[i]   -= f2;
            nlsp[i+1] += f2;
        }
    }
}

void HTS_world_initialize(WorldParameters *world_parameters,
                            const int m,
                            const int rate,
                            const double alpha,
                            const double beta,
                            const int f0_length,
                            const int offline)
{
	world_parameters->alpha = alpha;
	world_parameters->beta = beta;
	world_parameters->m = m;
	world_parameters->number_of_aperiodicities = NUMBER_OF_AP;
	world_parameters->fs = rate;
	world_parameters->frame_period = FRAMEPERIOD;
	world_parameters->fft_size = FFT_SIZE;
	world_parameters->f0_length = f0_length;
    world_parameters->offline = offline;

	world_parameters->f0 = (double *)malloc(sizeof(double)*world_parameters->f0_length);
	world_parameters->aperiodicity = (double **)malloc(sizeof(double *)*world_parameters->f0_length);
	world_parameters->spectrogram = (double **)malloc(sizeof(double *)*world_parameters->f0_length);
	world_parameters->coarse_aperiodicities = (double **)malloc(sizeof(double *)*world_parameters->f0_length);

    for(int i = 0; i < world_parameters->f0_length; i++)
	{
		world_parameters->aperiodicity[i] = (double *)malloc(sizeof(double)*(FFT_SIZE / 2 + 1));
		world_parameters->spectrogram[i] = (double *)malloc(sizeof(double)*(FFT_SIZE / 2 + 1));
		world_parameters->coarse_aperiodicities[i] = (double *)malloc(sizeof(double)*(world_parameters->number_of_aperiodicities));
	}
}

double log_conv(double x)
{
    double temp = log(fabs(x));
    if(temp < LSMALL)
    {
		return LZERO;
    }

    return temp;
}

double log_add(double x, double y)
{
    if(x == y)
    {
		return x + LOG2;
    }

    double lmin = (x < y) ? x : y;
    double lmax = (x < y) ? y : x;

    if(lmax > lmin + 50)
    {
		return lmax;
    }
	else
    {
		return lmax + log_conv(exp(lmin - lmax) + 1.0);
    }
}

double mel_conv(double a, double w)
{
	return w + 2.0 * atan(a * sin(w) / (1.0 - a * cos(w)));
}

void HTS_mgclsp2sp(double a, double g, double *lsp, const int m, double *x, const int l, const int gain)
{
	double w, eq1, eq2, ap = 0.0;

    for(int p = 0; p < l; p++)
    {
		eq1 = 0.0;
		eq2 = 0.0;
		w = mel_conv(a, p * (PI / (l - 1)));

        if(m % 2 == 0)
        {
            for(int i = 0; i < m / 2; i++)
            {
				eq1 += 2.0 * log_conv(cos(w) - cos(lsp[2 * i + gain]));
				eq2 += 2.0 * log_conv(cos(w) - cos(lsp[2 * i + 1 + gain]));
			}
			eq1 += 2.0 * log_conv(cos(w / 2.0));
			eq2 += 2.0 * log_conv(sin(w / 2.0));
			ap = m * log(2.0) + log_add(eq1, eq2);
		}
        else
        {
            for(int i = 0; i < (m + 1) / 2; i++)
            {
				eq1 += 2.0 * log_conv(cos(w) - cos(lsp[2 * i + gain]));
            }

            for(int i = 0; i < (m - 1) / 2; i++)
            {
				eq2 += 2.0 * log_conv(cos(w) - cos(lsp[2 * i + 1 + gain]));
            }

			eq2 += 2.0 * log_conv(sin(w));
			ap = (m - 1.0) * log(2.0) + log_add(eq1, eq2);
		}

		x[p] = -0.5 * ap;
		x[p] *= -(1.0 / g);
        if(gain == 1)
        {
			x[p] += lsp[0];
        }
	}
}

static void HTS_mgclsp2sp_lspchecked(double *mgc, const int m, const double a, const double g, double *x, double *y, const int flng)
{
	static double *c = NULL;
	static int size;
	double *lsp_buff = NULL;
	double *lspdiff = NULL;
    int order = m;

    if(c == NULL)
    {
		c = (double *)malloc((flng / 2 + 1)*sizeof(double));
		size = flng;
	}
    if(flng > size)
    {
        if(c)
        {
		    free(c);
		    c = NULL;
		}
		c = (double *)malloc((flng / 2 + 1)*sizeof(double));
		size = flng;
	}

	lsp_buff = (double *)malloc((order * 2 + 1)*sizeof(double));
	lspdiff = lsp_buff + order + 1;
	EM_lspcheck_modify(mgc + 1, m);
	lsp_buff[0] = mgc[0];
    for(int i = 0; i < HTRS_RH_PARA_TIMES; i++)
	{
        EM_lspcheck(mgc + 1, lspdiff, lsp_buff + 1, m);
        EM_lspcheck_modify(lsp_buff + 1, m);
		memcpy(mgc, lsp_buff, (m + 1)*sizeof(double));
	}

	if(lsp_buff)
	{
	    free(lsp_buff);
	    lsp_buff = NULL;
	}

    HTS_mgclsp2sp(0, -1, mgc, m, x, flng / 2 + 1, 1);
}

void WaveformSynthesis(WorldParameters *world_parameters, double *y, int y_length)
{
	double coarse_aperiodicity[NUMBER_OF_AP + 2] = { 0 };
	double coarse_frequency_axis[NUMBER_OF_AP + 2] = { 0 };
	double frequency_axis[FFT_SIZE / 2 + 1] = { 0 };

	coarse_aperiodicity[0] = -60.0;
	coarse_aperiodicity[NUMBER_OF_AP + 1] = 0.0;

    for(int i = 0; i <= NUMBER_OF_AP; ++i)
    {
		coarse_frequency_axis[i] = (double)(i* kFrequencyInterval);
    }
	coarse_frequency_axis[NUMBER_OF_AP + 1] = SAMPLERATE / 2.0;

    for(int i = 0; i <= FFT_SIZE / 2; ++i)
    {
		frequency_axis[i] = i*((double)SAMPLERATE / FFT_SIZE);
    }

    //load band ap values for this frame into coarse_aperiodicity
    //omp_set_num_threads(2);
    //#pragma omp parallel for
    for(int i = 0; i < world_parameters->f0_length; i++)
	{
        for(int k = 0; k < NUMBER_OF_AP; ++k)
        {
			coarse_aperiodicity[k + 1] = world_parameters->coarse_aperiodicities[i][k];
		}
        interp1(coarse_frequency_axis, coarse_aperiodicity, NUMBER_OF_AP + 2, frequency_axis, FFT_SIZE / 2 + 1, world_parameters->aperiodicity[i]);

        for(int j = 0; j <= FFT_SIZE / 2; ++j)
        {
			world_parameters->aperiodicity[i][j] = pow(10.0, world_parameters->aperiodicity[i][j] / 20.0);
        }
    }

    Synthesis(world_parameters->f0, world_parameters->f0_length, world_parameters->spectrogram,
              world_parameters->aperiodicity, world_parameters->fft_size,
              world_parameters->frame_period, world_parameters->fs, y_length, y);
}

//world vocoder
void HTS_world_par(WorldParameters *world_parameters,
                   double lf0,
                   double *bap,
                   double *spectrum,
                   int framenum)
{
	double *x2 = NULL;
	double *y2 = NULL;
	double *c2 = NULL;

    if(lf0 == 0.0)
    {
        world_parameters->f0[framenum] = 0.0;
    }
	else
    {
        world_parameters->f0[framenum] = exp(lf0);
    }

    for(int i = 0; i < world_parameters->number_of_aperiodicities; i++)
	{
        if(bap[i] > 0)
        {
            bap[i] = 0.0;
        }
        world_parameters->coarse_aperiodicities[framenum][i] = bap[i];
	}

	x2 = (double *)malloc((2 * FFT_SIZE + world_parameters->m + 1)*sizeof(double));
	y2 = x2 + FFT_SIZE;
	c2 = y2 + FFT_SIZE;
	memcpy(c2, spectrum, (world_parameters->m + 1)*sizeof(double));

    switch(world_parameters->offline)
    {
    case 1:  //offline
        HTS_mgclsp2sp_lspchecked(c2, world_parameters->m, world_parameters->alpha, world_parameters->beta, x2, y2, FFT_SIZE);
        break;
    case 0:  //online
        mgc2sp(c2, world_parameters->m, world_parameters->alpha, world_parameters->beta, x2, y2, FFT_SIZE);
        break;
    default:
        break;
    }

    for(int i = FFT_SIZE / 2 + 1; i--;)
	{
		world_parameters->spectrogram[framenum][i] = pow(exp(x2[i]) / 32768, 2);
	}

	if(x2)
	{
	    free(x2);
	    x2 = NULL;
	}
}

void HTS_world_clear(WorldParameters *world_parameters)
{
    if(world_parameters)
    {
        if(world_parameters->f0)
        {
    	    free(world_parameters->f0);
    	    world_parameters->f0 = NULL;
    	}
        for(int i = 0; i < world_parameters->f0_length; i++)
    	{
    	    if(world_parameters->aperiodicity[i])
    	    {
    		    free(world_parameters->aperiodicity[i]);
    		    world_parameters->aperiodicity[i] = NULL;
    		}
    		if(world_parameters->coarse_aperiodicities[i])
    		{
    		    free(world_parameters->coarse_aperiodicities[i]);
    		    world_parameters->coarse_aperiodicities[i] = NULL;
    		}
    		if(world_parameters->spectrogram[i])
    		{
    		    free(world_parameters->spectrogram[i]);
    		    world_parameters->spectrogram[i] = NULL;
    		}
    	}
    	if(world_parameters->aperiodicity)
    	{
            free(world_parameters->aperiodicity);
            world_parameters->aperiodicity = NULL;
    	}
    	if(world_parameters->coarse_aperiodicities)
        {
            free(world_parameters->coarse_aperiodicities);
            world_parameters->coarse_aperiodicities = NULL;
        }
        if(world_parameters->spectrogram)
        {
            free(world_parameters->spectrogram);
            world_parameters->spectrogram = NULL;
        }

        world_parameters = NULL;
    }
}



void save_parameters_to_disk(WorldParameters* world_parameters, const int num_frame){
    FILE *f0_file = fopen("f0_","w");
    if (f0_file==NULL) return;
    for (int i=0;i<num_frame;i++){
        fprintf(f0_file, "%lf\n", world_parameters->f0[i]);
    }
    fclose(f0_file);

    FILE *ap_file = fopen("ap_","w");
    if (ap_file==NULL) return;
    for (int i=0;i<num_frame;i++){
        for (int j=0;j<FFT_SIZE/2;j++){
            fprintf(ap_file, "%lf,", world_parameters->aperiodicity[i][j]);
        }
        fprintf(ap_file, "\n");
    }
    fclose(ap_file);

    FILE *sp_file = fopen("sp_","w");
    if (sp_file==NULL) return;
    for (int i=0;i<num_frame;i++){
        for (int j=0;j<FFT_SIZE/2;j++){
            fprintf(sp_file, "%lf,", world_parameters->spectrogram[i][j]);
        }
        fprintf(sp_file, "\n");
    }
    fclose(sp_file);
}

void HTS_World_Synthesize(float **world_input_param,
                          const double world_alpha,
                          const int num_spectrum,
                          const int num_frame,
                          const int starttime,
                          const int endtime,
                          const int num_skip,
                          const int offline,
                          const int y_length,
                          double *y)
{
    assert(offline == 0 || offline == 1);

#ifdef K16000
    int world_fs = 16000;
#else
    #ifdef K24000
        int world_fs = 24000;
    #else
        #ifdef K48000
            int world_fs = 48000;
        #else
            int world_fs = 8000;
        #endif
    #endif
#endif

    double world_beta = 0.0;
    int world_m = num_spectrum-1;
    int world_f0_length = num_frame-num_skip*2;

    WorldParameters world_parameters;
    HTS_world_initialize(&world_parameters,
                         world_m,
                         world_fs,
                         world_alpha,
                         world_beta,
                         world_f0_length,
                         offline);
    double world_lf0 = 0.0;
    double *world_mgc = (double*)malloc(num_spectrum*sizeof(double));
    double *world_bap = (double*)malloc(NUMBER_OF_AP*sizeof(double));

    for(int i = starttime; i < endtime; i++)
    {
        for(int j = 0; j < num_spectrum; j++)
        {
            world_mgc[j] = (double)(world_input_param[i][j]);
        }
        world_lf0 = (double)world_input_param[i][num_spectrum];
        for(int j = 0; j < NUMBER_OF_AP; j++)
        {
            world_bap[j] = (double)world_input_param[i][num_spectrum + 1 + j];
        }

        HTS_world_par(&world_parameters, world_lf0, world_bap, world_mgc, i-starttime);
    }

    if(world_mgc)
    {
        free(world_mgc);
        world_mgc = NULL;
    }

    if(world_bap)
    {
        free(world_bap);
        world_bap = NULL;
    }

    WaveformSynthesis(&world_parameters, y, y_length);

    //save_parameters_to_disk(&world_parameters, num_frame);

    HTS_world_clear(&world_parameters);
}


