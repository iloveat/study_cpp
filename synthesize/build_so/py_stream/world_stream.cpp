#include "./world_stream.h"
#include <sys/time.h>
#include <omp.h>
#include <pthread.h>


using namespace std;


inline long long currentTimeInMilliseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

StreamVocoder::StreamVocoder(vector<double> params,
                  const int mgc_dim,
                  const int bap_dim,
                  const int num_frame,
                  const int sample_rate,
                  const int fft_size,
                  const int buffer_size_,
                  const double mgc_alpha,
                  const int mgc2sp_omp_threads){
    omp_set_num_threads(mgc2sp_omp_threads);
    world_parameters = { 0 };
    world_parameters.fs = sample_rate;
    world_parameters.f0_length = num_frame;
    world_parameters.fft_size = fft_size;
    world_parameters.mgc_dim = mgc_dim;
    world_parameters.bap_dim = bap_dim;
    world_parameters.mgc_alpha = mgc_alpha;
    buffer_size = buffer_size_;


    world_parameters_init(world_parameters, params.data());
    pcm_all_length = static_cast<int>((world_parameters.f0_length - 1) *
      world_parameters.frame_period / 1000.0 * world_parameters.fs) + 1;


      InitializeSynthesizer(world_parameters.fs, world_parameters.frame_period,
          world_parameters.fft_size, buffer_size, 1, &synthesizer);


      // All parameters are added at the same time.
      AddParameters(world_parameters.f0, world_parameters.f0_length,
          world_parameters.spectrogram, world_parameters.aperiodicity,
          &synthesizer);
}

StreamVocoder::~StreamVocoder(){
    DestroySynthesizer(&synthesizer);
    DestroyParameters(&world_parameters);
}


void StreamVocoder::world_parameters_init(WorldParameters &world_parameters,const double *params){
    int mgc_dim = world_parameters.mgc_dim;
    int bap_dim = world_parameters.bap_dim;
    int params_dim = mgc_dim + bap_dim + 1;
    int num_frame = world_parameters.f0_length;
    int fft_size = world_parameters.fft_size;
    world_parameters.frame_period = 5.0;
    world_parameters.f0 = new double [num_frame];
    world_parameters.spectrogram = new double *[num_frame];
    world_parameters.aperiodicity = new double *[num_frame];
    world_parameters.coarse_aperiodicities = new double *[num_frame];



    long long t1 = currentTimeInMilliseconds();
    for (int i=0;i<num_frame;i++){

        //world_parameters.time_axis[i] = i * world_parameters.frame_period / 1000.0;
        world_parameters.aperiodicity[i] = new double [fft_size/2];
        world_parameters.coarse_aperiodicities[i] = new double [bap_dim];
        world_parameters.spectrogram[i] = new double [fft_size / 2];
    }

    #pragma omp parallel for //shared(world_parameters) private(i)
    for (int i=0;i<num_frame;i++){
        double lf0 = 0.0;
        double *mgc = new double[mgc_dim];
        double *bap = new double[bap_dim];
        for (int j=0;j<mgc_dim;j++){ mgc[j] = (double)params[i*params_dim+j];}
        for (int j=0;j<bap_dim;j++){ bap[j] = (double)params[i*params_dim+mgc_dim+1+j];}
        lf0 = (double)params[i*params_dim+mgc_dim];
        HTS_world_par(world_parameters, lf0, bap, mgc, i);
        delete[] mgc;
        delete[] bap;
    }

    //cout<<currentTimeInMilliseconds()-t1<<endl;


    cap2ap(world_parameters);
    //for (int i=0;i<num_frame;i++){
    //    cout<<world_parameters.spectrogram[i][0]<<",";
    //}

    //save_parameters_to_disk(world_parameters, num_frame);

    return;
}

vector<short> StreamVocoder::get_pcm_stream(){
    if (Synthesis2(&synthesizer)==0) return vector<short>();
    vector<short> pcm_part = vector<short>(buffer_size);
    for(int i = 0; i < buffer_size; i++) {
        short p = (short)(MyMaxInt(-32768, MyMinInt(32767, (int)(synthesizer.buffer[i]*32767))));
        pcm_part.at(i) = p;
    }
    return pcm_part;
}

void DestroyParameters(WorldParameters *world_parameters) {
  //delete[] world_parameters->time_axis;
  delete[] world_parameters->f0;
  for (int i = 0; i < world_parameters->f0_length; ++i) {
    delete[] world_parameters->spectrogram[i];
    delete[] world_parameters->aperiodicity[i];
    delete[] world_parameters->coarse_aperiodicities[i];
  }
  delete[] world_parameters->spectrogram;
  delete[] world_parameters->aperiodicity;
  delete[] world_parameters->coarse_aperiodicities;
}

void save_parameters_to_disk(WorldParameters &world_parameters,
                             const int num_frame){
    FILE *f0_file = fopen("f0_stream","w");
    if (f0_file==NULL) return;
    for (int i=0;i<num_frame;i++){
        fprintf(f0_file, "%lf\n", world_parameters.f0[i]);
    }
    fclose(f0_file);

    FILE *ap_file = fopen("ap_stream","w");
    if (ap_file==NULL) return;
    for (int i=0;i<num_frame;i++){
        for (int j=0;j<world_parameters.fft_size/2;j++){
            fprintf(ap_file, "%lf,", world_parameters.aperiodicity[i][j]);
        }
        fprintf(ap_file, "\n");
    }
    fclose(ap_file);

    FILE *sp_file = fopen("sp_stream","w");
    if (sp_file==NULL) return;
    for (int i=0;i<num_frame;i++){
        for (int j=0;j<world_parameters.fft_size/2;j++){
            fprintf(sp_file, "%lf,", world_parameters.spectrogram[i][j]);
        }
        fprintf(sp_file, "\n");
    }
    fclose(sp_file);

}

void HTS_world_par(WorldParameters &world_parameters,
                   double lf0,
                   double *bap,
                   double *mgc,
                   int framenum)
{
    int fft_size = world_parameters.fft_size;

    if(lf0 <= 0.0) world_parameters.f0[framenum] = 0.0;
    else world_parameters.f0[framenum] = exp(lf0);

    for(int i = 0; i < world_parameters.bap_dim; i++){
        if(bap[i] > 0) bap[i] = 0.0;
        world_parameters.coarse_aperiodicities[framenum][i] = bap[i];
    }

    double *x2 = new double[fft_size];
    double *y2 = new double[fft_size];
    mgc2sp(mgc, world_parameters.mgc_dim-1, world_parameters.mgc_alpha, 0.0, x2, y2, fft_size);
    for(int i=0;i<fft_size/2+1;i++){
        world_parameters.spectrogram[framenum][i] = pow(exp(x2[i]) / 32768, 2);
    }
    delete x2;
    delete y2;
}

void cap2ap(WorldParameters &world_parameters){
    double kFrequencyInterval = 3000.0;
    int bap_dim = world_parameters.bap_dim;
    int fft_size = world_parameters.fft_size;
    double sample_rate = world_parameters.fs;
    double coarse_aperiodicity[bap_dim + 2] = { 0 };
    double coarse_frequency_axis[bap_dim + 2] = { 0 };
    double frequency_axis[fft_size / 2 + 1] = { 0 };

    coarse_aperiodicity[0] = -60.0;
    coarse_aperiodicity[bap_dim + 1] = 0.0;

    for(int i = 0; i <= bap_dim; ++i)
    {
                coarse_frequency_axis[i] = (double)(i* kFrequencyInterval);
    }
        coarse_frequency_axis[bap_dim + 1] = sample_rate / 2.0;

    for(int i = 0; i <= fft_size / 2; ++i)
    {
                frequency_axis[i] = i*((double)sample_rate / fft_size);
    }

    //load band ap values for this frame into coarse_aperiodicity
    //omp_set_num_threads(2);
    //#pragma omp parallel for
    for(int i = 0; i < world_parameters.f0_length; i++)
        {
        for(int k = 0; k < bap_dim; ++k)
        {
                        coarse_aperiodicity[k + 1] = world_parameters.coarse_aperiodicities[i][k];
                }
        interp1(coarse_frequency_axis, coarse_aperiodicity, bap_dim + 2, frequency_axis, fft_size / 2 + 1, world_parameters.aperiodicity[i]);

        for(int j = 0; j <= fft_size / 2; ++j)
        {
             world_parameters.aperiodicity[i][j] = pow(10.0, world_parameters.aperiodicity[i][j] / 20.0);
        }
    }

}


int main(){
    int num_frame = 620;
    int mgc_dim = 60;
    int bap_dim = 1;
    int param_dim = mgc_dim + bap_dim +1;
    int fft_size = 512;
    int sample_rate = 16000;
    int buffer_size = 64;
    double mgc_alpha = 0.58;
    size_t* params_ = new size_t[num_frame*param_dim];
    FILE *file=fopen("./trash/params.bin","wb");
    fread(params_, sizeof(size_t)*num_frame*param_dim, 1, file);
    fclose(file);
    vector<double> params = vector<double>(num_frame*param_dim);
    for (int i=0;i<num_frame*param_dim;i++){
        params.at(i) = (double)params_[i];
    }
    vector<short> pcm;

    for (int i=0;i<1;i++){
        StreamVocoder *mVocoder = new StreamVocoder(params, mgc_dim, bap_dim, num_frame, sample_rate, fft_size, buffer_size, mgc_alpha, 4);
        while (1){
            vector<short> pcm_part = mVocoder->get_pcm_stream();
            if (pcm_part.size()!=buffer_size) break;
            pcm.insert(end(pcm), begin(pcm_part), end(pcm_part));
        }
        delete mVocoder;
    }
    cout<<"pcm length : "<<pcm.size()<<endl;
    cout<<"pcm length should be near : "<<num_frame * 5 / 1000.0 * sample_rate<<endl;
    delete[] params_;
    return 1;
}


