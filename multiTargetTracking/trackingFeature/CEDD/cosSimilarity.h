#ifndef _COSSIMILARITY_H_
#define _COSSIMILARITY_H_

class CosSimilarity
{
private:
	vector<double> sFeature;
	double sMetric;
public:
	CosSimilarity(vector<double> ceddFeature) :sFeature(ceddFeature){
		sMetric = calcMetric(sFeature);
	};
	~CosSimilarity(){};

	double getSimilarity(const vector<double>& destFeature)
	{
		if (isZearoVector(sFeature) || isZearoVector(destFeature))
			return 0;

		//���ƶ�
		double simi = 0.0;
		double dotvalue = calcDotProduct(sFeature, destFeature);	//���
		double destMetric = calcMetric(destFeature);				//����
		simi = dotvalue/(sMetric*destMetric);						//�������ƶ�

		return simi;
	}

protected:
	//�ж��Ƿ�Ϊ������
	bool isZearoVector(vector<double> v)
	{
		for (int i = 0; i < v.size();i++)
		{
			if (v[i] != 0)
				return false;
		}
		return true;
	}

	//������������
	double calcMetric(vector<double> v)
	{
		double metric = 0.0;

		for (vector<double>::iterator it = v.begin(); it != v.end();it++)
		{
			metric += (*it)*(*it);
		}
		return sqrt(metric);
	}
	//�����������
	double calcDotProduct(vector<double> v1, vector<double> v2)
	{
		assert(v1.size() == v2.size());

		double dotProduct = 0.0;

		for (int i = 0; i < v1.size();i++)
		{
			dotProduct += v1[i] * v2[i];
		}

		return dotProduct;
	}
};
#endif