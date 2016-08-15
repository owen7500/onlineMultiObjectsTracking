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
	stBG = ltBG.clone();	//��ʱ����

	ltFG = ltBG.clone();	//��ʱǰ��
	stFG = stBG.clone();	//��ʱǰ��

	ltFG.setTo(Scalar::all(0));	//����
	stFG.setTo(Scalar::all(0));	//����
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

			updateLTBG(tempImg);		//���³�ʱ����
			updateSTBG(tempImg);		//���¶�ʱ����
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
		updateLTFG(tempImg);		//���³�ʱǰ��
		updateSTFG(tempImg);		//���¶�ʱǰ��

		preImage = tempImg.clone();
	}
	++frameIndex;	//����֡��
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

//��ͨǰ������
void RDBG::updateCommonFG(const Mat& srcImg, const Mat& bgImg,Mat& fgImg,double threshValue)
{
	Mat diffMat;
	absdiff(srcImg, bgImg, diffMat);	//����

	threshold(diffMat, fgImg, threshValue, 255, CV_THRESH_BINARY);
}

//��ͨ��������
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

/*��������ǰ��ͼ�Ľ���*/
cv::Mat RDBG::getInterSectionFG(const Mat& ltFGImg,const Mat& stFGImg)
{
	int cols = ltFGImg.cols*ltFGImg.channels();
	int rows = ltFGImg.rows;

	//ǰ������
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

/*�����ʱǰ��ͼ�У�ǰ�����뱳����ı���*/
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

/*������ʵ��ǰ��ͼ*/
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

/*����ǰ��ͼ�ͱ���ͼ�Ĳ���*/
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

/*������Ч���ƽ������*/
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
					//������
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

		//���Զ�
		TImaxThresh = TImaxThresh - 0.4*abs(Ti - preTI);
}

/*
*�޸�Ĭ�ϲ���
*@param1 -tr �Ƿ������ղ�������ֵ����ʱǰ����ǰ�����ص��뱳�����ص�ı���
*@param2 -tl ��ͨ�����£���ʱ����ģ���У��ж�Ϊǰ������ֵ
*@param3 -alplvalue ��ͨ�����£���ʱ�����ĸ�����
*@param4 -lsUpdateRatio ��ͨ�����£���ʱ�����б��ж�Ϊǰ������ֵ
*@param5 -stThreshRatio ��ͨ�����£���ʱ�����б����ĸ�����
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

//ͼ�����ؼ��ļ���
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
	double avgValue = 0;	//��¼ƽ��ֵ
	double sigmaValue = 0;	//Ti

	Mat diffImg;
	absdiff(curImage, preImage, diffImg);		//��������ͼ��Ĳ�ͼ

	avgValue = calcAvgPixelValue(diffImg);		//��ͼƽ��ֵ

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

//����ͼ���ƽ������ֵ
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
	//��ʼ���������ͼ
	Mat samplePixels(rows,cols,CV_8UC1,Scalar::all(255));

	int sampleSize = cols*rows*sampleRatio;

	int x = 0, y = 0;	//����������

	//���������������ǰ���ͱ�����ı�������
	for (int i = 0; i < sampleSize; i++)
	{
		x = rand() % (rows - 1);
		y = rand() % (cols - 1);
		
		samplePixels.at<uchar>(x, y) = 0;
	}

	return samplePixels;
}
