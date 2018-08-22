# include dlib 
INCLUDES  = -I/usr/local/include
# include opencv
INCLUDES += `pkg-config opencv --cflags`
# include mxnet 
INCLUDES += -I/home/brycezou/program/mxnet-bc6e4b4cbc4e3e4f361acb5d7cebb60ffa36ec50/include

# link opencv 
LIBS  = `pkg-config opencv --libs`
# link mxnet 
LIBS += /home/brycezou/program/mxnet-bc6e4b4cbc4e3e4f361acb5d7cebb60ffa36ec50/lib/libmxnet.so
# link dlib 
LIBS += /usr/local/lib/libdlib.so
LIBS += -lblas -llapack


CXX = g++
CFLAGS = -Wall -std=c++11 $(INCLUDES)
LDFLAGS = $(LIBS)

SOURCES  = aligner/*.cpp
SOURCES += detector/*.cpp
SOURCES += predictor/*.cpp
SOURCES += emo_face.cpp


face_demo : 
	$(CXX) -o face_demo $(SOURCES) $(CFLAGS) $(LDFLAGS)



.PHONY : clean

clean :
	rm -f *.o face_demo *.jpg
