#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;


class MotionDetection
{
private:
    float mRatio;
    int mPadding;
    int mNumOfChangesThresh;

    cv::Mat mPreviousFrame;
    cv::Mat mMotion;
    cv::Rect mMotionRect;

    int mNumOfChanges;
    bool mHasMotion;

public:
    MotionDetection(int thresh=5);

    void setInputImage(const cv::Mat &org_frame);

    int getNumberOfChanges();

private:
    // single rectangle to describe the motion
    cv::Rect getMotionGestalt();

private:
    void getRectGray(const cv::Mat &current_frame);

    // convert motion map to single rectangle
    void motion2Rect();
};



