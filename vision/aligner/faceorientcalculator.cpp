#include "faceorientcalculator.h"

const static cv::Point3f P3D_SELLION(0., 0.,0.);
const static cv::Point3f P3D_LEFT_EYE(-65.5,-5.,20.);
const static cv::Point3f P3D_RIGHT_EYE(65.5,-5.,20.);
const static cv::Point3f P3D_LEFT_EAR(-77.5,-6.,100.);
const static cv::Point3f P3D_RIGHT_EAR(77.5,-6.,100.);
const static cv::Point3f P3D_NOSE(0., -48.0,-21.0);
const static cv::Point3f P3D_STOMMION(0., -75.0,-10.0);
const static cv::Point3f P3D_MENTON(0.,-133.0,0.);

enum FACIAL_FEATURE {
    NOSE=30,
    RIGHT_EYE=36,
    LEFT_EYE=45,
    RIGHT_SIDE=0,
    LEFT_SIDE=16,
    EYEBROW_RIGHT=21,
    EYEBROW_LEFT=22,
    MOUTH_UP=51,
    MOUTH_DOWN=57,
    MOUTH_RIGHT=48,
    MOUTH_LEFT=54,
    SELLION=27,
    MOUTH_CENTER_TOP=62,
    MOUTH_CENTER_BOTTOM=66,
    MENTON=8
};
float fL = 10000.0f;
float imWidth = 640.0f;
float imHeight = 480.0f;
cv::Mat cameraMatrix = (cv::Mat_<float>(3,3) << fL, 0, imHeight/2, 0, fL, imWidth/2, 0, 0, 1);

FaceOrientCalculator::FaceOrientCalculator()
{
    head_points.push_back(P3D_SELLION);
    head_points.push_back(P3D_RIGHT_EYE);
    head_points.push_back(P3D_LEFT_EYE);
    head_points.push_back(P3D_RIGHT_EAR);
    head_points.push_back(P3D_LEFT_EAR);
    head_points.push_back(P3D_MENTON);
    head_points.push_back(P3D_NOSE);
    head_points.push_back(P3D_STOMMION);
}

FaceOrientCalculator::~FaceOrientCalculator()
{
    std::vector<cv::Point3f> ept;
    head_points.clear();
    head_points.swap(ept);
}

cv::Vec3d FaceOrientCalculator::get_orient(std::vector<cv::Point2f> &points68)
{
    std::vector<cv::Point2f> detected_points;
    detected_points.push_back(points68[SELLION]);
    detected_points.push_back(points68[RIGHT_EYE]);
    detected_points.push_back(points68[LEFT_EYE]);
    detected_points.push_back(points68[RIGHT_SIDE]);
    detected_points.push_back(points68[LEFT_SIDE]);
    detected_points.push_back(points68[MENTON]);
    detected_points.push_back(points68[NOSE]);

    auto stomion = (points68[MOUTH_CENTER_TOP] + points68[MOUTH_CENTER_BOTTOM]) * 0.5;
    detected_points.push_back(stomion);
    centerlize(detected_points);

    // Find the 3D pose of our head
    solvePnP(head_points, detected_points,
            cameraMatrix, cv::noArray(),
            rvec, tvec, false,
            cv::ITERATIVE);

    cv::Mat rotCamerMatrix;
    cv::Rodrigues(rvec, rotCamerMatrix);
    cv::Vec3d eulerAngles;
    getEulerAngles(rotCamerMatrix, eulerAngles);

    eulerAngles[0] *= 1.5;  // add by yuhao
    return eulerAngles;
}

void FaceOrientCalculator::getEulerAngles(cv::Mat &rotCamerMatrix, cv::Vec3d &eulerAngles)
{
    cv::Mat cameraMatrix,mRotMatrix,transVect,mRotMatrixX,mRotMatrixY,mRotMatrixZ;
    double* _r = rotCamerMatrix.ptr<double>();
    double projMatrix[12] = {   _r[0],_r[1],_r[2],0,
                                _r[3],_r[4],_r[5],0,
                                _r[6],_r[7],_r[8],0     };
    decomposeProjectionMatrix( cv::Mat(3,4,CV_64FC1,projMatrix),
                               cameraMatrix,
                               mRotMatrix,
                               transVect,
                               mRotMatrixX,
                               mRotMatrixY,
                               mRotMatrixZ,
                               eulerAngles);
}

void FaceOrientCalculator::centerlize(std::vector<cv::Point2f> &detected_points)
{
    cv::Point2f p1;
    p1.x = - detected_points[6].x + projection(0,2);
    p1.y = - detected_points[6].y + projection(1,2);
    for (unsigned int i=0;i<detected_points.size();i++)
    {
        detected_points[i].x += p1.x;
        detected_points[i].y += p1.y;
    }
}
