#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include "TwoClassOtsu.h"

using namespace std;
using namespace cv;



int main(int argc, char **argv)
{
    if(argc < 2)
    {
        cout<<"at least 1 image needed,usage ./cvDepthThreshold *.png"<<endl;
        return -1;
    }
    for(int i = 1; i < argc; i++)
    {
        Mat srcImg = imread(argv[i], CV_LOAD_IMAGE_GRAYSCALE);
        Rect rct(0, 0, srcImg.cols, srcImg.rows);
        Point2f pt;
        if(srcImg.empty())
        {
            cout<<"read image failed"<<endl;
            return -1;
        }
        imshow("srcImg", srcImg);

        //核心代码，就这一行
        getLocation(srcImg, rct, pt);

        cout<<endl<<"result:"<<pt.x<<","<<pt.y<<endl;
        waitKey(0);
    }

    return 0;
}










