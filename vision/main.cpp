#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/opencv.h>
#include "detector/mtcnn_detector.h"
#include "aligner/aligner.h"
#include "predictor/gender.h"
#include "predictor/age.h"
#include "predictor/emotion.h"
#include "predictor/identify.h"

using namespace std;
using namespace cv;


/*
 * class for parameters' management
 */
class GlobalVariable
{
public:
    static int CAMERA_INDEX;
    static std::string MODEL_PATH;
    static std::string MODEL_JSON_IDENTIFY;
    static std::string MODEL_PARA_IDENTIFY;
    static std::string MODEL_JSON_GENDER;
    static std::string MODEL_PARA_GENDER;
    static std::string MODEL_JSON_AGE;
    static std::string MODEL_PARA_AGE;
    static std::string MODEL_JSON_EMOTION;
    static std::string MODEL_PARA_EMOTION;
    static std::string MODEL_FRONTAL;
    static std::string MODEL_SHAPE68;
    static std::string PATH_FACE_FEATURE_DB;
    static int DEVICE_TYPE_DETRCTOT;
    static int MIN_SIZE_FACE_DETECT;
    static std::vector<float> MTCNN_THRESHOLDS;
    static float MTCNN_FACTOR;
    static float FACE_SHARPNESS_THRESHOLD;
    static float FACE_ANGLE_THRESHOLD_X1;
    static float FACE_ANGLE_THRESHOLD_X2;
    static float FACE_ANGLE_THRESHOLD_Y1;
    static float FACE_ANGLE_THRESHOLD_Y2;
    static float FACE_ANGLE_THRESHOLD_Z1;
    static float FACE_ANGLE_THRESHOLD_Z2;
};

int GlobalVariable::CAMERA_INDEX = 0;
std::string GlobalVariable::MODEL_PATH = "models/";
std::string GlobalVariable::MODEL_JSON_IDENTIFY = "niubility_deploy-symbol.json";
std::string GlobalVariable::MODEL_PARA_IDENTIFY = "niubility_deploy-0001.params";
std::string GlobalVariable::MODEL_JSON_GENDER = "gender_0522-symbol.json";
std::string GlobalVariable::MODEL_PARA_GENDER = "gender_0522-0045.params";
std::string GlobalVariable::MODEL_JSON_AGE = "age_reg-symbol.json";
std::string GlobalVariable::MODEL_PARA_AGE = "age_reg-0100.params";
std::string GlobalVariable::MODEL_JSON_EMOTION = "emotion_deploy-symbol.json";
std::string GlobalVariable::MODEL_PARA_EMOTION = "emotion_deploy-0001.params";
std::string GlobalVariable::MODEL_FRONTAL = "frontal";
std::string GlobalVariable::MODEL_SHAPE68 = "dlib_pc_shape_predictor.dat";

// 离线的人脸特征库
std::string GlobalVariable::PATH_FACE_FEATURE_DB = "models/face.db";

/*
 * dev_type, 1: cpu, 2: gpu
 */
// 人脸检测运行设备
int GlobalVariable::DEVICE_TYPE_DETRCTOT = 2;

// 人脸检测最小尺寸
int GlobalVariable::MIN_SIZE_FACE_DETECT = 80;

// MTCNN算法参数
std::vector<float> GlobalVariable::MTCNN_THRESHOLDS = { 0.9f, 0.7f, 0.7f, 0.95f };
float GlobalVariable::MTCNN_FACTOR = 0.444f;

// 模糊度阈值
float GlobalVariable::FACE_SHARPNESS_THRESHOLD = 45;

// 人脸朝向阈值
float GlobalVariable::FACE_ANGLE_THRESHOLD_X1 = -30;
float GlobalVariable::FACE_ANGLE_THRESHOLD_X2 = 30;
float GlobalVariable::FACE_ANGLE_THRESHOLD_Y1 = -30;
float GlobalVariable::FACE_ANGLE_THRESHOLD_Y2 = 30;
float GlobalVariable::FACE_ANGLE_THRESHOLD_Z1 = -45;
float GlobalVariable::FACE_ANGLE_THRESHOLD_Z2 = 45;

float Min(const float &a, const float &b)
{
    return a < b ? a : b;
}

float Max(const float &a, const float &b)
{
    return a > b ? a : b;
}

// 修正mtcnn得到的人脸检测框
cv::Rect correctBoxRegion(const TuringOS::BoundingBox &box, const int &img_width, const int &img_height)
{
    float x1 = Max(box.x1, 0);
    float x2 = Min(box.x2, img_width);
    float y1 = Max(box.y1, 0);
    float y2 = Min(box.y2, img_height);
    cv::Rect rect(x1, y1, x2-x1, y2-y1);
    return rect;
}

// 检查人脸图像质量
bool checkValidFace(const TuringOS::FaceAttr &attr)
{
    if((!attr.passnpd) || (attr.sharpness < GlobalVariable::FACE_SHARPNESS_THRESHOLD))
    {
        return false;
    }
    if( attr.angle[0] >= GlobalVariable::FACE_ANGLE_THRESHOLD_X1 && attr.angle[0] <= GlobalVariable::FACE_ANGLE_THRESHOLD_X2 &&
        attr.angle[1] >= GlobalVariable::FACE_ANGLE_THRESHOLD_Y1 && attr.angle[1] <= GlobalVariable::FACE_ANGLE_THRESHOLD_Y2 &&
        attr.angle[2] >= GlobalVariable::FACE_ANGLE_THRESHOLD_Z1 && attr.angle[2] <= GlobalVariable::FACE_ANGLE_THRESHOLD_Z2 )
    {
        return true;
    }
    return false;
}

/*
 * NOTICE:
 * loadModel(int dev_type)
 * dev_type, 1: cpu, 2: gpu
 */
int main(int argc, char **argv)
{
	int ret_val = -1;

	// 初始化身份识别器
	TuringOS::IdentifyPredictor global_identify;
    global_identify.setParameter(GlobalVariable::MODEL_PATH,
                                 GlobalVariable::MODEL_JSON_IDENTIFY,
                                 GlobalVariable::MODEL_PARA_IDENTIFY, 128, 128, 3);
    ret_val = global_identify.loadModel(2);
    assert(global_identify.setIdStorage(GlobalVariable::PATH_FACE_FEATURE_DB));

	/*
    // 计算两幅已经裁好的人脸的相似度
	if(argc!=3)
		return -1;
    cout<<argv[1]<<endl;
    cout<<argv[2]<<endl;
    cv::Mat face1 = cv::imread(argv[1], IMREAD_COLOR);
    cv::Mat face2 = cv::imread(argv[2], IMREAD_COLOR);
    cout<<global_identify.getSimilarity(face1, face2)<<endl;
	return 0;
	*/

    // 初始化表情识别器
    TuringOS::EmotionPredictor emotion;
    emotion.setParameter(GlobalVariable::MODEL_PATH,
                         GlobalVariable::MODEL_JSON_EMOTION,
                         GlobalVariable::MODEL_PARA_EMOTION, 64, 64, 1);
    ret_val = emotion.loadModel(2);
    assert(ret_val > 0);

    // 初始化性别识别器
    TuringOS::GenderPredictor gender;
    gender.setParameter(GlobalVariable::MODEL_PATH,
                        GlobalVariable::MODEL_JSON_GENDER,
                        GlobalVariable::MODEL_PARA_GENDER, 64, 64, 1);
    ret_val = gender.loadModel(2);
    assert(ret_val > 0);

    // 初始化年龄识别器
    TuringOS::AgePredictor age;
    age.setParameter(GlobalVariable::MODEL_PATH,
                     GlobalVariable::MODEL_JSON_AGE,
                     GlobalVariable::MODEL_PARA_AGE, 111, 111, 1);
    ret_val = age.loadModel(2);
    assert(ret_val > 0);

    // 初始化人脸检测器
    TuringOS::MtcnnDetector detector(GlobalVariable::MIN_SIZE_FACE_DETECT,
                                     GlobalVariable::MTCNN_THRESHOLDS,
                                     GlobalVariable::MTCNN_FACTOR,
                                     GlobalVariable::DEVICE_TYPE_DETRCTOT,
                                     0);
    // 初始化人脸校正模块
    TuringOS::Aligner aligner(GlobalVariable::MODEL_PATH+GlobalVariable::MODEL_SHAPE68,
                              GlobalVariable::MODEL_PATH+GlobalVariable::MODEL_FRONTAL);

    // 初始化摄像头
    VideoCapture cap(GlobalVariable::CAMERA_INDEX);
    cv::Mat frame;
    char text2show[50];

    while(true)
    {
        cap >> frame;
        if(frame.empty())
        {
            cout<<"read frame failed, skip"<<endl;
            continue;
        }

        cv::resize(frame, frame, cv::Size(640, 480));

        // 检测人脸
        vector<TuringOS::BoundingBox> results = detector.detect_face2(frame, true);

        // 设置人脸校正输入图像
        aligner.setImage2(frame);

        for(unsigned int i = 0; i < results.size(); i++)
        {
            TuringOS::BoundingBox box = results[i];

			// 校正人脸，用于获取清晰度、朝向和人脸识别特征
            aligner.alignFace2(box.rect_transformed, 128, 0.27f, 0.1f, box.status);
            Mat aligned_face1 = aligner.getAligndFace2();
            // 获取人脸清晰度
            int sharpness = aligner.getSharpness();
            // 人脸是否通过NPD
            bool ispassnpd = aligner.checkNPD();
            // 获取人脸朝向
            vector<Point2f> p68 = aligner.get68Point2fs2(box.rect_transformed, box.status);
            Vec3d angle = aligner.getOrient(p68);
            float orients[3];
            orients[0] = angle[0];
            orients[1] = angle[1];
            orients[2] = angle[2];

            // 设置人脸属性
            Rect face_roi = correctBoxRegion(box, frame.cols, frame.rows);
            TuringOS::FaceAttr attr(face_roi, sharpness, ispassnpd, 0, "", orients);

            // 如果人脸质量满足要求，就进行识别，并绘制结果
            bool pass = checkValidFace(attr);
            if(pass)
            {
                // 身份识别
                TuringOS::FaceAttr id_attr = global_identify.getNearestId(aligned_face1, 0.65);
                attr.similarity = id_attr.similarity;
                attr.name = id_attr.name;
                // 表情识别
                aligner.alignFace2(box.rect_transformed, 64, 0.27f, 0.1f, box.status);
                Mat aligned_face2 = aligner.getAligndFace2();
                string emotion_result = emotion.getEmotion(aligned_face2);
                // 性别识别
                aligner.alignFace2(box.rect_transformed, 64, 0.27, 0.1f, box.status);
                Mat aligned_face3 = aligner.getAligndFace2();
                int gender_result = gender.getGender(aligned_face3);
                // 年龄识别
                aligner.alignFace2(box.rect_transformed, 111, 0.15f, 0.1f, box.status);
                Mat aligned_face4 = aligner.getAligndFace2();
                float age_result = age.getAge(aligned_face4);

                cout<<attr.name<<endl;
            	cv::rectangle(frame, attr.rect, cv::Scalar(0, 255, 0), 2);
                sprintf(text2show, "gender: %s", gender_result==0?"Female":"Male");
                putText(frame, text2show, cv::Point(attr.rect.x,attr.rect.y-85), CV_FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 255, 0));
                sprintf(text2show, "age: %d", (int)age_result);
                putText(frame, text2show, cv::Point(attr.rect.x,attr.rect.y-55), CV_FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 255, 0));
                sprintf(text2show, "emotion: %s", emotion_result.c_str());
                putText(frame, text2show, cv::Point(attr.rect.x,attr.rect.y-25), CV_FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 255, 0));
            }
            else
            {
            	cv::rectangle(frame, attr.rect, cv::Scalar(0, 0, 255), 2);
            }

            cv::imshow("align", aligned_face1);
            cv::waitKey(1);
        }

        cv::imshow("result", frame);
        cv::waitKey(5);
    }

	return 0;
}
