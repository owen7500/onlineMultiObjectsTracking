#ifndef CEDDFEATURE_HPP
#define CEDDFEATURE_HPP

#include <iostream>
#include <vector>
#include <opencv.hpp>
using namespace std;
using namespace cv;

#include "CEDD/CEDD_descriptor.h"
#include "CEDD/cosSimilarity.h"
#include "CEDD/Fuzzy10Bins.h"
#include "CEDD/Fuzzy24Bins.h"
#include "CEDD/CEDDQuant.h"
#include "feature_base.hpp"
using namespace ot;

class CEDDFeature:public FeatureBase
{
private:
    vector<double> sFeature;
    double similarity;
    CEDD* ceddModel;
public:
    //无参默认构造函数
    CEDDFeature(){
        ceddModel = new CEDD();
        similarity = 0;}
    CEDDFeature(const Mat& _input):FeatureBase(_input)
    {
        ceddModel = new CEDD();
        sFeature = ceddModel->Apply(image);
        similarity = 0;
    }
    //自有拷贝构造函数，防止浅拷贝
    CEDDFeature(const CEDDFeature& otherCedd):FeatureBase(otherCedd.image)
    {
        sFeature = otherCedd.sFeature;
        similarity = otherCedd.similarity;
        image = otherCedd.image.clone();

        ceddModel = new CEDD();
        *ceddModel = *(otherCedd.ceddModel);
        std::cout<<"CEDDFeature copy Constructor!"<<std::endl;
    }

    ~CEDDFeature()
    {
        if(!ceddModel)
            delete ceddModel;
    }

    double getProp(const Mat& dest)
    {
        vector<double> destFeature = ceddModel->Apply(dest);
        CosSimilarity myCosSimi(sFeature);
        similarity = myCosSimi.getSimilarity(destFeature);
        return similarity;
    }

    void updateFeature(const Mat& img)
    {
        this->image = img.clone();
        if(!sFeature.empty())
        {
            sFeature.clear();
        }

        sFeature = ceddModel->Apply(img);
    }

    void copyData(const FeatureBase& another)
    {
        this->image = another.image.clone();

        ceddModel = new CEDD();
        sFeature = ceddModel->Apply(image);
        similarity = 0;
    }
};
#endif // CEDDFEATURE_HPP

