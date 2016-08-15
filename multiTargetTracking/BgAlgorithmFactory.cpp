#include "BgAlgorithmFactory.h"

BgAlgorithmFactory::BgAlgorithmFactory()
{
    aType = MULTIBLOCKDIFF; //分块多阈值
    processAlg = nullptr;
}

BgAlgorithmFactory::~BgAlgorithmFactory()
{
    if(processAlg!=nullptr)
        delete processAlg;
}

/*选择对应的背景模型
*注意：该函数不可多次循环使用，否则将会造成内存泄漏
*/
void BgAlgorithmFactory::setAlgorithmType(BgAlgorithmFactory::ATYPE type)
{

    if(processAlg==nullptr)
    {
        this->aType = type;
        allocAlgMemory(aType);      //算法空间分配
    }
    else
    {
        /*如果类型不一致，表明需要切换算法*/
        if(aType!=type)
        {
            //如果之前使用其他的前景算法，则需要删除其分配空间
            //防止内存泄漏
            delete processAlg;
            this->aType = type;
            allocAlgMemory(aType);
        }
    }
}

void BgAlgorithmFactory::process()
{
    if(processAlg==nullptr)
        processAlg = new MultiBlockFrameDif();

    processAlg->process();
}

Mat BgAlgorithmFactory::getMask()
{
    return processAlg->getMask();
}

bool BgAlgorithmFactory::hasMovedObject(double ThreshValue)
{
    return processAlg->hasMovedObject(ThreshValue);
}

void BgAlgorithmFactory::updateImage(const Mat &img)
{
    processAlg->updateBFD(img);
}

void BgAlgorithmFactory::allocAlgMemory(BgAlgorithmFactory::ATYPE type)
{
    //算法选择
    switch(type)
    {
    case BASICFRAMEDIFF:
        processAlg = new BasicFrameDif();       //普通帧间差分
        break;
    case MULTIBLOCKDIFF:
        processAlg = new MultiBlockFrameDif();    //分块多阈值
        break;
    case VIBE:
        processAlg = new VibeFgAlgorithm(); //VIBE
        break;
    case FRIC:
        processAlg = new FricAlgorithm();   //快速光照鲁棒算法
        break;
    default:
        processAlg = new MultiBlockFrameDif();
        break;
    }
}

