#ifndef ALIGNER_H
#define ALIGNER_H

#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/opencv.h>
#include "faceorientcalculator.h"
#include "npddetector.h"


enum ImgRotateStatus
{
    ROTATION_NONE,  //original image without rotation
    ROTATION_CW_30,  //rotate the original image with 30 degrees in clockwise direction
    ROTATION_CC_30,  //rotate the original image with 30 degrees in counter clockwise direction
    ROTATION_NONE_PADDING  //pad in original image 2 detect very large faces
};

namespace TuringOS
{
    class Aligner
    {
    private:
        dlib::shape_predictor mShapePredictor;
        dlib::full_object_detection mShape;

        cv::Mat mAligndFace;
        cv::Mat mAligndFaceGray;
        cv::Mat mRotMat;

        FaceOrientCalculator *mFaceOrientCalculator;
        std::vector<cv::Point2f> mAligndFace68Points;
        NpdDetector *mNpdDetector;

        int mFaceDim;
        bool mIsImageSet;
        bool mIsShapeReady;

        cv::Mat mOriginalBGR;
        dlib::cv_image<dlib::bgr_pixel> mDlibOriginal;

        cv::Mat mRotCW30;
        dlib::cv_image<dlib::bgr_pixel> mDlibRotCW30;

        cv::Mat mRotCC30;
        dlib::cv_image<dlib::bgr_pixel> mDlibRotCC30;

        cv::Mat mPadPad;
        dlib::cv_image<dlib::bgr_pixel> mDlibPadPad;

        float mean_face_shape_x[51] =
        {
            0.000213256, 0.0752622, 0.18113, 0.29077, 0.393397, 0.586856, 0.689483, 0.799124,
            0.904991, 0.98004, 0.490127, 0.490127, 0.490127, 0.490127, 0.36688, 0.426036,
            0.490127, 0.554217, 0.613373, 0.121737, 0.187122, 0.265825, 0.334606, 0.260918,
            0.182743, 0.645647, 0.714428, 0.793132, 0.858516, 0.79751, 0.719335, 0.254149,
            0.340985, 0.428858, 0.490127, 0.551395, 0.639268, 0.726104, 0.642159, 0.556721,
            0.490127, 0.423532, 0.338094, 0.290379, 0.428096, 0.490127, 0.552157, 0.689874,
            0.553364, 0.490127, 0.42689
        };

        float mean_face_shape_y[51] =
        {
            0.106454, 0.038915, 0.0187482, 0.0344891, 0.0773906, 0.0773906, 0.0344891,
            0.0187482, 0.038915, 0.106454, 0.203352, 0.307009, 0.409805, 0.515625, 0.587326,
            0.609345, 0.628106, 0.609345, 0.587326, 0.216423, 0.178758, 0.179852, 0.231733,
            0.245099, 0.244077, 0.231733, 0.179852, 0.178758, 0.216423, 0.244077, 0.245099,
            0.780233, 0.745405, 0.727388, 0.742578, 0.727388, 0.745405, 0.780233, 0.864805,
            0.902192, 0.909281, 0.902192, 0.864805, 0.784792, 0.778746, 0.785343, 0.778746,
            0.784792, 0.824182, 0.831803, 0.824182
        };

    public:
        Aligner(const std::string &p68_path, const std::string &npd_path);
        ~Aligner();
    private:
        // version 1.0 functions
        void setImage(const cv::Mat &frame);
        bool alignFace(const cv::Rect &rect, const int &facedim, const float &padding, const float &tf);
        cv::Mat getAligndFace();
        std::vector<cv::Point2f> get68Point2fs(const cv::Rect &rect);
        std::vector<cv::Point2f> get68Point2fs();
    public:
        // version 2.0 functions
        void setImage2(const cv::Mat &frame);
        bool alignFace2(const cv::Rect &rect, const int &facedim, const float &padding, const float &tf, const int &rotate_status);
        cv::Mat getAligndFace2();
        std::vector<cv::Point2f> get68Point2fs2(const cv::Rect &rect, const int &rotate_status);
        std::vector<cv::Point2f> get68Point2fs2(const int &rotate_status);

        // commonly used functions
        bool checkNPD();
        cv::Vec3d getOrient(std::vector<cv::Point2f> &points68);
        cv::Vec3d getOrient(float* points68);
        float getSharpness();

    private:
        // version 1.0 functions
        bool predictShape(const cv::Rect &rect);
        cv::Mat align(const cv::Rect &rect, const int &facedim, const float &padding, const float &tf);
        // version 2.0 functions
        std::vector<cv::Point2f> get68Point2fs2();
        bool predictShape2(const cv::Rect &rect, const int &rotate_status);
        cv::Mat align2(const cv::Rect &rect, const int &facedim, const float &padding, const float &tf, const int &rotate_status);
        // commonly used functions
        inline cv::Rect dlib2cvRect(const dlib::rectangle &det);
        inline dlib::rectangle cvRect2dlib(const cv::Rect &rect);
        cv::Mat find_tform_between_points(cv::Mat &from_mat, cv::Mat &to_mat);
        cv::Vec2f p2line(const int &p1, const int &p2);
        cv::Vec4f getBound();
    };
}

#endif // ALIGNER_H
