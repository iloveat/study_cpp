TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += C++11

INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/include/opencv
INCLUDEPATH += /usr/local/include/opencv2

LIBS += /usr/local/lib/libopencv_legacy.so
LIBS += /usr/local/lib/libopencv_videostab.so
LIBS += /usr/local/lib/libopencv_stitching.so
LIBS += /usr/local/lib/libopencv_superres.so
LIBS += /usr/local/lib/libopencv_contrib.so
LIBS += /usr/local/lib/libopencv_nonfree.so
LIBS += /usr/local/lib/libopencv_ocl.so
LIBS += /usr/local/lib/libopencv_photo.so
LIBS += /usr/local/lib/libopencv_objdetect.so
LIBS += /usr/local/lib/libopencv_video.so
LIBS += /usr/local/lib/libopencv_ml.so
LIBS += /usr/local/lib/libopencv_calib3d.so
LIBS += /usr/local/lib/libopencv_features2d.so
LIBS += /usr/local/lib/libopencv_highgui.so
LIBS += /usr/local/lib/libopencv_imgproc.so
LIBS += /usr/local/lib/libopencv_flann.so
LIBS += /usr/local/lib/libopencv_core.so


HEADERS += MotionDetect.h

SOURCES += MotionDetect.cpp
SOURCES += main.cpp



