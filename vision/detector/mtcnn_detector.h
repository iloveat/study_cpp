/*!
 * \file mtcnn_detector.hpp
 * \brief light version of mtcnn detector, with scale reuse in pnet
 *        and lighten version of rnet and onet
 * \author yuanyang
 * \date 2017.02.20
 */

#ifndef MXNET_CPP_APP_MTCNN_DETECTOR_H
#define MXNET_CPP_APP_MTCNN_DETECTOR_H
#include <string>
#include <vector>
#include <math.h>
#include "opencv2/core/core.hpp"
#include "mxnet_cpp_helper.h"

using std::string;
using std::vector;


namespace TuringOS{

/**
 * image rotation status
 */
enum ImgRotateStatus{
    ROTATION_NONE,  //original image without rotation
    ROTATION_CW_30,  //rotate the original image with 30 degrees in clockwise direction
    ROTATION_CC_30,  //rotate the original image with 30 degrees in counter clockwise direction
    ROTATION_NONE_PADDING  //pad in original image 2 detect very large faces
};

/*!
 * \breif store BBoxReg
 */
struct BBoxReg{
    BBoxReg(){
        x1_shift=y1_shift=x2_shift=y2_shift=0.0;
    }
    BBoxReg(float x1_, float y1_, float x2_, float y2_):x1_shift(x1_),y1_shift(y1_),x2_shift(x2_),y2_shift(y2_)
    {}
    float x1_shift;
    float y1_shift;
    float x2_shift;
    float y2_shift;
};

/*!
* \breif detection result
*/
struct BoundingBox{
    BoundingBox(const float x1_,
                const float y1_,
                const float x2_,
                const float y2_,
                const float s,
                const BBoxReg reg_):x1(x1_),y1(y1_),x2(x2_),y2(y2_),score(s),reg(reg_)
    {}

    inline float area() const{
        if( x2 < x1 || y2 < y1)
            return -1.0;
        return (x2-x1+1)*(y2-y1+1);
    }

    inline float width() const{
        if(x2 < x1)
            return -1.0;
        return x2-x1+1;
    }

    inline float height() const{
        if(y2 < y1)
            return -1.0;
        return y2-y1+1;
    }
    inline void round(){
        x1 = std::round(x1);
        x2 = std::round(x2);
        y1 = std::round(y1);
        y2 = std::round(y2);
    }
    float x1,y1,x2,y2;
    float score; /* confidence score of the detection */
    BBoxReg reg;

    ImgRotateStatus status;  //用来标记是否需要旋转图像
    cv::Rect rect_transformed;  //在变换图像上的检测框
};

/**
 * img with its boundingbox
 */
struct NewRegRes{
    cv::Mat img;
    ImgRotateStatus status;
    vector<BoundingBox> dets;
    bool enabled;  //if there is no face in the img, enabled=false;
    NewRegRes(const cv::Mat &image, ImgRotateStatus stat, vector<BoundingBox> &bbs){
        img = image.clone();
        status = stat;
        dets = bbs;
        enabled = true;
    }
    NewRegRes(){
        enabled = true;
    }
};


class MtcnnDetector{
public:
    MtcnnDetector(const int minsize,
                  const vector<float> &threshold,
                  const float factor,
                  const int dev_type,
                  const int dev_id);
    ~MtcnnDetector();
    /* no copy&assign constructor */
    MtcnnDetector(const MtcnnDetector & that) = delete;
    MtcnnDetector& operator=(MtcnnDetector const&) = delete;
    /* detect_face2, designed by yuanyang, written by zoucheng, deals with rotated faces and very large faces */
    vector<BoundingBox> detect_face2(const Mat &input_image, bool do_rotation=true);

private:
    vector<BoundingBox> detect_face(const Mat &input_image);

    vector<BoundingBox> detect_pnet(const Mat &img,
                                    const float scale);
    void detect_rnet(const Mat &img,
                     vector<BoundingBox> &boxes);
    void detect_onet(const Mat &img,
                     vector<BoundingBox> &boxes);
    void detect_rnet2(vector<NewRegRes> &ret_list);

    void detect_onet2(vector<NewRegRes> &ret_list);

    void draw_bbox(const Mat &input_img,
                   const vector<BoundingBox> &bbox,
                   int timeout=0);

    void generate_box(vector<float> &cls_data,
                      const vector<mx_uint> &cls_shape,
                      vector<float> &bbox_data,
                      const vector<mx_uint> &bbox_shape,
                      const float threshold,
                      const float scale,
                      const int stride,
                      const int cellsize,
                      vector<BoundingBox> &dets);

    vector<int> nms(vector<BoundingBox> &bbox,
                    const float threshold,
                    const char method);

    void convert_to_square(vector<BoundingBox> &boxes);
    void calibrate_box(vector<BoundingBox> &boxes);

    void pad( vector<BoundingBox> &boxes,
              const Mat &input_image,
              vector<Mat> &box_images);

    /* data members */
    BufferFile pnet_json_;
    BufferFile pnet_param_;
    BufferFile rnet_json_;
    BufferFile rnet_param_;
    BufferFile onet_json_;
    BufferFile onet_param_;

    vector<float> threshold_;
    int minsize_;
    float factor_;

    int dev_type_;
    int dev_id_;
};

}
#endif //MXNET_CPP_APP_MTCNN_DETECTOR_H
