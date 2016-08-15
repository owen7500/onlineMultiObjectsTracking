/*
This file is part of BGSLibrary.

BGSLibrary is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BGSLibrary is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BGSLibrary.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "DPAdaptiveMedianBGS.h"
#include <fstream>

DPAdaptiveMedianBGS::DPAdaptiveMedianBGS() : firstTime(true), frameNumber(0),
    threshold(40), samplingRate(7), learningFrames(30), showOutput(false)
{
  std::cout << "DPAdaptiveMedianBGS()" << std::endl;
}

DPAdaptiveMedianBGS::~DPAdaptiveMedianBGS()
{
  std::cout << "~DPAdaptiveMedianBGS()" << std::endl;
}

void DPAdaptiveMedianBGS::process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel)
{
  if(img_input.empty())
    return;

//  loadConfig();

//  if(firstTime)
//    saveConfig();

  frame = new IplImage(img_input);
  
  if(firstTime)
    frame_data.ReleaseMemory(false);
  frame_data = frame;

  if(firstTime)
  {
    int width	= img_input.size().width;
    int height = img_input.size().height;

    lowThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    lowThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;

    highThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    highThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;

    params.SetFrameSize(width, height);
    params.LowThreshold() = threshold;
    params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
    params.SamplingRate() = samplingRate;
    params.LearningFrames() = learningFrames;
    
    bgs.Initalize(params);
    bgs.InitModel(frame_data);
  }

  bgs.Subtract(frameNumber, frame_data, lowThresholdMask, highThresholdMask);
  lowThresholdMask.Clear();
  bgs.Update(frameNumber, frame_data, lowThresholdMask);
  
  //cv::Mat foreground(highThresholdMask.Ptr());
  cv::Mat foreground = cv::cvarrToMat(highThresholdMask.Ptr());

  if (showOutput)
  {
	  cv::imshow("Adaptive Median (McFarlane&Schofield)", foreground);
  }
  
  foreground.copyTo(img_output);

  delete frame;
  firstTime = false;
  frameNumber++;
}

void DPAdaptiveMedianBGS::saveConfig()
{
    cv::FileStorage fs;
    std::ofstream ffs;
    ffs.open("DPAdaptiveMedianBGS.xml");
    ffs.close();

    if(!fs.open("DPAdaptiveMedianBGS.xml",cv::FileStorage::WRITE))
        throw "can not open file";
    else
    {
        fs<<"threshold"<<threshold;
        fs<<"samplingRate"<< samplingRate;
        fs<<"learningFrames"<<learningFrames;
        fs<<"showOutput"<<showOutput;
    }
}

void DPAdaptiveMedianBGS::loadConfig()
{
    cv::FileStorage fs("DPAdaptiveMedianBGS.xml",cv::FileStorage::READ);

    if(!fs.isOpened())
    {
        std::cout<<"can not open file!"<<std::endl;
        return;
    }
    else
    {
        threshold = (int)fs["threshold"];
        samplingRate = (int)fs["samplingRate"];
        learningFrames = (int)fs["learningFrames"];
        showOutput = (int)fs["showOutput"];
    }
}

void DPAdaptiveMedianBGS::setParam(int threshvalue/*=40*/, int splRate/*= 7*/, int lFrames/*=30*/, bool showOpt/*=true*/)
{
	threshold = threshvalue;
	samplingRate = splRate;
	learningFrames = lFrames;
	showOutput = showOpt;
}
