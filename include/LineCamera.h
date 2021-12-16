//
// Created by vangy on 2021/10/22.
//

#include <iostream>
#include <cstdio>
#include <iostream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <pthread.h>
#include "MvCameraControl.h"

#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

class CLCamera{
private:
    void* m_hDevHandle;
    static unsigned int nBufSize;
    pthread_t nThreadID;
public:
    static bool isExit;
    static bool isCollecting;
    // 用于保存图像的缓存
    unsigned char* m_pBufForSaveImage{};
    unsigned int m_nBufSizeForSaveImage{};
    // 用于从驱动获取图像的缓存
    unsigned char* m_pBufForDriver{};
    unsigned int m_nBufSizeForDriver{};
    // 设备信息列表结构体变量，用来存储设备列表
    MV_CC_DEVICE_INFO_LIST m_stDevList;
    //设备对象
    MV_CC_DEVICE_INFO *m_Device = nullptr;

    CLCamera();
    ~CLCamera();

    //声明相关变量及函数等
    //枚举相机设备列表
    static int EnumDevices(MV_CC_DEVICE_INFO_LIST* pstDevList);

    static bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);

    static void* WorkThread(void* pUser);

    static int ReadImageFromThread(void* pUser, cv::Mat &image);

    static void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);

    // 连接相机
    int connectCamera(int deviceIndex = -1);

    // Detection network optimal package size(It only works for the GigE camera)
    int setPacketSize(int nIndex);

    //设置相机触发模式
    int setTriggerMode(unsigned int TriggerModeNum);

    // 设置触发源
    int setTriggerSource();

    // Soft Trigger
    int softTrigger();

    //开启相机采集
    int startCamera();

    // read Buffer Size
    int readBufSize();

    //读取buffer
    int ReadBuffer(cv::Mat &image);
    int RegisterImageCallBack(void(* callback)(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser));

    // Create Thread
    int CreateWorkThread();

    //设置曝光时间
    int setExposureTime(float ExposureTimeNum);

    // Setting Camera Height
    int setCameraHeight(unsigned int height);

    // Setting Camera Width
    int setCameraWidth(unsigned int width);

    //关闭相机
    int closeCamera();
};
