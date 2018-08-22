#include "TwoClassOtsu.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

using namespace std;


unsigned int* calcuHistogram(Mat &input_img, int &max_val, int &min_val)
{
    if(input_img.empty())
    {
        cout<<"invalidate input image"<<endl;
        return NULL;
    }

    unsigned int *hist = new unsigned int[256];
    memset(hist,0,256*sizeof(unsigned int));
    max_val = -1;
    min_val = 65535;

    int nr = input_img.rows;
    int nl = input_img.cols * input_img.channels();
    for(int i = 0; i < nr; i++)
    {
        const uchar *data = input_img.ptr<uchar>(i);  //第i行的首指针
        for(int j = 0; j < nl; j++)
        {
            uchar p = data[j];
            if(p >= SENSOR_LOWER_BOUND_VALUE && p <= SENSOR_UPPER_BOUND_VALUE)  //只统计有效范围内的数据
            {
                hist[p]++;
                if(p >= max_val) max_val = p;
                if(p < min_val) min_val = p;
            }
        }
    }

    if(IS_DEBUG)
    {
        cout<<endl<<"min_value:"<<min_val<<endl<<"max_value:"<<max_val<<endl;
        ofstream of("hist_data");
        for(int i = 0; i < 256; i++)
            of<<hist[i]<<",";
        of<<endl;
        of.close();
    }

    return hist;
}


OtsuRES TwoClassOtsu(unsigned int *hist, int left, int right, int method)
{
    OtsuRES result;
    double max_sigmaB = -1;
    if(NULL == hist)
    {
        result.pivot = -1;
        return result;
    }

	for (int k = left+1; k < right-1; k++)
	{
		double p1=0, p2=0, m1=0, m2=0, mG=0;
		double s1=0, s2=0;
		double d1, d2, sigmaB;
		for (int i = left; i < right; i++)
		{
			if(i<=k)
			{
				p1 += hist[i];
				m1 += i*hist[i];
			}
			else
			{
				p2 += hist[i];
				m2 += i*hist[i];
			}
		}
		mG = m1+m2;
		m1 = m1/1.0/p1;
		m2 = m2/1.0/p2;
		d1 = m1-mG;
		d2 = m2-mG;
		for (int i = left; i <right; i++)
		{
			if(i<=k)
				s1 += (i-m1)*(i-m1)*hist[i];
			else
				s2 += (i-m2)*(i-m2)*hist[i];
		}

		switch(method)
		{
		case ORG_OPT:
			sigmaB = p1*d1*d1+p2*d2*d2; //original
			break;
		default:
			sigmaB = (p1*d1*d1+p2*d2*d2)/1.0/(s1/1.0/p1+s2/1.0/p2); //advanced 1
			break;
		}

		if (sigmaB > max_sigmaB)
		{
			max_sigmaB = sigmaB;
            result.pivot = k;
            result.e1 = m1;
            result.e2 = m2;
		}
	}

	return result;
}

OtsuRES getBodyDepth(const Mat &depth_image, Rect rct)
{
    OtsuRES result;
    if(depth_image.empty())
    {
        cout<<"read image failed"<<endl;
        result.pivot = -1;
        return result;
    }

    Mat roiImg;
    depth_image(rct).copyTo(roiImg);

    int min_value, max_value;
    unsigned int *hist = calcuHistogram(roiImg, max_value, min_value);
    if(NULL == hist)
    {
        cout<<"错误，直方图为NULL"<<endl;
        result.pivot = -1;
        return result;
    }

    result = TwoClassOtsu(hist, min_value, max_value, ORG_OPT);
    if(hist != NULL)
        delete []hist;

    return result;
}

float getRealDist(uchar gray_depth)
{
    return 7.5*gray_depth/255+0.5;
}

void getLocation(const Mat &img, Rect &rect, Point2f &location)
{
    //kinect param
    camera_param depth_cam(584.67324804, 314.997990062, 248.758860509, 0.00);

    OtsuRES res = getBodyDepth(img, rect);
    if(-1 == res.pivot)
    {
        cout<<"错误，OTSU算法失败"<<endl;
        return;
    }

    if(IS_DEBUG)
    {
        cout<<res.pivot<<endl;
        cout<<res.e1<<endl;
        cout<<res.e2<<endl;

        Mat roiImg;
        img(rect).copyTo(roiImg);

        int nr = roiImg.rows;
        int nl = roiImg.cols * roiImg.channels();
        for(int i = 0; i < nr; i++)
        {
            uchar *data = roiImg.ptr<uchar>(i);  //第i行的首指针
            for(int j = 0; j < nl; j++)
            {
                if(data[j] <= res.pivot)
                    data[j] = 255;
                else
                    data[j] = 0;
            }
        }
        imshow("roiImg",roiImg);
    }

    float z = getRealDist(res.e1);
    float x = z/depth_cam.f * (rect.x+rect.width/2 - depth_cam.cu);
    //float y = z/depth_cam.f * (rect.y+rect.height/2 - depth_cam.cv);
    location.x = x;
    location.y = z;
}
