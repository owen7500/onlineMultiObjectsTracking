#include "vibe.h"

//进入点的标记，1表示左下角，2表示右下角，3表示左上角，4表示右上角
#define BUTTOMLEFT 1
#define BUTTOMRIGHT 2
#define TOPRIGHT 4
#define TOPLEFT 3

using namespace std;
using namespace cv;

int c_xoff[9] = {-1,  0,  1, -1, 1, -1, 0, 1, 0};  //x的邻居点
int c_yoff[9] = {-1,  0,  1, -1, 1, -1, 0, 1, 0};  //y的邻居点

Vibe::Vibe()
{
    firstFrameFlag = true;
    Width = 80;          //矩形框宽为80个像素
    Height = 180;        //矩形框高为180个像素
}

Vibe::~Vibe()
{

}


/**************** Assign space and init ***************************/
void Vibe::init(const Mat _image)
{
     for(int i = 0; i < NUM_SAMPLES; i++)
     {
         m_samples[i] = Mat::zeros(_image.size(), CV_8UC1);
     }
     m_mask = Mat::zeros(_image.size(),CV_8UC1);
     m_foregroundMatchCount = Mat::zeros(_image.size(),CV_8UC1);
}

/**************** Init model from first frame ********************/
void Vibe::processFirstFrame(const Mat _image)
{
    RNG rng;
    int row, col;

    for(int i = 0; i < _image.rows; i++)
    {
        for(int j = 0; j < _image.cols; j++)
        {
             for(int k = 0 ; k < NUM_SAMPLES; k++)
             {
                 // Random pick up NUM_SAMPLES pixel in neighbourhood to construct the model
                 int random = rng.uniform(0, 9);

                 row = i + c_yoff[random];
                 if (row < 0)
                     row = 0;
                 if (row >= _image.rows)
                     row = _image.rows - 1;

                 col = j + c_xoff[random];
                 if (col < 0)
                     col = 0;
                 if (col >= _image.cols)
                     col = _image.cols - 1;

                 m_samples[k].at<uchar>(i, j) = _image.at<uchar>(row, col);
             }
        }
    }
}

/**************** Test a new frame and update model ********************/
void Vibe::testAndUpdate(const Mat _image)
{
    RNG rng;

    for(int i = 0; i < _image.rows; i++)
    {
        for(int j = 0; j < _image.cols; j++)
        {
            int matches(0), count(0);
            float dist;

            while(matches < MIN_MATCHES && count < NUM_SAMPLES)
            {
                dist = abs(m_samples[count].at<uchar>(i, j) - _image.at<uchar>(i, j));
                if (dist < RADIUS)
                    matches++;
                count++;
            }

            if (matches >= MIN_MATCHES)
            {
                // It is a background pixel
                m_foregroundMatchCount.at<uchar>(i, j) = 0;

                // Set background pixel to 0
                m_mask.at<uchar>(i, j) = 0;

                // 如果一个像素是背景点，那么它有 1 / defaultSubsamplingFactor 的概率去更新自己的模型样本值
                int random = rng.uniform(0, SUBSAMPLE_FACTOR);
                if (random == 0)
                {
                    random = rng.uniform(0, NUM_SAMPLES);
                    m_samples[random].at<uchar>(i, j) = _image.at<uchar>(i, j);
                }

                // 同时也有 1 / defaultSubsamplingFactor 的概率去更新它的邻居点的模型样本值
                random = rng.uniform(0, SUBSAMPLE_FACTOR);
                if (random == 0)
                {
                    int row, col;
                    random = rng.uniform(0, 9);
                    row = i + c_yoff[random];
                    if (row < 0)
                        row = 0;
                    if (row >= _image.rows)
                        row = _image.rows - 1;

                    random = rng.uniform(0, 9);
                    col = j + c_xoff[random];
                    if (col < 0)
                        col = 0;
                    if (col >= _image.cols)
                        col = _image.cols - 1;

                    random = rng.uniform(0, NUM_SAMPLES);
                    m_samples[random].at<uchar>(row, col) = _image.at<uchar>(i, j);
                }
            }
            else
            {
                // It is a foreground pixel
                m_foregroundMatchCount.at<uchar>(i, j)++;

                // Set background pixel to 255
                m_mask.at<uchar>(i, j) = 255;

                //如果某个像素点连续N次被检测为前景，则认为一块静止区域被误判为运动，将其更新为背景点
                if (m_foregroundMatchCount.at<uchar>(i, j) > 50)
                {
                    int random = rng.uniform(0, SUBSAMPLE_FACTOR);
                    if (random == 0)
                    {
                        random = rng.uniform(0, NUM_SAMPLES);
                        m_samples[random].at<uchar>(i, j) = _image.at<uchar>(i, j);
                    }
                }
            }
        }
    }
}

int Vibe::ForegroundPixel(Mat binaryImg)
{
    int PixelCount = 0;
    for(int i=0;i<binaryImg.rows;i++)
    {
        uchar *ptr = binaryImg.ptr<uchar>(i);
        for(int j=0;j<binaryImg.cols*binaryImg.channels();j++)
        {
            if(ptr[j]==255)
                PixelCount++;
        }
    }
    return PixelCount;
}

Mat Vibe::getColorImg(Mat binaryImg, Mat srcImg)
{
    int chanels = srcImg.channels();    //图像通道数

    int Width = 80;
    int Height = 180;

    cv::Point centerPoint = getCenterPoint(binaryImg);

    int X_top = centerPoint.x - Height/2;
    int X_buttom = centerPoint.x + Height/2;
    int Y_left =  centerPoint.y - Width/2;
    int Y_right = centerPoint.y + Width/2;

    for(int i=0;i<srcImg.rows;i++)
    {
        uchar *srcptr = srcImg.ptr<uchar>(i);
        for(int j=0;j<srcImg.cols*chanels;j += chanels)
        {
            if((i>X_top)&&(i<X_buttom))
            {
                if(((j/chanels)<Y_left)||((j/chanels)>Y_right))
                {
                    srcptr[j]   = 0;
                    srcptr[j+1] = 0;
                    srcptr[j+2] = 0;
                }
            }
            else
            {
                srcptr[j]   = 0;
                srcptr[j+1] = 0;
                srcptr[j+2] = 0;
            }
        }
    }
    return srcImg;
}

Mat Vibe::getColorForeImg(Mat binaryImg, Mat srcImg)
{
    int chanels = srcImg.channels();    //图像通道数

    for(int i=0;i<srcImg.rows;i++)
    {
        uchar *srcptr = srcImg.ptr<uchar>(i);
        uchar *binaryptr = binaryImg.ptr<uchar>(i);
        for(int j=0;j<srcImg.cols*chanels;j += chanels)
        {
            if(binaryptr[j/chanels] != 255)
            {
                srcptr[j]   = 0;
                srcptr[j+1] = 0;
                srcptr[j+2] = 0;
            }

        }
    }
    return srcImg;
}

Point Vibe::getCenterPoint(Mat binaryImg)
{
    int PixelCount = 0;                 //前景点个数
    int x_rows(0),y_cols(0);                  //行坐标和，列坐标和

    int x_center(0),y_center(0);              //中心坐标（x，y）

    int chanels = binaryImg.channels();

    for(int i=0;i<binaryImg.rows;i++)
    {
        uchar *ptr = binaryImg.ptr<uchar>(i);
        for(int j=0;j<binaryImg.cols*chanels;j+=chanels)
        {
            if(ptr[j]==255)
            {
                PixelCount++;
                x_rows += i;
                y_cols += j/chanels;
            }
        }
    }

    x_center = x_rows/PixelCount;
    y_center = y_cols/PixelCount;
    cv::Point centerPoint(x_center,y_center);

    return centerPoint;
}

//获取方位
int Vibe::getDerection(Mat binaryImg)
{
    int rows = binaryImg.rows;
    int cols = binaryImg.cols;

    cv::Point center = getCenterPoint(binaryImg);

    if((center.x <rows/2) &&(center.y < cols/2) ) //图片的左上角
    {
        return TOPLEFT;
    }
    else if((center.x <rows/2) &&(center.y >= cols/2)) //右上角
    {
        return TOPRIGHT;
    }
    else if((center.x >= rows/2) &&(center.y < cols/2)) //左下角
    {
        return BUTTOMLEFT;
    }
    else          //右下角
    {
        return BUTTOMRIGHT;
    }
}

void Vibe::setWidth(int width)
{
    Width = width;
}

void Vibe::setHeight(int height)
{
    Height = height;
}

void Vibe::process(const cv::Mat &img_input, cv::Mat &img_foreground, cv::Mat &img_background)
{
    if (firstFrameFlag)
    {
        init(img_input);
        processFirstFrame(img_input);
        firstFrameFlag = false;
    }
    else
    {
        testAndUpdate(img_input);
        img_foreground = getMask();         //获取二值前景图
        img_background = img_input.clone();
    }
}

void Vibe::saveConfig()
{

}

void Vibe::loadConfig()
{

}


