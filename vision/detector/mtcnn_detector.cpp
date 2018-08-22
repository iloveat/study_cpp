/*!
 * \file mtcnn_detector.cpp
 * \brief implementation of light version of mtcnn detector
 * \author yuanyang
 * \date 2017.02.20
 */

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "mxnet/c_api.h"
#include "mxnet_cpp_helper.h"
#include "mtcnn_detector.h"

#include "params_mxnet_detector.h"
#include "parse_string.h"

using namespace std;


namespace TuringOS{

MtcnnDetector::MtcnnDetector(const int minsize,
                             const vector<float> &threshold,
                             const float factor,
                             const int dev_type,
                             const int dev_id){
    /* load model into memory */
    string s1 = recoverFromString(str_pnet_all_symbol_json);
    pnet_json_.SetBuffer(s1.c_str(), s1.length());
    s1.clear();
    string s2 = recoverFromString(str_pnet_all_0016_params);
    pnet_param_.SetBuffer(s2.c_str(), s2.length());
    s2.clear();
    string s3 = recoverFromString(str_rnet_symbol_json);
    rnet_json_.SetBuffer(s3.c_str(), s3.length());
    s3.clear();
    string s4 = recoverFromString(str_rnet_0016_params);
    rnet_param_.SetBuffer(s4.c_str(), s4.length());
    s4.clear();
    string s5 = recoverFromString(str_onet_symbol_json);
    onet_json_.SetBuffer(s5.c_str(), s5.length());
    s5.clear();
    string s6 = recoverFromString(str_onet_0016_params);
    onet_param_.SetBuffer(s6.c_str(), s6.length());
    s6.clear();

    /* set other params */
    minsize_ = minsize;
    threshold_ = {0.0, 0.0, 0.0, 0.0};
    for(int i=0;i<4;i++){
        float f = threshold[i];
        threshold_[i] = f;
        assert(f > 0.0 && "threshold shoule be greater than 0.0");
    }
    //threshold_ = threshold;

    assert(factor < 1.0 && factor > 0.0 && "factor should in range (0.0, 1.0)");
    factor_ = factor;

    dev_type_ = dev_type;
    dev_id_ = dev_id;
}

MtcnnDetector::~MtcnnDetector(){
}

vector<BoundingBox> MtcnnDetector::detect_pnet(const Mat &img,
                                               const float scale){
    cv::Size img_size = img.size();
    mx_uint hs = static_cast<mx_uint>(img_size.height*scale);
    mx_uint ws = static_cast<mx_uint>(img_size.width*scale);
    mx_uint channels = img.channels();

    Mat resized_img;
    cv::resize(img, resized_img, cv::Size(ws,hs));

    /* 1 now rebind the network, get predict handle */
    std::vector<mx_uint> input_shape = {1, channels, hs, ws};

    PredictorHandle pred_hnd = 0;
    BindPredictor((const char*)pnet_json_.GetBuffer(),
                  (const char*)pnet_param_.GetBuffer(),
                  static_cast<size_t>(pnet_param_.GetLength()),
                  input_shape,
                  dev_type_,
                  dev_id_,
                  pred_hnd);
    assert(pred_hnd);

    /* 2 prepare data for network input */
    vector<Mat> input_images;
    input_images.push_back(resized_img);
    std::vector<mx_float> out_data;
    ConvertMats2Data(input_images, out_data, 127.5, 0.0078125, cv::Size(0,0), nullptr);

    MXPredSetInput(pred_hnd, "data", out_data.data(), out_data.size());

    /* 3 Do Predict Forward */
    MXPredForward(pred_hnd);

    // 4 Get Output Result
    int output_size = 0;

    /*bbox_pred1, cls_prob1, bbox_pred2, cls_prob2*/
    vector<mx_uint> output_index = {0, 1, 2, 3};
    vector<mx_uint> bbox_shapes1;
    std::vector<float> bbox_data1;
    GetOutput(pred_hnd, output_index[0], bbox_shapes1, bbox_data1, output_size);

    vector<mx_uint> cls_shapes1;
    std::vector<float> cls_data1;
    GetOutput(pred_hnd, output_index[1], cls_shapes1, cls_data1, output_size);

    vector<mx_uint> bbox_shapes2;
    std::vector<float> bbox_data2;
    GetOutput(pred_hnd, output_index[2], bbox_shapes2, bbox_data2, output_size);

    vector<mx_uint> cls_shapes2;
    std::vector<float> cls_data2;
    GetOutput(pred_hnd, output_index[3], cls_shapes2, cls_data2, output_size);

    MXPredFree(pred_hnd);

    /* generator box */
    vector<BoundingBox> bbox1, bbox2;
    generate_box(cls_data1, cls_shapes1, bbox_data1, bbox_shapes1, threshold_[0], scale, 2, 12, bbox1);
    generate_box(cls_data2, cls_shapes2, bbox_data2, bbox_shapes2, threshold_[1], scale, 4, 18, bbox2);

    /* nms */
    nms(bbox1, 0.5, 'u');
    nms(bbox2, 0.5, 'u');

    bbox1.insert(bbox1.end(), bbox2.begin(), bbox2.end());
    return bbox1;
}

vector<BoundingBox> MtcnnDetector::detect_face(const Mat &input_image){
    const float MIN_IMAGE_SIZE = 18.0; /* this is set by the model, DO NOT CHANGE it */

    if(input_image.empty() || input_image.rows < MIN_IMAGE_SIZE ||
       input_image.cols < MIN_IMAGE_SIZE || input_image.channels() != 3){
        cerr<<"Input image wrong in function detect_face, please check "<<endl;
        return vector<BoundingBox>();
    }

    vector<float> scales;
    float minl = std::min(input_image.cols, input_image.rows);
    float m = MIN_IMAGE_SIZE/minsize_;
    minl *= m;
    float factor_count = 0;
    while(minl > MIN_IMAGE_SIZE){
        scales.push_back(m*std::pow(factor_, factor_count++));
        minl *= factor_;
    }

    /* run pnet detection */
    vector<BoundingBox> total_boxes;
    for( auto &scale: scales){
        vector<BoundingBox> results = detect_pnet(input_image, scale);
        total_boxes.insert(total_boxes.end(), results.begin(), results.end());
    }

    if(total_boxes.empty())
        return total_boxes;

    /* merge detections before rnet and onet */
    // nms(total_boxes, 0.7, 'u');

    /* refine the dets */
    calibrate_box(total_boxes);
    convert_to_square(total_boxes);
    nms(total_boxes, 0.7, 'u');

    /* run rnet detection */
    detect_rnet(input_image, total_boxes);

    /* run onet detection */
    detect_onet(input_image, total_boxes);

    return total_boxes;
}

void MtcnnDetector::detect_rnet(const Mat &img,
                                vector<BoundingBox> &boxes){
    if(boxes.empty())
        return;

    /* run rnet detection*/

    /* face images list */
    vector<Mat> in_data_rnet;
    pad(boxes, img, in_data_rnet);

    /* input dim for rnet */
    const mx_uint num_det = static_cast<mx_uint>(in_data_rnet.size());
    const mx_uint channels = 3;
    const mx_uint hs = 18;
    const mx_uint ws = 18;

    /* 1 now rebind the network, get predict handle */
    std::vector<mx_uint> input_shape = {num_det,
                                        channels,
                                        hs,
                                        ws};

    PredictorHandle pred_hnd = 0;
    BindPredictor((const char*)rnet_json_.GetBuffer(),
                  (const char*)rnet_param_.GetBuffer(),
                  static_cast<size_t>(rnet_param_.GetLength()),
                  input_shape,
                  dev_type_,
                  dev_id_,
                  pred_hnd);
    assert(pred_hnd);

    /* 2 prepare data for network input */
    std::vector<mx_float> out_data;
    ConvertMats2Data(in_data_rnet, out_data, 127.5, 0.0078125, cv::Size(ws,hs), nullptr);

    MXPredSetInput(pred_hnd, "data", out_data.data(), out_data.size());

    /* 3 Do Predict Forward */
    MXPredForward(pred_hnd);

    // 4 Get Output Result
    int output_size = 0;

    /*bbox_pred, cls_prob*/
    vector<mx_uint> output_index = {0, 1};
    vector<mx_uint> bbox_shape;
    std::vector<float> bbox_data;
    GetOutput(pred_hnd, output_index[0], bbox_shape, bbox_data, output_size);

    vector<mx_uint> cls_shape;
    std::vector<float> cls_data;
    GetOutput(pred_hnd, output_index[1], cls_shape, cls_data, output_size);

    MXPredFree(pred_hnd);

    /* update the detections with new confidence score */
    vector<BoundingBox> updated_boxes;
    const float *cls_ptr = &cls_data[0];
    const float *bbox_ptr = &bbox_data[0];
    for( mx_uint i=0;i<num_det;i++){
        const float det_score = cls_ptr[2*i+1];
        if(det_score < threshold_[2])
            continue;
        /* update score and bbox reg */
        BoundingBox b=boxes[i];
        b.score = det_score;
        b.reg = BBoxReg(bbox_ptr[i*4], bbox_data[i*4+1], bbox_data[i*4+2], bbox_data[i*4+3]);
        updated_boxes.push_back(b);
    }

    /* pre nms */
    nms(updated_boxes, 0.7, 'u');

    /* refine the detection */
    calibrate_box(updated_boxes);
    convert_to_square(updated_boxes);

    /* post nms */
    nms(updated_boxes, 0.7, 'u');
    boxes = updated_boxes;
}

void MtcnnDetector::detect_onet(const Mat &img,
                                vector<BoundingBox> &boxes){
    if(boxes.empty())
        return;

    /* run rnet detection*/
    vector<Mat> in_data_onet;
    pad(boxes, img, in_data_onet);

    /* input dim for rnet */
    const mx_uint num_det = static_cast<mx_uint>(in_data_onet.size());
    const mx_uint channels = 3;
    const mx_uint hs = 36;
    const mx_uint ws = 36;

    /* 1 now rebind the network, get predict handle */
    std::vector<mx_uint> input_shape = {num_det,
                                        channels,
                                        hs,
                                        ws};

    PredictorHandle pred_hnd = 0;
    BindPredictor((const char*)onet_json_.GetBuffer(),
                  (const char*)onet_param_.GetBuffer(),
                  static_cast<size_t>(onet_param_.GetLength()),
                  input_shape,
                  dev_type_,
                  dev_id_,
                  pred_hnd);
    assert(pred_hnd);

    /* 2 prepare data for network input */
    std::vector<mx_float> out_data;
    ConvertMats2Data(in_data_onet, out_data, 127.5, 0.0078125, cv::Size(ws,hs), nullptr);

    MXPredSetInput(pred_hnd, "data", out_data.data(), out_data.size());

    /* 3 Do Predict Forward */
    MXPredForward(pred_hnd);

    // 4 Get Output Result
    int output_size = 0;

    /*bbox_pred, cls_prob*/
    vector<mx_uint> output_index = {0, 1};
    vector<mx_uint> bbox_shape;
    std::vector<float> bbox_data;
    GetOutput(pred_hnd, output_index[0], bbox_shape, bbox_data, output_size);

    vector<mx_uint> cls_shape;
    std::vector<float> cls_data;
    GetOutput(pred_hnd, output_index[1], cls_shape, cls_data, output_size);

    MXPredFree(pred_hnd);

    /* update the detections with new confidence score */
    vector<BoundingBox> updated_boxes;
    const float *cls_ptr = &cls_data[0];
    const float *bbox_ptr = &bbox_data[0];
    for( mx_uint i=0;i<num_det;i++){
        const float det_score = cls_ptr[2*i+1];
        if(det_score < threshold_[2])
            continue;
        /* update score and bbox reg */
        BoundingBox b=boxes[i];
        b.score = det_score;
        b.reg = BBoxReg(bbox_ptr[i*4], bbox_data[i*4+1], bbox_data[i*4+2], bbox_data[i*4+3]);
        updated_boxes.push_back(b);
    }

    /* refine the detection */
    calibrate_box(updated_boxes);

    /* post nms */
    nms(updated_boxes, 0.7, 'm');
    boxes = updated_boxes;
}

void MtcnnDetector::convert_to_square(vector<BoundingBox> &boxes){
    for( auto &box:boxes){
        float w = box.width();
        float h = box.height();
        float max_side = std::max(w, h);
        box.x1 = box.x1+w*0.5-max_side*0.5;
        box.y1 = box.y1+h*0.5-max_side*0.5;
        box.x2 = box.x1+max_side-1;
        box.y2 = box.y1+max_side-1;
        box.round();
    }
}

void MtcnnDetector::pad( vector<BoundingBox> &boxes,
                         const Mat &input_image,
                         vector<Mat> &box_images){
    box_images.clear();
    int w = input_image.cols;
    int h = input_image.rows;

    for( auto &box:boxes){
        float tmpw = box.width();
        float tmph = box.height();
        float dx=0, dy=0, edx=box.width()-1, edy=box.height()-1;
        if(box.x1<0){
            dx = 0 - box.x1;
            box.x1 = 0;
        }
        if(box.y1<0){
            dy = 0 - box.y1;
            box.y1 = 0;
        }
        if(box.x2>w-1){
            edx = tmpw+w-2-box.x2;
            box.x2 = w-1;
        }
        if(box.y2>h-1){
            edy = tmph+h-2-box.y2;
            box.y2 = h-1;
        }
        Mat tmp_target(int(tmph), int(tmpw), CV_8UC3);
        cv::Rect ori_roi(box.x1, box.y1, box.x2-box.x1+1, box.y2-box.y1+1);
        cv::Rect target_roi(dx, dy, edx-dx+1, edy-dy+1);

        input_image(ori_roi).copyTo(tmp_target(target_roi));
        box_images.push_back(tmp_target);
    }
}

void MtcnnDetector::calibrate_box(vector<BoundingBox> &boxes){
    for( auto &box:boxes){
        float width = box.width();
        float height = box.height();
        box.x1 += box.reg.x1_shift*width;
        box.y1 += box.reg.y1_shift*height;
        box.x2 += box.reg.x2_shift*width;
        box.y2 += box.reg.y2_shift*height;
    }
}

vector<int> MtcnnDetector::nms(vector<BoundingBox> &bbox,
                                       const float threshold,
                                       const char method){
    vector<int> pick_index;
    vector<BoundingBox> bbox_nms;
    std::sort(bbox.begin(),
              bbox.end(),
              [&](BoundingBox a, BoundingBox b){ return a.score>b.score;});

    int num_bbox = bbox.size();
    int select_idx = 0;
    vector<int> mask_merged(num_bbox,0);
    vector<float> areas;
    for( auto &i:bbox)
        areas.push_back(i.area());

    while(true){
        while(select_idx<num_bbox && mask_merged[select_idx]==1)
            select_idx++;
        if(select_idx == num_bbox) /* done */
            break;

        /* choose the next detection */
        mask_merged[select_idx] = 1;
        pick_index.push_back(select_idx);

        BoundingBox chosen_one = bbox[select_idx];
        bbox_nms.push_back(chosen_one);

        /* remove all the detection overlap with it*/
        for(auto i=select_idx; i<num_bbox;i++){
            if(mask_merged[i] == 1)
                continue;

            BoundingBox intersection(std::max(chosen_one.x1, bbox[i].x1),
                                     std::max(chosen_one.y1, bbox[i].y1),
                                     std::min(chosen_one.x2, bbox[i].x2),
                                     std::min(chosen_one.y2, bbox[i].y2),
                                     0.0,
                                     BBoxReg());
            if(intersection.width() < 0 || intersection.height()<0)
                continue;

            float inter_area = intersection.area();
            float chosen_area = chosen_one.area();
            float process_area = bbox[i].area();
            switch(method){
                case 'u':
                    if(inter_area/(chosen_area+process_area-inter_area) > threshold )
                        mask_merged[i] = 1;
                    break;
                case 'm':
                    if(inter_area/std::min(chosen_area,process_area) > threshold )
                        mask_merged[i] = 1;
                    break;
                default:
                    break;
            }
        }
    }
    bbox = bbox_nms; /* switch */
    return pick_index;
}

void MtcnnDetector::draw_bbox(const Mat &input_img,
                              const vector<BoundingBox> &bbox,
                              int timeout){
    Mat draw;
    input_img.copyTo(draw);
    for( auto &item:bbox) {
        cv::rectangle(draw,
                      cv::Point(item.x1, item.y1),
                      cv::Point(item.x2, item.y2),
                      cv::Scalar(255,0,0),
                      2);
        //cout<<item.x1<<","<<item.y1<<","<<item.x2<<","<<item.y2<<","<<endl;
        cout<<item.status<<endl;
    }
    cv::imshow("show", draw);
    cv::waitKey(timeout);
}

void  MtcnnDetector::generate_box(vector<float> &cls_data,
                                  const vector<mx_uint> &cls_shape,
                                  vector<float> &bbox_data,
                                  const vector<mx_uint> &bbox_shape,
                                  const float threshold,
                                  const float scale,
                                  const int stride,
                                  const int cellsize,
                                  vector<BoundingBox> &dets){
    if(cls_data.empty() || cls_shape.empty() || bbox_data.empty() || bbox_shape.empty())
        return;

    /* assertion */
    assert(cls_shape[0] == bbox_shape[0] && cls_shape[2] == bbox_shape[2] && cls_shape[3] == bbox_shape[3]);

    /* feature map dims */
    int f_height = cls_shape[2];
    int f_width = cls_shape[3];
    int f_size = f_height*f_width;

    /* extract data with no memory copy, skip the neg channel , only need positive channel */
    Mat cls_map(f_height, f_width, CV_32FC1, &cls_data[0]+f_size);

    dets.clear();

    /* scan the image for high confidence detection */
    const float *reg_data = &bbox_data[0];
    for(auto y=0;y<cls_map.rows;y++){
        const float *d_ptr = cls_map.ptr<float>(y);
        for(auto x=0;x<cls_map.cols;x++){
            if(d_ptr[x] > threshold){
                BBoxReg reg(reg_data[y*f_width+x],
                            reg_data[f_size + y*f_width+x],
                            reg_data[2*f_size + y*f_width+x],
                            reg_data[3*f_size + y*f_width+x]);

                BoundingBox b((x*stride)/scale,
                              (y*stride)/scale,
                              (x*stride+cellsize)/scale,
                              (y*stride+cellsize)/scale,
                              d_ptr[x],
                              reg);
                dets.push_back(b);
            }
        }
    }
}

vector<BoundingBox> MtcnnDetector::detect_face2(const Mat &input_image, bool do_rotation){

    const float MIN_IMAGE_SIZE = 18.0; /* this is set by the model, DO NOT CHANGE it */

    if(input_image.empty() || input_image.rows < MIN_IMAGE_SIZE ||
       input_image.cols < MIN_IMAGE_SIZE || input_image.channels() != 3) {
        cerr<<"Input image wrong in function detect_face, please check "<<endl;
        //return vector<NewRegRes>();
        return vector<BoundingBox>();
    }

    vector<float> scales;
    float minl = std::min(input_image.cols, input_image.rows);
    float m = MIN_IMAGE_SIZE/minsize_;
    minl *= m;
    float factor_count = 0;
    while(minl > MIN_IMAGE_SIZE){
        scales.push_back(m*std::pow(factor_, factor_count++));
        minl *= factor_;
    }

    if(scales.empty()){
        cerr<<"image size is too small"<<endl;
        return vector<BoundingBox>();
    }

    //tmp result list
    vector<NewRegRes> ret_list;
    float min_scale = *(scales.rbegin());

    // no rotation
    vector<BoundingBox> total_boxes;
    for(auto &scale: scales){
        vector<BoundingBox> results = detect_pnet(input_image, scale);
        if(results.size() > 0){
            total_boxes.insert(total_boxes.end(), results.begin(), results.end());
        }
    }

    // push original image and its bboxes 2 the list
    if(total_boxes.size() > 0){
        NewRegRes nrr(input_image, ROTATION_NONE, total_boxes);
        ret_list.push_back(nrr);
    }

    ///////////////////////////////////////////////////////////////////////////////////pnet on original image finished

    cv::Mat rot_mat_cw30(2, 3, CV_32FC1);
    cv::Mat rot_mat_cc30(2, 3, CV_32FC1);

    if(do_rotation) {

        // clockwise 30
        cv::Point center = cv::Point(input_image.cols/2, input_image.rows/2);
        rot_mat_cw30 = getRotationMatrix2D(center, -30, 1.0);
        cv::Mat r1_img;
        warpAffine(input_image, r1_img, rot_mat_cw30, input_image.size());

        vector<BoundingBox> total_boxes_r1 = detect_pnet(r1_img, min_scale);
        if(total_boxes_r1.size() > 0){
            NewRegRes nrr1(r1_img, ROTATION_CW_30, total_boxes_r1);
            ret_list.push_back(nrr1);
        }

        // counter clockwise 30
        rot_mat_cc30 = getRotationMatrix2D(center, 30, 1.0);
        cv::Mat r2_img;
        warpAffine(input_image, r2_img, rot_mat_cc30, input_image.size());

        vector<BoundingBox> total_boxes_r2 = detect_pnet(r2_img, min_scale);
        if(total_boxes_r2.size() > 0){
            NewRegRes nrr2(r2_img, ROTATION_CC_30, total_boxes_r2);
            ret_list.push_back(nrr2);
        }

        // add border
        int h_pad = input_image.rows/2;
        int w_pad = input_image.cols/2;
        cv::Mat large_img;
        copyMakeBorder(input_image, large_img, h_pad, h_pad, w_pad, w_pad, cv::BORDER_CONSTANT, cv::Scalar(128, 128, 128));

        vector<float> scales;
        float minl = std::min(large_img.cols, large_img.rows);
        float m = MIN_IMAGE_SIZE/minsize_;
        minl *= m;
        float factor_count = 0;
        while(minl > MIN_IMAGE_SIZE){
            scales.push_back(m*std::pow(factor_, factor_count++));
            minl *= factor_;
        }
/*
        vector<BoundingBox> total_boxes_r3;
        for(auto &scale: scales){
            vector<BoundingBox> results = detect_pnet(large_img, scale);
            if(results.size() > 0){
                total_boxes_r3.insert(total_boxes_r3.end(), results.begin(), results.end());
            }
        }
*/
        float min_scale = *(scales.rbegin());
        vector<BoundingBox> total_boxes_r3 = detect_pnet(large_img, min_scale);
        if(total_boxes_r3.size() > 0){
            NewRegRes nrr3(large_img, ROTATION_NONE_PADDING, total_boxes_r3);
            ret_list.push_back(nrr3);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////pnet on rotated image finished

//    // refine the dets
//    for(unsigned int i = 0; i < ret_list.size(); i++)
//    {
//        calibrate_box(ret_list[i].dets);
//        convert_to_square(ret_list[i].dets);
//        nms(ret_list[i].dets, 0.7, 'u');
//
//        // run rnet detection
//        detect_rnet(ret_list[i].img, ret_list[i].dets);
//        if(ret_list[i].dets.empty())
//        {
//            ret_list[i].enabled = false;
//            continue;
//        }
//
//        // run onet detection
//        detect_onet(ret_list[i].img, ret_list[i].dets);
//        if(ret_list[i].dets.empty())
//        {
//            ret_list[i].enabled = false;
//            continue;
//        }
//    }


    // refine the dets
    // update rnet and onet to rnet2 and onet2, 20170505
    for(unsigned int i = 0; i < ret_list.size(); i++)
    {
        calibrate_box(ret_list[i].dets);
        convert_to_square(ret_list[i].dets);
        //nms(ret_list[i].dets, 0.7, 'u');
    }

    detect_rnet2(ret_list);
    detect_onet2(ret_list);

    ///////////////////////////////////////////////////////////////////////////////////rnet and onet on all images finished

    vector<BoundingBox> bboxes_in_org;
    for(unsigned int i = 0; i < ret_list.size(); i++)
    {
        if(ret_list[i].enabled)
        {
            switch(ret_list[i].status)
            {
               case ROTATION_NONE:
                    for(vector<BoundingBox>::iterator it = ret_list[i].dets.begin(); it != ret_list[i].dets.end(); it++)
                    {
                        BoundingBox bb = *it;
                        cv::Rect rect_org(bb.x1, bb.y1, bb.width(), bb.height());
                        (*it).rect_transformed = rect_org;
                        (*it).status = ROTATION_NONE;

                        bboxes_in_org.push_back(*(it));
                    }
                    break;
                case ROTATION_CW_30:
                    for(vector<BoundingBox>::iterator it = ret_list[i].dets.begin(); it != ret_list[i].dets.end(); it++)
                    {
                        //获取旋转图像上检测框的4个点坐标，存储在pts_in_rotated
                        BoundingBox bb = *it;
                        cv::Point p1(bb.x1, bb.y1);
                        cv::Point p2(bb.x1+bb.width(), bb.y1);
                        cv::Point p3(bb.x1+bb.width(), bb.y1+bb.height());
                        cv::Point p4(bb.x1, bb.y1+bb.height());
                        vector<cv::Point> pts_in_rotated;
                        pts_in_rotated.push_back(p1);
                        pts_in_rotated.push_back(p2);
                        pts_in_rotated.push_back(p3);
                        pts_in_rotated.push_back(p4);
                        //计算这4个点在原图上的坐标，存储在pts_in_original
                        vector<cv::Point> pts_in_original;
                        cv::transform(pts_in_rotated, pts_in_original, rot_mat_cc30);
                        //计算原图上的包围盒，并进行修正
                        int min_x = 1000000;
                        int min_y = 1000000;
                        int max_x = -100000;
                        int max_y = -100000;
                        for(vector<cv::Point>::iterator it2 = pts_in_original.begin(); it2 != pts_in_original.end(); it2++)
                        {
                            cv::Point ppt = *it2;
                            if(ppt.x < min_x) min_x = ppt.x;
                            if(ppt.x > max_x) max_x = ppt.x;
                            if(ppt.y < min_y) min_y = ppt.y;
                            if(ppt.y > max_y) max_y = ppt.y;
                        }

                        /**inplace update*/
                        cv::Rect rect_rot(bb.x1, bb.y1, bb.width(), bb.height());
                        float dx = fabs(max_x-min_x-bb.width())/2;
                        float dy = fabs(max_y-min_y-bb.height())/2;
                        (*it).x1 = min_x+dx;  /**在原图上的坐标*/
                        (*it).y1 = min_y+dy;
                        (*it).x2 = max_x-dx;
                        (*it).y2 = max_y-dy;
                        (*it).status = ROTATION_CW_30;
                        (*it).rect_transformed = rect_rot;  /**在旋转后的图像上的坐标*/

                        bboxes_in_org.push_back(*(it));
                    }
                    break;

                case ROTATION_CC_30:
                    for(vector<BoundingBox>::iterator it = ret_list[i].dets.begin(); it != ret_list[i].dets.end(); it++)
                    {
                        BoundingBox bb = *it;
                        cv::Point p1(bb.x1, bb.y1);
                        cv::Point p2(bb.x1+bb.width(), bb.y1);
                        cv::Point p3(bb.x1+bb.width(), bb.y1+bb.height());
                        cv::Point p4(bb.x1, bb.y1+bb.height());
                        vector<cv::Point> pts_in_rotated;
                        pts_in_rotated.push_back(p1);
                        pts_in_rotated.push_back(p2);
                        pts_in_rotated.push_back(p3);
                        pts_in_rotated.push_back(p4);
                        vector<cv::Point> pts_in_original;
                        cv::transform(pts_in_rotated, pts_in_original, rot_mat_cw30);
                        int min_x = 1000000;
                        int min_y = 1000000;
                        int max_x = -100000;
                        int max_y = -100000;
                        for(vector<cv::Point>::iterator it2 = pts_in_original.begin(); it2 != pts_in_original.end(); it2++)
                        {
                            cv::Point ppt = *it2;
                            if(ppt.x < min_x) min_x = ppt.x;
                            if(ppt.x > max_x) max_x = ppt.x;
                            if(ppt.y < min_y) min_y = ppt.y;
                            if(ppt.y > max_y) max_y = ppt.y;
                        }

                        /**inplace update*/
                        cv::Rect rect_rot(bb.x1, bb.y1, bb.width(), bb.height());
                        float dx = fabs(max_x-min_x-bb.width())/2;
                        float dy = fabs(max_y-min_y-bb.height())/2;
                        (*it).x1 = min_x+dx;  /**在原图上的坐标*/
                        (*it).y1 = min_y+dy;
                        (*it).x2 = max_x-dx;
                        (*it).y2 = max_y-dy;
                        (*it).status = ROTATION_CC_30;
                        (*it).rect_transformed = rect_rot;  /**在旋转后的图像上的坐标*/

                        bboxes_in_org.push_back(*(it));
                    }
                    break;

                case ROTATION_NONE_PADDING:
                    int h_pad = input_image.rows/2;
                    int w_pad = input_image.cols/2;
                    for(vector<BoundingBox>::iterator it = ret_list[i].dets.begin(); it != ret_list[i].dets.end(); it++)
                    {
                        BoundingBox bb = *it;
                        cv::Rect rect_pad(bb.x1, bb.y1, bb.width(), bb.height());
                        (*it).x1 = bb.x1 - w_pad;  /**在原图上的坐标*/
                        (*it).y1 = bb.y1 - h_pad;
                        (*it).x2 = bb.x2 - w_pad;
                        (*it).y2 = bb.y2 - h_pad;
                        (*it).status = ROTATION_NONE_PADDING;
                        (*it).rect_transformed = rect_pad;  /**在padding后的大图上的坐标*/

                        bboxes_in_org.push_back(*(it));
                    }
                    break;
            }
        }
    }

    if(bboxes_in_org.empty()){
        return vector<BoundingBox>();
    }

    nms(bboxes_in_org, 0.5, 'u');
    nms(bboxes_in_org, 0.7, 'm');

    //return ret_list;
    return bboxes_in_org;
}

void MtcnnDetector::detect_rnet2(vector<NewRegRes> &ret_list)
{
    if(ret_list.empty())
        return;

    /* face image list with corresponding status */
    vector<BoundingBox> boxes;
    vector<Mat> in_data_rnet;
    vector<ImgRotateStatus> in_data_status;
    vector<bool> in_data_enabled;

    /* initialize face icons and their labels */
    for(vector<NewRegRes>::iterator it = ret_list.begin(); it != ret_list.end(); it++)
    {
        NewRegRes nrr = *it;
        vector<BoundingBox> dets = nrr.dets;
        Mat img = nrr.img;
        ImgRotateStatus status = nrr.status;

        if(dets.empty())
            continue;

        vector<Mat> face_icons;
        pad(dets, img, face_icons);

        boxes.insert(boxes.end(), dets.begin(), dets.end());
        in_data_rnet.insert(in_data_rnet.end(), face_icons.begin(), face_icons.end());
        for(unsigned int i = 0; i < face_icons.size(); i++)
        {
            in_data_status.push_back(status);
            in_data_enabled.push_back(true);
        }
    }

    if(in_data_rnet.empty())
        return;
    if(boxes.size()!=in_data_rnet.size() || boxes.size()!=in_data_status.size() || boxes.size()!=in_data_enabled.size())
    {
        std::cerr<<"detect_rnet2 vector size doesn\'t match"<<std::endl;
        return;
    }

    /* input dim for rnet */
    const mx_uint num_det = static_cast<mx_uint>(in_data_rnet.size());
    const mx_uint channels = 3;
    const mx_uint hs = 18;
    const mx_uint ws = 18;

    /* 1 now rebind the network, get predict handle */
    std::vector<mx_uint> input_shape = {num_det,
                                        channels,
                                        hs,
                                        ws};

    PredictorHandle pred_hnd = 0;
    BindPredictor((const char*)rnet_json_.GetBuffer(),
                  (const char*)rnet_param_.GetBuffer(),
                  static_cast<size_t>(rnet_param_.GetLength()),
                  input_shape,
                  dev_type_,
                  dev_id_,
                  pred_hnd);
    assert(pred_hnd);

    /* 2 prepare data for network input */
    std::vector<mx_float> out_data;
    ConvertMats2Data(in_data_rnet, out_data, 127.5, 0.0078125, cv::Size(ws,hs), nullptr);

    MXPredSetInput(pred_hnd, "data", out_data.data(), out_data.size());

    /* 3 Do Predict Forward */
    MXPredForward(pred_hnd);

    /* 4 Get Output Result */
    int output_size = 0;

    /*bbox_pred, cls_prob*/
    vector<mx_uint> output_index = {0, 1};
    vector<mx_uint> bbox_shape;
    std::vector<float> bbox_data;
    GetOutput(pred_hnd, output_index[0], bbox_shape, bbox_data, output_size);

    vector<mx_uint> cls_shape;
    std::vector<float> cls_data;
    GetOutput(pred_hnd, output_index[1], cls_shape, cls_data, output_size);

    MXPredFree(pred_hnd);

    /* update the detections with new confidence score */
    vector<BoundingBox> updated_boxes;
    const float *cls_ptr = &cls_data[0];
    const float *bbox_ptr = &bbox_data[0];
    for(mx_uint i=0; i<num_det; i++)
    {
        const float det_score = cls_ptr[2*i+1];
        if(det_score < threshold_[2])
        {
            in_data_enabled[i] = false;
            //continue;
        }

        /* update score and bbox reg */
        BoundingBox b = boxes[i];
        b.score = det_score;
        b.reg = BBoxReg(bbox_ptr[i*4], bbox_data[i*4+1], bbox_data[i*4+2], bbox_data[i*4+3]);
        updated_boxes.push_back(b);
    }

    vector<BoundingBox> bboxes;
    for(unsigned int i = 0; i < in_data_enabled.size(); i++)
    {
        if(in_data_enabled[i])
        {
            BoundingBox box = updated_boxes[i];
            box.status = in_data_status[i];
            bboxes.push_back(box);
        }
    }

    /* pre nms */
    nms(bboxes, 0.7, 'u');

    /* refine the detection */
    calibrate_box(bboxes);
    convert_to_square(bboxes);

    /* post nms */
    nms(bboxes, 0.7, 'u');

    NewRegRes new_reg_res[4];
    for(unsigned int i = 0; i < bboxes.size(); i++)
    {
        BoundingBox bb = bboxes[i];
        new_reg_res[bb.status].dets.push_back(bb);
        new_reg_res[bb.status].status = bb.status;
    }

    vector<NewRegRes> new_nrr_list;
    for(int i = 0; i < 4; i++)
    {
        if(new_reg_res[i].dets.empty())
            continue;

        for(unsigned int j = 0; j < ret_list.size(); j++)
        {
            if(new_reg_res[i].status == ret_list[j].status)
            {
                new_reg_res[i].img = ret_list[j].img;
                new_nrr_list.push_back(new_reg_res[i]);
                break;
            }
        }
    }

    ret_list.swap(new_nrr_list);
}

void MtcnnDetector::detect_onet2(vector<NewRegRes> &ret_list)
{
   if(ret_list.empty())
        return;

    /* face image list with corresponding status */
    vector<BoundingBox> boxes;
    vector<Mat> in_data_rnet;
    vector<ImgRotateStatus> in_data_status;
    vector<bool> in_data_enabled;

    /* initialize face icons and their labels */
    for(vector<NewRegRes>::iterator it = ret_list.begin(); it != ret_list.end(); it++)
    {
        NewRegRes nrr = *it;
        vector<BoundingBox> dets = nrr.dets;
        Mat img = nrr.img;
        ImgRotateStatus status = nrr.status;

        if(dets.empty())
            continue;

        vector<Mat> face_icons;
        pad(dets, img, face_icons);

        boxes.insert(boxes.end(), dets.begin(), dets.end());
        in_data_rnet.insert(in_data_rnet.end(), face_icons.begin(), face_icons.end());
        for(unsigned int i = 0; i < face_icons.size(); i++)
        {
            in_data_status.push_back(status);
            in_data_enabled.push_back(true);
        }
    }

    if(in_data_rnet.empty())
        return;
    if(boxes.size()!=in_data_rnet.size() || boxes.size()!=in_data_status.size() || boxes.size()!=in_data_enabled.size())
    {
        std::cerr<<"detect_onet2 vector size doesn\'t match"<<std::endl;
        return;
    }

    /* input dim for rnet */
    const mx_uint num_det = static_cast<mx_uint>(in_data_rnet.size());
    const mx_uint channels = 3;
    const mx_uint hs = 36;
    const mx_uint ws = 36;

    /* 1 now rebind the network, get predict handle */
    std::vector<mx_uint> input_shape = {num_det,
                                        channels,
                                        hs,
                                        ws};

    PredictorHandle pred_hnd = 0;
    BindPredictor((const char*)onet_json_.GetBuffer(),
                  (const char*)onet_param_.GetBuffer(),
                  static_cast<size_t>(onet_param_.GetLength()),
                  input_shape,
                  dev_type_,
                  dev_id_,
                  pred_hnd);
    assert(pred_hnd);

    /* 2 prepare data for network input */
    std::vector<mx_float> out_data;
    ConvertMats2Data(in_data_rnet, out_data, 127.5, 0.0078125, cv::Size(ws,hs), nullptr);

    MXPredSetInput(pred_hnd, "data", out_data.data(), out_data.size());

    /* 3 Do Predict Forward */
    MXPredForward(pred_hnd);

    /* 4 Get Output Result */
    int output_size = 0;

    /*bbox_pred, cls_prob*/
    vector<mx_uint> output_index = {0, 1};
    vector<mx_uint> bbox_shape;
    std::vector<float> bbox_data;
    GetOutput(pred_hnd, output_index[0], bbox_shape, bbox_data, output_size);

    vector<mx_uint> cls_shape;
    std::vector<float> cls_data;
    GetOutput(pred_hnd, output_index[1], cls_shape, cls_data, output_size);

    MXPredFree(pred_hnd);

    /* update the detections with new confidence score */
    vector<BoundingBox> updated_boxes;
    const float *cls_ptr = &cls_data[0];
    const float *bbox_ptr = &bbox_data[0];
    for(mx_uint i=0; i<num_det; i++)
    {
        const float det_score = cls_ptr[2*i+1];
        if(det_score < threshold_[2])
        {
            in_data_enabled[i] = false;
            //continue;
        }

        /* update score and bbox reg */
        BoundingBox b = boxes[i];
        b.score = det_score;
        b.reg = BBoxReg(bbox_ptr[i*4], bbox_data[i*4+1], bbox_data[i*4+2], bbox_data[i*4+3]);
        updated_boxes.push_back(b);
    }

    vector<BoundingBox> bboxes;
    for(unsigned int i = 0; i < in_data_enabled.size(); i++)
    {
        if(in_data_enabled[i])
        {
            BoundingBox box = updated_boxes[i];
            box.status = in_data_status[i];
            bboxes.push_back(box);
        }
    }

    /* refine the detection */
    calibrate_box(bboxes);

    /* post nms */
    nms(bboxes, 0.7, 'm');

    NewRegRes new_reg_res[4];
    for(unsigned int i = 0; i < bboxes.size(); i++)
    {
        BoundingBox bb = bboxes[i];
        new_reg_res[bb.status].dets.push_back(bb);
        new_reg_res[bb.status].status = bb.status;
    }

    vector<NewRegRes> new_nrr_list;
    for(int i = 0; i < 4; i++)
    {
        if(new_reg_res[i].dets.empty())
            continue;

        for(unsigned int j = 0; j < ret_list.size(); j++)
        {
            if(new_reg_res[i].status == ret_list[j].status)
            {
                new_reg_res[i].img = ret_list[j].img;
                new_nrr_list.push_back(new_reg_res[i]);
                break;
            }
        }
    }

    ret_list.swap(new_nrr_list);
}

}
