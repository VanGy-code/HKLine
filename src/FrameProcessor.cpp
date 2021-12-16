//
// Created by vangy on 2021/10/24.
//

#include "FrameProcessor.h"

int FrameProcessor::forceCombine(std::vector<cv::Mat> frameBucket) {
    /**
     * @description: 硬拼接
     */
    int height = 120 * frameBucket.size();
    int width = 4096;
    cv::Mat result = cv::Mat(height, width,CV_8UC3, cv::Scalar::all(0));
    int current_y = 0;
    for (cv::Mat frame : frameBucket){
        cv::Mat ROI = result(cv::Rect(0, current_y, width, frame.rows));
        current_y += 120;
        frame.copyTo(ROI);
    }
//    cv::imshow("reuslt", result);
    cv::imwrite("output/result.bmp", result);
    std::cout << "拼接完成" << std::endl;
    return 0;
}

std::vector<cv::Mat> FrameProcessor::readTestImage() {
    std::vector<cv::Mat> imgs;
    std::vector<std::string> imageList;
    cv::glob("output/*.bmp", imageList, false);

    for(const std::string& img_name : imageList)
    {
        cv::Mat src = cv::imread(img_name);
        if (src.type() == CV_8UC3)//彩色
        {
            //判断是否正确读入图片
            if (src.empty()) {
                std::cout << "could not load image..." << std::endl;
                break;
            }
            imgs.push_back(src);
        }
    }
    return imgs;
}

int FrameProcessor::stitcherCombine(std::vector <Mat> frameBucket) {
    /**
     * @description: 软拼接
     */

    return 0;
}
