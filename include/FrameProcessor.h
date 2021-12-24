//
// Created by vangy on 2021/10/24.
//

#ifndef HKVS_DISPLAYDEMO_FRAMEPROCESSOR_H
#define HKVS_DISPLAYDEMO_FRAMEPROCESSOR_H

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <ctime>
#include <chrono>
#include <cstdlib>

#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/highgui.hpp"

#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include "opencv2/xfeatures2d/nonfree.hpp"

typedef struct
{
    cv::Point2f left_top;
    cv::Point2f left_bottom;
    cv::Point2f right_top;
    cv::Point2f right_bottom;
}four_corners_t;

class FrameProcessor{
private:
    four_corners_t corners;
public:
    FrameProcessor()= default;
    ~FrameProcessor()= default;
    std::vector<cv::Mat> readTestImage(const std::string pathPattern);

    void flipRight(cv::Mat &matSrc);

    void CalcCorners(const cv::Mat& H, const cv::Mat& src);
    void OptimizeSeam(cv::Mat& img1, cv::Mat& trans, cv::Mat& dst);

    int combine(std::vector<cv::Mat>& frameBucket, bool softCombine, const std::string savePath, const std::string saveType);
    cv::Mat orbCombine(cv::Mat &src, cv::Mat &transform);
    int surfCombine(std::vector<cv::Mat> frameBucket);
    static std::time_t getTimeStamp();
};

#endif //HKVS_DISPLAYDEMO_FRAMEPROCESSOR_H
