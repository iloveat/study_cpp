# coding:utf-8

import re, codecs
import os, shutil
import librosa, wave
import numpy as np
import audioop
import soundfile as sf

def downsampleWav(src, dst, inrate, outrate, inchannels, outchannels):
    # if not os.path.exists(src):
    #     print ('Source not found!')
    #     return False
    #
    # if not os.path.exists(os.path.dirname(dst)):
    #     os.makedirs(os.path.dirname(dst))

    try:
        s_read = wave.open(src, 'r')
        s_write = wave.open(dst, 'wb')
    except:
        print('Failed to open files!')
        return False

    n_frames = s_read.getnframes()
    data = s_read.readframes(n_frames)

    try:
        converted,newstate = audioop.ratecv(data, 2, inchannels, inrate, outrate, None)
        if outchannels == 1 and inchannels == 2:
            converted = audioop.tomono(converted[0], 2, 1, 0)
    except:
        print('Failed to downsample wav')
        return False

    try:
        s_write.setparams((outchannels, 2, outrate, 0, 'NONE', 'Uncompressed'))
        converted = np.fromstring(converted, dtype=np.short)
        s_write.writeframes(converted)
    except:
        print('Failed to write wav')
        return False

    try:
        s_read.close()
        s_write.close()
    except:
        print('Failed to close wav files')

def wavresamplewithsf(wav_48k, wav_16k, resamplewavrate): #这一版音质更好，做了平滑处理
    y, sr = librosa.load(wav_48k, sr=None)
    # print(sr)
    # if y.shape[0] == 2:
    #     y = librosa.to_mono(y)
    if sr == 16000:
        y_16k = librosa.resample(y, 16000, resamplewavrate)

    # librosa.output.write_wav('1013000new1.wav', y_16k, 16000)
        sf.write(wav_16k, y_16k, resamplewavrate, 'PCM_16')
    else:
        print(wav_48k)

if __name__ == '__main__':
    resampleratename = '8k'
    resamplewavrate = int(resampleratename.replace('k','000'))
    if not os.path.exists(resampleratename):
        os.mkdir(resampleratename)

    for wav in os.listdir('16k'):
        wav_src = './16k/' + wav
        wav_dst = './' + resampleratename + '/' + wav
        wavresamplewithsf(wav_src, wav_dst, resamplewavrate)
        print(wav)
















