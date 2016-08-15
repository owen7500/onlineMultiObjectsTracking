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
#include "FrameDifferenceBGS.h"

FrameDifferenceBGS::FrameDifferenceBGS() : firstTime(true), enableThreshold(true), threshold(15), showOutput(false)
{
  std::cout << "FrameDifferenceBGS()" << std::endl;
}

FrameDifferenceBGS::~FrameDifferenceBGS()
{
  std::cout << "~FrameDifferenceBGS()" << std::endl;
}

void FrameDifferenceBGS::process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel)
{
  if(img_input.empty())
    return;

//  loadConfig();

//  if(firstTime)
//    saveConfig();

  //高斯去噪
  GaussianBlur(img_input, img_input, cv::Size(5, 5), 0, 0);

  if(img_input_prev.empty())
  {
    img_input.copyTo(img_input_prev);
    return;
  }

  cv::absdiff(img_input_prev, img_input, img_foreground);

  if(img_foreground.channels() == 3)
    cv::cvtColor(img_foreground, img_foreground, CV_BGR2GRAY);

  if (enableThreshold)
      //cv::threshold(img_foreground, img_foreground, threshold, 255, cv::THRESH_BINARY);
      cv::adaptiveThreshold(img_foreground, img_foreground, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 21, 7);

  if(showOutput)
    cv::imshow("Frame Difference", img_foreground);

  img_foreground.copyTo(img_output);

  img_input.copyTo(img_input_prev);

  firstTime = false;
}

void FrameDifferenceBGS::saveConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/FrameDifferenceBGS.xml", 0, CV_STORAGE_WRITE);

  cvWriteInt(fs, "enableThreshold", enableThreshold);
  cvWriteInt(fs, "threshold", threshold);
  cvWriteInt(fs, "showOutput", showOutput);

  cvReleaseFileStorage(&fs);
}

void FrameDifferenceBGS::loadConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/FrameDifferenceBGS.xml", 0, CV_STORAGE_READ);

  enableThreshold = cvReadIntByName(fs, 0, "enableThreshold", true);
  threshold = cvReadIntByName(fs, 0, "threshold", 15);
  showOutput = cvReadIntByName(fs, 0, "showOutput", true);

  cvReleaseFileStorage(&fs);
}
