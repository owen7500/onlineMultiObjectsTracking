#ifndef _FUZZY10BINS_H_
#define _FUZZY10BINS_H_

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

typedef struct _FuzzyRules
{
public:
	int Input1;
	int Input2;
	int Input3;
	int Output;
}FuzzyRules10;

class Fuzzy10Bins
{
private:
		bool KeepPreviuesValues;
		vector<double> HueMembershipValues; // Table Dimensions= Number Of Triangles X 4 (Start - Stop)
		vector<double> SaturationMembershipValues;
		vector<double> ValueMembershipValues;
		vector<FuzzyRules10> Fuzzy10BinRules;
		vector<double> Fuzzy10BinHisto;
		vector<double> HueActivation;
		vector<double> SaturationActivation;
		vector<double> ValueActivation;
        vector<vector<int> > Fuzzy10BinRulesDefinition;

public:
	Fuzzy10Bins() :HueMembershipValues(32), SaturationMembershipValues(8), 
		ValueMembershipValues(12), Fuzzy10BinRules(48), Fuzzy10BinHisto(10),
		HueActivation(8), SaturationActivation(2), ValueActivation(3), 
		Fuzzy10BinRulesDefinition(48,vector<int>(4))
	{
		KeepPreviuesValues = false;

		HueMembershipValues = { 0, 0, 5, 10,
			5, 10, 35, 50,
			35, 50, 70, 85,
			70, 85, 150, 165,
			150, 165, 195, 205,
			195, 205, 265, 280,
			265, 280, 315, 330,
			315, 330, 360, 360 };

		SaturationMembershipValues = { 0, 0, 10, 75,
			10, 75, 255, 255 };

		ValueMembershipValues = { 0, 0, 10, 75,
			10, 75, 180, 220,
			180, 220, 255, 255 };

		Fuzzy10BinRulesDefinition = {
			{ 0, 0, 0, 2 },
			{ 0, 1, 0, 2 },
			{ 0, 0, 2, 0 },
			{ 0, 0, 1, 1 },
			{ 1, 0, 0, 2 },
			{ 1, 1, 0, 2 },
			{ 1, 0, 2, 0 },
			{ 1, 0, 1, 1 },
			{ 2, 0, 0, 2 },
			{ 2, 1, 0, 2 },
			{ 2, 0, 2, 0 },
			{ 2, 0, 1, 1 },
			{ 3, 0, 0, 2 },
			{ 3, 1, 0, 2 },
			{ 3, 0, 2, 0 },
			{ 3, 0, 1, 1 },
			{ 4, 0, 0, 2 },
			{ 4, 1, 0, 2 },
			{ 4, 0, 2, 0 },
			{ 4, 0, 1, 1 },
			{ 5, 0, 0, 2 },
			{ 5, 1, 0, 2 },
			{ 5, 0, 2, 0 },
			{ 5, 0, 1, 1 },
			{ 6, 0, 0, 2 },
			{ 6, 1, 0, 2 },
			{ 6, 0, 2, 0 },
			{ 6, 0, 1, 1 },
			{ 7, 0, 0, 2 },
			{ 7, 1, 0, 2 },
			{ 7, 0, 2, 0 },
			{ 7, 0, 1, 1 },
			{ 0, 1, 1, 3 },
			{ 0, 1, 2, 3 },
			{ 1, 1, 1, 4 },
			{ 1, 1, 2, 4 },
			{ 2, 1, 1, 5 },
			{ 2, 1, 2, 5 },
			{ 3, 1, 1, 6 },
			{ 3, 1, 2, 6 },
			{ 4, 1, 1, 7 },
			{ 4, 1, 2, 7 },
			{ 5, 1, 1, 8 },
			{ 5, 1, 2, 8 },
			{ 6, 1, 1, 9 },
			{ 6, 1, 2, 9 },
			{ 7, 1, 1, 3 },
			{ 7, 1, 2, 3 }
		};
	}
	Fuzzy10Bins(bool KeepPreviuesValues) :HueMembershipValues(32), SaturationMembershipValues(8),
		ValueMembershipValues(12), Fuzzy10BinRules(48), Fuzzy10BinHisto(10),
		HueActivation(8), SaturationActivation(2), ValueActivation(3),
		Fuzzy10BinRulesDefinition(48, vector<int>(4))
	{
		this->KeepPreviuesValues = KeepPreviuesValues;

		HueMembershipValues = { 0, 0, 5, 10,
			5, 10, 35, 50,
			35, 50, 70, 85,
			70, 85, 150, 165,
			150, 165, 195, 205,
			195, 205, 265, 280,
			265, 280, 315, 330,
			315, 330, 360, 360 };

		SaturationMembershipValues = { 0, 0, 10, 75,
			10, 75, 255, 255 };

		ValueMembershipValues = { 0, 0, 10, 75,
			10, 75, 180, 220,
			180, 220, 255, 255 };

		Fuzzy10BinRulesDefinition = {
			{ 0, 0, 0, 2 },
			{ 0, 1, 0, 2 },
			{ 0, 0, 2, 0 },
			{ 0, 0, 1, 1 },
			{ 1, 0, 0, 2 },
			{ 1, 1, 0, 2 },
			{ 1, 0, 2, 0 },
			{ 1, 0, 1, 1 },
			{ 2, 0, 0, 2 },
			{ 2, 1, 0, 2 },
			{ 2, 0, 2, 0 },
			{ 2, 0, 1, 1 },
			{ 3, 0, 0, 2 },
			{ 3, 1, 0, 2 },
			{ 3, 0, 2, 0 },
			{ 3, 0, 1, 1 },
			{ 4, 0, 0, 2 },
			{ 4, 1, 0, 2 },
			{ 4, 0, 2, 0 },
			{ 4, 0, 1, 1 },
			{ 5, 0, 0, 2 },
			{ 5, 1, 0, 2 },
			{ 5, 0, 2, 0 },
			{ 5, 0, 1, 1 },
			{ 6, 0, 0, 2 },
			{ 6, 1, 0, 2 },
			{ 6, 0, 2, 0 },
			{ 6, 0, 1, 1 },
			{ 7, 0, 0, 2 },
			{ 7, 1, 0, 2 },
			{ 7, 0, 2, 0 },
			{ 7, 0, 1, 1 },
			{ 0, 1, 1, 3 },
			{ 0, 1, 2, 3 },
			{ 1, 1, 1, 4 },
			{ 1, 1, 2, 4 },
			{ 2, 1, 1, 5 },
			{ 2, 1, 2, 5 },
			{ 3, 1, 1, 6 },
			{ 3, 1, 2, 6 },
			{ 4, 1, 1, 7 },
			{ 4, 1, 2, 7 },
			{ 5, 1, 1, 8 },
			{ 5, 1, 2, 8 },
			{ 6, 1, 1, 9 },
			{ 6, 1, 2, 9 },
			{ 7, 1, 1, 3 },
			{ 7, 1, 2, 3 }
		};

		for (int R = 0; R < 48; R++)
		{

			Fuzzy10BinRules[R].Input1 = Fuzzy10BinRulesDefinition[R][0];
			Fuzzy10BinRules[R].Input2 = Fuzzy10BinRulesDefinition[R][1];
			Fuzzy10BinRules[R].Input3 = Fuzzy10BinRulesDefinition[R][2];
			Fuzzy10BinRules[R].Output = Fuzzy10BinRulesDefinition[R][3];

		}

		this->KeepPreviuesValues = KeepPreviuesValues;
	}

	void FindMembershipValueForTriangles(double Input, const vector<double>& Triangles, vector<double>& MembershipFunctionToSave)
	{
		int Temp = 0;

		for (int i = 0; i <= Triangles.size() - 1; i += 4)
		{

			MembershipFunctionToSave[Temp] = 0;

			if (Input >= Triangles[i + 1] && Input <= +Triangles[i + 2])
			{
				MembershipFunctionToSave[Temp] = 1;
			}
   
			if (Input >= Triangles[i] && Input < Triangles[i + 1])
			{
				MembershipFunctionToSave[Temp] = (Input - Triangles[i]) / (Triangles[i + 1] - Triangles[i]);
			}  

			if (Input > Triangles[i + 2] && Input <= Triangles[i + 3])
			{
				MembershipFunctionToSave[Temp] = (Input - Triangles[i + 2]) / (Triangles[i + 2] - Triangles[i + 3]) + 1;
			}

			Temp += 1;
		}

	}

	void LOM_Defazzificator(const vector<FuzzyRules10> Rules, const vector<double>& Input1, const vector<double>& Input2,
		const vector<double>& Input3, vector<double>& ResultTable)
	{
		int RuleActivation = -1;
		double LOM_MAXofMIN = 0;

		for (int i = 0; i < Rules.size(); i++)
		{

			if ((Input1[Rules[i].Input1] > 0) && (Input2[Rules[i].Input2] > 0) && (Input3[Rules[i].Input3] > 0))
			{

				double minValue = 0;
				minValue = min(Input1[Rules[i].Input1], min(Input2[Rules[i].Input2], Input3[Rules[i].Input3]));

				if (minValue > LOM_MAXofMIN)
				{
					LOM_MAXofMIN = minValue;
					RuleActivation = Rules[i].Output;
				}
			}
		}
		ResultTable[RuleActivation]++;
	}

	void MultiParticipate_Equal_Defazzificator(const vector<FuzzyRules10>& Rules, const vector<double>& Input1,
		const vector<double>& Input2, const vector<double>& Input3, vector<double>& ResultTable)
	{

		int RuleActivation = -1;

		for (int i = 0; i < Rules.size(); i++)
		{
			if ((Input1[Rules[i].Input1] > 0) && (Input2[Rules[i].Input2] > 0) && (Input3[Rules[i].Input3] > 0))
			{
				RuleActivation = Rules[i].Output;
				ResultTable[RuleActivation]++;
			}
		}
	}

	void MultiParticipate_Defazzificator(const vector<FuzzyRules10>& Rules, const vector<double>& Input1, const vector<double>& Input2,
		const vector<double>& Input3, vector<double>& ResultTable)
	{

		int RuleActivation = -1;

		for (int i = 0; i < Rules.size(); i++)
		{
			if ((Input1[Rules[i].Input1] > 0) && (Input2[Rules[i].Input2] > 0) && (Input3[Rules[i].Input3] > 0))
			{
				RuleActivation = Rules[i].Output;
				double minValue = 0;
				minValue = min(Input1[Rules[i].Input1], min(Input2[Rules[i].Input2], Input3[Rules[i].Input3]));

				ResultTable[RuleActivation] += minValue;
			}
		}
	}

	vector<double> ApplyFilter(double Hue, double Saturation, double Value, int Method)
	{
		if (KeepPreviuesValues == false)
		{
			for (int i = 0; i < 10; i++)
			{
				Fuzzy10BinHisto[i] = 0;
			}

		}

		FindMembershipValueForTriangles(Hue, HueMembershipValues, HueActivation);
		FindMembershipValueForTriangles(Saturation, SaturationMembershipValues, SaturationActivation);
		FindMembershipValueForTriangles(Value, ValueMembershipValues, ValueActivation);


		if (Method == 0) LOM_Defazzificator(Fuzzy10BinRules, HueActivation, SaturationActivation, ValueActivation, Fuzzy10BinHisto);
		if (Method == 1) MultiParticipate_Equal_Defazzificator(Fuzzy10BinRules, HueActivation, SaturationActivation, ValueActivation, Fuzzy10BinHisto);
		if (Method == 2) MultiParticipate_Defazzificator(Fuzzy10BinRules, HueActivation, SaturationActivation, ValueActivation, Fuzzy10BinHisto);

		return (Fuzzy10BinHisto);
	}
};
#endif
