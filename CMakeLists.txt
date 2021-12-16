cmake_minimum_required(VERSION 3.20)

#设置项目名称
SET(PROJECT_NAME HKVS_DisplayDemo)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR})

# Import Self-defined lib
include_directories(include/)

# Import X11
find_package(X11 REQUIRED)
if(NOT X11_FOUND)
    message("ERROR: x11 not found")
endif(NOT X11_FOUND)
include_directories(${X11_INCLUDE_DIR})

# Import Opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
# directory of opencv library
link_directories(${OpenCV_LIBRARY_DIRS})

# print opencv status
MESSAGE(STATUS "Project: ${PROJECT_NAME}")
MESSAGE(STATUS "OpenCV library status:")
MESSAGE(STATUS "    version: ${OpenCV_VERSION}")
MESSAGE(STATUS "    libraries: ${OpenCV_LIBS}")
MESSAGE(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Import Threads
find_package(Threads)

# Import HKVS SDK Head
link_directories(${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/)
link_directories(${PROJECT_SOURCE_DIR}/lib/)
link_directories(${PROJECT_SOURCE_DIR}/src/)

add_library(camera include/LineCamera.h src/LineCamera.cpp src/FrameProcessor.cpp include/FrameProcessor.h)

#ADD_EXECUTABLE(${PROJECT_NAME} main.cpp ${DIR_SRCS})
ADD_EXECUTABLE(${PROJECT_NAME} src/main.cpp src/LineCamera.cpp)
# opencv libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# multi-thread libraries
#target_link_libraries(${PROJECT_NAME} /usr/lib/x86_64-linux-gnu/libpthread.so)

# HKVS libraries


IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux")
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libMvCameraControl.so)

    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libhpr.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCCore.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libhcnetsdk.so)

    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libanalyzedata.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCAlarm.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCCoreDevCfg.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCDisplay.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCGeneralCfgMgr.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCIndustry.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCPlayBack.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCPreview.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libHCVoiceTalk.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libiconv2.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libStreamTransClient.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libSystemTransform.so)

    #以下3个包有引用关系，必须按顺序添加。
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libPlayCtrl.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libSuperRender.so)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/HCNetSDKCom/libAudioRender.so)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
ELSEIF (APPLE)
    MESSAGE(STATUS "current platform: MacOS")
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/MacPackage/libMvCameraControl.dylib)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/MacPackage/libMediaProcess.dylib)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/MacPackage/libMVGigEVisionSDK.dylib)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/MacPackage/libusb-1.0.0.dylib)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(${PROJECT_NAME} camera)