
#include <LineCamera.h>


unsigned int CLCamera::nBufSize = 0;
bool CLCamera::isExit = true;
bool CLCamera::isCollecting = false;

CLCamera::CLCamera() {
    this->m_hDevHandle = nullptr;
}


int CLCamera::EnumDevices(MV_CC_DEVICE_INFO_LIST *pstDevList) {
    int nRet= MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, pstDevList);
    if (nRet != MV_OK) {
        // return false
        printf("Enum Devices fail! nRet [0x%x]\n", nRet);
        return nRet;
    }
    return MV_OK;
}

bool CLCamera::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo){
    if (nullptr == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE){
        // GIGE Camera
        unsigned int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        unsigned int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        unsigned int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        unsigned int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    } else if(pstMVDevInfo->nTLayerType == MV_USB_DEVICE){
        // USB Camera
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }else
    {
        printf("Not support.\n");
    }

    return true;
}

int CLCamera::connectCamera(int deviceIndex) {
    int nRet= EnumDevices(&m_stDevList);
    if( nRet != MV_OK) return nRet;

    if (m_stDevList.nDeviceNum > 0){
        deviceIndex = 0;
        for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
        {
            printf("[device %d]:\n", i);
            MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
            PrintDeviceInfo(pDeviceInfo);
////        if (nullptr == pDeviceInfo)
////        {
////            continue;
////        }
////        //qDebug() << (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName;//自定义相机名称
////        //qDebug() << (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber;//相机序列号
////        if(id == (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName||id == (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber)
////        {
////            m_Device= m_stDevList.pDeviceInfo[i];
////            break;
////        }else
////        {
////            continue;
////        }
        }
    }
    else{
        printf("Find No Devices!\n");
        return -1;
    }


    // Create handle
    nRet = MV_CC_CreateHandle(&m_hDevHandle, m_stDevList.pDeviceInfo[deviceIndex]);
    if (nRet != MV_OK) {
        // Error cause when create Handle
        std::cout << "Create Handle fail! nRet [0x" << nRet << "]" << std::endl;
        return nRet;
    }
    // Open device
    nRet = MV_CC_OpenDevice(m_hDevHandle);
    if (MV_OK != nRet) {
        std::cout << "Open Device fail! nRet [" << nRet << "]" << std::endl;
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle = nullptr;
        return nRet;
    }
    isExit = false;
    nRet = setPacketSize(deviceIndex);
    return MV_OK;
}

int CLCamera::setPacketSize(int nIndex) {
    if (m_stDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {
        int nPacketSize = MV_CC_GetOptimalPacketSize(m_hDevHandle);
        if (nPacketSize > 0)
        {
            int nRet = MV_CC_SetIntValue(m_hDevHandle,"GevSCPSPacketSize",nPacketSize);
            if(nRet != MV_OK)
            {
                printf("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet);
                return nRet;
            }
        }
        else
        {
            printf("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize);
            return -1;
        }
    }
    return MV_OK;

}

int CLCamera::readBufSize() {
    MVCC_INTVALUE stIntValue; //获取一帧数据的大小
    memset(&stIntValue, 0, sizeof(MVCC_INTVALUE));

    int nRet = MV_CC_GetIntValue(m_hDevHandle, "PayloadSize", &stIntValue);
    if (nRet != MV_OK) {
        std::cout << "Get PayloadSize fail! nRet [" << nRet << "]" << std::endl;
        return nRet;
    }

    CLCamera::nBufSize = stIntValue.nCurValue;
    return MV_OK;
}

int CLCamera::setTriggerMode(unsigned int TriggerModeNum) {
    int nRet = MV_CC_SetTriggerMode(m_hDevHandle,TriggerModeNum);
    if( nRet != MV_OK){
        std::cout << "Set TriggerMode Failed !" << std::endl;
    }
    else{
        if (TriggerModeNum == 0){
            std::cout << "Set TriggerMode off !" << std::endl;
        }
        else if(TriggerModeNum == 1){
            std::cout << "Set TriggerMode on !" << std::endl;
        }
    }
    return nRet;
}

int CLCamera::setTriggerSource() {
    // ch:设置触发源 | en:Set trigger source
    int nRet = MV_CC_SetEnumValue(m_hDevHandle, "TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
    if (MV_OK != nRet)
    {
        printf("MV_CC_SetTriggerSource fail! nRet [%x]\n", nRet);
        return nRet;
    }
    else{
        printf("MV_CC_SetTriggerSource Success!\n");
    }
    return nRet;
}


int CLCamera::startCamera() {
    int nRet = MV_CC_StartGrabbing(m_hDevHandle);
    if (nRet != MV_OK) {
        std::cout << "Start Grabbing fail!" << std::endl;
        return nRet;
    }
    isCollecting = true;
    return nRet;
}

int CLCamera::ReadBuffer(cv::Mat &image) {
    int nRet;
    m_pBufForDriver = (unsigned char *)malloc(CLCamera::nBufSize);

    MV_FRAME_OUT_INFO_EX stImageInfo;
    memset(&stImageInfo,0,sizeof(MV_FRAME_OUT_INFO));

//    qDebug() << MV_CC_StartGrabbing(m_hDevHandle);
//    startCamera();

    int timeout= MV_CC_GetOneFrameTimeout(m_hDevHandle, m_pBufForDriver, CLCamera::nBufSize, &stImageInfo, 1000);
    if(timeout!= MV_OK)
    {
        std::cout << "[ERROR] GetOneFrameTimeout [0x" << nRet << "]" << std::endl;
//        qDebug() << "GetOneFrameTimeout失败";
        return timeout;
    }


    // Core Code for transfer image data from SDK to opencv
    m_nBufSizeForSaveImage = stImageInfo.nWidth * stImageInfo.nHeight * 3 + 2048;
    m_pBufForSaveImage = (unsigned char*)malloc(m_nBufSizeForSaveImage); //向系统申请M_nBufSizeForSaveImage内存空间

    bool isMono;//判断是否为黑白图像
    switch (stImageInfo.enPixelType) //像素格式
    {
        case PixelType_Gvsp_Mono8:
        case PixelType_Gvsp_Mono10:
        case PixelType_Gvsp_Mono10_Packed:
        case PixelType_Gvsp_Mono12:
        case PixelType_Gvsp_Mono12_Packed:
            isMono=true;
            break;
        default:
            isMono=false;
            break;
    }

    if(isMono)
    {
        image=cv::Mat(stImageInfo.nHeight,stImageInfo.nWidth,CV_8UC1,m_pBufForDriver);
    }
    else
    {
        //转换图像格式为BGR8
        MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
        memset(&stConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
        stConvertParam.nWidth = stImageInfo.nWidth;                 //ch:图像宽 | en:image width
        stConvertParam.nHeight = stImageInfo.nHeight;               //ch:图像高 | en:image height
        stConvertParam.pSrcData = m_pBufForDriver;                  //ch:输入数据缓存 | en:input data buffer
        stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;         //ch:输入数据大小 | en:input data size
        stConvertParam.enSrcPixelType = stImageInfo.enPixelType;    //ch:输入像素格式 | en:input pixel format
        //stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed; //ch:输出像素格式 | en:output pixel format  适用于OPENCV的图像格式
        stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed; //ch:输出像素格式 | en:output pixel format
        stConvertParam.pDstBuffer = m_pBufForSaveImage;                    //ch:输出数据缓存 | en:output data buffer
        stConvertParam.nDstBufferSize = m_nBufSizeForSaveImage;            //ch:输出缓存大小 | en:output buffer size
        MV_CC_ConvertPixelType(m_hDevHandle, &stConvertParam);
        image=cv::Mat(stImageInfo.nHeight,stImageInfo.nWidth,CV_8UC3,m_pBufForSaveImage);
    }
    return MV_OK;
}

int CLCamera::setExposureTime(float ExposureTimeNum) {
    int nRet = MV_CC_SetFloatValue(m_hDevHandle, "ExposureTime", ExposureTimeNum);
    if (MV_OK == nRet)
    {
        printf("set exposure time OK!\n");
    }
    else
    {
        printf("set exposure time failed! nRet [%x]\n\n", nRet);
        return nRet;
    }
    return nRet;
}

int CLCamera::closeCamera() {
    if (nullptr == m_hDevHandle)
    {
        std::cout << "No handle exist!" << std::endl;
        return MV_OK;
    }
    int nRet = MV_CC_StopGrabbing(m_hDevHandle);
    if (nRet != MV_OK) {
        std::cout << "Stop Grabbing fail!" << std::endl;
        return nRet;
    }
    nRet = MV_CC_CloseDevice(m_hDevHandle);
    if (nRet != MV_OK) {
        std::cout << "Close Device fail!" << std::endl;
        return nRet;
    }
    nRet = MV_CC_DestroyHandle(m_hDevHandle);
    if (nRet != MV_OK) {
        std::cout << "Destroy Handle fail!" << std::endl;
        return nRet;
    }
    m_hDevHandle = nullptr;
    return nRet;
}
int CLCamera::softTrigger() {
    int nRet= MV_CC_SetCommandValue(m_hDevHandle, "TriggerSoftware");
    if(MV_OK != nRet)
    {
        printf("failed in TriggerSoftware[%x]\n", nRet);
    }
    else
    {
        CLCamera::isCollecting = false;
    }
    return MV_OK;
}


int CLCamera::setCameraHeight(unsigned int height) {
    int nRet = MV_CC_SetIntValue(m_hDevHandle, "Height", height);
    if (MV_OK == nRet)
    {
        printf("set height OK!\n");
    }
    else
    {
        printf("set height failed! nRet [%x]\n", nRet);
        return nRet;
    }
    return nRet;
}

int CLCamera::setCameraWidth(unsigned int width) {
    int nRet = MV_CC_SetIntValue(m_hDevHandle, "Width", width);
    if (MV_OK == nRet)
    {
        printf("set width OK!\n");
    }
    else
    {
        printf("set width failed! nRet [%x]\n", nRet);
        return nRet;
    }
    return nRet;
}

CLCamera::~CLCamera() {
    if (this->m_hDevHandle)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        this->m_hDevHandle = nullptr;
    }
}

void CLCamera::ImageCallBackEx(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser) {
    auto* camera = (CLCamera*)pUser;
    if (pFrameInfo)
    {
        printf("GetOneFrame, Width[%d], Height[%d], nFrameNum[%d]\n",
               pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
        CLCamera::isCollecting = true;
    }
}

int CLCamera::RegisterImageCallBack(void(* callback)(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)) {
    int nRet = MV_CC_RegisterImageCallBackEx(m_hDevHandle, callback, this);
    if (MV_OK != nRet)
    {
        printf("MV_CC_RegisterImageCallBackEx fail! nRet [%x]\n", nRet);
        return nRet;
    }
    else{
        printf("MV_CC_RegisterImageCallBackEx Success! \n");
    }
    return nRet;
}

int CLCamera::CreateWorkThread() {
    int nRet = pthread_create(&nThreadID, nullptr ,WorkThread , this);
    if (nRet != 0)
    {
        printf("thread create failed.ret = %d\n",nRet);
        return nRet;
    }
    return nRet;
}


int CLCamera::ReadImageFromThread(void *pUser, cv::Mat &image) {
    auto* camera = (CLCamera*)pUser;
    auto* m_pBufForDriver = (unsigned char *)malloc(CLCamera::nBufSize);

    MV_FRAME_OUT_INFO_EX stImageInfo;
    memset(&stImageInfo,0,sizeof(MV_FRAME_OUT_INFO));

//    qDebug() << MV_CC_StartGrabbing(m_hDevHandle);
//    startCamera();

    int nRet= MV_CC_GetOneFrameTimeout(camera->m_hDevHandle, m_pBufForDriver, CLCamera::nBufSize, &stImageInfo, 1000);
    if(nRet!= MV_OK)
    {
        std::cout << "[ERROR] GetOneFrameTimeout [0x" << nRet << "]" << std::endl;
//        qDebug() << "GetOneFrameTimeout失败";
        return nRet;
    }
    printf("GetOneFrame, Width[%d], Height[%d], nFrameNum[%d]\n",
                       stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);


    // Core Code for transfer image data from SDK to opencv
    unsigned int m_nBufSizeForSaveImage = stImageInfo.nWidth * stImageInfo.nHeight * 3 + 2048;
    auto* m_pBufForSaveImage = (unsigned char*)malloc(m_nBufSizeForSaveImage); //向系统申请M_nBufSizeForSaveImage内存空间

    bool isMono;//判断是否为黑白图像
    switch (stImageInfo.enPixelType) //像素格式
    {
        case PixelType_Gvsp_Mono8:
        case PixelType_Gvsp_Mono10:
        case PixelType_Gvsp_Mono10_Packed:
        case PixelType_Gvsp_Mono12:
        case PixelType_Gvsp_Mono12_Packed:
            isMono=true;
            break;
        default:
            isMono=false;
            break;
    }

    if(isMono)
    {
        image=cv::Mat(stImageInfo.nHeight,stImageInfo.nWidth,CV_8UC1,m_pBufForDriver);
    }
    else
    {
        //转换图像格式为BGR8
        MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
        memset(&stConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
        stConvertParam.nWidth = stImageInfo.nWidth;                 //ch:图像宽 | en:image width
        stConvertParam.nHeight = stImageInfo.nHeight;               //ch:图像高 | en:image height
        stConvertParam.pSrcData = m_pBufForDriver;                  //ch:输入数据缓存 | en:input data buffer
        stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;         //ch:输入数据大小 | en:input data size
        stConvertParam.enSrcPixelType = stImageInfo.enPixelType;    //ch:输入像素格式 | en:input pixel format
        //stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed; //ch:输出像素格式 | en:output pixel format  适用于OPENCV的图像格式
        stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed; //ch:输出像素格式 | en:output pixel format
        stConvertParam.pDstBuffer = m_pBufForSaveImage;                    //ch:输出数据缓存 | en:output data buffer
        stConvertParam.nDstBufferSize = m_nBufSizeForSaveImage;            //ch:输出缓存大小 | en:output buffer size
        MV_CC_ConvertPixelType(camera->m_hDevHandle, &stConvertParam);
        image=cv::Mat(stImageInfo.nHeight,stImageInfo.nWidth,CV_8UC3,m_pBufForSaveImage);
    }
    return MV_OK;
}

void *CLCamera::WorkThread(void *pUser) {
    auto* camera = (CLCamera*)pUser;
    while(true)
    {
        if(CLCamera::isExit)
        {
            printf("Exit.\n");
            break;
        }
        if (CLCamera::isCollecting)
        {
            camera->softTrigger();
            cv::Mat frame;
            int nRet = ReadImageFromThread(camera, frame);
            if (nRet == MV_OK)
            {
                isCollecting = true;
            }
        }
        else
        {
            printf("Stop Collecting !\n");
            CLCamera::isExit = true;
            continue;
        }
    }
//    free(pData);
//    pData = nullptr;
}


