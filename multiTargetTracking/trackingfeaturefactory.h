#ifndef TRACKINGFEATUREFACTORY_H
#define TRACKINGFEATUREFACTORY_H

#include "trackingFeature/colorFeature.hpp"
#include "trackingFeature/hashFeature.hpp"
//#include "trackingFeature/surffeature.hpp"
#include "trackingFeature/ceddfeature.hpp"
#include "trackingFeature/feature_base.hpp"

#include "videotools.hpp"
#include <opencv.hpp>
using namespace cv;
using namespace ot;

class trackingFeatureFactory
{
public:
    trackingFeatureFactory();
    trackingFeatureFactory(const Mat& originImg,ObjectTrackerFeatureType type);
    trackingFeatureFactory(const trackingFeatureFactory& others);
    ~trackingFeatureFactory();

    void setFeatureType(ObjectTrackerFeatureType type);
    void allocFeatureMemory(ObjectTrackerFeatureType type);
    void updateFeature(const Mat& img);
    double getProp(const Mat& img);
    void setFeatureImage(const Mat&img);
private:
    FeatureBase* trackingFeature;
    ObjectTrackerFeatureType fType;
};

#endif // TRACKINGFEATUREFACTORY_H
