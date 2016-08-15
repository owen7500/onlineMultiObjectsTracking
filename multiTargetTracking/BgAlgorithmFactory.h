#ifndef BGALGORITHMFACTORY_H
#define BGALGORITHMFACTORY_H
#include "abstractbgalgorithm.h"
#include "basicframediff.hpp"
#include "multiblockframediff.h"
#include "fricalgorithm.hpp"
#include "vibefgalgorithm.hpp"

class BgAlgorithmFactory
{
public:
    enum ATYPE
    {
        BASICFRAMEDIFF,
        MULTIBLOCKDIFF,
        VIBE,
        FRIC
    };

    BgAlgorithmFactory();
    ~BgAlgorithmFactory();


    void setAlgorithmType(ATYPE type);
    void process();
    Mat getMask();
    bool hasMovedObject(double ThreshValue);
    void updateImage(const Mat& img);

private:
    void allocAlgMemory(BgAlgorithmFactory::ATYPE type);  //分配算法空间

    ATYPE aType;
    Mat mask;
    bool hasMovingObject;
    AbstractBgAlgorithm* processAlg;
};

#endif // BGALGORITHMFACTORY_H
