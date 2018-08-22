#include <opencv2/opencv.hpp>

using namespace cv;


//两个基本阈值，深度图像中，位于此范围之外的可视为：超出量程
#define SENSOR_UPPER_BOUND_VALUE (253)
#define SENSOR_LOWER_BOUND_VALUE (2)
#define IS_DEBUG (false)


//OTSU算法的目标函数
enum OTSU_OPT_FUN
{
	ORG_OPT, ADV_OPT
};

//保存双峰分布的结果
typedef struct OtsuRES
{
    int pivot;  //最佳切分点
    double e1;  //第1个峰的期望
    double e2;  //第2个峰的期望
}
OtsuRES;


typedef struct camera_param
{
     double f;
     double cu;
     double cv;
     double b;
     camera_param(double p1, double p2, double p3, double p4)
     {
        f = p1;
        cu = p2;
        cv = p3;
        b = p4;
     }
}
camera_param;


//接口函数
void getLocation(const Mat &img, Rect &rect, Point2f &location);

//得到深度像素值对应的实际距离数据
float getRealDist(uchar gray_depth);

//输入深度图和目标区域，返回目标区域的前景背景均值
OtsuRES getBodyDepth(const Mat &depth_image, Rect rct);

//参数分别为直方图，最小灰度，最大灰度，优化方法
OtsuRES TwoClassOtsu(unsigned int *hist, int left, int right, int method);

//计算输入深度图像的直方图，其中max_val和min_val为有效深度值的最大和最小值
unsigned int* calcuHistogram(Mat &input_img, int &max_val, int &min_val);
