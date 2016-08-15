#ifndef COLORFEATURE_HPP
#define COLORFEATURE_HPP

#include <iostream>
#include <opencv.hpp>
#include "feature_base.hpp"
#include <math.h>
using namespace cv;
using namespace ot;

/**
* 颜色直方图特征
*/
class ColorHist : public FeatureBase {
protected:
    int bins;
    MatND hist;
public:
    ColorHist() {bins=16;}

    ColorHist(Mat& _input, Rect roi) :FeatureBase(_input)
    {
        bins = 16;
        fixROI(roi, _input);

        image = Mat(_input, roi);
        cvtColor(image,image,COLOR_BGR2HSV);
        calc(image,hist);
    }

    ColorHist(Mat& _input):FeatureBase(_input)
    {
        bins = 16;
        calc(image,hist);
    }

    //自有拷贝构造函数，防止浅拷贝
    ColorHist(const ColorHist& others):FeatureBase(others.image)
    {
        this->bins = others.bins;
        this->hist = others.hist.clone();
    }

    ~ColorHist() {}

    //获取相似度
    double getProp(const Mat& dest)
    {
        MatND dstHist;
        calc(dest,dstHist);
        double p = compareHist(dstHist, hist, CV_COMP_BHATTACHARYYA);
        p = exp(-20.0 * (p * p));
        p = std::isnan(p) ? 0 : p;

        return p;
    }

    //更新特征
    void updateFeature(const Mat& img)
    {
        this->image = img.clone();
        calc(image,hist);
    }

    void copyData(const FeatureBase& another)
    {
        this->image = another.image.clone();

        if(image.empty())
        {
            std::cout<<"empty image in copyData() of ColorHist"<<std::endl;
            return;
        }
        bins = 16;
        calc(image,hist);
    }

private:

    /*将一个 ROI 调整为不超过图像边界*/
    Rect fixROI(Rect& roi, const Mat& _input) {
        if (roi.x + roi.width > _input.cols || roi.width <= 0) {
            roi.width = 1;
        }
        if (roi.y + roi.height > _input.rows || roi.height <= 0) {
            roi.height = 1;
        }
        if (roi.x >= _input.cols) {
            roi.x = _input.cols - 1;
        }
        else if (roi.x < 0) {
            roi.x = 0;
        }
        if (roi.y >= _input.rows) {
            roi.y = _input.rows - 1;
        }
        else if (roi.y < 0) {
            roi.y = 0;
        }

        return roi;
    }

    /* 计算输入图片的特征*/
    void calc(const Mat& img,MatND& Hist)
    {
        Mat iGray;
        //cvtColor(img, iGray, CV_BGR2HSV);    //在外部已经做过转换了，不需要每次都转换，这样可以提升性能
        iGray = img.clone();

        int hist_size[] = { bins, bins };
        float hrange[] = { 0, 180 };
        float srange[] = { 0, 256 };
        const float *ranges[] = { hrange, srange };
        int channels[] = { 0, 1 };

        calcHist(&iGray, 1, channels, Mat(), // do not use mask
            Hist, 2, hist_size, ranges,
            true, // the histogram is uniform
            false);

        // 转换为概率密度分布
        normalize(Hist, Hist);
    }

    /* 计算输入特征是当前特征的概率*/
    double prob(ColorHist& feature)
    {
        double p;

        p = compareHist(feature.hist, hist, CV_COMP_BHATTACHARYYA);
        p = exp(-20.0 * (p * p));
        p = std::isnan(p) ? 0 : p;

        return p;
    }

    void updateHist(ColorHist &newFeature){
        this->hist = newFeature.hist.clone();
    }

    void getHist(MatND& Hist)
    {
        Hist = hist.clone();
    }
};

#endif // COLORFEATURE_HPP

