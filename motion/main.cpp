#include <opencv2/opencv.hpp>
#include <iostream>
#include "MotionDetect.h"

using namespace std;
using namespace cv;


int main(int argc, char **argv)
{
    MotionDetection motion_detector;
    VideoCapture cap(0);

    while(true)
    {
        cv::Mat frame;
        cap >> frame;
        if(frame.empty())
        {
            cout<<"read image failed, skip"<<endl;
            continue;
        }

        motion_detector.setInputImage(frame);
        int nchanges = motion_detector.getNumberOfChanges();
        cout<<nchanges<<endl;

        cv::waitKey(30);
    }

    return 0;
}
