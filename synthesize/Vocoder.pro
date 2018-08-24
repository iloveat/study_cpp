TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += vocoder/consts.h
SOURCES += vocoder/common.c
SOURCES += vocoder/common.h
SOURCES += vocoder/fft.c
SOURCES += vocoder/fft.h
SOURCES += vocoder/matlabfuns.c
SOURCES += vocoder/matlabfuns.h
SOURCES += vocoder/sptk.c
SOURCES += vocoder/sptk.h
SOURCES += vocoder/synthesis.c
SOURCES += vocoder/synthesis.h
SOURCES += vocoder/world.c
SOURCES += vocoder/world.h


SOURCES += main.cpp


