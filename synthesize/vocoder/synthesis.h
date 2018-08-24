//-----------------------------------------------------------------------------
// Copyright 2012-2016 Masanori Morise. All Rights Reserved.
// Author: mmorise [at] yamanashi.ac.jp (Masanori Morise)
//-----------------------------------------------------------------------------
#define K16000

#ifdef K16000
    #define NUMBER_OF_AP				(1)         //try 1, to be 5, usually 5-20
    #define FFT_SIZE					(512)
    #define FRAMEPERIOD					(5.0)
    #define SAMPLERATE					(16000)
    #define FRAME_LENGTH				(80+1)
#else
    #ifdef K24000
        #define NUMBER_OF_AP				(3)
        #define FFT_SIZE					(1024)
        #define FRAMEPERIOD					(5.0)
        #define SAMPLERATE					(24000)
        #define FRAME_LENGTH				(120+1)
    #else
        #ifdef K48000
            #define NUMBER_OF_AP				(5)
            #define FFT_SIZE					(2048)
            #define FRAMEPERIOD					(5.0)
            #define SAMPLERATE					(48000)
            #define FRAME_LENGTH				(240+1)
        #else
            #define NUMBER_OF_AP				(0)
            #define FFT_SIZE					(256)
            #define FRAMEPERIOD					(5.0)
            #define SAMPLERATE					(8000)
            #define FRAME_LENGTH				(40+1)
        #endif
    #endif
#endif

//-----------------------------------------------------------------------------
// Synthesis() synthesize the voice based on f0, spectrogram and
// aperiodicity (not excitation signal).
//
// Input:
//   f0                   : f0 contour
//   f0_length            : Length of f0
//   spectrogram          : Spectrogram estimated by CheapTrick
//   fft_size             : FFT size
//   aperiodicity         : Aperiodicity spectrogram based on D4C
//   frame_period         : Temporal period used for the analysis
//   fs                   : Sampling frequency
//   y_length             : Length of the output signal (Memory of y has been
//                          allocated in advance)
// Output:
//   y                    : Calculated speech
//-----------------------------------------------------------------------------
void Synthesis(const double *f0,
               int f0_length,
               double **const spectrogram,
               double **const aperiodicity,
               int fft_size,
               double frame_period,
               int fs,
               int y_length,
               double *y);

