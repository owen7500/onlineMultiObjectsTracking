#ifndef ABSTRACTBGALGORITHM
#define ABSTRACTBGALGORITHM

#include <core.hpp>
#include <imgproc.hpp>
#include <highgui.hpp>
#include <iostream>
using namespace cv;

class AbstractBgAlgorithm
{
public:
    virtual void updateBFD(const Mat& Frame)=0;
    virtual void process()=0;
    virtual double getFGPercentage()=0;
    double getForeGroundPixelPercent(const Mat& frame)
    {
        int forePixelCount = 0;
        float forePixelPercent = 0;

        int rows = frame.rows;      //行数
        int cols = frame.cols*frame.channels();      //列数

        if (frame.isContinuous())
        {
            cols = rows*cols;
            rows = 1;
        }

        for (int j = 0; j < rows; j++)
        {
            const uchar* mdata = frame.ptr<uchar>(j);
            for (int i = 0; i<cols; i++)
            {
                if (mdata[i] == 255)
                    forePixelCount++;
            }
        }

        forePixelPercent = (float)(forePixelCount / ((float)(rows*cols)));
        forePixelPercent *= 100;		//化为百分比

        return forePixelPercent;
    }
    virtual Mat getMask()=0;
    virtual bool hasMovedObject(double ThreshValue)=0;

protected:
    double fgPercentage;
    Mat mask;
};
#endif // ABSTRACTBGALGORITHM

