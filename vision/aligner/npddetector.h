#ifndef NPDDETECTOR_H
#define NPDDETECTOR_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include "LearnGAB.hpp"

class NpdDetector
{
public:
    GAB Gab;
    std::vector<cv::Rect>  rects;
    std::vector<float> scores;

    NpdDetector(std::string path);

    ~NpdDetector();

    int npd_detect(const cv::Mat &frameGRAY, int minFace, float threshold);

    std::vector<cv::Rect> getResults();
};

#endif // NPDDETECTOR_H
