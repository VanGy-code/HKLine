#include <iostream>
#include <cstdio>
//#include <vector>
#include "unistd.h"
#include "include/LineCamera.h"
#include "include/FrameProcessor.h"

void PressEnterToExit(void)
{
    int c;
    while ( (c = getchar()) != '\n' && c != EOF );
    fprintf( stderr, "\nPress enter to exit.\n");
    while( getchar() != '\n');
    sleep(1);
    CLCamera::isCollecting = false;
}

//auto *camera = new CLCamera();
auto* processor = new FrameProcessor();
int main(){
//    cv::Mat image;
//
//    int nRet = camera->connectCamera();
//    if(nRet != MV_OK) return 0;
//    nRet = camera->readBufSize();
//    if(nRet != MV_OK) return 0;
//    nRet = camera->setTriggerMode(1);
//    if(nRet != MV_OK) return 0;
//    nRet = camera->setTriggerSource();
//    if(nRet != MV_OK) return 0;
//    nRet = camera->setCameraWidth(4096);
//    if(nRet != MV_OK) return 0;
//    nRet = camera->setCameraHeight(120);
//    if(nRet != MV_OK) return 0;
//    nRet = camera->setExposureTime(1000);
//    if(nRet != MV_OK) return 0;
//    nRet = camera->startCamera();
//    if(nRet != MV_OK) return 0;
//
//
//    int count = 0;
//    while (count++ < 15){
//        nRet = camera->softTrigger();
//        if(nRet != MV_OK) return 0;
//        nRet = camera->ReadBuffer(image);
//        if(nRet != MV_OK) return 0;
//        CLCamera::isCollecting = true;
//        std::string filename = "output/image_" + std::to_string(count) + ".bmp";
//        cv::imwrite(filename, image);
//        std::cout<< "Save " + filename << std::endl;
//    }
    std::vector<cv::Mat> images = processor->readTestImage("output/*.bmp");
    if (!images.empty()){
        std::cout << "Get Images Succeed!" << std::endl;
        std::cout << "Image numbers:" <<  images.size() <<std::endl;
    }
//    processor->forceCombine(images);
    processor->combine(images, true, "./", ".bmp");
//    processor->surfCombine(images);
//    processor->briskCombine(images);


//
//    nRet = camera->closeCamera();
//    if(nRet != MV_OK) return 0;
    return 0;
}