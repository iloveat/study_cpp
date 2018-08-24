//-----------------------------------------------------------------------------
// Copyright 2012-2016 Masanori Morise. All Rights Reserved.
// Author: mmorise [at] yamanashi.ac.jp (Masanori Morise)
//
// Voice synthesis based on f0, spectrogram and aperiodicity.
// forward_real_fft, inverse_real_fft and minimum_phase are used to speed up.
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include "common.h"
#include "consts.h"
#include "matlabfuns.h"
#include "synthesis.h"

static void GetNoiseSpectrum(int noise_size,
                             int fft_size,
                             const ForwardRealFFT *forward_real_fft)
{
    double average = 0.0;

    for (int i = 0; i < noise_size; ++i)
    {
        forward_real_fft->waveform[i] = randn();
        average += forward_real_fft->waveform[i];
    }

    average /= noise_size;

    for (int i = 0; i < noise_size; ++i)
    {
        forward_real_fft->waveform[i] -= average;
    }

    for (int i = noise_size; i < fft_size; ++i)
    {
        forward_real_fft->waveform[i] = 0.0;
    }

    fft_execute(forward_real_fft->forward_fft);
}

//-----------------------------------------------------------------------------
// GetAperiodicResponse() calculates an aperiodic response.
//-----------------------------------------------------------------------------
static void GetAperiodicResponse(int noise_size,
                                 int fft_size,
                                 const double *spectrum,
                                 const double *aperiodic_ratio,
                                 double current_vuv,
                                 const ForwardRealFFT *forward_real_fft,
                                 const InverseRealFFT *inverse_real_fft,
                                 const MinimumPhaseAnalysis *minimum_phase,
                                 double *aperiodic_response)
{
    GetNoiseSpectrum(noise_size, fft_size, forward_real_fft);

    if (current_vuv != 0.0)
    {
        for (int i = 0; i <= minimum_phase->fft_size / 2; ++i)
        {
            minimum_phase->log_spectrum[i] = log(spectrum[i] * aperiodic_ratio[i]) / 2.0;
        }
    }
    else
    {
        for (int i = 0; i <= minimum_phase->fft_size / 2; ++i)
        {
            minimum_phase->log_spectrum[i] = log(spectrum[i]) / 2.0;
        }
    }

    GetMinimumPhaseSpectrum(minimum_phase);

    for (int i = 0; i <= fft_size / 2; ++i)
    {
        inverse_real_fft->spectrum[i][0] = minimum_phase->minimum_phase_spectrum[i][0];
        inverse_real_fft->spectrum[i][1] = minimum_phase->minimum_phase_spectrum[i][1];

        inverse_real_fft->spectrum[i][0] = minimum_phase->minimum_phase_spectrum[i][0] * forward_real_fft->spectrum[i][0]
                - minimum_phase->minimum_phase_spectrum[i][1] * forward_real_fft->spectrum[i][1];
        inverse_real_fft->spectrum[i][1] = minimum_phase->minimum_phase_spectrum[i][0] * forward_real_fft->spectrum[i][1]
                + minimum_phase->minimum_phase_spectrum[i][1] * forward_real_fft->spectrum[i][0];
    }

    fft_execute(inverse_real_fft->inverse_fft);
    fftshift(inverse_real_fft->waveform, fft_size, aperiodic_response);
}

//-----------------------------------------------------------------------------
// GetPeriodicResponse() calculates an periodic response.
//-----------------------------------------------------------------------------
static void GetPeriodicResponse(int fft_size,
                                const double *spectrum,
                                const double *aperiodic_ratio,
                                double current_vuv,
                                const InverseRealFFT *inverse_real_fft,
                                const MinimumPhaseAnalysis *minimum_phase,
                                double *periodic_response)
{
    if (current_vuv <= 0.5 || aperiodic_ratio[0] > 0.999)
    {
        for (int i = 0; i < fft_size; ++i)
        {
            periodic_response[i] = 0.0;
        }
        return;
    }

    for (int i = 0; i <= minimum_phase->fft_size / 2; ++i)
    {
        minimum_phase->log_spectrum[i] = log(spectrum[i] * (1.0 - aperiodic_ratio[i]) + kMySafeGuardMinimum) / 2.0;
    }

    GetMinimumPhaseSpectrum(minimum_phase);

    for (int i = 0; i <= fft_size / 2; ++i)
    {
        inverse_real_fft->spectrum[i][0] = minimum_phase->minimum_phase_spectrum[i][0];
        inverse_real_fft->spectrum[i][1] = minimum_phase->minimum_phase_spectrum[i][1];
    }

    fft_execute(inverse_real_fft->inverse_fft);
    fftshift(inverse_real_fft->waveform, fft_size, periodic_response);
}

static void GetSpectralEnvelope(double current_time,
                                double frame_period,
                                int f0_length,
                                double **const spectrogram,
                                int fft_size,
                                double *spectral_envelope)
{
    int current_frame_floor = MyMinInt(f0_length - 1, (int)(floor(current_time / frame_period)));
    int current_frame_ceil = MyMinInt(f0_length - 1, (int)(ceil(current_time / frame_period)));
    double interpolation = current_time / frame_period - current_frame_floor;

    if (current_frame_floor == current_frame_ceil)
    {
        for (int i = 0; i <= fft_size / 2; ++i)
        {
            spectral_envelope[i] = fabs(spectrogram[current_frame_floor][i]);
        }
    }
    else
    {
        for (int i = 0; i <= fft_size / 2; ++i)
        {
            spectral_envelope[i] = (1.0 - interpolation) * fabs(spectrogram[current_frame_floor][i])
                    + interpolation * fabs(spectrogram[current_frame_ceil][i]);
        }
    }
}

static void GetAperiodicRatio(double current_time,
                              double frame_period,
                              int f0_length,
                              double **const aperiodicity,
                              int fft_size,
                              double *aperiodic_spectrum)
{
    int current_frame_floor = MyMinInt(f0_length - 1, (int)(floor(current_time / frame_period)));
    int current_frame_ceil = MyMinInt(f0_length - 1, (int)(ceil(current_time / frame_period)));
    double interpolation = current_time / frame_period - current_frame_floor;

    if (current_frame_floor == current_frame_ceil)
    {
        for (int i = 0; i <= fft_size / 2; ++i)
        {
            aperiodic_spectrum[i] = pow(GetSafeAperiodicity(aperiodicity[current_frame_floor][i]), 2.0);
        }
    }
    else
    {
        for (int i = 0; i <= fft_size / 2; ++i)
        {
            aperiodic_spectrum[i] = pow((1.0 - interpolation) * GetSafeAperiodicity(aperiodicity[current_frame_floor][i])
                                        + interpolation * GetSafeAperiodicity(aperiodicity[current_frame_ceil][i]), 2.0);
        }
    }
}

//-----------------------------------------------------------------------------
// GetOneFrameSegment() calculates a periodic and aperiodic response at a time.
//-----------------------------------------------------------------------------
static void GetOneFrameSegment(double current_vuv,
                               int noise_size,
                               double **const spectrogram,
                               int fft_size,
                               double **const aperiodicity,
                               int f0_length,
                               double frame_period,
                               double current_time,
                               int fs,
                               const ForwardRealFFT *forward_real_fft,
                               const InverseRealFFT *inverse_real_fft,
                               const MinimumPhaseAnalysis *minimum_phase,
                               double *response)
{
    double *aperiodic_response = NULL;
    double *periodic_response = NULL;
    double *spectral_envelope = NULL;
    double *aperiodic_ratio = NULL;

    aperiodic_response = (double *)malloc(sizeof(double)*fft_size);
    periodic_response = (double *)malloc(sizeof(double)*fft_size);
    spectral_envelope = (double *)malloc(sizeof(double)*fft_size);
    aperiodic_ratio = (double *)malloc(sizeof(double)*fft_size);

    GetSpectralEnvelope(current_time, frame_period, f0_length, spectrogram, fft_size, spectral_envelope);
    GetAperiodicRatio(current_time, frame_period, f0_length, aperiodicity, fft_size, aperiodic_ratio);

    // Synthesis of the periodic response
    GetPeriodicResponse(fft_size, spectral_envelope, aperiodic_ratio, current_vuv, inverse_real_fft, minimum_phase, periodic_response);

    // Synthesis of the aperiodic response
    GetAperiodicResponse(noise_size, fft_size, spectral_envelope, aperiodic_ratio, current_vuv, forward_real_fft, inverse_real_fft, minimum_phase, aperiodic_response);

    double sqrt_noise_size = sqrt((double)(noise_size));

    for (int i = 0; i < fft_size; ++i)
    {
        response[i] = (periodic_response[i] * sqrt_noise_size + aperiodic_response[i]) / fft_size;
    }

    if(spectral_envelope)
    {
        free(spectral_envelope);
        spectral_envelope = NULL;
    }
    if(aperiodic_ratio)
    {
        free(aperiodic_ratio);
        aperiodic_ratio = NULL;
    }
    if(periodic_response)
    {
        free(periodic_response);
        periodic_response = NULL;
    }
    if(aperiodic_response)
    {
        free(aperiodic_response);
        aperiodic_response = NULL;
    }
}

static void GetTemporalParametersForTimeBase(const double *f0,
                                             int f0_length,
                                             int fs,
                                             int y_length,
                                             double frame_period,
                                             double *time_axis,
                                             double *coarse_time_axis,
                                             double *coarse_f0,
                                             double *coarse_vuv)
{
    for (int i = 0; i < y_length; ++i)
        time_axis[i] = i / (double)(fs);
    for (int i = 0; i < f0_length; ++i)
        coarse_time_axis[i] = i * frame_period;
    for (int i = 0; i < f0_length; ++i)
        coarse_f0[i] = f0[i];

    coarse_f0[f0_length] = coarse_f0[f0_length - 1] * 2 - coarse_f0[f0_length - 2];

    for (int i = 0; i < f0_length; ++i)
        coarse_vuv[i] = f0[i] == 0.0 ? 0.0 : 1.0;

    coarse_vuv[f0_length] = coarse_vuv[f0_length - 1] * 2 - coarse_vuv[f0_length - 2];
}

static int GetPulseLocationsForTimeBase(const double *interpolated_f0,
                                        const double *time_axis,
                                        int y_length,
                                        int fs,
                                        double *pulse_locations,
                                        int *pulse_locations_index)
{
    double *total_phase = NULL;
    double *wrap_phase = NULL;
    double *wrap_phase_abs = NULL;

    total_phase = (double *)malloc(sizeof(double)*y_length);
    wrap_phase = (double *)malloc(sizeof(double)*y_length);
    wrap_phase_abs = (double *)malloc(sizeof(double)*y_length);

    total_phase[0] = 2.0 *  kPi * interpolated_f0[0] / fs;
    for (int i = 1; i < y_length; ++i)
        total_phase[i] = total_phase[i - 1] + 2.0 *  kPi * interpolated_f0[i] / fs;

    for (int i = 0; i < y_length; ++i)
        wrap_phase[i] = fmod(total_phase[i], 2.0 *  kPi);

    for (int i = 0; i < y_length - 1; ++i)
        wrap_phase_abs[i] = fabs(wrap_phase[i + 1] - wrap_phase[i]);

    int number_of_pulses = 0;
    for (int i = 0; i < y_length - 1; ++i)
    {
        if (wrap_phase_abs[i] >  kPi)
        {
            pulse_locations[number_of_pulses] = time_axis[i];
            pulse_locations_index[number_of_pulses] = (int)(matlab_round(pulse_locations[number_of_pulses] * fs));
            ++number_of_pulses;
        }
    }

    if(wrap_phase_abs)
    {
        free(wrap_phase_abs);
        wrap_phase_abs = NULL;
    }
    if(wrap_phase)
    {
        free(wrap_phase);
        wrap_phase = NULL;
    }
    if(total_phase)
    {
        free(total_phase);
        total_phase = NULL;
    }
    return number_of_pulses;
}

static int GetTimeBase(const double *f0,
                       int f0_length,
                       int fs,
                       double frame_period,
                       int y_length,
                       double *pulse_locations,
                       int *pulse_locations_index,
                       double *interpolated_vuv)
{
    double *time_axis = NULL;
    double *coarse_time_axis = NULL;
    double *coarse_f0 = NULL;
    double *coarse_vuv = NULL;
    double *interpolated_f0 = NULL;

    time_axis = (double *)malloc(sizeof(double)*y_length);
    coarse_time_axis = (double *)malloc(sizeof(double)*(f0_length + 1));
    coarse_f0 = (double *)malloc(sizeof(double)*(f0_length + 1));
    coarse_vuv = (double *)malloc(sizeof(double)*(f0_length + 1));
    interpolated_f0 = (double *)malloc(sizeof(double)*y_length);

    GetTemporalParametersForTimeBase(f0, f0_length, fs, y_length, frame_period, time_axis, coarse_time_axis, coarse_f0, coarse_vuv);

    interp1(coarse_time_axis, coarse_f0, f0_length + 1, time_axis, y_length, interpolated_f0);
    interp1(coarse_time_axis, coarse_vuv, f0_length + 1, time_axis, y_length, interpolated_vuv);
    for (int i = 0; i < y_length; ++i)
    {
        interpolated_vuv[i] = interpolated_vuv[i] > 0.5 ? 1.0 : 0.0;
    }

    for (int i = 0; i < y_length; ++i)
    {
        interpolated_f0[i] = interpolated_vuv[i] == 0.0 ?  kDefaultF0 : interpolated_f0[i];
    }

    int number_of_pulses = GetPulseLocationsForTimeBase(interpolated_f0, time_axis, y_length, fs, pulse_locations, pulse_locations_index);

    if(coarse_vuv)
    {
        free(coarse_vuv);
        coarse_vuv = NULL;
    }
    if(coarse_f0)
    {
        free(coarse_f0);
        coarse_f0 = NULL;
    }
    if(coarse_time_axis)
    {
        free(coarse_time_axis);
        coarse_time_axis = NULL;
    }
    if(time_axis)
    {
        free(time_axis);
        time_axis = NULL;
    }
    if(interpolated_f0)
    {
        free(interpolated_f0);
        interpolated_f0 = NULL;
    }
    return number_of_pulses;
}

void Synthesis(const double *f0,
               int f0_length,
               double **const spectrogram,
               double **const aperiodicity,
               int fft_size,
               double frame_period,
               int fs,
               int y_length,
               double *y)
{
    double *impulse_response = NULL;
    double *pulse_locations = NULL;
    int *pulse_locations_index = NULL;
    double *interpolated_vuv = NULL;

    impulse_response = (double *)malloc(sizeof(double)*fft_size);
    pulse_locations = (double *)malloc(sizeof(double)*y_length);
    pulse_locations_index = (int *)malloc(sizeof(int)*y_length);
    interpolated_vuv = (double *)malloc(sizeof(double)*y_length);

    for (int i = 0; i < y_length; ++i) y[i] = 0.0;

    MinimumPhaseAnalysis minimum_phase = {0};
    InitializeMinimumPhaseAnalysis(fft_size, &minimum_phase);
    InverseRealFFT inverse_real_fft = {0};
    InitializeInverseRealFFT(fft_size, &inverse_real_fft);
    ForwardRealFFT forward_real_fft = {0};
    InitializeForwardRealFFT(fft_size, &forward_real_fft);

    int number_of_pulses = GetTimeBase(f0, f0_length, fs, frame_period / 1000.0, y_length, pulse_locations, pulse_locations_index, interpolated_vuv);

    frame_period /= 1000.0;
    int noise_size;

    for (int i = 0; i < number_of_pulses; ++i)
    {
        noise_size = pulse_locations_index[MyMinInt(number_of_pulses - 1, i + 1)] - pulse_locations_index[i];
        GetOneFrameSegment(interpolated_vuv[pulse_locations_index[i]], noise_size, spectrogram,
                fft_size, aperiodicity, f0_length, frame_period, pulse_locations[i], fs,
                &forward_real_fft, &inverse_real_fft, &minimum_phase, impulse_response);

        int safe_index = 0;

        for (int j = 0; j < fft_size; ++j)
        {
            safe_index = MyMinInt(y_length - 1,
            MyMaxInt(0, j + pulse_locations_index[i] - fft_size / 2 + 1));
            y[safe_index] += impulse_response[j];
        }
    }

    DestroyMinimumPhaseAnalysis(&minimum_phase);
    DestroyInverseRealFFT(&inverse_real_fft);
    DestroyForwardRealFFT(&forward_real_fft);

    if(impulse_response)
    {
        free(impulse_response);
        impulse_response = NULL;
    }
    if(pulse_locations)
    {
        free(pulse_locations);
        pulse_locations = NULL;
    }
    if(pulse_locations_index)
    {
        free(pulse_locations_index);
        pulse_locations_index = NULL;
    }
    if(interpolated_vuv)
    {
        free(interpolated_vuv);
        interpolated_vuv = NULL;
    }
}
