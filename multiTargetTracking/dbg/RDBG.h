#include <string>
#include <opencv.hpp>
using namespace cv;
using namespace std;

#define CV_CVX_WHITE    CV_RGB(0xff,0xff,0xff)
#define CV_CVX_BLACK    CV_RGB(0x00,0x00,0x00)

class RDBG
{
public:
	RDBG();
	~RDBG();

	void setParams(double tr,double tl,double alplvalue,double lsUpdateRatio=10,double stThreshRatio=0.4);	//���ò���

	void update(const Mat& srcImg);

	//��ȡǰ��ͼ��
	//@param index 0�����ȡ��ʱǰ���Ͷ�ʱǰ���Ľ���
    //			   1�����ȡ��ʱǰ�����ʱǰ���Ĳ��(S+L)-(S-L)��
	Mat getForeImg(int index);

	//��ͨ����
	void connectMaskAera(Mat &mask_process, int poly1_hull0, float perimScale, int number = 0,
        Rect bounding_box = Rect(), Point contour_centers = Point(-1, -1));

	//����TIֵ
	double calcTI(const Mat& curImage, const Mat& preImage);

protected:
	void InitRDBG(const Mat& srcImg);	//��ʼ��

	void updateLTBG(const Mat& srcImg);		//���³�ʱ����
	void updateSTBG(const Mat& srcImg);		//���¶�ʱ����
	void updateLTFG(const Mat& srcImg);		//���³�ʱǰ��
	void updateSTFG(const Mat& srcImg);		//���¶�ʱǰ��

	//��ͨ����ǰ��
	void updateCommonFG(const Mat& srcImg, const Mat& bgImg, Mat& fgImg, double threshValue);

	//��ͨ��������
	void updateCommonBG(const Mat& srcImg, Mat& bgImg,double updateRatio);

	//��ȡǰ��ͼ��Ľ���
	Mat getInterSectionFG(const Mat& ltFGImg, const Mat& stFGImg);

	//��ȡ��ͼ�����ص�Ĳ������޳��㣩
	void getUnionImgPixel(const Mat& FGImg, const Mat& bgImg,Mat& unionPixelImg);

	/*�����ʱǰ����ǰ���뱳���ı���*/
	double calcFBRatio();

	//����ƽ������
	double calcAverageLumi(const Mat& Img,const Mat& rejectedPixels);

	//����EAIC
	void updateEAIC(const Mat& srcImg);

	//��ȡ���������
	Mat getSamplePixel(int cols,int rows, double sampleRatio);

	//���ղ�������
	void compensationProcess(const Mat& srcImg, double Ti, int decreaseCounter);

	void BgCompensation(const Mat& srcImg, Mat& bgImg, double EAIC, double Ti, int decreaseCounter);

	void getImgSubstractedSet(const Mat& subtractor,const Mat& minuend,Mat& result);

	double calcAvgPixelValue(const Mat& diffMat);
private:
	Mat ltBG;	//��ʱ����
	Mat stBG;	//��ʱ����
	Mat ltFG;	//��ʱǰ��
	Mat stFG;	//��ʱǰ��

	Mat preImage;	//ǰһ֡ͼ��
	double preTI;	//ǰһ֡TIֵ
	double curTI;	//��ǰTIֵ
	double TImaxThresh;	//��������ͻ��ʱ��TI�����ֵ
	int frameIndex;

	double Tr;	//����ͻ�����ֵ
	double Tl;	//��ʱǰ����ֵ
	double Tlu;	//�������������ù��ղ�����ǰ����ֵ
	double Tld;

	double Ts;	//��ʱǰ����ֵ
	double Tsu;	//���ղ�����ǰ����ֵ

	double alphaL;	//��ʱ����������
	double alphaS;	//��ʱ����������,����10����ʱ������

	double fbRatio;	//ǰ���뱳���ı���
	double ltEAIC;	//��ʱƽ�����Ȳ�
	double stEAIC;	//��ʱƽ�����Ȳ�

	double bgUpdateMultiRatio;
	double lstMultiThresh;
};
