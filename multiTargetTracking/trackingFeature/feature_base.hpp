#ifndef FEATURE_BASE_HPP
#define FEATURE_BASE_HPP


#include <opencv2/opencv.hpp>

using namespace cv;


namespace ot {
    /**
    * 特征类基类
    */
    class FeatureBase {
    public:
        FeatureBase()
        {

        }

        FeatureBase(const Mat& img)
        {
            image = img.clone();
        }

        FeatureBase(const FeatureBase& another){
            image = another.image.clone();
        }

        //设置图像
        void setImage(const Mat& img){image = img.clone();}

        //获取基类图像
        Mat getImage(){return image;}

        virtual double getProp(const Mat& dest)=0;

        virtual void updateFeature(const Mat& img)=0;

        virtual void copyData(const FeatureBase& another)=0;
//        virtual void copyData(FeatureBase* ptrAnother) = 0;

        Mat image;
    };

}
#endif // FEATURE_BASE_HPP

