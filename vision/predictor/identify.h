/*!
 * \file identify.h
 * \brief implementation of identify predictor
 * \author zoucheng
 * \date 2017.05.15
 */

#ifndef IDENTIFY_H
#define IDENTIFY_H

#include <opencv2/opencv.hpp>
#include <mxnet/c_predict_api.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;


namespace TuringOS
{
    /*
     * identify data structure
     * name, feature dimension and feature vector
     */
    typedef struct IdentifyNode
    {
        std::string name;
        int dim;
        std::vector<float> feaVec;
        IdentifyNode(const std::string &name_, const int &dim_)
        {
            name = name_;
            dim = dim_;
            feaVec = std::vector<float>(dim);
        }
    } IdentifyNode;

    /*
     * face attributes node
     */
    typedef struct FaceAttr
    {
        cv::Rect rect;
        int sharpness;
        bool passnpd;
        float similarity;
        std::string name;
        float angle[3];

        FaceAttr(const float &simi, const std::string &nm)
        {
            similarity = simi;
            name = nm;
        }
        FaceAttr(const cv::Rect &rt, const int &sharp, const bool &npd, const float &simi, const std::string &nm, float* agl)
        {
            rect = rt;
            sharpness = sharp;
            passnpd = npd;
            similarity = simi;
            name = nm;
            for(int i = 0; i < 3; i++)
            {
                angle[i] = agl[i];
            }
        }
    } FaceAttr;

    class IdentifyPredictor
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
        // 身份字典，用于识别
        std::vector<IdentifyNode> mIdStorage;

    public:
        IdentifyPredictor();

        ~IdentifyPredictor();

        void setParameter(const std::string &model_path,
                          const std::string &json_name,
                          const std::string &para_name,
                          const int &width,
                          const int &height,
                          const int &channels);

        // dev_type, 1: cpu, 2: gpu
        int loadModel(int dev_type=1);

        // 从文件中生成人脸特征库，用于人脸识别
        bool setIdStorage(const std::string &filename);

        // 计算输入3通道人脸图像的256维特征向量
        std::vector<float> predict(const cv::Mat &img3c);

        // 计算两个256维人脸特征的相似度
        float getSimilarity(const std::vector<float> &vec1, const std::vector<float> &vec2);

        // 直接计算两幅3通道人脸图像的相似度
        float getSimilarity(const cv::Mat &face1, const cv::Mat &face2);

        // 在人脸库中找最相似的人脸，输入为3通道人脸图像，相似度小于阈值则不认识
        FaceAttr getNearestId(const cv::Mat &img3c, const float &thresh);

        // 在人脸库中找最相似的人脸，输入为256维的人脸特征，相似度小于阈值则不认识
        FaceAttr getNearestId(const std::vector<float> &vec, const float &thresh);

        // 向人脸库中新增一个人脸
        void addNewPerson(IdentifyNode idnode);

    private:
        // 人脸特征向量归一化
        void normalizeArray(std::vector<float> &vec);

        // 计算两个向量的内积
        float vectorProduct(const std::vector<float> &vec1, const std::vector<float> &vec2);
    };
}

#endif // IDENTIFY_H
