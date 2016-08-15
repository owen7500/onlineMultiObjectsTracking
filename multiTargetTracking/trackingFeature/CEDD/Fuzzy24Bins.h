#ifndef _FUZZY24BINS_H_
#define _FUZZY24BINS_H_

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

typedef struct _FuzzyRules24
{
	int Input1;
	int Input2;
	int Output;
}FuzzyRules24;

class Fuzzy24Bins
{
private:
	bool KeepPreviusValues;
	vector<double> ResultsTable;
	vector<double> Fuzzy24BinHisto;
	vector<double> SaturationMembershipValues;
	vector<double> ValueMembershipValues;
	vector<double> SaturationActivation;
	vector<double> ValueActivation;
	vector<vector<int>> Fuzzy24BinRulesDefinition;
	vector<FuzzyRules24> Fuzzy24BinRules;
public:
	Fuzzy24Bins() :ResultsTable(3), Fuzzy24BinHisto(24), SaturationMembershipValues(8),
		ValueMembershipValues(8), Fuzzy24BinRules(4), SaturationActivation(2),
		ValueActivation(2), Fuzzy24BinRulesDefinition(4,vector<int>(3))
	{
		this->KeepPreviusValues = false;
		
		SaturationMembershipValues = {  0, 0, 68, 188,
			68, 188, 255, 255};
		ValueMembershipValues = {  0, 0, 68, 188,
			68, 188, 255, 255};

		Fuzzy24BinRulesDefinition = {
			{ 1, 1, 1 },
			{ 0, 0, 2 },
			{ 0, 1, 0 },
			{ 1, 0, 2 }
		};
	}
	Fuzzy24Bins(bool KeepPreviuesValues) :ResultsTable(3), Fuzzy24BinHisto(24), SaturationMembershipValues(8),
		ValueMembershipValues(8), Fuzzy24BinRules(4), SaturationActivation(2),
		ValueActivation(2), Fuzzy24BinRulesDefinition(4, vector<int>(3))
	{
		SaturationMembershipValues = { 0, 0, 68, 188,
			68, 188, 255, 255 };
		ValueMembershipValues = {  0, 0, 68, 188,
			68, 188, 255, 255};

		Fuzzy24BinRulesDefinition = {
			{ 1, 1, 1 },
			{ 0, 0, 2 },
			{ 0, 1, 0 },
			{ 1, 0, 2 }
		};

		for (int R = 0; R < 4; R++)
		{
			Fuzzy24BinRules[R].Input1 = Fuzzy24BinRulesDefinition[R][0];
			Fuzzy24BinRules[R].Input2 = Fuzzy24BinRulesDefinition[R][1];
			Fuzzy24BinRules[R].Output = Fuzzy24BinRulesDefinition[R][2];
		}
		this->KeepPreviusValues = KeepPreviuesValues;
	}

	void FindMembershipValueForTriangles(double Input, const vector<double>& Triangles,
		vector<double>& MembershipFunctionToSave)
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

	void LOM_Defazzificator(const vector<FuzzyRules24>& Rules, const vector<double>& Input1, const vector<double>& Input2,
		vector<double>& ResultTable)
	{
		int RuleActivation = -1;
		double LOM_MAXofMIN = 0;

		for (int i = 0; i < Rules.size(); i++)
		{

			if ((Input1[Rules[i].Input1] > 0) && (Input2[Rules[i].Input2] > 0))
			{

				double Min = 0;
				Min = min(Input1[Rules[i].Input1], Input2[Rules[i].Input2]);

				if (Min > LOM_MAXofMIN)
				{
					LOM_MAXofMIN = Min;
					RuleActivation = Rules[i].Output;
				}
			}
		}
		ResultTable[RuleActivation]++;
	}

	void MultiParticipate_Equal_Defazzificator(const vector<FuzzyRules24>& Rules, const vector<double>& Input1, 
		const vector<double>& Input2, vector<double>& ResultTable)
	{

		int RuleActivation = -1;

		for (int i = 0; i < Rules.size(); i++)
		{
			if ((Input1[Rules[i].Input1] > 0) && (Input2[Rules[i].Input2] > 0))
			{
				RuleActivation = Rules[i].Output;
				ResultTable[RuleActivation]++;
			}

		}
	}

	void MultiParticipate_Defazzificator(const vector<FuzzyRules24>& Rules, const vector<double>& Input1, 
		const vector<double>& Input2, vector<double>& ResultTable)
	{

		int RuleActivation = -1;
		double Min = 0;
		for (int i = 0; i < Rules.size(); i++)
		{
			if ((Input1[Rules[i].Input1] > 0) && (Input2[Rules[i].Input2] > 0))
			{
				Min = min(Input1[Rules[i].Input1], Input2[Rules[i].Input2]);

				RuleActivation = Rules[i].Output;
				ResultTable[RuleActivation] += Min;
			}

		}
	}


	vector<double> ApplyFilter(double Hue, double Saturation, double Value, const vector<double>& ColorValues, int Method)
	{
		ResultsTable[0] = 0;
		ResultsTable[1] = 0;
		ResultsTable[2] = 0;
		double Temp = 0;


		FindMembershipValueForTriangles(Saturation, SaturationMembershipValues, SaturationActivation);
		FindMembershipValueForTriangles(Value, ValueMembershipValues, ValueActivation);


		if (this->KeepPreviusValues == false)
		{
			for (int i = 0; i < 24; i++)
			{
				Fuzzy24BinHisto[i] = 0;
			}
		}

		for (int i = 3; i < 10; i++)
		{
			Temp += ColorValues[i];
		}

		if (Temp > 0)
		{
			if (Method == 0) LOM_Defazzificator(Fuzzy24BinRules, SaturationActivation, ValueActivation, ResultsTable);
			if (Method == 1) MultiParticipate_Equal_Defazzificator(Fuzzy24BinRules, SaturationActivation, ValueActivation, ResultsTable);
			if (Method == 2) MultiParticipate_Defazzificator(Fuzzy24BinRules, SaturationActivation, ValueActivation, ResultsTable);
		}

		for (int i = 0; i < 3; i++)
		{
			Fuzzy24BinHisto[i] += ColorValues[i];
		}


		for (int i = 3; i < 10; i++)
		{
			Fuzzy24BinHisto[(i - 2) * 3] += ColorValues[i] * ResultsTable[0];
			Fuzzy24BinHisto[(i - 2) * 3 + 1] += ColorValues[i] * ResultsTable[1];
			Fuzzy24BinHisto[(i - 2) * 3 + 2] += ColorValues[i] * ResultsTable[2];
		}

		return (Fuzzy24BinHisto);
	}

};
#endif