SOURCES += ../../world-master/src/common.cpp
SOURCES += ../../world-master/src/fft.cpp
SOURCES += ../../world-master/src/matlabfunctions.cpp
SOURCES += ../../world-master/src/synthesisrealtime.cpp
SOURCES += ../../world-master/src/sptk.cpp
SOURCES += ./world_stream.cpp


all: 
	g++ -fPIC -o test $(SOURCES) --std=c++11 -DDEBUG -g -fopenmp

.PHONY : clean

clean : 
	rm -f test









