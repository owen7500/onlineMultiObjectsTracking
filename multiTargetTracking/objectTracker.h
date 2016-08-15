#ifndef _OBJECTTRACKER_H
#define _OBJECTTRACKER_H


#include <iostream>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include "Object.h"
#include "targetObject.hpp"
#include <io.h>
#include <direct.h>

using namespace cv;
using namespace std;

class ObjectTracker
{
private:
    int FRAME_WIDTH;
    int FRAME_HEIGHT;
    int MAX_NUM_OBJECTS;
    double MIN_OBJECT_AREA;
    double MAX_OBJECT_AREA;

    int MIN_RECT_WIDTH;
    int MIN_RECT_HEIGHT;
    int MAX_RECT_WIDTH;
    int MAX_RECT_HEIGHT;
    int MIN_DIST_THRESH;

    cv::Point centerPoint;
    cv::Mat targetRectImg;
    vector<targetObject> targetVector;				//target in this video segment
    vector<targetObject> trackedTargetVector;		//tracked target in this segment
    vector<targetObject> curImgTargetVector;		//target in this frame
    int ObjectNum;
    int delayCounter;				//a counter for clearing the target vector
    bool videoSegmentFirstFlag;		//in a video segment,it's the first time appear target
    int TrackedTargetThresh;		//threshold of the tracked target
    int targetLifeTimeThresh;

    ObjectTrackerFeatureType oType;

protected:
    //整型转string
    string intToString(int number);
    //冒泡排序
    void bubleSort(int *arr,int len);
    //计算两点之间的距离
    double getPointDist(Point Pa, Point Pb);
    //计算两个矩形框之间的距离
    double getRectDist(Rect Ra, Rect Rb);
    //判断两个矩形框是否相邻
    bool isNearBy_rect(Rect Ra, Rect Rb);

    Mat threshColorImg(const Mat &colorImg, const Mat &maskImg);	//use the mask image to process a color image
                                                    //if the pix of mask is false(zero),the coordinate in color image should be zero
public:

    ObjectTracker();
    ~ObjectTracker(){};

    //设置跟踪器使用的特征类型,类型参见ObjectTrackerFeatureType
    void setTrackingFeatureType(ObjectTrackerFeatureType type);

    //形态学处理、腐蚀膨胀
    void morphOps(Mat &thresh, int erodeTimes = 1, int dilateTimes = 3);

    //目标跟踪算法
    void realTrackFilteredObjects(const Mat& threshold, Mat &cameraFeed, int timeStamp, int stairs);

    //根据二值前景图获取运动前景的中心位置，提取固定矩形框80*100内的彩色运动目标,返回图像大小为原来srcImg的大小
    Mat getColorImg(const Mat& binaryImg, const Mat& srcImg);

    //根据二值前景图获取对应位置的彩色图
    Mat getColorForeImg(const Mat& binaryImg, const Mat& srcImg);

    //计算二值前景图中，运动目标的中心位置（默认每帧图像中只存在一个运动目标）
    cv::Point getCenterPoint(const Mat& binaryImg);

    //根据运动目标的中心，获得该物体在实际场景中对应的方位（通道口）
    int getDerection(const cv::Point& center, int rows, int cols);

    //对相邻的矩形框进行聚类合并
    //思想：在通一帧图像中由于图像质量较差检测的前景图像不连续不完整
    //默认相邻的前景图是同一个运动目标产生的
    void clusterAndConstructRect(const vector<Rect>& mainRect, vector<Rect> &reconRectVector);

    //获取被跟踪的目标向量
    vector<targetObject> getTrackedTargetVector(){ return trackedTargetVector; }

    //设置最多允许跟踪的目标数
    void setTrackedTargetThresh(int objectNum);

    //检索相邻的摄像头，是否出现类似目标，如果出现则对该目标进行标注，并打印相关信息
    void searchNeighbourCamera(Mat &camera, vector<targetObject> &neighTargetVector);

    //保存运动目标图像
    void saveObjectImg(targetObject& curTarget);
};
#endif // !_OBJECTTRACKER_H

