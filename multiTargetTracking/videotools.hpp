#ifndef VIDEOTOOLS_HPP
#define VIDEOTOOLS_HPP

#include <sstream>
#include <string>
#include <iostream>
#include <opencv.hpp>
using namespace std;
using namespace cv;

enum IBGSType
{
    FRMAEDIFF=0,         //普通帧间差分
    ADAPTMIDFILTER,     //自适应中值滤波
    VIBE,               //VIBE
    FRIC,               //快速光照鲁棒
    LBASOM              //自适应自组织网络
};

enum ObjectTrackerFeatureType
{
    COLORHISTGRAM=0,     //颜色直方图
    IMAGEHASH,          //图像HASH
    SURFFEATURE,        //SURF特征
    CEDDFEATURE,        //CEDD特征
    HOG,                //HOG+SVM行人检测
    DPM,                //DPM检测算子
    ACF                 //ACF检测算子
};

string int2String(int num);

//获取单通道图像的加权平均图像
void getAvgImg_U1(const Mat& srcImg, Mat& avgImg);

//获取减除图像
void getSubImg(const Mat& srcImg, const Mat& bgImg, Mat& subedImg);

void getTransImg(const Mat& srcImg, Mat& transImg);

//写日志
bool writeLogFile(string text);

#endif // VIDEOTOOLS_HPP

