#ifndef HASHFEATURE_HPP
#define HASHFEATURE_HPP

#include <iostream>
#include <bitset>
#include <string>
#include <iomanip>
#include <opencv.hpp>
#include "feature_base.hpp"

using namespace std;
using namespace cv;
using namespace ot;
#define hashLength 256

class HashFeature:public FeatureBase
{
private:
    int size;
    bitset<hashLength> ahash;

public:
    HashFeature()
    {
        size = 16;
    }
    HashFeature(const Mat &img):FeatureBase(img)
    {
        size = 16;

        if (img.empty())
            cout << "empty image yet!\n" << endl;
        else
        {
            resize(image, image, Size(size, size), 0, 0, INTER_CUBIC);

            if (image.channels() == 3)
                cvtColor(image, image, COLOR_BGR2GRAY);

            float averagePix = calcAverage(image, size);    // 计算灰度化的均值
            fingerPrint(image, size, ahash, averagePix);    // 得到均值hash
        }
    }

    HashFeature(const HashFeature& others):FeatureBase(others.image)
    {
        this->size = others.size;
        this->ahash = others.ahash;
    }

    void updateFeature(const Mat& img)
    {
        image = img.clone();

        if (img.empty())
            cout << "empty image yet!\n" << endl;
        else
        {
            resize(image, image, Size(size, size), 0, 0, INTER_CUBIC);

            if (image.channels() == 3)
                cvtColor(image, image, COLOR_BGR2GRAY);

            float averagePix = calcAverage(image, size);    // 计算灰度化的均值
            fingerPrint(image, size, ahash, averagePix);    // 得到均值hash
        }
    }

    double getProp(const Mat& img)
    {
        Mat target = img.clone();

        if (target.empty())
        {
            double p = 0;
            cout << "empty image!" << endl;
            return p;
        }
        resize(target, target, Size(size, size), 0, 0, INTER_CUBIC);

        if (target.channels() == 3)
            cvtColor(target, target, COLOR_BGR2GRAY);

        float averagePix2 = calcAverage(target, size);
        bitset<hashLength> ahash2;
        fingerPrint(target, size, ahash2, averagePix2);
        int distance = hammingDistance(ahash, ahash2);      // 计算汉明距离

        double p;
        if (distance>64)
            p = 0;
        else
            p = distance*0.015625;

        return p;
    }

    void copyData(const FeatureBase &another)
    {
        this->image = another.image.clone();
        size = 16;
        if (image.empty())
            cout << "empty image yet!\n" << endl;
        else
        {
            resize(image, image, Size(size, size), 0, 0, INTER_CUBIC);

            if (image.channels() == 3)
                cvtColor(image, image, COLOR_BGR2GRAY);

            float averagePix = calcAverage(image, size);    // 计算灰度化的均值
            fingerPrint(image, size, ahash, averagePix);    // 得到均值hash
        }
    }
private:
    // 计算size*size图像的平均灰度
    float calcAverage(Mat_<uchar> image, const int &size){
        float sum = 0;
        for (int i = 0; i < size; i++){
            for (int j = 0; j < size; j++){
                sum += image(i, j);
            }
        }
        return sum / (size*size);
    }

    /* 计算hash值
    image:size*size的灰度图像
    size: 图像大小  size*size
    ahahs:存放size*size位hash值
    averagePix: 灰度值的平均值
    */
    void fingerPrint(Mat_<uchar> image, const int &size, bitset<hashLength> &ahash, const float &averagePix){
        ahash.reset();
        for (int i = 0; i < size; i++){
            int pos = i * size;
            for (int j = 0; j < size; j++){
                ahash[pos + j] = image(i, j) >= averagePix ? 1 : 0;
            }
        }
    }

    /*计算汉明距离*/
    int hammingDistance(const bitset<hashLength> &query, const bitset<hashLength> &target){
        int distance = 0;
        for (int i = 0; i < hashLength; i++){
            distance += (query[i] == target[i] ? 0 : 1);
        }
        return distance;
    }

    string bitTohex(const bitset<hashLength> &target){
        string str;
        for (int i = 0; i < hashLength; i = i + 4){
            int sum = 0;
            string s;
            sum += target[i] + (target[i + 1] << 1) + (target[i + 2] << 2) + (target[i + 3] << 3);
            stringstream ss;
            ss << hex << sum;    // 以十六进制保存
            ss >> s;
            str += s;
        }
        return str;
    }

};

#endif // HASHFEATURE_HPP

