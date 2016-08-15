#include "objectTracker.h"


ObjectTracker::ObjectTracker() :FRAME_HEIGHT(288),FRAME_WIDTH(352),MAX_NUM_OBJECTS(50)
{
    MIN_RECT_HEIGHT = FRAME_HEIGHT/4;
    MIN_RECT_WIDTH = FRAME_WIDTH/4;
    MIN_DIST_THRESH = FRAME_HEIGHT/3;	//if distance of two point less than this value,
                                        //they should be the same class

    MAX_RECT_HEIGHT = FRAME_HEIGHT / 3;
    MAX_RECT_WIDTH = FRAME_WIDTH / 3;

    MAX_OBJECT_AREA = FRAME_WIDTH*FRAME_HEIGHT / 1.5;
    MIN_OBJECT_AREA = 40*40;

    videoSegmentFirstFlag = true;
    ObjectNum = 0;
    delayCounter = 0;
    TrackedTargetThresh = 50;
    targetLifeTimeThresh = 10;
}

void ObjectTracker::setTrackingFeatureType(ObjectTrackerFeatureType type)
{
    oType = type;
}

std::string ObjectTracker::intToString(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

void ObjectTracker::morphOps(Mat &thresh,int erodeTimes,int dilateTimes)
{
    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle

    Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

    erode(thresh, thresh, erodeElement,cv::Point(-1,-1),erodeTimes);

    dilate(thresh, thresh, dilateElement,cv::Point(-1,-1),dilateTimes);
}

void ObjectTracker::realTrackFilteredObjects(const Mat &binaryImage, Mat &cameraFeed, int timeStamp, int stairs)
{

    Mat temp;
    binaryImage.copyTo(temp);
    if(temp.channels()==3)
    {
        cvtColor(temp,temp,COLOR_BGR2GRAY);
        cv::threshold(temp,temp,5,255,THRESH_BINARY_INV);
    }

    Mat storeOriginImg = cameraFeed.clone();

    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //the key point of the contours get from the optimal Rectangle
    vector<Rect> conMainRect;

    //find contours of filtered image using openCV findContours function
    findContours(temp, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    if (!curImgTargetVector.empty())
        curImgTargetVector.clear();

    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0)
    {
        int numObjects = hierarchy.size();

        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if (numObjects<MAX_NUM_OBJECTS)
        {
            //contour search method,orderly search
            for (int index = 0; index >= 0; index = hierarchy[index][0])
            {
                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
                cv::Rect bundRect = boundingRect(contours[index]);
                //cv::Rect bundRect = cv::minAreaRect(contours[index]);
                conMainRect.push_back(bundRect);

                //get the biggest area
                refArea = refArea<area ? area : refArea;
            }

            //if the refArea is less than 40 px by 40px then it is probably just noise
            //if the refArea is the same as the 3/2 of the image size, probably just a bad filter
            if (refArea>MIN_OBJECT_AREA && refArea<MAX_OBJECT_AREA)
            {
                objectFound = true;
                delayCounter = 0;
            }
            else
            {
                objectFound = false;
                delayCounter++;			//add the delayCounter

                //clear the vector of target object
                //if the counter larger than 25,it means another video segment
                //and the target vector should be cleared!
                if (delayCounter>25)
                {
                    videoSegmentFirstFlag = true;
                    if (targetVector.size() > 0)
                    {
                        for (vector<targetObject>::iterator iter = targetVector.begin(); iter != targetVector.end();iter++)
                        {
                            saveObjectImg(*iter);
                            if ((*iter).getTargetLifeTime()>targetLifeTimeThresh/*&&((*iter).getTargetLifeTime()%3==0)*/)
                            {
                                trackedTargetVector.push_back(*iter);
                                if (trackedTargetVector.size() > TrackedTargetThresh)
                                    trackedTargetVector.erase(trackedTargetVector.begin());
                            }
                        }
                        targetVector.clear();
                    }
                }
            }
            //let user know you found an object
            if (objectFound)
            {
                vector<Rect> reConstructedRect;

                //reConstruct the rectangles,in order to joint the nearby rectangles
                //in case of the same object
                clusterAndConstructRect(conMainRect, reConstructedRect);

                //ergodic the vector of object rectangles
                //to track real target and draw it
                for (int i = 0; i < reConstructedRect.size(); i++)
                {
                    //center point and direction of current rectangle
                    Point cPoint = Point((reConstructedRect[i].x + reConstructedRect[i].width / 2), (reConstructedRect[i].y + reConstructedRect[i].height / 2));
                    int direction = getDerection(cPoint,cameraFeed.rows,cameraFeed.cols);

                    //construct a targetObject
                    targetObject targetObj(storeOriginImg,oType, timeStamp, cPoint, reConstructedRect[i],
                                                          direction, direction, ObjectNum, cPoint);
                    targetObj.setObjectStairs(stairs);

                    //only pick the motion area
                    //Mat motionImg = threshColorImg(storeOriginImg, temp);

                    //Mat roiImg = Mat(motionImg, reConstructedRect[i]);
                    Mat roiImg = Mat(storeOriginImg,reConstructedRect[i]);

                    targetObj.insertObjectImg(roiImg);

                    //store the target object into the current image target vector
                    curImgTargetVector.push_back(targetObj);


                    //if first object in this video segment
                    //push back all the objects in this frame into the targetVector
                    if (videoSegmentFirstFlag)
                    {
                        //draw the object
                        rectangle(cameraFeed, reConstructedRect[i], Scalar(225, 125, 125), 3);
                        string text = intToString(ObjectNum);
                        putText(cameraFeed,text,cPoint,1,2,Scalar(255,125,50),3);

                        targetVector.push_back(targetObj);
                        ObjectNum++;
                    }
                    //if not the first frame of the segment
                    //we should compare the current object to the previous objects
                    else
                    {
                        bool isCompared = false;
                        int objectIndex = 0;

                        //compare all the previous targets
                        for (int k = 0; k < targetVector.size();k++)
                        {
                            if (targetVector[k].isSameTarget(targetObj))
                            {
                                isCompared = true;
                                //update the properties of the target object
                                targetVector[k].updateTarget(targetObj);
                                objectIndex = targetVector[k].getObjectIndex();
                                break;		//if find a compared target,break the loop
                            }
                        }
                        //if not compared,
                        if (!isCompared)
                        {
                            rectangle(cameraFeed, reConstructedRect[i], Scalar(25, 125, 125), 3);
                            string text = intToString(ObjectNum);
                            putText(cameraFeed, text, cPoint, 1, 2, Scalar(255, 0, 0),3);

                            targetVector.push_back(targetObj);
                            ObjectNum++;
                        }
                        else
                        {
                            rectangle(cameraFeed, reConstructedRect[i], Scalar(255, 200, 50), 3);
                            string text = intToString(objectIndex);
                            putText(cameraFeed, text, cPoint, 1, 2, Scalar(25, 225, 25,3),3);
                        }

                    }
                }
                //reset the flag;
                if (videoSegmentFirstFlag)
                    videoSegmentFirstFlag = false;
            }

        }
        else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
    }
}

cv::Point ObjectTracker::getCenterPoint(const Mat &binaryImg)
{
    int PixelCount = 0;                 //前景点个数
    int x_rows(0), y_cols(0);                  //行坐标和，列坐标和

    int x_center(0), y_center(0);              //中心坐标（x，y）

    int chanels = binaryImg.channels();

    for (int i = 0; i < binaryImg.rows; i++)
    {
        const uchar *ptr = binaryImg.ptr<uchar>(i);
        for (int j = 0; j < binaryImg.cols*chanels; j += chanels)
        {
            if (ptr[j] == 255)
            {
                PixelCount++;
                x_rows += i;
                y_cols += j / chanels;
            }
        }
    }

    x_center = x_rows / PixelCount;
    y_center = y_cols / PixelCount;
    cv::Point centerPoint(x_center, y_center);

    return centerPoint;
}

int ObjectTracker::getDerection(const Point &center, int rows, int cols)
{
    if ((center.x < rows / 2) && (center.y < cols / 2)) //图片的左上角
    {
        return TOPLEFT;
    }
    else if ((center.x < rows / 2) && (center.y >= cols / 2)) //右上角
    {
        return TOPRIGHT;
    }
    else if ((center.x >= rows / 2) && (center.y < cols / 2)) //左下角
    {
        return BUTTOMLEFT;
    }
    else          //右下角
    {
        return BUTTOMRIGHT;
    }
}

cv::Mat ObjectTracker::getColorForeImg(const Mat& binaryImg, const Mat& srcImg)
{
    int chanels = srcImg.channels();    //图像通道数
    Mat temp = srcImg.clone();

    for (int i = 0; i < temp.rows; i++)
    {
        uchar *tptr = temp.ptr<uchar>(i);
        const uchar *binaryptr = binaryImg.ptr<uchar>(i);
        for (int j = 0; j < temp.cols*chanels; j += chanels)
        {
            if (binaryptr[j / chanels] != 255)
            {
                tptr[j] = 0;
                tptr[j + 1] = 0;
                tptr[j + 2] = 0;
            }

        }
    }
    return temp;
}

/*reorder the array in ascending order*/
void ObjectTracker::bubleSort(int *arr, int len)
{
    int temp;
    for (int i = 0; i < len;i++)
    {
        for (int j = 0; j < (len-i-1);j++)
        {
            if (arr[j]>arr[j + 1])
            {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void ObjectTracker::clusterAndConstructRect(const vector<Rect>& mainRect, vector<Rect> &reconRectVector)
{
    //vector contains the index of the center point
    vector<int> centerIndexArray;
    int processedNum=0;

    Point topLeft, rightButtom;

    //set the default point value;
    topLeft.x = 352;
    topLeft.y = 288;
    rightButtom.x = 0;
    rightButtom.y = 0;

    if (mainRect.size()==1)
    {
        reconRectVector.push_back(mainRect[0]);
        return;
    }

    //initialize the vectors
    for (int i = 0; i < mainRect.size();i++)
    {
        centerIndexArray.push_back(-1);
    }

    for (int i = 0; i < mainRect.size();i++)
    {
        if (processedNum == mainRect.size())
            break;

        if (centerIndexArray[i] == -1)
        {
            centerIndexArray[i] = i;
            ++processedNum;

            topLeft.x = mainRect[i].x;
            topLeft.y = mainRect[i].y;

            rightButtom.x = mainRect[i].x+mainRect[i].width;
            rightButtom.y = mainRect[i].y+mainRect[i].height;

            for (int j = i + 1; j < mainRect.size(); j++)
            {
                //dist = getRectDist(mainRect[i], mainRect[j])
                if (isNearBy_rect(mainRect[i], mainRect[j]))
                {
                    centerIndexArray[j] = i;
                    ++processedNum;

                    topLeft.x = std::min(topLeft.x, mainRect[j].x);
                    topLeft.y = std::min(topLeft.y, mainRect[j].y);
                    rightButtom.x = std::max(rightButtom.x, (mainRect[j].x+mainRect[j].width));
                    rightButtom.y = std::max(rightButtom.y, (mainRect[j].y+mainRect[j].height));
                }
            }

            Rect iRect = Rect(topLeft,rightButtom);
            reconRectVector.push_back(iRect);
        }
    }
}

double ObjectTracker::getPointDist(Point Pa, Point Pb)
{
    return sqrt((Pa.x - Pb.x)*(Pa.x - Pb.x) + (Pa.y - Pb.y)*(Pa.y - Pb.y));
}

double ObjectTracker::getRectDist(Rect Ra, Rect Rb)
{
    return sqrt((Ra.x - Rb.x)*(Ra.x - Rb.x) + (Ra.y - Rb.y)*(Ra.y - Rb.y));
}

bool ObjectTracker::isNearBy_rect(Rect Ra, Rect Rb)
{
    int raTopx = Ra.x;
    int raTopy = Ra.y;
    int raButtomx = Ra.x + Ra.width;
    int raButtomy = Ra.y + Ra.height;

    int rbTopx = Rb.x;
    int rbTopy = Rb.y;
    int rbButtomx = Rb.x + Rb.width;
    int rbButtomy = Rb.y + Rb.height;

    int xMinDist, yMinDist;

    int xToptoTopDist = abs(raTopx - rbTopx);
    int xToptoButtomDist = abs((raTopx - rbButtomx));
    int xButtomtoTopDist = abs(raButtomx - rbTopx);
    int xButtomtoButtomDist = abs(raButtomx - rbButtomx);

    //find the min distance of the x coordinate
    xMinDist = std::min(xToptoTopDist, std::min(xToptoButtomDist, std::min(xButtomtoTopDist, xButtomtoButtomDist)));

    //calculate the distance of the y coordinate
    int yToptoTopDist = abs(raTopy - rbTopy);
    int yToptoButtomDist = abs((raTopy - rbButtomy));
    int yButtomtoTopDist = abs(raButtomy - rbTopy);
    int yButtomtoButtomDist = abs(raButtomy - rbButtomy);

    //find the min distance of the y coordinate
    yMinDist = std::min(yToptoTopDist, std::min(yToptoButtomDist, std::min(yButtomtoButtomDist, yButtomtoTopDist)));

    //whether the rectangle is nearby
    return ((xMinDist < MIN_RECT_WIDTH/2) && (yMinDist < MIN_RECT_HEIGHT/2));
}

void ObjectTracker::setTrackedTargetThresh(int objectNum)
{
    TrackedTargetThresh = objectNum;
}

void ObjectTracker::searchNeighbourCamera(Mat &camera, vector<targetObject>& neighTargetVector)
{
    if (neighTargetVector.empty())
        return;

    double p=0;
    bool best_Comp= false;
    float maxP = 0;
    int compIndex = -1;

    for (int j = 0; j < curImgTargetVector.size(); j++)
    {
        for (int i = 0; i < neighTargetVector.size(); i++)
        {
            if (curImgTargetVector[j].isTheNearSame(neighTargetVector[i], p))
            {
                maxP = p>maxP ? p : maxP;
                compIndex = i;
                best_Comp = true;
            }
        }
        if (best_Comp)
        {
            int neighStair = neighTargetVector[compIndex].getTargetStairs();
            int neighIndex = neighTargetVector[compIndex].getObjectIndex();

            string neighMsg = intToString(neighStair) + string("_") + intToString(neighIndex);
            Point pos = curImgTargetVector[j].getObjectCenterPoint() + Point(0, 5);

            string curMsg = intToString(curImgTargetVector[j].getTargetStairs())
                + string("_") + intToString(curImgTargetVector[j].getObjectIndex());

            cout << "find a compared object:" << curMsg << "<=>" << neighMsg << endl;
            putText(camera, neighMsg, pos, CV_FONT_HERSHEY_SCRIPT_SIMPLEX, 1, Scalar(0, 0, 255), 1);
        }
    }
}

void ObjectTracker::saveObjectImg(targetObject& curTarget)
{
    if (curTarget.getTargetLifeTime()>targetLifeTimeThresh)
    {
        vector<Mat> tgtVector = curTarget.getTargetImgVector();
        int stairsIndex = curTarget.getTargetStairs();
        int objectIndex = curTarget.getObjectIndex();
        int startFrameNum = curTarget.getTargetTimeStamp();

        cout << "stairs" << stairsIndex << ":writing image of object-" << objectIndex << endl;

        string dirName = intToString(stairsIndex);
        if (_access(dirName.c_str(),0)==-1)
        {
            _mkdir(dirName.c_str());
        }
        string imgName = dirName + string("\\") + intToString(startFrameNum) + string("_") + intToString(objectIndex)+string("_");

        for (int i = 0; i < tgtVector.size();i++)
        {
            string imgName_i = imgName + intToString(i) + string(".jpg");
            imwrite(imgName_i.c_str(), tgtVector[i]);
        }
    }
}

cv::Mat ObjectTracker::threshColorImg(const Mat& colorImg,const Mat& maskImg)
{
    //check the arguments
    assert(maskImg.channels()==1);
    assert(colorImg.channels()==3);

    Mat tempImg = colorImg.clone();

    int colorCols = colorImg.cols*colorImg.channels();
    int rows = colorImg.rows;
    int cchannels = colorImg.channels();

    if (colorImg.isContinuous()&&maskImg.isContinuous())
    {
        colorCols = colorCols*rows;
        rows = 1;
    }
    for (int j = 0; j < rows;j++)
    {
        unsigned char* cdata = tempImg.ptr<uchar>(j);
        const uchar* mdata = maskImg.ptr<uchar>(j);
        for (int i = 0; i < colorCols;i++)
        {
            if (mdata[i / cchannels] == 0)
                cdata[i] = 0;
        }
    }
    return tempImg;
}

