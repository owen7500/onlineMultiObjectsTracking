#ifndef TARGETTRACKING_H
#define TARGETTRACKING_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QLineEdit>

#include <string>
#include <iostream>
#include <vector>

#include "Object.h"
#include "vibe.h"
#include "FrameDifferenceBGS.h"
#include "dp/DPAdaptiveMedianBGS.h"
#include "lb/LBAdaptiveSOM.h"
#include "FricBgs.hpp"
#include "objectTracker.h"
#include "videotools.hpp"

using namespace std;

namespace Ui {
class TargetTracking;
}

class TargetTracking : public QMainWindow
{
    Q_OBJECT

public:
    explicit TargetTracking(QWidget *parent = 0);
    ~TargetTracking();
protected:
    void closeEvent(QEvent *e);

private slots:
    void on_fileOpenAction_triggered();

    void on_exitAction_triggered();

    void on_actionFgFrameDiff_triggered();

    void on_actionFgVIBE_triggered();

    void on_actionFgFric_triggered();

    void on_actionFgAdptiveMiddleFilter_triggered();

    void on_actionFeatureHistgram_triggered();

    void on_actionFgLBASOM_triggered();

    void on_actionFeatureHash_triggered();

    void on_actionSurf_triggered();

    void on_startPushButton_clicked();

    void on_cancelPushButton_clicked();

    void on_actionSingleCamera_triggered();

    void on_actionMultiCamera_triggered();

    void on_actionVersion_triggered();

    void on_actionTrackingAccuracy_triggered();

    void on_actionAboutAuthor_triggered();

    void on_actionCEDDFeature_triggered();

    void on_actionTargetSavePath_triggered();

    void getOutFileName();

    void on_searchActionColorHistGram_triggered();

    void on_searchActionCEDD_triggered();

    void on_searchActionHash_triggered();

private:
    //Mat转QPixmap用于显示图像，如果图像尺寸过大，将进行压缩
    //最大显示尺寸480*320
    QPixmap Mat2Pixmap(const Mat& frame);
    void showImg(const Mat& frame);

    bool setOutPutFileNameByInput(QString inputName);

    //分配空间
    bool allocBgsMemory();
    bool allocTrackerMemory();

    Ui::TargetTracking *ui;

    Mat cameraFeed;
    Mat binaryImage;
    Mat backmodelImg;
    Mat transedImg;

    VideoCapture* capture;      //视频捕获器
    string videoFilePath;       //视频文件路径名

    IBGS *bgs;                  //背景模型
    ObjectTracker *autoTracker; //跟踪器

    IBGSType bgType;            //背景模型
    ObjectTrackerFeatureType MotFeatureType;  //跟踪器特征类型


    bool isStopProcess;

    string outPutFileName;      //输出文件名
    QLineEdit* outputFileLineEdit;
};

#endif // TARGETTRACKING_H
