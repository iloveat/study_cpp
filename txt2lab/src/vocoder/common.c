//-----------------------------------------------------------------------------
// Copyright 2012-2016 Masanori Morise. All Rights Reserved.
// Author: mmorise [at] yamanashi.ac.jp (Masanori Morise)
//
// common.cpp includes functions used in at least two files.
// (1) Common functions
// (2) FFT, IFFT and minimum phase analysis.
//
// In FFT analysis and minimum phase analysis,
// Functions "Initialize*()" allocate the mamory.
// Functions "Destroy*()" free the accolated memory.
// FFT size is used for initialization, and structs are used to keep the memory.
// Functions "GetMinimumPhaseSpectrum()" calculate minimum phase spectrum.
// Forward and inverse FFT do not have the function "Get*()",
// because forward FFT and inverse FFT can run in one step.
//
//-----------------------------------------------------------------------------
#include <string.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include "common.h"
#include "consts.h"
#include "matlabfuns.h"
#include "synthesis.h"

static void SetParametersForLinearSmoothing(int boundary,
                                            int fft_size,
                                            int fs,
                                            double width,
                                            const double *power_spectrum,
                                            double *mirroring_spectrum,
                                            double *mirroring_segment,
                                            double *frequency_axis)
{
    for (int i = 0; i < boundary; ++i)
        mirroring_spectrum[i] = power_spectrum[boundary - i];
    for (int i = boundary; i < fft_size / 2 + boundary; ++i)
        mirroring_spectrum[i] = power_spectrum[i - boundary];
    for (int i = fft_size / 2 + boundary; i <= fft_size / 2 + boundary * 2; ++i)
        mirroring_spectrum[i] = power_spectrum[fft_size / 2 - (i - (fft_size / 2 + boundary))];

    mirroring_segment[0] = mirroring_spectrum[0] * fs / fft_size;

    for (int i = 1; i < fft_size / 2 + boundary * 2 + 1; ++i)
        mirroring_segment[i] = mirroring_spectrum[i] * fs / fft_size + mirroring_segment[i - 1];

    for (int i = 0; i <= fft_size / 2; ++i)
        frequency_axis[i] =((double)(i) / (double)fft_size) * fs - width / 2.0;
}

int GetSuitableFFTSize(int sample)
{
    return (int)(pow(2.0, (int)(log((double)(sample)) / kLog2) + 1.0));
}

void DCCorrection(const double *input, double f0, int fs, int fft_size, double *output)
{
    int upper_limit = 2 + (int)(f0 * fft_size / fs);
    double *low_frequency_replica = NULL;
    double *low_frequency_axis = NULL;

    low_frequency_replica = (double *)malloc(upper_limit*sizeof(double));
    low_frequency_axis = (double *)malloc(upper_limit*sizeof(double));

    for (int i = 0; i < upper_limit; ++i)
        low_frequency_axis[i] = (double)(i) * fs / fft_size;

    int upper_limit_replica = upper_limit - 1;
    interp1Q(f0 - low_frequency_axis[0], -(double)(fs) / fft_size, input,
            upper_limit + 1, low_frequency_axis, upper_limit_replica, low_frequency_replica);

    for (int i = 0; i < upper_limit_replica; ++i)
        output[i] = input[i] + low_frequency_replica[i];

    if(low_frequency_replica)
    {
        free(low_frequency_replica);
        low_frequency_replica = NULL;
    }
    if(low_frequency_axis)
    {
        free(low_frequency_axis);
        low_frequency_axis = NULL;
    }
}

void LinearSmoothing(const double *input, double width, int fs, int fft_size, double *output)
{
    int boundary = (int)(width * fft_size / fs) + 1;

    double *mirroring_spectrum = NULL;
    double *mirroring_segment = NULL;
    double frequency_axis[FFT_SIZE / 2 + 1] = { 0 };
    mirroring_spectrum = (double *)malloc((fft_size / 2 + boundary * 2 + 1)*sizeof(double));
    mirroring_segment = (double *)malloc((fft_size / 2 + boundary * 2 + 1)*sizeof(double));

    SetParametersForLinearSmoothing(boundary, fft_size, fs, width, input,
                                    mirroring_spectrum, mirroring_segment, frequency_axis);

    double low_levels[ FFT_SIZE/ 2 + 1] = { 0 };
    double high_levels[FFT_SIZE / 2 + 1] = { 0 };
    double origin_of_mirroring_axis = -(boundary - 0.5) * fs / fft_size;
    double discrete_frequency_interval = (double)(fs) / fft_size;

    interp1Q(origin_of_mirroring_axis, discrete_frequency_interval, mirroring_segment,
             fft_size / 2 + boundary * 2 + 1, frequency_axis, fft_size / 2 + 1, low_levels);

    for (int i = 0; i <= fft_size / 2; ++i)
        frequency_axis[i] += width;

    interp1Q(origin_of_mirroring_axis, discrete_frequency_interval, mirroring_segment,
             fft_size / 2 + boundary * 2 + 1, frequency_axis, fft_size / 2 + 1, high_levels);

    for (int i = 0; i <= fft_size / 2; ++i)
        output[i] = (high_levels[i] - low_levels[i]) / width;

    if(mirroring_spectrum)
    {
        free(mirroring_spectrum);
        mirroring_spectrum = NULL;
    }
    if(mirroring_segment)
    {
        free(mirroring_segment);
        mirroring_segment = NULL;
    }
}

void NuttallWindow(int y_length, double *y)
{
    double tmp;
    for (int i = 0; i < y_length; ++i)
    {
        tmp  = i / (y_length - 1.0);
        y[i] = 0.355768 - 0.487396 * cos(2.0 * kPi * tmp) + 0.144232 * cos(4.0 * kPi * tmp) - 0.012604 * cos(6.0 * kPi * tmp);
    }
}

// FFT, IFFT and minimum phase analysis
void InitializeForwardRealFFT(int fft_size, ForwardRealFFT *forward_real_fft)
{
    forward_real_fft->waveform = (double *)malloc(FFT_SIZE*sizeof(double));
	forward_real_fft->spectrum = (fft_complex *)malloc(FFT_SIZE*sizeof(fft_complex));
    forward_real_fft->fft_size = fft_size;
    forward_real_fft->forward_fft = fft_plan_dft_r2c_1d(fft_size,
    forward_real_fft->waveform, forward_real_fft->spectrum, FFT_ESTIMATE);
}

void DestroyForwardRealFFT(ForwardRealFFT *forward_real_fft)
{
    if(forward_real_fft)
    {
        fft_destroy_plan(forward_real_fft->forward_fft);
        if(forward_real_fft->spectrum)
        {
            free(forward_real_fft->spectrum);
            forward_real_fft->spectrum = NULL;
        }
        if(forward_real_fft->waveform)
        {
            free(forward_real_fft->waveform);
            forward_real_fft->waveform = NULL;
        }

        forward_real_fft = NULL;
    }
}

void InitializeInverseRealFFT(int fft_size, InverseRealFFT *inverse_real_fft)
{
    inverse_real_fft->fft_size = fft_size;
    inverse_real_fft->waveform = (double *)malloc(FFT_SIZE*sizeof(double));
    inverse_real_fft->spectrum = (fft_complex *)malloc(FFT_SIZE*sizeof(fft_complex));
    inverse_real_fft->inverse_fft = fft_plan_dft_c2r_1d(fft_size,
    inverse_real_fft->spectrum, inverse_real_fft->waveform, FFT_ESTIMATE);
}

void DestroyInverseRealFFT(InverseRealFFT *inverse_real_fft)
{
    if(inverse_real_fft)
    {
        fft_destroy_plan(inverse_real_fft->inverse_fft);
        if(inverse_real_fft->spectrum)
        {
            free(inverse_real_fft->spectrum);
            inverse_real_fft->spectrum = NULL;
        }
        if(inverse_real_fft->waveform)
        {
            free(inverse_real_fft->waveform);
            inverse_real_fft->waveform = NULL;
        }

        inverse_real_fft = NULL;
    }
}

void InitializeMinimumPhaseAnalysis(int fft_size, MinimumPhaseAnalysis *minimum_phase)
{
    minimum_phase->log_spectrum = (double *)malloc(FFT_SIZE*sizeof(double));
    minimum_phase->minimum_phase_spectrum = (fft_complex *)malloc(FFT_SIZE*sizeof(fft_complex));
    minimum_phase->cepstrum = (fft_complex *)malloc(FFT_SIZE*sizeof(fft_complex));
    minimum_phase->fft_size = fft_size;

    minimum_phase->inverse_fft = fft_plan_dft_r2c_1d(fft_size, minimum_phase->log_spectrum, minimum_phase->cepstrum, FFT_ESTIMATE);
    minimum_phase->forward_fft = fft_plan_dft_1d(fft_size, minimum_phase->cepstrum, minimum_phase->minimum_phase_spectrum, FFT_FORWARD, FFT_ESTIMATE);
}

void GetMinimumPhaseSpectrum(const MinimumPhaseAnalysis *minimum_phase)
{
    // Mirroring
    for (int i = minimum_phase->fft_size / 2 + 1; i < minimum_phase->fft_size; ++i)
        minimum_phase->log_spectrum[i] = minimum_phase->log_spectrum[minimum_phase->fft_size - i];

    // This fft_plan carries out "forward" FFT.
    // To carriy out the Inverse FFT, the sign of imaginary part
    // is inverted after FFT.
    fft_execute(minimum_phase->inverse_fft);
    minimum_phase->cepstrum[0][1] *= -1.0;
    for (int i = 1; i < minimum_phase->fft_size / 2; ++i)
    {
        minimum_phase->cepstrum[i][0] *= 2.0;
        minimum_phase->cepstrum[i][1] *= -2.0;
    }

    minimum_phase->cepstrum[minimum_phase->fft_size / 2][1] *= -1.0;

    for (int i = minimum_phase->fft_size / 2 + 1; i < minimum_phase->fft_size; ++i)
    {
        minimum_phase->cepstrum[i][0] = 0.0;
        minimum_phase->cepstrum[i][1] = 0.0;
    }

    fft_execute(minimum_phase->forward_fft);

    // Since x is complex number, calculation of exp(x) is as following.
    // Note: This FFT library does not keep the aliasing.
    double tmp;
    for (int i = 0; i <= minimum_phase->fft_size / 2; ++i)
    {
        tmp = exp(minimum_phase->minimum_phase_spectrum[i][0] / minimum_phase->fft_size);
        minimum_phase->minimum_phase_spectrum[i][0] = tmp * cos(minimum_phase->minimum_phase_spectrum[i][1] / minimum_phase->fft_size);
        minimum_phase->minimum_phase_spectrum[i][1] = tmp * sin(minimum_phase->minimum_phase_spectrum[i][1] / minimum_phase->fft_size);
    }
}

void DestroyMinimumPhaseAnalysis(MinimumPhaseAnalysis *minimum_phase)
{
    if(minimum_phase)
    {
        fft_destroy_plan(minimum_phase->forward_fft);
        fft_destroy_plan(minimum_phase->inverse_fft);

        if(minimum_phase->cepstrum)
        {
            free(minimum_phase->cepstrum);
            minimum_phase->cepstrum = NULL;
        }

        if(minimum_phase->log_spectrum)
        {
            free(minimum_phase->log_spectrum);
            minimum_phase->log_spectrum = NULL;
        }

        if(minimum_phase->minimum_phase_spectrum)
        {
            free(minimum_phase->minimum_phase_spectrum);
            minimum_phase->minimum_phase_spectrum = NULL;
        }

        minimum_phase = NULL;
    }
}
