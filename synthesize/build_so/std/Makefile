SOURCES += ../../vocoder/common.c
SOURCES += ../../vocoder/matlabfuns.c
SOURCES += ../../vocoder/fft.c
SOURCES += ../../vocoder/sptk.c
SOURCES += ../../vocoder/synthesis.c
SOURCES += ../../vocoder/world.c
SOURCES += interface.cpp


libvocoder.so :
	g++ -shared -fPIC -o libvocoder.so $(SOURCES) --std=c++11


.PHONY : clean

clean :
	rm -f libvocoder.so
