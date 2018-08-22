/*!
 * \file mxnet_cpp_helper.cpp
 * \brief implementation of mxnet_cpp_helper
 * \author yuanyang
 * 2017.02.18
 */

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "mxnet_cpp_helper.h"

using std::vector;
using cv::Mat;

int ConvertMat2Data(const cv::Mat &im_ori,
                     mx_float *output_ptr,
                     const mx_float mean_value,
                     const mx_float scale,
                     const cv::Size resize_size,
                     const mx_float* mean_data)
{
    if (im_ori.empty()){
        std::cerr << "Input image im_ori in ConvertMat2Data is empty, check again. \n";
        return -1;
    }
    int channels = im_ori.channels();

    cv::Mat im;
    if( resize_size.height != 0 and resize_size.width != 0 )
        cv::resize(im_ori, im, resize_size);
    else
        im = im_ori;

    int size = im.rows * im.cols * channels;

    mx_float* ptr_image_b = output_ptr;
    mx_float* ptr_image_g = output_ptr + size / 3;
    mx_float* ptr_image_r = output_ptr + size / 3 * 2;

    float mean_b, mean_g, mean_r;
    mean_b = mean_g = mean_r = mean_value;

    for (int i = 0; i < im.rows; i++) {
        uchar* data = im.ptr<uchar>(i);
        for (int j = 0; j < im.cols; j++) {
            if (mean_data) {
                mean_r = *mean_data;
                if (channels > 1)
                {
                    mean_g = *(mean_data + size / 3);
                    mean_b = *(mean_data + size / 3 * 2);
                }
               mean_data++;
            }
            if (channels > 1) {
                *ptr_image_b++ = scale*(static_cast<mx_float>(*data++) - mean_b);
                *ptr_image_g++ = scale*(static_cast<mx_float>(*data++) - mean_g);
            }
            *ptr_image_r++ = scale*(static_cast<mx_float>(*data++) - mean_r);
        }
    }
    return 0;
}

int ConvertMats2Data(const vector<Mat> &input_images,
                     vector<mx_float> &output_data,
                     const mx_float mean_value,
                     const mx_float scale,
                     const cv::Size resize_size,
                     const mx_float* mean_data)
{
    if(input_images.empty())
        return 0;

    /* input data shape */
    unsigned long number_of_images = input_images.size();
    int channel = 0; /* should be the same for all images */
    int width = 0; /* should be the same if resize_size is not set */
    int height = 0; /* should be the same if resize_size is not set */

    channel = input_images[0].channels();
    for (auto &img:input_images) {
        if(img.channels()!= channel) {
            std::cerr<<"Images should have the same channel in function ConvertMats2Data"<<std::endl;
            return -1;
        }
    }

    /* no resize, so all the image should have the identical shape */
    if(resize_size.width == 0 || resize_size.height == 0) {
        /* input data shape */
        width = input_images[0].size().width;
        height = input_images[0].size().height;

        /* check the images, should be of the same shape */
        for (auto &img:input_images) {
            if (img.size().width != width || img.size().height != height) {
                std::cerr << "Image should be of the same shape in function ConvertMats2Data" << std::endl;
                return -1;
            }
        }
    }
    else {
        width = resize_size.width;
        height = resize_size.height;
    }

    /* prepare the output data array */
    output_data.resize(number_of_images*channel*width*height);
    unsigned long image_size = static_cast<unsigned long>(channel*width*height);
    mx_float *output_ptr = nullptr;
    unsigned int counter = 0;
    for(auto &img:input_images){
        output_ptr = output_data.data() + counter*image_size;
        ConvertMat2Data(img, output_ptr, mean_value, scale, resize_size, mean_data);
        counter++;
    }
    return 0;
}

void BindPredictor(const char *json_buffer,
                   const char *param_buffer,
                   size_t param_size,
                   const std::vector<mx_uint> &input_size,
                   const int dev_type,
                   const int dev_id,
                   PredictorHandle &handle)
{
    if(input_size.size() != 4) {
        std::cerr << "input_size of function BindPredictor should be "
                "length 4 of(n,c,h,w)" << std::endl;
        return ;
    }
    /* TODO
     * these setting should be opened if we have complex input
     * other than images */
    mx_uint num_input_nodes = 1;  // 1 for feedforward
    const char* input_key[1] = {"data"};
    const char** input_keys = input_key;
    const mx_uint input_shape_indptr[2] = { 0, 4 };

    // Create Predictor
    int status = MXPredCreate(json_buffer,
                 param_buffer,
                 param_size,
                 dev_type,
                 dev_id,
                 num_input_nodes,
                 input_keys,
                 input_shape_indptr,
                 input_size.data(),
                 &handle);
    //std::cout<<"MXPredCreate done"<<std::endl;
    if (status<0) std::cout<<"mxnet Error : "<< MXGetLastError()<<std::endl;
}

void GetOutput(PredictorHandle pred_hnd,
               const int output_index,
               vector<mx_uint> &shapes,
               std::vector<float> &data,
               int &output_size){

    mx_uint *shape = 0;
    mx_uint shape_len;

    shapes.clear();

    // Get Output Result
    MXPredGetOutputShape(pred_hnd, output_index, &shape, &shape_len);

    size_t size = 1;
    for (mx_uint i = 0; i < shape_len; ++i){
        size *= shape[i];
        shapes.push_back(shape[i]);
    }
    /* prepare the space for output */
    data.resize(size);
    output_size = size;
    MXPredGetOutput(pred_hnd, output_index, &(data[0]), size);
}


