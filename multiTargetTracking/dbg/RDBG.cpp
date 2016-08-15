#include "RDBG.h"
#include <cmath>

//0.4, 60, 0.005,10,0.4
RDBG::RDBG() :Tr(0.4), Tl(40), alphaL(0.0025), bgUpdateMultiRatio(10), lstMultiThresh(0.4), frameIndex(0), preTI(1)
{
	alphaS = 10 * alphaL;
	Ts = 0.4*Tl;
	Tld = Tl / 2;
	Tlu = Tl * 2;

	Tsu = Ts * 2;

	ltEAIC = 0;
	stEAIC = 0;
	TImaxThresh = 0;
    std::cout<<"init rdbg()"<<std::endl;
}

RDBG::~RDBG()
{
    std::cout<<"destruct rdbg()"<<std::endl;
}

void RDBG::InitRDBG(const Mat& srcImg)
{
	ltBG = srcImg.clone();
	stBG = ltBG.clone();	//短时背景

	ltFG = ltBG.clone();	//长时前景
	stFG = stBG.clone();	//短时前景

	ltFG.setTo(Scalar::all(0));	//归零
	stFG.setTo(Scalar::all(0));	//归零
}

void RDBG::update(const Mat& srcImg)
{
	Mat tempImg;

	cvtColor(srcImg, tempImg, COLOR_BGR2GRAY);

	if (frameIndex ==0)
	{
		InitRDBG(tempImg);
		preImage = tempImg.clone();
	}
	else
	{
		fbRatio = calcFBRatio();
		curTI = calcTI(tempImg, preImage);

		/*if (fbRatio < Tr)*/
		if ((frameIndex == 1) || abs(curTI - preTI)<3)
		{
			double tempAlph = curTI / (255 * 2);

			alphaS = alphaS>tempAlph ? alphaS : tempAlph;
			alphaL = alphaL/bgUpdateMultiRatio;

			updateLTBG(tempImg);		//更新长时背景
			updateSTBG(tempImg);		//更新短时背景
		}
		else
		{
			static int counter = 0;
			double subTempTi = abs(curTI - preTI);

			if (counter++<15)
				TImaxThresh = TImaxThresh > subTempTi ? TImaxThresh : subTempTi;
			else
			{
				TImaxThresh = subTempTi;
				counter = 0;
			}
			

			updateEAIC(tempImg);
			compensationProcess(tempImg, curTI, counter);
		}
		preTI = curTI;
		updateLTFG(tempImg);		//更新长时前景
		updateSTFG(tempImg);		//更新短时前景

		preImage = tempImg.clone();
	}
	++frameIndex;	//处理帧数
}

void RDBG::updateLTBG(const Mat& srcImg)
{
	updateCommonBG(srcImg, ltBG,alphaL);
}

void RDBG::updateSTBG(const Mat& srcImg)
{
	updateCommonBG(srcImg, stBG,alphaS);
}

void RDBG::updateLTFG(const Mat& srcImg)
{
	updateCommonFG(srcImg, ltBG, ltFG, Tl);
}

void RDBG::updateSTFG(const Mat& srcImg)
{
	updateCommonFG(srcImg, stBG, stFG, Ts);
}

//普通前景更新
void RDBG::updateCommonFG(const Mat& srcImg, const Mat& bgImg,Mat& fgImg,double threshValue)
{
	Mat diffMat;
	absdiff(srcImg, bgImg, diffMat);	//减除

	threshold(diffMat, fgImg, threshValue, 255, CV_THRESH_BINARY);
}

//普通背景更新
void RDBG::updateCommonBG(const Mat& srcImg, Mat& bgImg, double updateRatio)
{
	int cols = srcImg.cols*srcImg.channels();
	int rows = srcImg.rows;

	if (srcImg.isContinuous() && bgImg.isContinuous())
	{
		cols = cols*rows;
		rows = 1;
	}

	for (int i = 0; i < rows; ++i)
	{
		const uchar* sdata = srcImg.ptr<uchar>(i);
		uchar* bdata = bgImg.ptr<uchar>(i);
		for (int j = 0; j < cols; ++j)
		{
			bdata[j] = saturate_cast<uchar>(updateRatio*sdata[j] + (1 - updateRatio)*bdata[j]);
		}
	}
}

/*计算两幅前景图的交集*/
cv::Mat RDBG::getInterSectionFG(const Mat& ltFGImg,const Mat& stFGImg)
{
	int cols = ltFGImg.cols*ltFGImg.channels();
	int rows = ltFGImg.rows;

	//前景交集
	Mat interSecFG = Mat(ltFGImg.rows, ltFGImg.cols, ltFGImg.type(), Scalar::all(0));

	if (ltFGImg.isContinuous() && stFGImg.isContinuous() && interSecFG.isContinuous())
	{
		cols = cols*rows;
		rows = 1;
	}

	for (int i = 0; i < rows;++i)
	{
		const uchar* ldata = ltFGImg.ptr<uchar>(i);
		const uchar* sdata = stFGImg.ptr<uchar>(i);
		uchar* idata = interSecFG.ptr<uchar>(i);
		for (int j = 0; j < cols;++j)
		{
			if (ldata[j] && sdata[j])
				idata[j] = 255;
			else
				idata[j] = 0;
		}
	}

	return interSecFG;
}

/*计算短时前景图中，前景点与背景点的比例*/
double RDBG::calcFBRatio()
{
	int forePixelCounter = 0;
	int backPixelCounter = 0;

	int cols = stFG.cols*stFG.channels();
	int rows = stFG.rows;

	if (stFG.isContinuous())
	{
		cols = cols*rows;
		rows = 1;
	}

	for (int i = 0; i < rows;++i)
	{
		const uchar* sdata = stFG.ptr<uchar>(i);
		for (int j = 0; j < cols;++j)
		{
			if (sdata[j])
				++forePixelCounter;
			else
				++backPixelCounter;
		}
	}

	if (backPixelCounter == 0)
		return 1;
	else
		return (forePixelCounter / (double)backPixelCounter);
}

/*返回真实的前景图*/
cv::Mat RDBG::getForeImg(int index)
{
	Mat fgImg;
	Mat unionResult, subResult;
	Mat sInversedFg;

	Mat insResultMat;
	Mat subUnionMat;

	insResultMat = getInterSectionFG(ltFG, stFG);

	threshold(stFG, sInversedFg, 128, 255, CV_THRESH_BINARY_INV);
	getUnionImgPixel(ltFG, sInversedFg, unionResult);
	getImgSubstractedSet(ltFG, stFG, subResult);
	getImgSubstractedSet(unionResult, subResult, subUnionMat);

	switch (index)
	{
	case 0:
		fgImg = insResultMat;
		break;
	case 1:
		fgImg = subUnionMat;
		break;
	case 2:
		if (curTI < 12)
			fgImg = subUnionMat;
		else
			fgImg = insResultMat;
		break;
	default:
		fgImg = insResultMat;
		break;
	}
	return fgImg;
}

/*计算前景图和背景图的并集*/
void RDBG::getUnionImgPixel(const Mat& FGImg, const Mat& bgImg, Mat& unionPixelImg)
{
	int cols = FGImg.cols*FGImg.channels();
	int rows = FGImg.rows;

	if (unionPixelImg.empty())
		unionPixelImg.create(FGImg.size(),FGImg.type());
	unionPixelImg.setTo(Scalar::all(0));

	if (FGImg.isContinuous()&&bgImg.isContinuous()&&unionPixelImg.isContinuous())
	{
		cols = cols*rows;
		rows = 1;
	}

	for (int i = 0; i < rows;++i)
	{
		const uchar* fdata = FGImg.ptr<uchar>(i);
		const uchar* bdata = bgImg.ptr<uchar>(i);
		uchar* udata = unionPixelImg.ptr<uchar>(i);
		for (int j = 0; j < cols;++j)
		{
			if (fdata[j] || (!bdata[j]))
				udata[j] = 255;
			else
				udata[j] = 0;
		}
	}
}

/*计算有效点的平局亮度*/
double RDBG::calcAverageLumi(const Mat& Img, const Mat& rejectedPixels)
{
	assert(Img.channels() == 1);
	assert(rejectedPixels.channels() == 1);

	int luminaceCounter = 0;
	int validPixelCounter = 0;
	double averageLumi = 0.0;

	int cols = Img.cols;
	int rows = Img.rows;

	if (Img.isContinuous()&&rejectedPixels.isContinuous())
	{
		cols = cols*rows;
		rows = 1;
	}

	for (int i = 0; i < rows;++i)
	{
		const uchar* idata = Img.ptr<uchar>(i);
		const uchar* rdata = rejectedPixels.ptr<uchar>(i);

		for (int j = 0; j < cols;++j)
		{
			if (!rdata[j])
			{
				luminaceCounter += idata[j];
				++validPixelCounter;
			}
		}
	}

	averageLumi = luminaceCounter / ((double)validPixelCounter);
	return averageLumi;
}

void RDBG::updateEAIC(const Mat& srcImg)
{
	Mat rejectedPixels = getSamplePixel(srcImg.cols, srcImg.rows, 0.6);

	double mIt = calcAverageLumi(srcImg,rejectedPixels);
	double mltBG = calcAverageLumi(ltBG, rejectedPixels);
	double mstBG = calcAverageLumi(stBG, rejectedPixels);

	ltEAIC = abs(mIt - mltBG);
	stEAIC = abs(mIt - mstBG);
}

void RDBG::compensationProcess(const Mat& srcImg, double Ti, int decreaseCounter)
{
	BgCompensation(srcImg, ltBG, ltEAIC, Ti, decreaseCounter);
	BgCompensation(srcImg, stBG, stEAIC, Ti, decreaseCounter);
}

void RDBG::BgCompensation(const Mat& srcImg,Mat& bgImg,double EAIC, double Ti, int decreaseCounter)
{
	int cols = srcImg.cols;
	int rows = srcImg.rows;
	double betaEAIC = 0.5;

		if (srcImg.isContinuous() && bgImg.isContinuous())
		{
			cols = cols*rows;
			rows = 1;
		}

		for (int i = 0; i < rows; ++i)
		{
			const uchar* sdata = srcImg.ptr<uchar>(i);
			uchar* bdata = bgImg.ptr<uchar>(i);

			for (int j = 0; j < cols; ++j)
			{
				int pixelDiff = abs(sdata[j] - bdata[j]);

				if (pixelDiff < EAIC)
				{
					//策略三
					double k = -5;
					double offset = 8;

					double sigmod = 1 / (1 + exp(k*(pixelDiff - EAIC)/EAIC));
					bdata[j] = saturate_cast<uchar>(bdata[j] + sigmod*(sdata[j] - bdata[j]));
				}
				else
				{
					if (bdata[j] < sdata[j])
						bdata[j] = saturate_cast<uchar>(bdata[j] + TImaxThresh);
					else
						bdata[j] = saturate_cast<uchar>(bdata[j] - TImaxThresh);
				}
			}
		}

		//策略二
		TImaxThresh = TImaxThresh - 0.4*abs(Ti - preTI);
}

/*
*修改默认参数
*@param1 -tr 是否发生光照补偿的阈值，短时前景中前景像素点与背景像素点的比例
*@param2 -tl 普通场景下，长时背景模型中，判定为前景的阈值
*@param3 -alplvalue 普通场景下，长时背景的更新率
*@param4 -lsUpdateRatio 普通场景下，短时背景中被判定为前景的阈值
*@param5 -stThreshRatio 普通场景下，短时背景中背景的更新率
*/
void RDBG::setParams(double tr, double tl, double alplvalue, double lsUpdateRatio, double stThreshRatio)
{
	bgUpdateMultiRatio = lsUpdateRatio;
	lstMultiThresh = stThreshRatio;

	Tr = tr;
	Tl = tl;
	Ts = lstMultiThresh*Tl;
	//Ts = 0.4*Tl;

	alphaL = alplvalue;
	alphaS = alphaL * bgUpdateMultiRatio;
	//alphaS = alphaL * 10;

	Tld = Tl / 2;
	Tlu = Tl * 2;

	Tsu = Ts * 2;

	ltEAIC = 0;
	stEAIC = 0;
}

//图像像素集的减法
void RDBG::getImgSubstractedSet(const Mat& subtractor, const Mat& minuend, Mat& result)
{
	int cols = subtractor.cols;
	int rows = subtractor.rows;

	if (result.empty())
	{
		result.create(subtractor.size(), subtractor.type());
		result.setTo(cv::Scalar::all(0));
	}

	if (subtractor.isContinuous()&&minuend.isContinuous()&&result.isContinuous())
	{
		cols = rows* cols;
		rows = 1;
	}

	for (int i = 0; i < rows;++i)
	{
		const uchar* sdata = subtractor.ptr<uchar>(i);
		const uchar* mdata = minuend.ptr<uchar>(i);
		uchar* rdata = result.ptr<uchar>(i);
		for (int j = 0; j < cols;++j)
		{
			if (sdata[j] && (!mdata[j]))
				rdata[j] = 255;
		}
	}
}

double RDBG::calcTI(const Mat& curImage, const Mat& preImage)
{
	double avgValue = 0;	//记录平均值
	double sigmaValue = 0;	//Ti

	Mat diffImg;
	absdiff(curImage, preImage, diffImg);		//计算两幅图像的差图

	avgValue = calcAvgPixelValue(diffImg);		//差图平均值

	int cols = diffImg.cols;
	int rows = diffImg.rows;

	if (diffImg.isContinuous())
	{
		cols = cols*rows;
		rows = 1;
	}

	for (int i = 0; i < rows; ++i)
	{
		const uchar* data = diffImg.ptr<uchar>(i);
		for (int j = 0; j < cols; ++j)
		{
			sigmaValue += pow((data[j]-avgValue),2);
		}
	}


	return sqrt(sigmaValue / (cols*rows));
}

//计算图像的平均像素值
double RDBG::calcAvgPixelValue(const Mat& diffMat)
{
	assert(diffMat.channels() == 1);

	int pixelValueCounter = 0;

	int cols = diffMat.cols;
	int rows = diffMat.rows;

	if (diffMat.isContinuous())
	{
		cols = cols*rows;
		rows = 1;
	}

	for (int i = 0; i < rows;++i)
	{
		const uchar* data = diffMat.ptr<uchar>(i);
		for (int j = 0; j < cols;++j)
		{
			pixelValueCounter += data[j];
		}
	}

	return (double)pixelValueCounter / (double)(cols*rows);
}

cv::Mat RDBG::getSamplePixel(int cols,int rows,double sampleRatio)
{
	//初始化随机采样图
	Mat samplePixels(rows,cols,CV_8UC1,Scalar::all(255));

	int sampleSize = cols*rows*sampleRatio;

	int x = 0, y = 0;	//采样点坐标

	//随机采样，并计算前景和背景点的比例参数
	for (int i = 0; i < sampleSize; i++)
	{
		x = rand() % (rows - 1);
		y = rand() % (cols - 1);
		
		samplePixels.at<uchar>(x, y) = 0;
	}

	return samplePixels;
}
