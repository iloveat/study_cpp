#include "aligner.h"

using namespace TuringOS;


#define max(x,y) (((x)<(y))?(y):(x))
#define min(x,y) (((x)<(y))?(x):(y))

Aligner::Aligner(const std::string &p68_path, const std::string &npd_path)
{
    dlib::deserialize(p68_path) >> mShapePredictor;
    mFaceOrientCalculator = new FaceOrientCalculator();
    mNpdDetector = new NpdDetector(npd_path);
    mIsImageSet = false;
    mIsShapeReady = false;
}

Aligner::~Aligner()
{
    delete mFaceOrientCalculator;
    mFaceOrientCalculator = NULL;
    delete mNpdDetector;
    mNpdDetector = NULL;
}

// version 1.0 code, without dealing with rotated faces and very large faces
void Aligner::setImage(const cv::Mat &frame)
{
    mOriginalBGR = frame.clone();
    mDlibOriginal = dlib::cv_image<dlib::bgr_pixel>(mOriginalBGR);
    mIsImageSet = true;
    mIsShapeReady = false;
}

bool Aligner::alignFace(const cv::Rect &rect, const int &facedim, const float &padding, const float &tf)
{
    mFaceDim = facedim;
    /*
    if(padding < 0.27f)
    {
        cout<<"error: padding < 0.27f"<<endl;
        return false;
    }
    */

    mAligndFace = align(rect, facedim, padding, tf);

    std::vector<cv::Point2f> points68 = get68Point2fs();
    std::vector<cv::Point3f> points68_3d;
    for (int i = 0; i < 68; i++)
    {
        points68_3d.push_back(cv::Point3f(points68[i].x, points68[i].y, 1.0));
    }
    cv::transform(points68_3d, mAligndFace68Points, mRotMat);
    cvtColor(mAligndFace, mAligndFaceGray, cv::COLOR_BGR2GRAY);

    return true;
}

cv::Mat Aligner::align(const cv::Rect &rect, const int &facedim, const float &padding, const float &tf)
{
    predictShape(rect);
    cv::Mat from_mat = cv::Mat(51,2,CV_32F);
    cv::Mat to_mat = cv::Mat(51,2,CV_32F);
    for (int i = 17; i < 68; i++)
    {
        float x = (padding + mean_face_shape_x[i-17])/(2*padding+1)*facedim;
        float y = (padding + tf + mean_face_shape_y[i-17])/(2*padding+1)*facedim;
        from_mat.at<float>(i-17,0) = mShape.part(i).x();
        from_mat.at<float>(i-17,1) = mShape.part(i).y();
        to_mat.at<float>(i-17,0) = x;
        to_mat.at<float>(i-17,1) = y;
    }

    mRotMat = find_tform_between_points(from_mat, to_mat);
    cv::Mat aligned_face;
    cv::warpAffine(mOriginalBGR, aligned_face, mRotMat, cv::Size(facedim, facedim));
    return aligned_face;
}

bool Aligner::predictShape(const cv::Rect &rect)
{
    if(!mIsImageSet)
        return false;

    dlib::rectangle det = cvRect2dlib(rect);
    mShape = mShapePredictor(mDlibOriginal, det);

    mIsShapeReady = true;
    return true;
}

cv::Mat Aligner::getAligndFace()
{
    return mAligndFace;
}

std::vector<cv::Point2f> Aligner::get68Point2fs(const cv::Rect &rect)
{
    predictShape(rect);
    return get68Point2fs();
}

std::vector<cv::Point2f> Aligner::get68Point2fs()
{
    std::vector<cv::Point2f> points;
    for (int i = 0; i < 68; i++)
    {
        cv::Point2f p1;
        p1.x = mShape.part(i).x();
        p1.y = mShape.part(i).y();
        points.push_back(p1);
    }
    return points;
}


// version 2.0 code, dealing with rotated faces and very large faces
void Aligner::setImage2(const cv::Mat &frame)
{
    // original image
    mOriginalBGR = frame.clone();
    // clockwise 30
    cv::Point center = cv::Point(frame.cols/2, frame.rows/2);
    cv::Mat rot_mat_cw30 = getRotationMatrix2D(center, -30, 1.0);
    warpAffine(frame, mRotCW30, rot_mat_cw30, frame.size());
    // counter clockwise 30
    cv::Mat rot_mat_cc30 = getRotationMatrix2D(center, 30, 1.0);
    warpAffine(frame, mRotCC30, rot_mat_cc30, frame.size());
    // pad
    int h_pad = frame.rows/2;
    int w_pad = frame.cols/2;
    copyMakeBorder(frame, mPadPad, h_pad, h_pad, w_pad, w_pad, cv::BORDER_CONSTANT);

    mDlibOriginal = dlib::cv_image<dlib::bgr_pixel>(mOriginalBGR);
    mDlibRotCW30 = dlib::cv_image<dlib::bgr_pixel>(mRotCW30);
    mDlibRotCC30 = dlib::cv_image<dlib::bgr_pixel>(mRotCC30);
    mDlibPadPad = dlib::cv_image<dlib::bgr_pixel>(mPadPad);

    mIsImageSet = true;
    mIsShapeReady = false;
}

bool Aligner::alignFace2(const cv::Rect &rect, const int &facedim, const float &padding, const float &tf, const int &rotate_status)
{
    mFaceDim = facedim;
    /*
    if(padding < 0.27f)
    {
        cout<<"error: padding < 0.27f"<<endl;
        return false;
    }
    */

    mAligndFace = align2(rect, facedim, padding, tf, rotate_status);

    std::vector<cv::Point2f> points68 = get68Point2fs2();  //得到68点在（旋转，padding）图像上的坐标，用于校正人脸
    std::vector<cv::Point3f> points68_3d;
    for (int i = 0; i < 68; i++)
    {
        points68_3d.push_back(cv::Point3f(points68[i].x, points68[i].y, 1.0));
    }
    cv::transform(points68_3d, mAligndFace68Points, mRotMat);
    cvtColor(mAligndFace, mAligndFaceGray, cv::COLOR_BGR2GRAY);

    return true;
}

cv::Mat Aligner::align2(const cv::Rect &rect, const int &facedim, const float &padding, const float &tf, const int &rotate_status)
{
    predictShape2(rect, rotate_status);
    cv::Mat from_mat = cv::Mat(51,2,CV_32F);
    cv::Mat to_mat = cv::Mat(51,2,CV_32F);
    for (int i = 17; i < 68; i++)
    {
        float x = (padding + mean_face_shape_x[i-17])/(2*padding+1)*facedim;
        float y = (padding + tf + mean_face_shape_y[i-17])/(2*padding+1)*facedim;
        from_mat.at<float>(i-17,0) = mShape.part(i).x();
        from_mat.at<float>(i-17,1) = mShape.part(i).y();
        to_mat.at<float>(i-17,0) = x;
        to_mat.at<float>(i-17,1) = y;
    }

    mRotMat = find_tform_between_points(from_mat, to_mat);
    cv::Mat aligned_face;

    switch(rotate_status)
    {
        case ROTATION_NONE:
            cv::warpAffine(mOriginalBGR, aligned_face, mRotMat, cv::Size(facedim, facedim));
            break;
        case ROTATION_CW_30:
            cv::warpAffine(mRotCW30, aligned_face, mRotMat, cv::Size(facedim, facedim));
            break;
        case ROTATION_CC_30:
            cv::warpAffine(mRotCC30, aligned_face, mRotMat, cv::Size(facedim, facedim));
            break;
        case ROTATION_NONE_PADDING:
            cv::warpAffine(mPadPad, aligned_face, mRotMat, cv::Size(facedim, facedim));
            break;
        default:
            cv::warpAffine(mOriginalBGR, aligned_face, mRotMat, cv::Size(facedim, facedim));
            break;
    }

    return aligned_face;
}

bool Aligner::predictShape2(const cv::Rect &rect, const int &rotate_status)
{
    if(!mIsImageSet)
        return false;

    dlib::rectangle det = cvRect2dlib(rect);

    switch(rotate_status)
    {
        case ROTATION_NONE:
            mShape = mShapePredictor(mDlibOriginal, det);
            break;
        case ROTATION_CW_30:
            mShape = mShapePredictor(mDlibRotCW30, det);
            break;
        case ROTATION_CC_30:
            mShape = mShapePredictor(mDlibRotCC30, det);
            break;
        case ROTATION_NONE_PADDING:
            mShape = mShapePredictor(mDlibPadPad, det);
            break;
        default:
            mShape = mShapePredictor(mDlibOriginal, det);
            break;
    }

    mIsShapeReady = true;
    return true;
}

cv::Mat Aligner::getAligndFace2()
{
    return mAligndFace;
}

//得到68点在（旋转，padding）图像上的坐标，用于校正人脸
std::vector<cv::Point2f> Aligner::get68Point2fs2()
{
    std::vector<cv::Point2f> points;
    for (int i = 0; i < 68; i++)
    {
        cv::Point2f p1;
        p1.x = mShape.part(i).x();
        p1.y = mShape.part(i).y();
        points.push_back(p1);
    }
    return points;
}

//得到68点在原图上的坐标，用于显示
std::vector<cv::Point2f> Aligner::get68Point2fs2(const cv::Rect &rect, const int &rotate_status)
{
    predictShape2(rect, rotate_status);
    return get68Point2fs2(rotate_status);
}

//得到68点在原图上的坐标
std::vector<cv::Point2f> Aligner::get68Point2fs2(const int &rotate_status)
{
    std::vector<cv::Point2f> pts_in_rotated;
    for (int i = 0; i < 68; i++)
    {
        cv::Point2f p1;
        p1.x = mShape.part(i).x();
        p1.y = mShape.part(i).y();
        pts_in_rotated.push_back(p1);
    }

    cv::Mat rot_mat_cw30(2, 3, CV_32FC1);
    cv::Mat rot_mat_cc30(2, 3, CV_32FC1);
    cv::Point center = cv::Point(mOriginalBGR.cols/2, mOriginalBGR.rows/2);
    rot_mat_cw30 = getRotationMatrix2D(center, -30, 1.0);
    rot_mat_cc30 = getRotationMatrix2D(center, 30, 1.0);

    vector<cv::Point2f> pts_in_original;

    switch(rotate_status)
    {
        case ROTATION_NONE:
            pts_in_original.swap(pts_in_rotated);
            break;
        case ROTATION_CW_30:
            cv::transform(pts_in_rotated, pts_in_original, rot_mat_cc30);
            break;
        case ROTATION_CC_30:
            cv::transform(pts_in_rotated, pts_in_original, rot_mat_cw30);
            break;
        case ROTATION_NONE_PADDING:
            int h_pad = mOriginalBGR.rows/2;
            int w_pad = mOriginalBGR.cols/2;
            for(vector<cv::Point2f>::iterator it = pts_in_rotated.begin(); it != pts_in_rotated.end(); it++)
            {
                pts_in_original.push_back(cv::Point((*it).x-w_pad, (*it).y-h_pad));
            }
            break;
    }

    return pts_in_original;
}

// commonly used functions
inline cv::Rect Aligner::dlib2cvRect(const dlib::rectangle &det)
{
    return cv::Rect(int(det.left()), int(det.top()), int(det.right()-det.left()), int(det.bottom()-det.top()));
}

inline dlib::rectangle Aligner::cvRect2dlib(const cv::Rect &rect)
{
    return dlib::rectangle((long)rect.tl().x, (long)rect.tl().y, (long)rect.br().x - 1, (long)rect.br().y - 1);
}

cv::Mat Aligner::find_tform_between_points(cv::Mat &from_mat, cv::Mat &to_mat)
{
    cv::Mat cov, s, u, d, vt, Rm, Tm, R;
    cv::Mat from_mean, to_mean;
    float sigma_from, sigma_to;
    cov = cv::Mat::zeros(2, 2, CV_32F);
    sigma_from = 0;
    sigma_to = 0;
    cv::reduce(from_mat, from_mean, 0, CV_REDUCE_AVG);
    cv::reduce(to_mat, to_mean, 0, CV_REDUCE_AVG);

    for (int i = 0; i < from_mat.rows; i++)
    {
        float xcf = from_mat.at<float>(i,0) - from_mean.at<float>(0,0);
        float ycf = from_mat.at<float>(i,1) - from_mean.at<float>(0,1);
        sigma_from += xcf*xcf + ycf*ycf;
        float xct = to_mat.at<float>(i,0) - to_mean.at<float>(0,0);
        float yct = to_mat.at<float>(i,1) - to_mean.at<float>(0,1);
        sigma_to += xct*xct + yct*yct;
        cov.at<float>(0,0) += xct*xcf;
        cov.at<float>(1,0) += yct*xcf;
        cov.at<float>(0,1) += xct*ycf;
        cov.at<float>(1,1) += yct*ycf;
    }

    sigma_from /= to_mat.rows;
    sigma_to /= to_mat.rows;
    cov /= to_mat.rows;
    s = cv::Mat::eye(2, 2, CV_32F);
    cv::SVD::compute(cov, d, u, vt);
    if (cv::determinant(cov) < 0)
    {
        if (d.at<float>(1, 0) < d.at<float>(0, 0))
            s = -s;
    }

    double c = 1.0;
    if (sigma_from != 0)
    {
        cv::Scalar trace = cv::trace(cv::Mat::diag(d.t())*s);
        c = 1.0f / sigma_from * trace[0];
    }
    Rm = c* u * s * vt;

    Tm = to_mean.t() - Rm * from_mean.t();
    cv::hconcat(Rm,Tm,R);

    return R;
}

bool Aligner::checkNPD()
{
    int size = mNpdDetector->npd_detect(mAligndFaceGray, mFaceDim/2, 0.0f);
    return size > 0 ? true:false;
}

cv::Vec3d Aligner::getOrient(std::vector<cv::Point2f> &points68)
{
    return mFaceOrientCalculator->get_orient(points68);
}

cv::Vec3d Aligner::getOrient(float* points68)
{
    std::vector<cv::Point2f> vp;
    for (int i = 0; i < 68; i++)
    {
        vp.push_back(cv::Point2f(points68[i], points68[i+68]));
    }
    return mFaceOrientCalculator->get_orient(vp);
}

float Aligner::getSharpness()
{
    cv::Mat lap1;
    cv::Laplacian(mAligndFaceGray, lap1, CV_32F);
    cv::Vec2f l1 = p2line(17, 48);
    cv::Vec2f l2 = p2line(45, 54);

    float k1 = l1[0];
    float b1 = l1[1];
    float k2 = l2[0];
    float b2 = l2[1];

    cv::Vec4f bound = getBound();
    for(int y = 0; y < mFaceDim; y++)
    {
        for(int x = 0; x < mFaceDim; x++)
        {
            if((y>k1*x+b1)||(y>k2*x+b2)||(x<bound[0])||(x>bound[2])||(y<bound[1])||(y>bound[3]))
            {
                lap1.at<float>(y,x) = 0.0;
            }
        }
    }

    cv::Scalar mean, stddev;
    meanStdDev(lap1, mean, stddev);
    float sharpness = stddev[0] /14 * mFaceDim;
    return sharpness;
}

cv::Vec2f Aligner::p2line(const int &p1, const int &p2)
{
    // get line equation from 2 points
    float x1,x2,y1,y2,k,b;
    x1 = mAligndFace68Points[p1].x;
    x2 = mAligndFace68Points[p2].x;
    y1 = mAligndFace68Points[p1].y;
    y2 = mAligndFace68Points[p2].y;
    k = (y1-y2)/(x1-x2);
    b = y1-k*x1;
    return cv::Vec2f(k,b);
}

cv::Vec4f Aligner::getBound()
{
    // crop inner face
    int minx,miny,maxx,maxy;
    minx = mFaceDim;
    miny = mFaceDim;
    maxx = 0;
    maxy = 0;
    for(int i = 17; i < 68; i++)
    {
        minx = int(min(minx, mAligndFace68Points[i].x));
        maxx = int(max(maxx, mAligndFace68Points[i].x));
        miny = int(min(miny, mAligndFace68Points[i].y));
        maxy = int(max(maxy, mAligndFace68Points[i].y));
    }
    minx = max(0, minx);
    miny = max(0, miny);
    maxx = min(mFaceDim, maxx);
    maxy = min(mFaceDim, maxy);
    return cv::Vec4f(minx, miny, maxx, maxy);
}


