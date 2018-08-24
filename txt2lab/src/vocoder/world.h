
typedef struct
{
    int fs;
    int f0_length;
    int fft_size;
    int number_of_aperiodicities;
    int m;
    int offline; //1:offline,0:online
	double *f0;
	double **spectrogram;
	double **aperiodicity;
	double **coarse_aperiodicities;
	double *time_axis;
    double frame_period;
	double alpha;
	double beta;
}
WorldParameters;

#ifdef __cplusplus
extern "C" {
#endif

void HTS_world_initialize(
        WorldParameters *world_parameters,
        const int m,
        const int rate,
        const double alpha,
        const double beta,
        const int f0_length,
        const int offline);

void HTS_world_par(
        WorldParameters *world_parameters,
        double lf0,
        double bap,
        double *spectrum,
        int framenum);

void WaveformSynthesis(
        WorldParameters *world_parameters,
        double *y,
        int y_length);

void HTS_world_clear(
        WorldParameters *world_parameters);

// Interface
void HTS_World_Synthesize(
        float **world_input_param,
        const double world_alpha,
        const int num_spectrum,
        const int num_frame,
        const int starttime,
        const int endtime,
        const int num_skip,
        const int offline,
        const int y_length,
        double *y);

#ifdef __cplusplus
}
#endif

