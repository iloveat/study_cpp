#include <iostream>
#include <assert.h>
#include <cstring>
#include <vector>

#include "../../world-master/src/world/sptk.h"
#include "../../world-master/src/world/common.h"
#include "../../world-master/src/world/matlabfunctions.h"
#include "../../world-master/src/world/synthesisrealtime.h"

using namespace std;

typedef struct {
  double frame_period;
  int fs;
  int bap_dim;
  int mgc_dim;
  double mgc_alpha;
  double *f0;

  //double *time_axis;
  int f0_length;

  double **spectrogram;
  double **aperiodicity;
  double **coarse_aperiodicities;
  int fft_size;
} WorldParameters;

void DestroyParameters(WorldParameters *world_parameters);

void save_parameters_to_disk(WorldParameters &world_parameters,
                             const int num_frame);


void HTS_world_par(WorldParameters &world_parameters,
                   double lf0,
                   double *bap,
                   double *mgc,
                   int framenum);


void cap2ap(WorldParameters &world_parameters);

class StreamVocoder{
public:
    WorldSynthesizer synthesizer = { 0 };
    int buffer_size;
    int pcm_all_length;
    WorldParameters world_parameters;
    StreamVocoder(vector<double> params,
                  const int mgc_dim,
                  const int bap_dim,
                  const int num_frame,
		  const int sample_rate, 
                  const int fft_size,
                  const int buffer_size_,
                  const double mgc_alpha, 
		  const int mgc2sp_omp_threads);
    ~StreamVocoder();

    void world_parameters_init(WorldParameters &world_parameters,const double *params);

    vector<short> get_pcm_stream();
};

