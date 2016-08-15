#include "targetTrackingUI.h"
#include "ui_targettracking.h"
#include <QFileDialog>
#include <QActionGroup>
#include <QMessageBox>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDialog>
#include <QPushButton>

TargetTracking::TargetTracking(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TargetTracking),isStopProcess(false)
{
    ui->setupUi(this);
    setWindowTitle("多目标跟踪demo");

    //指针初始化（置空）
    bgs = nullptr;
    autoTracker = nullptr;
    capture = nullptr;

    //算法默认类型
    bgType = ADAPTMIDFILTER;
    MotFeatureType = COLORHISTGRAM;

    //背景模型选择动作设置
    QActionGroup *bgActionGroup = new QActionGroup(this);
    bgActionGroup->addAction(ui->actionFgFrameDiff);
    bgActionGroup->addAction(ui->actionFgAdptiveMiddleFilter);
    bgActionGroup->addAction(ui->actionFgVIBE);
    bgActionGroup->addAction(ui->actionFgFric);
    bgActionGroup->addAction(ui->actionFgLBASOM);
    bgActionGroup->setExclusive(true);
    ui->actionFgAdptiveMiddleFilter->setChecked(true);    //默认自适应中值滤波

    //跟踪器特征选择动作设置
    QActionGroup *motFeatureActionGroup = new QActionGroup(this);
    motFeatureActionGroup->addAction(ui->actionFeatureHash);
    motFeatureActionGroup->addAction(ui->actionFeatureHistgram);
    motFeatureActionGroup->addAction(ui->actionSurf);
    motFeatureActionGroup->addAction(ui->actionCEDDFeature);
    motFeatureActionGroup->setExclusive(true);
    ui->actionFeatureHistgram->setChecked(true);

    //开始处理按钮禁用
    ui->startPushButton->setEnabled(false);
    //显示欢迎界面
    QImage welcomeImage;
    welcomeImage.load("welcome.jpg");
    welcomeImage = welcomeImage.scaled(QSize(400,300));
    ui->videoLabel->setPixmap(QPixmap::fromImage(welcomeImage));
    ui->videoLabel->setAlignment(Qt::AlignCenter);
}

TargetTracking::~TargetTracking()
{
    if(bgs!=nullptr)
        delete bgs;
    if(autoTracker!=nullptr)
        delete autoTracker;
    if(capture!=nullptr)
        delete capture;

    delete ui;
}

void TargetTracking::closeEvent(QEvent *e)
{
    isStopProcess = true;
    exit(0);
}

//文件打开
void TargetTracking::on_fileOpenAction_triggered()
{
    QString qvideoPath = QFileDialog::getOpenFileName(this,QString("目标视频"),QString(),"*.avi *.mp4 *.wmv");
    if(qvideoPath.isEmpty())
    {
        QMessageBox::warning(this,"waring","No video selected!",QMessageBox::Ok);
        return;
    }
    else
    {
        videoFilePath = qvideoPath.toStdString();
        ui->startPushButton->setEnabled(true);

        QImage readyImage;
        readyImage.load("ready.jpg");
        readyImage = readyImage.scaled(QSize(400,300));
        ui->videoLabel->setPixmap(QPixmap::fromImage(readyImage));
        ui->videoLabel->setAlignment(Qt::AlignCenter);
    }
}

//开始处理
void TargetTracking::on_startPushButton_clicked()
{
    if(capture!=nullptr)
    {
        delete capture;
        delete bgs;
        delete autoTracker;
        isStopProcess = false;
    }

    //分配算法空间
    capture = new VideoCapture(videoFilePath);
    if(!allocBgsMemory())
        return;
    if(!allocTrackerMemory())
        return;

    if(!capture->isOpened())
    {
        QMessageBox::warning(this,"waring",
                             QString("can not open the videoFile:")+QString::fromStdString(videoFilePath),
                             QMessageBox::Ok);
        ui->startPushButton->setEnabled(false);
        return;
    }
    else
    {
        while(!isStopProcess)
        {
            capture->read(cameraFeed);

            if(cameraFeed.empty())
               break;

            //getTransImg(transedImg,cameraFeed);

            bgs->process(cameraFeed,binaryImage,backmodelImg);

            int timeStamp = capture->get(CV_CAP_PROP_POS_FRAMES);

            if (!binaryImage.empty())
            {
                autoTracker->morphOps(binaryImage, 2, 3);

                autoTracker->realTrackFilteredObjects(binaryImage, cameraFeed, timeStamp, 18);

                //show frames
                showImg(cameraFeed);
//                cv::imshow("cameraFeed",binaryImage);
//                cv::waitKey(1);
                qApp->processEvents();
            }
        }

        ui->statusBar->showMessage(QString("处理完毕！"));
    }
}

//取消
void TargetTracking::on_cancelPushButton_clicked()
{
    isStopProcess = true;
}

//退出
void TargetTracking::on_exitAction_triggered()
{
    isStopProcess = true;
    exit(0);
}

/*以下五个action用于选择前景检测算法*/
void TargetTracking::on_actionFgFrameDiff_triggered()
{
    bgType = FRMAEDIFF;
}

void TargetTracking::on_actionFgVIBE_triggered()
{
    bgType = VIBE;
}

void TargetTracking::on_actionFgFric_triggered()
{
    bgType = FRIC;
}

void TargetTracking::on_actionFgAdptiveMiddleFilter_triggered()
{
    bgType = ADAPTMIDFILTER;
}

void TargetTracking::on_actionFgLBASOM_triggered()
{
        bgType = LBASOM;
}

//以下四个action用于跟踪器特征选择
void TargetTracking::on_actionFeatureHistgram_triggered()
{
    MotFeatureType = COLORHISTGRAM;
}

void TargetTracking::on_actionFeatureHash_triggered()
{
    MotFeatureType = IMAGEHASH;
}

void TargetTracking::on_actionSurf_triggered()
{
    MotFeatureType = SURFFEATURE;
}

void TargetTracking::on_actionCEDDFeature_triggered()
{
    MotFeatureType = CEDDFEATURE;
}

//Mat转QPixmap用于显示图像，如果图像尺寸过大，将进行压缩
//最大显示尺寸480*320
QPixmap TargetTracking::Mat2Pixmap(const Mat &frame)
{
    Size defaultSize(480,320);
    Mat dst;

    if(frame.size().height>defaultSize.height||frame.size().width>defaultSize.width)
        cv::resize(frame,dst,defaultSize);  //线性拉伸
    else
        dst = frame.clone();

    //cvtColor(dst,dst,COLOR_BGR2RGB);
    QImage qimg = QImage(frame.data,frame.cols,frame.rows,QImage::Format_RGB888);
    QPixmap pix = QPixmap::fromImage(qimg);
    return pix;
}

//显示图像
void TargetTracking::showImg(const Mat &frame)
{
    Mat temp;
    cvtColor(frame,temp,COLOR_BGR2RGB);
    QPixmap pix = Mat2Pixmap(temp);
    ui->videoLabel->setPixmap(pix);
    ui->videoLabel->setAlignment(Qt::AlignCenter);
    this->update();
}

//文件路径和文件名自动补全
//暂时未完善
bool TargetTracking::setOutPutFileNameByInput(QString inputName)
{
    string prefix;
    QFileInfo file = QFileInfo(inputName);
    QString fileName = file.fileName();
    QString pathName = file.absolutePath();
}

bool TargetTracking::allocBgsMemory()
{
    switch(bgType)
    {
    case FRMAEDIFF:
        bgs = new FrameDifferenceBGS();
        break;
    case VIBE:
        bgs = new Vibe();
        break;
    case ADAPTMIDFILTER:
        bgs = new DPAdaptiveMedianBGS();
        break;
    case LBASOM:
        bgs = new LBAdaptiveSOM();
        break;
    case FRIC:
        bgs = new FricBgs();
        break;
    default:
        bgs = new DPAdaptiveMedianBGS();
        break;
    }
    return bgs!=nullptr;
}

bool TargetTracking::allocTrackerMemory()
{
    autoTracker = new ObjectTracker();
    if(autoTracker==nullptr)
        return false;
    else
    {
        autoTracker->setTrackingFeatureType(MotFeatureType);
        return true;
    }

}

void TargetTracking::getOutFileName()
{
    QString filename = outputFileLineEdit->text();
    outPutFileName = filename.toStdString();

    ui->statusBar->showMessage(QString::fromStdString(outPutFileName));
}

//选择单摄像头模式
void TargetTracking::on_actionSingleCamera_triggered()
{

}

//选择多摄像头模式
void TargetTracking::on_actionMultiCamera_triggered()
{

}

//跟踪精度设置（特征精度设置）
void TargetTracking::on_actionTrackingAccuracy_triggered()
{

}

//版本显示
void TargetTracking::on_actionVersion_triggered()
{
    QString version= "说明：本程序用于离线视频的单一视频或多个关联视频的跟踪\n"
                     "版本：beta0.1";
    QMessageBox::information(this,QString("版本"),version,QMessageBox::Ok);
}

//关于作者
void TargetTracking::on_actionAboutAuthor_triggered()
{
    QString authorMsg = "作者：刘文\n学校：华侨大学\n学历：硕士\n主研方向：视频分析，"
                        "包括视频编解码，前景检测，目标跟踪，图像检索等\n联系方式：QQ779525293";
    QMessageBox::information(this,QString("作者简介"),authorMsg,QMessageBox::Ok);
}


void TargetTracking::on_actionTargetSavePath_triggered()
{
    string outPutFileName;

    QDialog* dlg = new QDialog(this);
    outputFileLineEdit = new QLineEdit(dlg);
    dlg->setWindowTitle(QString("设置输出文件名"));
    QPushButton *okBtn = new QPushButton(dlg);
    QPushButton *cancelBtn = new QPushButton(dlg);
    QLabel *fileLabel = new QLabel(dlg);

    okBtn->setText(QString("确定"));
    cancelBtn->setText(QString("取消"));
    fileLabel->setText(QString("输出文件名"));

    QHBoxLayout* hlayoutEdit = new QHBoxLayout;
    hlayoutEdit->addWidget(fileLabel);
    hlayoutEdit->addWidget(outputFileLineEdit);
    hlayoutEdit->setSpacing(50);

    QHBoxLayout* hlayoutBtn = new QHBoxLayout;
    hlayoutBtn->insertWidget(1,okBtn,0,Qt::AlignHCenter);
    hlayoutBtn->insertWidget(2,cancelBtn,0,Qt::AlignHCenter);
    hlayoutBtn->setSpacing(50);

    QVBoxLayout *VLayout = new QVBoxLayout;
    VLayout->addLayout(hlayoutEdit);
    VLayout->addLayout(hlayoutBtn);
    VLayout->setSpacing(20);

    QGridLayout* globLayout = new QGridLayout;
    globLayout->addLayout(VLayout,2,10);

    dlg->setLayout(globLayout);
    dlg->setFixedSize(300,100);

    connect(okBtn,SIGNAL(clicked()),this,SLOT(getOutFileName()));
    connect(okBtn,SIGNAL(clicked()),dlg,SLOT(close()));
    connect(cancelBtn,SIGNAL(clicked()),dlg,SLOT(close()));


    outputFileLineEdit->setText(QString::fromStdString(outPutFileName));
    bool outputNameFlag = true;
    dlg->show();

    string msg = "设置文件名";
    msg = msg+outPutFileName;
    writeLogFile(msg);
}

//以下三个用于设置邻域摄像头检索特征的设置
void TargetTracking::on_searchActionColorHistGram_triggered()
{

}

void TargetTracking::on_searchActionCEDD_triggered()
{

}

void TargetTracking::on_searchActionHash_triggered()
{

}
