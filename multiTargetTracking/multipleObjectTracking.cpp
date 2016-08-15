//Written by  Owen   2016

#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#include "Object.h"
#include "vibe.h"
#include "FrameDifferenceBGS.h"
#include "dp\DPAdaptiveMedianBGS.h"
#include "objectTracker.h"

using namespace std;

//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "background model";
const string windowName2 = "Thresholded Image";



int main(int argc, char* argv[])
{
    //Matrix to store each frame of the webcam feed
    Mat cameraFeed;
    Mat threshold;
    Mat backmodelImg;
    Mat transedImg;

    IBGS *bgs = new DPAdaptiveMedianBGS;
    //IBGS *bgs = new FrameDifferenceBGS;
    //IBGS *bgs = new Vibe;
    ObjectTracker *autoTracker = new ObjectTracker;

    char* videoPath = "E:\\srcVideo\\test.avi";
    //char* videoPath = "E:\\srcVideo\\exp\\Browse1.mpg";

    VideoCapture capture;
    //open capture object at location zero (default location for webcam)
    capture.open(videoPath);
    //capture.set(CV_CAP_PROP_POS_FRAMES,3200);


    //string imgsPath = "E:\\srcVideo\\stdDataSet\\seq02-img-left\\";
    //string fileNamePrefix = "image_";
    //string fileNameSubfix = "_0.png";
    //static int imgIndex = 930;


    while (1){
        //store image to matrix
        //capture.read(cameraFeed);
        capture.read(transedImg);
        getTransImg(transedImg, cameraFeed);
        /*string imgName = imgsPath + fileNamePrefix + int2String(imgIndex++) + fileNameSubfix;*/

        //cameraFeed = imread(imgName.c_str());
        //cout << imgName << endl;

        if (!cameraFeed.data)
        {
            return -1;
        }

        bgs->process(cameraFeed, threshold, backmodelImg);

        int timeStamp = capture.get(CV_CAP_PROP_POS_FRAMES);

        if (!threshold.empty())
        {
            autoTracker->morphOps(threshold, 1, 3);

            /// Show the image
            //autoTracker->trackFilteredObject(threshold, cameraFeed);
            autoTracker->realTrackFilteredObjects(threshold, cameraFeed, timeStamp, 18);

            //show frames
            imshow(windowName2, threshold);
            imshow(windowName, cameraFeed);
            if (!backmodelImg.empty())
                imshow(windowName1, backmodelImg);

            //delay 30ms so that screen can refresh.
            //image will not appear without this waitKey() command
            waitKey(30);
        }
    }
    return 0;
}



