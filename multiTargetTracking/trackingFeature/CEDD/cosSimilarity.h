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

		//相似度
		double simi = 0.0;
		double dotvalue = calcDotProduct(sFeature, destFeature);	//点积
		double destMetric = calcMetric(destFeature);				//距离
		simi = dotvalue/(sMetric*destMetric);						//余弦相似度

		return simi;
	}

protected:
	//判断是否为零向量
	bool isZearoVector(vector<double> v)
	{
		for (int i = 0; i < v.size();i++)
		{
			if (v[i] != 0)
				return false;
		}
		return true;
	}

	//计算向量距离
	double calcMetric(vector<double> v)
	{
		double metric = 0.0;

		for (vector<double>::iterator it = v.begin(); it != v.end();it++)
		{
			metric += (*it)*(*it);
		}
		return sqrt(metric);
	}
	//计算向量点击
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