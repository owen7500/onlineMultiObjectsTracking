#ifndef _CEDDQUANT_H_
#define _CEDDQUANT_H_

#include <algorithm>
#include <vector>
using namespace std;

class CEDDQuant
{
private:
	vector<double> QuantTable;
	vector<double> QuantTable2;
	vector<double> QuantTable3;
	vector<double> QuantTable4;
	vector<double> QuantTable5;
	vector<double> QuantTable6;

public:
	CEDDQuant()
	{
		QuantTable = { 180.19686541079636, 23730.024499150866, 61457.152912541605,
			113918.55437576842, 179122.46400035513, 260980.3325940354, 341795.93301552488, 554729.98648386425 };

		QuantTable2 = { 209.25176965926232, 22490.5872862417345, 60250.8935141849988,
			120705.788057580583, 181128.08709063051, 234132.081356900555, 325660.617733105708, 520702.175858657472 };

		QuantTable3 = { 405.4642173212585, 4877.9763319071481, 10882.170090625908,
			18167.239081219657, 27043.385568785292, 38129.413201299016, 52675.221316293857, 79555.402607004813 };

		QuantTable4 = { 405.4642173212585, 4877.9763319071481, 10882.170090625908,
			18167.239081219657, 27043.385568785292, 38129.413201299016, 52675.221316293857, 79555.402607004813 };

		QuantTable5 = { 968.88475977695578, 10725.159033657819, 24161.205360376698,
			41555.917344385321, 62895.628446402261, 93066.271379694881, 136976.13317822068, 262897.86056221306 };

		QuantTable6 = { 968.88475977695578, 10725.159033657819, 24161.205360376698,
			41555.917344385321, 62895.628446402261, 93066.271379694881, 136976.13317822068, 262897.86056221306 };
	};
	~CEDDQuant(){};

	vector<double> Apply(const vector<double>& Local_Edge_Histogram)
	{
		vector<double> Edge_HistogramElement(Local_Edge_Histogram.size());
		vector<double> ElementsDistance(8);
		double Max = 1;

		for (int i = 0; i < 24; i++)
		{
			Edge_HistogramElement[i] = 0;
			for (int j = 0; j < 8; j++)
			{
				ElementsDistance[j] = abs(Local_Edge_Histogram[i] - QuantTable[j] / 1000000);
			}
			Max = 1;
			for (int j = 0; j < 8; j++)
			{
				if (ElementsDistance[j] < Max)
				{
					Max = ElementsDistance[j];
					Edge_HistogramElement[i] = j;
				}
			}
		}

		for (int i = 24; i < 48; i++)
		{
			Edge_HistogramElement[i] = 0;
			for (int j = 0; j < 8; j++)
			{
				ElementsDistance[j] = abs(Local_Edge_Histogram[i] - QuantTable2[j] / 1000000);
			}
			Max = 1;
			for (int j = 0; j < 8; j++)
			{
				if (ElementsDistance[j] < Max)
				{
					Max = ElementsDistance[j];
					Edge_HistogramElement[i] = j;
				}
			}
		}

		for (int i = 48; i < 72; i++)
		{
			Edge_HistogramElement[i] = 0;
			for (int j = 0; j < 8; j++)
			{
				ElementsDistance[j] = abs(Local_Edge_Histogram[i] - QuantTable3[j] / 1000000);
			}
			Max = 1;
			for (int j = 0; j < 8; j++)
			{
				if (ElementsDistance[j] < Max)
				{
					Max = ElementsDistance[j];
					Edge_HistogramElement[i] = j;
				}
			}
		}

		for (int i = 72; i < 96; i++)
		{
			Edge_HistogramElement[i] = 0;
			for (int j = 0; j < 8; j++)
			{
				ElementsDistance[j] = abs(Local_Edge_Histogram[i] - QuantTable4[j] / 1000000);
			}
			Max = 1;
			for (int j = 0; j < 8; j++)
			{
				if (ElementsDistance[j] < Max)
				{
					Max = ElementsDistance[j];
					Edge_HistogramElement[i] = j;
				}
			}
		}
		for (int i = 96; i < 120; i++)
		{
			Edge_HistogramElement[i] = 0;
			for (int j = 0; j < 8; j++)
			{
				ElementsDistance[j] = abs(Local_Edge_Histogram[i] - QuantTable5[j] / 1000000);
			}
			Max = 1;
			for (int j = 0; j < 8; j++)
			{
				if (ElementsDistance[j] < Max)
				{
					Max = ElementsDistance[j];
					Edge_HistogramElement[i] = j;
				}
			}
		}

		for (int i = 120; i < 144; i++)
		{
			Edge_HistogramElement[i] = 0;
			for (int j = 0; j < 8; j++)
			{
				ElementsDistance[j] = abs(Local_Edge_Histogram[i] - QuantTable6[j] / 1000000);
			}
			Max = 1;
			for (int j = 0; j < 8; j++)
			{
				if (ElementsDistance[j] < Max)
				{
					Max = ElementsDistance[j];
					Edge_HistogramElement[i] = j;
				}
			}
		}
		return Edge_HistogramElement;
	}
};
#endif