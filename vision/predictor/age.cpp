/*!
 * \file age.cpp
 * \brief implementation of age predictor
 * \author zoucheng
 * \date 2017.05.12
 */

#include "age.h"
#include <opencv2/opencv.hpp>
#include <mxnet/c_predict_api.h>
#include <iostream>
#include <vector>
#include "memfilebuffer.h"

using namespace std;
using namespace cv;
using namespace TuringOS;


AgePredictor::AgePredictor()
{

}

AgePredictor::~AgePredictor()
{
    if(mHandle > 0)
    {
        MXPredFree(mHandle);
    }
}

void AgePredictor::setParameter(const std::string &model_path,
                                const std::string &json_name,
                                const std::string &para_name,
                                const int &width,
                                const int &height,
                                const int &channels)
{
    mWidth = width;
    mHeight = height;
    mChannels = 1;
    mImageSize = mWidth*mHeight*mChannels;

    mModelPath = model_path;
    mJsonFileName = json_name;
    mParaFileName = para_name;

    mInitialSuccessful = false;
}

// dev_type, 1: cpu, 2: gpu
int AgePredictor::loadModel(int dev_type)
{
    mInitialSuccessful = false;
    MemFileBuffer json_data(mModelPath+mJsonFileName);
    MemFileBuffer para_data(mModelPath+mParaFileName);

    // feedforward's default parameters
    int dev_id = 0;
    mx_uint num_input_nodes = 1;
    const char* input_key[1] = {"data"};
    const char** input_keys = input_key;
    const mx_uint input_shape_indptr[2] = { 0, 4 };

    const mx_uint input_shape_data[4] = { 1,
                                          static_cast<mx_uint>(mChannels),
                                          static_cast<mx_uint>(mWidth),
                                          static_cast<mx_uint>(mHeight) };

    // create predictor
    mHandle = 0;
    MXPredCreate((const char*)json_data.getBuffer(),
                 (const char*)para_data.getBuffer(),
                  static_cast<size_t>(para_data.getLength()),
                  dev_type,
                  dev_id,
                  num_input_nodes,
                  input_keys,
                  input_shape_indptr,
                  input_shape_data,
                  &mHandle);

    if(mHandle <= 0)
    {
        std::cerr<<"Load age predictor model failed.\n";
        return -1;
    }

    mInitialSuccessful = true;
    return 1;
}

vector<float> AgePredictor::predict(const cv::Mat &img3c)
{
    if(!mInitialSuccessful)
    {
        std::cerr<<"Load age predictor model failed.\n";
        return vector<float>();
    }

    cv::Mat img1c = img3c.clone();
    if(img3c.cols!=mWidth || img3c.rows!=mHeight)
    {
        cv::resize(img1c, img1c, cv::Size(mWidth, mHeight));
    }

    if(img1c.channels() == 3)
        cv::cvtColor(img1c, img1c, cv::COLOR_BGR2GRAY);

    // convert Mat to vector<mx_float>
    std::vector<mx_float> image_data = std::vector<mx_float>(mImageSize);
    mx_float *ptr_image_data = image_data.data();

    for(int i = 0; i < img1c.rows; i++)
    {
        uchar *data = img1c.ptr<uchar>(i);
        for(int j = 0; j < img1c.cols; j++)
        {
            mx_float pix = static_cast<mx_float>(*data++);
            *ptr_image_data++ = pix;
        }
    }

    MXPredSetInput(mHandle, "data", image_data.data(), mImageSize);
    MXPredForward(mHandle);

    mx_uint output_index = 0;
    mx_uint *output_shape = 0;
    mx_uint shape_length;

    // Get Output Result
    MXPredGetOutputShape(mHandle, output_index, &output_shape, &shape_length);

    size_t size = 1;
    for(mx_uint i = 0; i < shape_length; i++)
        size *= output_shape[i];

    std::vector<float> result(size);
    MXPredGetOutput(mHandle, output_index, &(result[0]), size);

    return result;
}

float AgePredictor::getAge(const cv::Mat &aligned_face)
{
    std::vector<float> age_range = {1, 5, 10, 18, 28, 40, 50, 70};
    std::vector<float> age_result = predict(aligned_face);
    if(age_result.empty())
    {
        std::cerr<<"Load age predictor model failed.\n";
        return -1;
    }

    float result = age_result[0];
    int base = (int)floor(result);
    float rest = result-base;
    float base_age = age_range[base];
    float rest_age = (age_range[base+1]-age_range[base])*rest;
    float ret = base_age+rest_age;
    if(ret < 1) ret = 1;
    return ret;
}


