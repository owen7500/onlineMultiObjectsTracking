#include "trackingfeaturefactory.h"

trackingFeatureFactory::trackingFeatureFactory()
{
    trackingFeature = nullptr;
    fType = COLORHISTGRAM;
}

trackingFeatureFactory::trackingFeatureFactory(const Mat& originImg, ObjectTrackerFeatureType type)
{
    fType = type;
    allocFeatureMemory(fType);
    this->trackingFeature->updateFeature(originImg);
}

trackingFeatureFactory::trackingFeatureFactory(const trackingFeatureFactory &others)
{
    this->fType = others.fType;

    switch (fType) {
    case COLORHISTGRAM:
        trackingFeature = new ColorHist(others.trackingFeature->image);
        break;
    case IMAGEHASH:
        trackingFeature = new HashFeature(others.trackingFeature->image);
        break;
    case CEDDFEATURE:
        trackingFeature = new CEDDFeature(others.trackingFeature->image);
        break;
    case SURFFEATURE:
        //trackingFeature = new mySurfFeature();
        break;
    case HOG:
    case DPM:
    case ACF:
    default:
        trackingFeature = new ColorHist(others.trackingFeature->image);
        break;
    }

//    allocFeatureMemory(fType);

//    if(others.trackingFeature->image.empty())
//    {
//        std::cout<<"empty image in trackingFeatureFactory"<<std::endl;
//        return;
//    }

//    this->trackingFeature->copyData(*others.trackingFeature);
}

trackingFeatureFactory::~trackingFeatureFactory()
{
    if(trackingFeature!= nullptr)
        delete trackingFeature;
}


/**************************************
 * COLORHISTGRAM=0,     //颜色直方图
    IMAGEHASH,          //图像HASH
    SURFFEATURE,        //SURF特征
    CEDDFEATURE,        //CEDD特征
    HOG,                //HOG+SVM行人检测
    DPM,                //DPM检测算子
    ACF                 //ACF检测算子
 **************************************/
void trackingFeatureFactory::setFeatureType(ObjectTrackerFeatureType type)
{
    //如果当前特征为空，则说明是首次分配空间
    if(trackingFeature==nullptr)
    {
        fType = type;
        allocFeatureMemory(fType);
    }
    else
    {
        //如果类型不一致，说明需要切换算法
        //此时需要先释放原来的内存空间防止内存泄漏
        if(fType!=type)
        {
            delete trackingFeature;
            this->fType = type;
            allocFeatureMemory(fType);
        }
    }
}

//分配特征空间
void trackingFeatureFactory::allocFeatureMemory(ObjectTrackerFeatureType type)
{
    switch (type) {
    case COLORHISTGRAM:
        trackingFeature = new ColorHist();
        break;
    case IMAGEHASH:
        trackingFeature = new HashFeature();
        break;
    case CEDDFEATURE:
        trackingFeature = new CEDDFeature();
        break;
    case SURFFEATURE:
        //trackingFeature = new mySurfFeature();
        break;
    case HOG:
    case DPM:
    case ACF:
    default:
        trackingFeature = new ColorHist();
        break;
    }
}

//更新特征
void trackingFeatureFactory::updateFeature(const Mat &img)
{
    trackingFeature->updateFeature(img);
}

//获取相似度
double trackingFeatureFactory::getProp(const Mat &img)
{
    return trackingFeature->getProp(img);
}

void trackingFeatureFactory::setFeatureImage(const Mat &img)
{

}

