#include "npddetector.h"


NpdDetector::NpdDetector(std::string path){
    if (!Gab.LoadFrontalModel(path))
        cout<< "failed to load NPD detector"<<endl;
}

NpdDetector::~NpdDetector(){}

int NpdDetector::npd_detect(const cv::Mat &frameGRAY, int minFace, float threshold) {
    rects.clear();
    scores.clear();
    std::vector<cv::Rect> rects0;
    std::vector<float> scores0;
    std::vector<int> index0;

    index0 = Gab.DetectFace(frameGRAY,rects0,scores0,minFace);

    for (unsigned int i=0; i<index0.size(); i++) {
       if (scores0[index0[i]]>0.0) {
           rects.push_back(rects0[index0[i]]);
           scores.push_back(scores0[index0[i]]);
       }
    }
    return rects.size();
}

std::vector<cv::Rect> NpdDetector::getResults() {
    return rects;
}

