/*!
 * \file emotion.cpp
 * \brief implementation of emotion predictor
 * \author zoucheng
 * \date 2017.05.15
 */

#include "emotion.h"
#include <opencv2/opencv.hpp>
#include <mxnet/c_predict_api.h>
#include <iostream>
#include <vector>
#include "memfilebuffer.h"

using namespace std;
using namespace cv;
using namespace TuringOS;


EmotionPredictor::EmotionPredictor()
{

}

EmotionPredictor::~EmotionPredictor()
{
    if(mHandle > 0)
    {
        MXPredFree(mHandle);
    }
}

void EmotionPredictor::setParameter(const std::string &model_path,
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
int EmotionPredictor::loadModel(int dev_type)
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
        std::cerr<<"Load emotion predictor model failed.\n";
        return -1;
    }

    mInitialSuccessful = true;
    return 1;
}

std::vector<float> EmotionPredictor::predict(const cv::Mat &img3c)
{
    if(!mInitialSuccessful)
    {
        std::cerr<<"Load emotion predictor model failed.\n";
        return std::vector<float>();
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
            *ptr_image_data++ = pix/1.0/256;
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

    // vec is a 6-element array
    std::vector<float> result(size);
    MXPredGetOutput(mHandle, output_index, &(result[0]), size);

    return result;
}

std::string EmotionPredictor::getEmotion(const cv::Mat &aligned_face)
{
    std::vector<std::string> emotion_range = { "angry", "fear", "happy", "neutral", "sad", "surprise" };
    //std::vector<std::string> emotion_range = { "生气", "害怕", "开心", "平静", "不开心", "吃惊" };
    std::vector<float> emotion_result = predict(aligned_face);
    if(emotion_result.empty())
    {
        std::cerr<<"Load emotion predictor model failed.\n";
        return std::string("");
    }

    int index_max = 0;
    float value_max = emotion_result[0];

    for(unsigned int p = 1; p < emotion_result.size(); p++)
    {
        if(emotion_result[p] > value_max)
        {
            value_max = emotion_result[p];
            index_max = p;
        }
    }

    return emotion_range[index_max];
}


