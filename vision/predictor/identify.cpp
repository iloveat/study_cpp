/*!
 * \file identify.cpp
 * \brief implementation of identify predictor
 * \author zoucheng
 * \date 2017.05.15
 */

#include "identify.h"
#include <opencv2/opencv.hpp>
#include <mxnet/c_predict_api.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "memfilebuffer.h"
#include "assert.h"

using namespace std;
using namespace cv;
using namespace TuringOS;


IdentifyPredictor::IdentifyPredictor()
{

}

IdentifyPredictor::~IdentifyPredictor()
{
    if(mHandle > 0)
    {
        MXPredFree(mHandle);
    }
    vector<IdentifyNode> ept;
    mIdStorage.clear();
    mIdStorage.swap(ept);
}

void IdentifyPredictor::setParameter(const std::string &model_path,
                                     const std::string &json_name,
                                     const std::string &para_name,
                                     const int &width,
                                     const int &height,
                                     const int &channels)
{
    mWidth = width;
    mHeight = height;
    mChannels = 3;
    mImageSize = mWidth*mHeight*mChannels;

    mModelPath = model_path;
    mJsonFileName = json_name;
    mParaFileName = para_name;

    mInitialSuccessful = false;
}

// dev_type, 1: cpu, 2: gpu
int IdentifyPredictor::loadModel(int dev_type)
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
        std::cerr<<"Load identify predictor model failed.\n";
        return -1;
    }

    mInitialSuccessful = true;
    return 1;
}

/*
 * 从文件中生成人脸特征库，用于人脸识别
 * load face features from file, ie 'face.db', to mIdStorage
 * file format:
 * name1 256
 * a list of 256 float numbers, separated by blanks
 * name2 256
 * a list of 256 float numbers, separated by blanks
 * ...
 * nameN 256
 * a list of 256 float numbers, separated by blanks
 */
bool IdentifyPredictor::setIdStorage(const std::string &filename)
{
    ifstream ifile(filename);
    if(!ifile.is_open())
        return false;

    while(true)
    {
        std::string name;
        int dim;
        ifile>>name>>dim;
        if(name=="")
            break;
        IdentifyNode idnode(name, dim);
        for(int i = 0; i < dim; i++)
        {
            ifile>>(idnode.feaVec)[i];
        }
        mIdStorage.push_back(idnode);
    }

    ifile.close();
    return true;
}

// 计算输入3通道人脸图像的256维特征向量
std::vector<float> IdentifyPredictor::predict(const cv::Mat &img3c)
{
    if(!mInitialSuccessful)
    {
        std::cerr<<"Load identify predictor model failed.\n";
        return vector<float>();
    }

    cv::Mat img_resize = img3c.clone();
    if(img3c.cols!=mWidth || img3c.rows!=mHeight)
    {
        cv::resize(img_resize, img_resize, cv::Size(mWidth, mHeight));
    }

    // convert Mat to vector<mx_float>
    std::vector<mx_float> image_data = std::vector<mx_float>(mImageSize);
    mx_float *ptr_image_data = image_data.data();
    mx_float* ptr_image_r = ptr_image_data;
    mx_float* ptr_image_g = ptr_image_data + mImageSize / 3;
    mx_float* ptr_image_b = ptr_image_data + mImageSize / 3 * 2;

    for(int i = 0; i < img_resize.rows; i++)
    {
        uchar *data = img_resize.ptr<uchar>(i);
        for(int j = 0; j < img_resize.cols; j++)
        {
            mx_float bb = static_cast<mx_float>(*data++);
            mx_float gg = static_cast<mx_float>(*data++);
            mx_float rr = static_cast<mx_float>(*data++);
            *ptr_image_b++ = bb;
            *ptr_image_g++ = gg;
            *ptr_image_r++ = rr;
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

    // vec is a 256-element array
    std::vector<float> result(size);
    MXPredGetOutput(mHandle, output_index, &(result[0]), size);
    normalizeArray(result);

    return result;
}

/*
 * 人脸特征向量归一化
 * x_i/sqrt(sum{x_i*x_i})
 */
void IdentifyPredictor::normalizeArray(std::vector<float> &vec)
{
    float total = 0;
    for(std::vector<float>::iterator it = vec.begin(); it != vec.end(); it++) {
        float t = *(it);
        total += t*t;
    }

    total = sqrt(total);

    for(std::vector<float>::iterator it = vec.begin(); it != vec.end(); it++) {
        float t = *(it);
        t = t/total;
        *(it) = t;
    }
}

// 直接计算两幅3通道人脸图像的相似度
float IdentifyPredictor::getSimilarity(const cv::Mat &face1, const cv::Mat &face2)
{
    std::vector<float> vec1 = predict(face1);
    std::vector<float> vec2 = predict(face2);
    return vectorProduct(vec1, vec2);
}

// 计算两个256维人脸特征的相似度
float IdentifyPredictor::getSimilarity(const std::vector<float> &vec1, const std::vector<float> &vec2)
{
    return vectorProduct(vec1, vec2);
}

// 计算两个向量的内积
float IdentifyPredictor::vectorProduct(const std::vector<float> &vec1, const std::vector<float> &vec2)
{
    assert(vec1.size()==vec2.size() && vec1.size()>0);
    float result = 0;
    for(unsigned int i = 0; i < vec1.size(); i++)
    {
        result += vec1[i] * vec2[i];
    }
    return result;
}

// 在人脸库中找最相似的人脸，输入为3通道人脸图像，相似度小于阈值则不认识
FaceAttr IdentifyPredictor::getNearestId(const Mat &img3c, const float &thresh)
{
    std::vector<float> vec = predict(img3c);
    return getNearestId(vec, thresh);
}

// 在人脸库中找最相似的人脸，输入为256维的人脸特征，相似度小于阈值则不认识
FaceAttr IdentifyPredictor::getNearestId(const std::vector<float> &vec, const float &thresh)
{
    // 如果人脸库为空
    if(mIdStorage.size() <= 0)
    {
        FaceAttr fa(0, "???");
        return fa;
    }

    std::string name = mIdStorage[0].name;
    std::vector<float> feaVec = (mIdStorage[0]).feaVec;

    float max_product = vectorProduct(feaVec, vec);
    for(unsigned int i = 1; i < mIdStorage.size(); i++)
    {
        std::vector<float> pp = (mIdStorage[i]).feaVec;
        float prdct = vectorProduct(pp, vec);
        if(prdct > max_product)
        {
            max_product = prdct;
            name = mIdStorage[i].name;
        }
    }

    if(max_product < thresh)
    {
        FaceAttr fa(max_product, "???");
        return fa;
    }

    FaceAttr fa(max_product, name);
    return fa;
}

// 向人脸库中新增一个人脸
void IdentifyPredictor::addNewPerson(IdentifyNode idnode)
{
    mIdStorage.push_back(idnode);
}


