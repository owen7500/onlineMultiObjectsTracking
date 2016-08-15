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

	void setParams(double tr,double tl,double alplvalue,double lsUpdateRatio=10,double stThreshRatio=0.4);	//设置参数

	void update(const Mat& srcImg);

	//获取前景图像
	//@param index 0代表获取长时前景和短时前景的交集
    //			   1代表获取长时前景与短时前景的差集（(S+L)-(S-L)）
	Mat getForeImg(int index);

	//连通域处理
	void connectMaskAera(Mat &mask_process, int poly1_hull0, float perimScale, int number = 0,
        Rect bounding_box = Rect(), Point contour_centers = Point(-1, -1));

	//计算TI值
	double calcTI(const Mat& curImage, const Mat& preImage);

protected:
	void InitRDBG(const Mat& srcImg);	//初始化

	void updateLTBG(const Mat& srcImg);		//更新长时背景
	void updateSTBG(const Mat& srcImg);		//更新短时背景
	void updateLTFG(const Mat& srcImg);		//更新长时前景
	void updateSTFG(const Mat& srcImg);		//更新短时前景

	//普通更新前景
	void updateCommonFG(const Mat& srcImg, const Mat& bgImg, Mat& fgImg, double threshValue);

	//普通背景更新
	void updateCommonBG(const Mat& srcImg, Mat& bgImg,double updateRatio);

	//获取前景图像的交集
	Mat getInterSectionFG(const Mat& ltFGImg, const Mat& stFGImg);

	//获取两图像像素点的并集（剔除点）
	void getUnionImgPixel(const Mat& FGImg, const Mat& bgImg,Mat& unionPixelImg);

	/*计算短时前景中前景与背景的比例*/
	double calcFBRatio();

	//计算平均亮度
	double calcAverageLumi(const Mat& Img,const Mat& rejectedPixels);

	//更新EAIC
	void updateEAIC(const Mat& srcImg);

	//获取随机采样点
	Mat getSamplePixel(int cols,int rows, double sampleRatio);

	//光照补偿处理
	void compensationProcess(const Mat& srcImg, double Ti, int decreaseCounter);

	void BgCompensation(const Mat& srcImg, Mat& bgImg, double EAIC, double Ti, int decreaseCounter);

	void getImgSubstractedSet(const Mat& subtractor,const Mat& minuend,Mat& result);

	double calcAvgPixelValue(const Mat& diffMat);
private:
	Mat ltBG;	//长时背景
	Mat stBG;	//短时背景
	Mat ltFG;	//长时前景
	Mat stFG;	//短时前景

	Mat preImage;	//前一帧图像
	double preTI;	//前一帧TI值
	double curTI;	//当前TI值
	double TImaxThresh;	//发生光照突变时，TI的最差值
	int frameIndex;

	double Tr;	//光照突变的阈值
	double Tl;	//长时前景阈值
	double Tlu;	//下面两个参数用光照补偿的前景阈值
	double Tld;

	double Ts;	//短时前景阈值
	double Tsu;	//光照补偿的前景阈值

	double alphaL;	//长时背景更新率
	double alphaS;	//短时背景更新率,等于10倍长时更新率

	double fbRatio;	//前景与背景的比例
	double ltEAIC;	//长时平均亮度差
	double stEAIC;	//短时平均亮度差

	double bgUpdateMultiRatio;
	double lstMultiThresh;
};
