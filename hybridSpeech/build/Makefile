INCLUDE += -I/home/brycezou/program/jdk1.8.0_131/include
INCLUDE += -I/home/brycezou/program/jdk1.8.0_131/include/linux

SOURCES += ../src/*.c
SOURCES += ../src/vocoder/*.c
SOURCES += ../main.cpp

LIBS += /home/brycezou/program/jdk1.8.0_131/jre/lib/amd64/server/libjvm.so


XXFLAGS += -Wall -W -std=c++11 -lm -lstdc++

CXX = gcc


ttsdemo : 
	$(CXX) $(INCLUDE) $(SOURCES) -o ttsdemo $(XXFLAGS) $(LIBS)


clean :
	rm -f ttsdemo 

.PHONY : clean


