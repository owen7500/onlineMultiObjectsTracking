#ifndef _CEDD_DESCRIPTOR_H_
#define _CEDD_DESCRIPTOR_H_

#include <iostream>
#include <vector>
#include <opencv.hpp>
#include <math.h>

#include "Fuzzy10Bins.h"
#include "Fuzzy24Bins.h"
#include "CEDDQuant.h"

using namespace std;
using namespace cv;

typedef struct _MaskResults
{
public:
	double Mask1;
	double Mask2;
	double Mask3;
	double Mask4;
	double Mask5;
}MaskResults;

typedef struct _Neighborhood
{
public:
	double Area1;
	double Area2;
	double Area3;
	double Area4;
}Neighborhood;

class CEDD
{
private:
	double T0;
	double T1;
	double T2;
	double T3;
	bool Compact;

protected:
    vector<int> rgb2HSVApplyFilter(int red, int green, int blue)
    {
        vector<int> Results(3,0);
        int HSV_H = 0;
        int HSV_S = 0;
        int HSV_V = 0;

        double MaxHSV = (double)(std::max(red, std::max(green, blue)));
        double MinHSV = (double)(std::min(red, std::min(green, blue)));

        HSV_V = (int)(MaxHSV);
        HSV_S = 0;

        if (MaxHSV != 0) HSV_S = (int)(255 - 255 * (MinHSV / MaxHSV));

        if (MaxHSV != MinHSV)
        {
            int IntegerMaxHSV = (int)(MaxHSV);

            if (IntegerMaxHSV == red && green >= blue)
            {
                HSV_H = (int)(60 * (green - blue) / (MaxHSV - MinHSV));
            }

            else if (IntegerMaxHSV == red && green < blue)
            {
                HSV_H = (int)(359 + 60 * (green - blue) / (MaxHSV - MinHSV));
            }
            else if (IntegerMaxHSV == green)
            {
                HSV_H = (int)(119 + 60 * (blue - red) / (MaxHSV - MinHSV));
            }
            else if (IntegerMaxHSV == blue)
            {
                HSV_H = (int)(239 + 60 * (red - green) / (MaxHSV - MinHSV));
            }
        }
        else HSV_H = 0;

        Results[0] = HSV_H;
        Results[1] = HSV_S;
        Results[2] = HSV_V;

        return (Results);
    }

public:
	CEDD(double Th0, double Th1, double Th2, double Th3, bool CompactDescriptor)
	{
		this->T0 = Th0;
		this->T1 = Th1;
		this->T2 = Th2;
		this->T3 = Th3;
		this->Compact = CompactDescriptor;
	}

	CEDD()
	{
		this->T0 = 14;
		this->T1 = 0.68;
		this->T2 = 0.98;
		this->T3 = 0.98;
		this->Compact = false;
	}

    CEDD(const CEDD& otherCEDD) //拷贝构造函数,防止浅拷贝
    {
        this->T0 = otherCEDD.T0;
        this->T1 = otherCEDD.T1;
        this->T2 = otherCEDD.T2;
        this->T3 = otherCEDD.T3;
        this->Compact = otherCEDD.Compact;
    }
	vector<double> Apply(const cv::Mat& srcImg)
	{
		Fuzzy10Bins Fuzzy10(false);
		Fuzzy24Bins Fuzzy24(false);

		vector<int> HSV(3, 0);

		vector<double> Fuzzy10BinResultTable(10,0);
		vector<double> Fuzzy24BinResultTable(24,0);
		vector<double> CEDDFeature(144,0);


		int width = srcImg.cols;
		int height = srcImg.rows;

		Mat ImageGrid;
		vector<Mat> channels;
		Mat ImageGridRed;
		Mat ImageGridGreen;
		Mat ImageGridBlue;
		vector<vector<double>> PixelCount(2,vector<double>(2,0.0));

		int NumberOfBlocks = 1600; // blocks
		//int NumberOfBlocks = 3600;

        //子块数必须为整数
		int Step_X = (int)floor(width / sqrt(NumberOfBlocks));
		int Step_Y = (int)floor(height / sqrt(NumberOfBlocks));

		//针对偶数步长处理,子块个数需为偶数个
		if ((Step_X % 2) != 0)
		{
			Step_X = Step_X - 1;
		}
		if ((Step_Y % 2) != 0)
		{
			Step_Y = Step_Y - 1;
		}

        //如果检索图像的大小小于子块的的大小，则返回空向量
        if(Step_X==0||Step_Y==0)
            return CEDDFeature;
		//此处不能设为2
		//保证处理区域在图像内部，不能出现越界操作
		Step_Y = Step_Y < 2 ? 1 : Step_Y;
		Step_X = Step_X < 2 ? 1 : Step_X;

        int tempBlockNum = 0;
        if (Step_X<2)
        {
            Step_X = 2;
            tempBlockNum = width / Step_X;
            NumberOfBlocks = NumberOfBlocks > tempBlockNum ? tempBlockNum : NumberOfBlocks;
        }
        if (Step_Y<2)
        {
            Step_Y = 2;
            int temp = height / Step_Y;
            tempBlockNum = tempBlockNum>temp ? temp : tempBlockNum;
            NumberOfBlocks = NumberOfBlocks > tempBlockNum ? tempBlockNum : NumberOfBlocks;
        }

		vector<int> Edges(6,-1);

		MaskResults MaskValues;
		Neighborhood PixelsNeighborhood;

		cvtColor(srcImg, ImageGrid, COLOR_BGR2GRAY);
		split(srcImg, channels);
		ImageGridBlue = channels[0];
		ImageGridGreen = channels[1];
		ImageGridRed = channels[2];

		vector<int> CororRed(Step_Y * Step_X);
		vector<int> CororGreen(Step_Y * Step_X);
		vector<int> CororBlue(Step_Y * Step_X);

		vector<int> CororRedTemp(Step_Y * Step_X);
		vector<int> CororGreenTemp(Step_Y * Step_X);
		vector<int> CororBlueTemp(Step_Y * Step_X);

		int MeanRed = 0, MeanGreen = 0, MeanBlue = 0;
		int T = -1;

		int TempSum = 0;
		double Max = 0;

		int TemoMAX_X = Step_X * (int)sqrt(NumberOfBlocks);
		int TemoMAX_Y = Step_Y * (int)sqrt(NumberOfBlocks);;

		for (int y = 0; y < TemoMAX_Y; y += Step_Y)
		{
			for (int x = 0; x < TemoMAX_X; x += Step_X)
			{
				//初始化数据
				MeanRed = 0;
				MeanGreen = 0;
				MeanBlue = 0;
				PixelsNeighborhood.Area1 = 0;
				PixelsNeighborhood.Area2 = 0;
				PixelsNeighborhood.Area3 = 0;
				PixelsNeighborhood.Area4 = 0;

				Edges = vector<int>(6,0);
				PixelCount = vector<vector<double>>(2,vector<double>(2,0.0));

				TempSum = 0;

				for (int i = y; i < y + Step_Y; i++)
				{
					for (int j = x; j < x + Step_X; j++)
					{
						// Color Information
						CororRed[TempSum] = (int)ImageGridRed.at<uchar>(i,j);
						CororGreen[TempSum] = (int)ImageGridGreen.at<uchar>(i, j);
						CororBlue[TempSum] = (int)ImageGridBlue.at<uchar>(i, j);

						CororRedTemp[TempSum] = (int)ImageGridRed.at<uchar>(i, j);
						CororGreenTemp[TempSum] = (int)ImageGridGreen.at<uchar>(i, j);
						CororBlueTemp[TempSum] = (int)ImageGridBlue.at<uchar>(i, j);

						TempSum++;

						// Texture Information
						if (j < (x + Step_X / 2) && i < (y + Step_Y / 2)) 
							PixelsNeighborhood.Area1 += ((int)(ImageGrid.at<uchar>(i, j)));
						if (j >= (x + Step_X / 2) && i < (y + Step_Y / 2))
							PixelsNeighborhood.Area2 += ((int)(ImageGrid.at<uchar>(i, j)));
						if (j < (x + Step_X / 2) && i >= (y + Step_Y / 2))
							PixelsNeighborhood.Area3 += ((int)(ImageGrid.at<uchar>(i, j)));
						if (j >= (x + Step_X / 2) && i >= (y + Step_Y / 2))
							PixelsNeighborhood.Area4 += ((int)(ImageGrid.at<uchar>(i, j)));
					}
				}
				PixelsNeighborhood.Area1 = (int)(PixelsNeighborhood.Area1 * (4.0 / (Step_X * Step_Y)));

				PixelsNeighborhood.Area2 = (int)(PixelsNeighborhood.Area2 * (4.0 / (Step_X * Step_Y)));

				PixelsNeighborhood.Area3 = (int)(PixelsNeighborhood.Area3 * (4.0 / (Step_X * Step_Y)));

				PixelsNeighborhood.Area4 = (int)(PixelsNeighborhood.Area4 * (4.0 / (Step_X * Step_Y)));


				MaskValues.Mask1 = abs(PixelsNeighborhood.Area1 * 2 + PixelsNeighborhood.Area2 * -2 + PixelsNeighborhood.Area3 * -2 + PixelsNeighborhood.Area4 * 2);
				MaskValues.Mask2 = abs(PixelsNeighborhood.Area1 * 1 + PixelsNeighborhood.Area2 * 1 + PixelsNeighborhood.Area3 * -1 + PixelsNeighborhood.Area4 * -1);
				MaskValues.Mask3 = abs(PixelsNeighborhood.Area1 * 1 + PixelsNeighborhood.Area2 * -1 + PixelsNeighborhood.Area3 * 1 + PixelsNeighborhood.Area4 * -1);
				MaskValues.Mask4 = abs(PixelsNeighborhood.Area1 * sqrt(2) + PixelsNeighborhood.Area2 * 0 + PixelsNeighborhood.Area3 * 0 + PixelsNeighborhood.Area4 * -sqrt(2));
				MaskValues.Mask5 = abs(PixelsNeighborhood.Area1 * 0 + PixelsNeighborhood.Area2 * sqrt(2) + PixelsNeighborhood.Area3 * -sqrt(2) + PixelsNeighborhood.Area4 * 0);


				Max = max(MaskValues.Mask1, max(MaskValues.Mask2, max(MaskValues.Mask3, max(MaskValues.Mask4, MaskValues.Mask5))));

				MaskValues.Mask1 = MaskValues.Mask1 / Max;
				MaskValues.Mask2 = MaskValues.Mask2 / Max;
				MaskValues.Mask3 = MaskValues.Mask3 / Max;
				MaskValues.Mask4 = MaskValues.Mask4 / Max;
				MaskValues.Mask5 = MaskValues.Mask5 / Max;

				T = -1;

				if (Max < T0)
				{
					Edges[0] = 0;
					T = 0;
				}
				else
				{
					T = -1;

					if (MaskValues.Mask1 > T1)
					{
						T++;
						Edges[T] = 1;
					}
					if (MaskValues.Mask2 > T2)
					{
						T++;
						Edges[T] = 2;
					}
					if (MaskValues.Mask3 > T2)
					{
						T++;
						Edges[T] = 3;
					}
					if (MaskValues.Mask4 > T3)
					{
						T++;
						Edges[T] = 4;
					}
					if (MaskValues.Mask5 > T3)
					{
						T++;
						Edges[T] = 5;
					}
				}
				for (int i = 0; i < (Step_Y * Step_X); i++)
				{
					MeanRed += CororRed[i];
					MeanGreen += CororGreen[i];
					MeanBlue += CororBlue[i];
				}

				MeanRed = int(MeanRed / (Step_Y * Step_X));
				MeanGreen = int(MeanGreen / (Step_Y * Step_X));
				MeanBlue = int(MeanBlue / (Step_Y * Step_X));

				HSV = rgb2HSVApplyFilter(MeanRed, MeanGreen, MeanBlue);

				if (this->Compact == false)
				{
					Fuzzy10BinResultTable = Fuzzy10.ApplyFilter(HSV[0], HSV[1], HSV[2], 2);
					Fuzzy24BinResultTable = Fuzzy24.ApplyFilter(HSV[0], HSV[1], HSV[2], Fuzzy10BinResultTable, 2);

					for (int i = 0; i <= T; i++)
					{
						for (int j = 0; j < 24; j++)
						{
							if (Fuzzy24BinResultTable[j] > 0) CEDDFeature[24 * Edges[i] + j] += Fuzzy24BinResultTable[j];
						}
					}
				}
				else
				{
					Fuzzy10BinResultTable = Fuzzy10.ApplyFilter(HSV[0], HSV[1], HSV[2], 2);

					for (int i = 0; i <= T; i++)
					{
						for (int j = 0; j < 10; j++)
						{
							if (Fuzzy10BinResultTable[j] > 0) CEDDFeature[10 * Edges[i] + j] += Fuzzy10BinResultTable[j];
						}
					}
				}
			}
		}

		double Sum = 0;

		for (int i = 0; i < 144; i++)
		{
			Sum += CEDDFeature[i];
		}

		for (int i = 0; i < 144; i++)
		{
			CEDDFeature[i] = CEDDFeature[i] / Sum;
		}


		CEDDQuant* Quantization = new CEDDQuant();

		CEDDFeature = Quantization->Apply(CEDDFeature);

		return (CEDDFeature);
	}
};
#endif
