/*!
 * \file gender.h
 * \brief implementation of gender predictor
 * \author zoucheng
 * \date 2017.05.12
 */

#ifndef GENDER_H
#define GENDER_H

#include <opencv2/opencv.hpp>
#include <mxnet/c_predict_api.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;


namespace TuringOS
{
    class GenderPredictor
    {
    private:
        PredictorHandle mHandle;
        // 神经网络输入图像的尺寸
        int mWidth;
        int mHeight;
        int mChannels;
        int mImageSize;
        // 神经网络模型文件的路径和名称
        std::string mModelPath;
        std::string mJsonFileName;
        std::string mParaFileName;
        // 模型加载是否成功
        bool mInitialSuccessful;

    public:
        GenderPredictor();

        ~GenderPredictor();

        void setParameter(const std::string &model_path,
                          const std::string &json_name,
                          const std::string &para_name,
                          const int &width,
                          const int &height,
                          const int &channels);

        // dev_type, 1: cpu, 2: gpu
        int loadModel(int dev_type=1);

        std::vector<float> predict(const cv::Mat &img3c);

        // 0:female, 1:male
        int getGender(const cv::Mat &aligned_face);
    };
}

#endif // GENDER_H
