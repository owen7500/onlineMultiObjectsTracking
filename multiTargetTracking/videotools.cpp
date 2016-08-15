#include "videotools.hpp"
#include <fstream>
const string logFileName = "motLog.edt";

string int2String(int num)
{
    char cc[9];
    sprintf_s(cc, "%08d", num);
    stringstream sstr;
    sstr << cc;
    string str = sstr.str();
    return str;
}

//获取单通道图像的加权平均图像
void getAvgImg_U1(const Mat& srcImg, Mat& avgImg)
{
    //确保只对单通道图像处理
    assert(srcImg.channels() == 1);

    int cols = srcImg.cols;
    int rows = srcImg.rows;

    //如果图像连续，则进行连续性处理，可以提高速度
    if (srcImg.isContinuous() && avgImg.isContinuous())
    {
        cols = cols*rows;
        rows = 1;
    }

    for (int i = 0; i < rows; i++)
    {
        const uchar* sdata = srcImg.ptr<uchar>(i);
        uchar* adata = avgImg.ptr<uchar>(i);
        for (int j = 0; j < cols; j++)
        {
            adata[j] = (sdata[j] + adata[j]) / 2;
        }
    }
}

//获取减除图像
void getSubImg(const Mat& srcImg, const Mat& bgImg, Mat& subedImg)
{
    assert(srcImg.channels() == 1 && bgImg.channels() == 1);

    int cols = srcImg.cols;
    int rows = srcImg.rows;

    //如果图像连续，则进行连续性处理，可以提高速度
    if (srcImg.isContinuous() && bgImg.isContinuous())
    {
        cols = cols*rows;
        rows = 1;
    }

    for (int i = 0; i < rows; i++)
    {
        const uchar* sdata = srcImg.ptr<uchar>(i);	//原图像的数据
        const uchar* adata = bgImg.ptr<uchar>(i);	//指向平均背景图像的数据指针
        uchar* bdata = subedImg.ptr<uchar>(i);		//减除图像结果

        for (int j = 0; j < cols; j++)
        {
            bdata[j] = sdata[j] - adata[j];
        }
    }

    cv::adaptiveThreshold(subedImg, subedImg, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 21, 7);
}

void getTransImg(const Mat& srcImg, Mat& transImg)
{
    Mat hsvImg;
    vector<Mat> channelImgs;
    Mat binImg, mergedHSV;

    static Mat  avgVImg = Mat(srcImg.size(), CV_8UC1, Scalar::all(0));

    static int frameCount = 1;


    cvtColor(srcImg, hsvImg, COLOR_RGB2HSV);

    cv::split(hsvImg, channelImgs);

    if (1 == frameCount)
    {
        avgVImg = channelImgs[2].clone();
    }
    else
    {
        getAvgImg_U1(channelImgs[2], avgVImg);

        binImg.create(avgVImg.size(), CV_8UC1);
        binImg.setTo(Scalar::all(0));

        getSubImg(channelImgs[2], avgVImg, binImg);
    }

    channelImgs[2] = avgVImg.clone();
    merge(channelImgs, mergedHSV);
    cvtColor(mergedHSV, transImg, COLOR_HSV2RGB);

    //计数
    frameCount++;

}

bool writeLogFile(string text)
{
    ofstream logFile(logFileName.c_str(),ios::out|ios::app);
    if(!logFile.is_open())
    {
        cout<<"can not open log file"<<endl;
        return false;
    }
    logFile<<text<<endl;
    logFile.close();

    return true;
}
