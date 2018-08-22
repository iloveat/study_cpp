/*!
 * \file mxnet_cpp_helper.h
 * \brief c++ helper functions when use mxnet with opencv,
 * \relates https://github.com/dmlc/mxnet/blob/master/example/image-classification/predict-cpp/image-classification-predict.cc
 * \author yuanyang
 * 2017.02.18
 */

#ifndef MXNET_CPP_APP_MXNET_CPP_HELPER_H
#define MXNET_CPP_APP_MXNET_CPP_HELPER_H

#include <iostream>
#include <string>
#include <fstream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "mxnet/c_predict_api.h"

using std::vector;
using cv::Mat;

/*!
 *  Read file to buffer, used in MXnet creation
 */
class BufferFile{
 public :
    BufferFile()
    {}
    BufferFile(std::string file_path):file_path_(file_path){
        SetFile(file_path);
    }
    int GetLength(){
        return length_;
    }
    char* GetBuffer(){
        return buffer_;
    }
    ~BufferFile(){
        if (buffer_){
          delete[] buffer_;
          buffer_ = NULL;
        }
    }

    void SetFile(const std::string &file_path){
        file_path_ = file_path;
        std::ifstream ifs(file_path.c_str(), std::ios::in | std::ios::binary);
        if (!ifs) {
            std::cerr << "Can't open the file. Please check " << file_path << ". \n";
            length_ = 0;
            buffer_ = NULL;
            return;
        }
        ifs.seekg(0, std::ios::end);
        length_ = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        std::cout << file_path.c_str() << " ... "<< length_ << " bytes\n";

        buffer_ = new char[sizeof(char) * length_];
        ifs.read(buffer_, length_);
        ifs.close();
    }

    void SetBuffer(const char *buff, int len){
        buffer_ = new char[sizeof(char) * len];
        memcpy(buffer_, buff, sizeof(char) * len);
        length_ = len;
    }

private:
    std::string file_path_;
    int length_;
    char* buffer_;
};

/*!
 *  convert from opencv's Mat to mx_float array(ready for Forward)
 *  with optional image transform(mean and scale)
 *  IMPORTANT -> output is in BGR order
 *  p_new = scale *( p_old - mean_value )
 *
 * @param im_ori        input, input image
 * @param image_data    output, converted array, should be allocated before function call
 * @param mean_value    input, single number mean value
 * @param scale         input, scale
 * @param resize_size   input, desired image size
 * @param mean_data     input, mean_data, this will overwrite mean_value
 * @return 0 -> success, -1 -> failure
 */
int ConvertMat2Data(const Mat &im_ori,
                    mx_float *image_data,
                    const mx_float mean_value = 0.0,
                    const mx_float scale = 1.0,
                    const cv::Size resize_size = cv::Size(0,0),
                    const mx_float* mean_data = nullptr);

/*!
 *  convert opencv Mats to vector<mx_float>, check ConvertMat2Data for
 *  parameter descriptions
 * @param input_images
 * @param output_data
 * @param mean_value
 * @param scale
 * @param resize_size
 * @param mean_data
 * @return
 */
int ConvertMats2Data(const vector<Mat> &input_images,
                     vector<mx_float> &output_data,
                     const mx_float mean_value = 0.0,
                     const mx_float scale = 1.0,
                     const cv::Size resize_size = cv::Size(0,0),
                     const mx_float* mean_data = nullptr);

/*!
 *  bind the symbol to predictor
 *  should re-bind the predictor if the input is altered
 * @param json_buffer   input: symbol
 * @param param_buffer  input: params
 * @param param_size    input: size of params
 * @param input_size    input: shoule be of length 4 -> (n, c, h, w)
 * @param dev_type      input: 1 -> cpu, 2 -> gpu
 * @param dev_id        input: no idea ...id of GPU?
 * @param handle        output: returned handle
 */
void BindPredictor(const char *json_buffer,
                   const char *param_buffer,
                   size_t param_size,
                   const vector<mx_uint> &input_size,
                   const int dev_type,
                   const int dev_id,
                   PredictorHandle &handle);

void GetOutput(PredictorHandle pred_hnd,
               const int output_index,
               vector<mx_uint> &shapes,
               std::vector<float> &data,
               int &output_size);
#endif //MXNET_CPP_APP_MXNET_CPP_HELPER_H
