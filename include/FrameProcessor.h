//
// Created by vangy on 2021/10/24.
//

#ifndef HKVS_DISPLAYDEMO_FRAMEPROCESSOR_H
#define HKVS_DISPLAYDEMO_FRAMEPROCESSOR_H

#include <vector>
#include <fstream>

#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"


class FrameProcessor{
private:

public:
    FrameProcessor()= default;
    ~FrameProcessor()= default;
    std::vector<cv::Mat> readTestImage();
    int forceCombine(std::vector<cv::Mat> frameBucket);
    int stitcherCombine(std::vector<cv::Mat> frameBucket);
};

#endif //HKVS_DISPLAYDEMO_FRAMEPROCESSOR_H
