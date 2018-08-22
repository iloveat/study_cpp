#include <opencv2/opencv.hpp>
#include <iostream>
#include "MotionDetect.h"

using namespace std;
using namespace cv;


MotionDetection::MotionDetection(int thresh)
{
    mRatio = 0.3f;
    mPadding = 10;
    mHasMotion = false;
    mNumOfChanges = 0;
    mNumOfChangesThresh = thresh;
}

void MotionDetection::setInputImage(const cv::Mat &org_frame)
{
    int width = org_frame.cols;
    int height = org_frame.rows;

    // resize to get current frame
    cv::Mat current_frame;
    cv::resize(org_frame, current_frame, cv::Size((int)(width*mRatio), (int)(height*mRatio)));
    if(mPreviousFrame.rows != current_frame.rows || mPreviousFrame.cols != current_frame.cols)
    {
        current_frame.copyTo(mPreviousFrame);
    }
    assert(current_frame.cols == mPreviousFrame.cols && current_frame.rows == mPreviousFrame.rows);

    // begin to detect motion
    mNumOfChanges = 0;
    getRectGray(current_frame);
    current_frame.copyTo(mPreviousFrame);
}

int MotionDetection::getNumberOfChanges()
{
    return mNumOfChanges;
}

// single rectangle to describe the motion
cv::Rect MotionDetection::getMotionGestalt()
{
    int x = (int)(mMotionRect.x/mRatio);
    int y = (int)(mMotionRect.y/mRatio);
    int width = (int)(mMotionRect.width/mRatio);
    int height = (int)(mMotionRect.height/mRatio);
    return cv::Rect(x, y, width, height);
}

void MotionDetection::getRectGray(const cv::Mat &current_frame)
{
    cv::absdiff(mPreviousFrame, current_frame, mMotion);
    cv::threshold(mMotion, mMotion, 35, 255, CV_THRESH_BINARY);
    //cv::imshow("motion_before_erode", mMotion);

    cv::Mat kernel_erode = getStructuringElement(MORPH_RECT, cv::Size(3,3));
    cv::erode(mMotion, mMotion, kernel_erode);
    //cv::imshow("motion_after_erode", mMotion);

    motion2Rect();
}

// convert motion map to single rectangle
void MotionDetection::motion2Rect()
{
    int min_x = mMotion.cols;
    int max_x = 0;
    int min_y = mMotion.rows;
    int max_y = 0;

    int x_start = mPadding;
    int y_start = mPadding;
    int x_stop = mPreviousFrame.cols-mPadding-1;
    int y_stop = mPreviousFrame.rows-mPadding-1;

    // loop over image and detect changes
    for(int j = y_start; j < y_stop; j+=2)
    {
        for(int i = x_start; i < x_stop; i+=2)
        {
            // check if at pixel (j,i) intensity is equal to 255, this means that the pixel is
            // different in the sequence of images (prev_frame, current_frame, next_frame)
            if(static_cast<int>(mMotion.at<uchar>(j,i)) == 255)
            {
                mNumOfChanges++;
                if(min_x>i) min_x = i;
                if(max_x<i) max_x = i;
                if(min_y>j) min_y = j;
                if(max_y<j) max_y = j;
            }
        }
    }

    if(mNumOfChanges > mNumOfChangesThresh)
    {
        // draw rectangle round the changed pixels
        if(min_x-10 > 0)
            min_x -= 10;
        else
            min_x = 0;

        if(min_y-10 > 0)
            min_y -= 10;
        else
            min_y=0;

        if(max_x+10 < mMotion.cols-1)
            max_x += 10;
        else
            max_x = mMotion.cols-1;

        if(max_y+10 < mMotion.rows-1)
            max_y += 10;
        else
            max_y = mMotion.rows-1;

        cv::Point p1(min_x, min_y);
        cv::Point p2(max_x, max_y);
        mMotionRect = cv::Rect(p1, p2);
        mHasMotion = true;
    }
    else
    {
        mMotionRect = cv::Rect(cv::Point(-1,-1), cv::Point(-1,-1));
        mHasMotion = false;
    }
}



