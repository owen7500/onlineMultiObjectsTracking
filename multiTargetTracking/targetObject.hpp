#ifndef _TARGETOBJECT_HPP
#define _TARGETOBJECT_HPP

#include "trackingfeaturefactory.h"
#include <opencv.hpp>
using namespace cv;

//definition of direction，1=left buttom，2=right buttom，3=left top，4=right top
enum DIRECTION
{
    BUTTOMLEFT = 1, BUTTOMRIGHT, TOPLEFT, TOPRIGHT
};


class targetObject
{
private:
    int rows;
    int cols;

protected:
    cv::Point centerPoint;		//current center point;
    cv::Rect currentRect;		//current object rectangle;
    int enterDirection;			//enter direction
    int exitDirection;			//exit direction
    int objectIndex;			//index of the object in this video segment
    vector<Point> objectTrace;	//trace of the object,sequence of point
    trackingFeatureFactory* featureFactory;     //tracking feature

    int targetLifeTime;			//life time of the object in the video segment
    int tartgetTimeStamp;		//time stamp of the object
    Mat currentImg;
    vector<Mat> targetImgVector;		//vector of the targetImg
    vector<trackingFeatureFactory> targetFeatureVector;	//vector of the target feature
    int stairNumber;

    bool isDirection(int dir){
        return (dir > 0 && dir < 5);
    }

    double getPointDist(Point newPoint)
    {
        return sqrt((centerPoint.x - newPoint.x)*(centerPoint.x - newPoint.x) + (centerPoint.y - newPoint.y)*(centerPoint.y - newPoint.y));
    }
public:
    targetObject()
    {
        centerPoint = Point();
        currentRect = Rect();
        enterDirection = 0;
        exitDirection = 0;
        objectIndex = -1;
        featureFactory = new trackingFeatureFactory();
    }

    targetObject(const Mat& inputImg,ObjectTrackerFeatureType ftype=COLORHISTGRAM,
                 int timeStamp=0,Point center=cv::Point(-1,-1), Rect region=Rect(-1,-1,0,0),
                 int enterDir=0, int exitDir=0, int index=0, Point tracePoint=Point(-1,-1)
                 )
    {
        //参数检测
        assert(!inputImg.empty());

        centerPoint = center;
        currentRect = region;
        enterDirection = enterDir;
        exitDirection = exitDir;
        objectIndex = index;
        objectTrace.push_back(tracePoint);

        rows = inputImg.rows;
        cols = inputImg.cols;

        targetLifeTime = 1;
        tartgetTimeStamp = timeStamp;

        currentImg = inputImg(region).clone();
        targetImgVector.push_back(currentImg);

        featureFactory = new trackingFeatureFactory();
        featureFactory->setFeatureType(ftype);
        featureFactory->updateFeature(currentImg);
        targetFeatureVector.push_back(*featureFactory);
    }

    targetObject(const targetObject& another)
    {
        this->objectIndex = another.objectIndex;
        this->rows = another.rows;
        this->cols = another.cols;
        this->centerPoint = another.centerPoint;
        this->currentRect = another.currentRect;
        this->enterDirection = another.enterDirection;
        this->exitDirection = another.exitDirection;
        this->objectIndex = another.objectIndex;
        this->objectTrace = another.objectTrace;
        this->targetLifeTime = another.targetLifeTime;
        this->tartgetTimeStamp = another.tartgetTimeStamp;
        this->currentImg = another.currentImg.clone();
        this->targetImgVector = another.targetImgVector;
        this->stairNumber = another.stairNumber;

        this->featureFactory = new trackingFeatureFactory(*another.featureFactory);
        //*this->featureFactory = *another.featureFactory;

    }
    ~targetObject(){
        //std::cout<<"~targetObject()"<<"\n delete featureFactory"<<endl;
        delete featureFactory;
    }

    void setFeatureType(ObjectTrackerFeatureType ftype=COLORHISTGRAM){
        featureFactory->setFeatureType(ftype);
    }

    bool isSameTarget(targetObject& newTarget,double threshValue=0.5)
    {        
        Mat img = newTarget.getCurrentImg();
        double p = this->featureFactory->getProp(img);
        return p>threshValue;
    }

    bool isTheNearSame(targetObject& newTarget,double& p,double threshValue=0.6)
    {
        p= featureFactory->getProp(newTarget.getCurrentImg());
        return p>threshValue;
    }

    //更新当前目标信息
    void updateTarget(targetObject& newTarget)
    {
        this->centerPoint = newTarget.centerPoint;          //更新中心点位置
        this->currentRect = newTarget.currentRect;          //更新当前的矩形框
        this->exitDirection = newTarget.exitDirection;      //更新退出区域
        targetLifeTime++;                                   //生命周期自加
        featureFactory->updateFeature(newTarget.getCurrentImg());   //更新跟踪特征

        if (targetLifeTime%5==0)            //每5帧保存一次信息，保存之前的特征与目标图像
        {
            //if (targetFeatureVector.size() > 7)
            //	targetFeatureVector.erase(targetFeatureVector.begin());//erase the first element
            targetFeatureVector.push_back(*(this->featureFactory));
            targetImgVector.push_back(newTarget.getCurrentImg());
        }
    }

    void setCenterPoint(Point newCenter)
    {
        centerPoint = newCenter;
    }

    void setCurrentRect(Rect newRect){
        currentRect = newRect;
    }

    void setEnterDirection(int direction){
        if (isDirection(direction))
            enterDirection = direction;
    }

    void setExitDirection(int direction){
        if (isDirection(direction))
            exitDirection = direction;
    }

    void setObjectIndex(int indexNum){
        objectIndex = indexNum;
    }

    void setObjectStairs(int stairs){ stairNumber = stairs; }

    void insertObjectImg(Mat roiImg){
        currentImg = roiImg.clone();
        targetImgVector.push_back(roiImg);
    }

    void updateObjectTrace(Point newTracePoint)
    {
        objectTrace.push_back(newTracePoint);
    }

    Mat getCurrentImg(){ return currentImg; }

    //获取目标的索引，该摄像头中稳定目标的序号
    int getObjectIndex(){ return objectIndex; }

    //获取目标的进入区域
    int getObjectEnterDir(){ return enterDirection; }

    //目标离开区域
    int getObjectExitDir(){ return exitDirection; }

    //目标的中心点
    Point getObjectCenterPoint(){ return centerPoint; }

    //获取目标矩阵（区域）
    Rect getObjectRect(){ return currentRect; }

    //获取目标的路径
    vector<Point> getObjectTrace(){ return objectTrace; }

    //显示路径
    void showTrace(){
        //Mat traceImg = Mat(rows,cols,CV_8UC1,0);
        Mat traceImg = Mat::zeros(rows, cols, CV_8UC1);
        for (int i = 0; i < objectTrace.size();i++)
        {
            traceImg.at<uchar>(objectTrace[i].x, objectTrace[i].y) = 255;
        }
        imshow("traceImg", traceImg);
    }

    //保存路径
    void saveTrace(char* saveName)
    {
        Mat traceImg = Mat::zeros(rows, cols, CV_8UC1);
        for (int i = 0; i < objectTrace.size(); i++)
        {
            traceImg.at<uchar>(objectTrace[i].x, objectTrace[i].y) = 255;
        }
        cv::imwrite(saveName, traceImg);
    }

    //获取目标的生命周期
    int getTargetLifeTime(){ return targetLifeTime; }

    //目标的时间戳，表示该目标的序号
    int getTargetTimeStamp(){ return tartgetTimeStamp; }

    //获取目标特征向量
    vector<trackingFeatureFactory> getTargetFeatureVector(){ return targetFeatureVector; }

    //目标所处的楼梯号
    int getTargetStairs(){ return stairNumber; }

    vector<Mat> getTargetImgVector(){ return targetImgVector; }
};
#endif
