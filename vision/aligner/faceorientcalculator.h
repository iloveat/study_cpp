#ifndef FACEORIENTCALCULATOR_H
#define FACEORIENTCALCULATOR_H
#include <opencv2/opencv.hpp>

using namespace std;

class FaceOrientCalculator
{
public:
    cv::Matx33f projection;
    cv::Mat rvec, tvec;
    vector<cv::Point3f> head_points;

    FaceOrientCalculator();

    ~FaceOrientCalculator();

    cv::Vec3d get_orient(std::vector<cv::Point2f> &points68);

    void getEulerAngles(cv::Mat &rotCamerMatrix, cv::Vec3d &eulerAngles);

    void centerlize(std::vector<cv::Point2f> &detected_points);
};

#endif // FACEORIENTCALCULATOR_H
