//
// Created by Bryce Cater on 10/5/15.
//


#ifndef RCORE_RCAMERA_H
#define RCORE_RCAMERA_H

//#define USE_OPEN_CV

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "linux/videodev2.h"

#include <string>
#include <queue>
#include "rWifi.h"

#define LOCAL_STREAM

#ifdef USE_OPEN_CV
    #include <opencv2/opencv.hpp>
#endif
namespace RVR
{

//    typedef void (FrameCallback)(Frame* frame, Camera* cam);
    typedef uint32_t PixelFormat;

    class Frame
    {

    };

    class Camera
    {
    private:
        int frameWidth;
        int frameHeight;

        int fps;

        struct v4l2_format frameFormat;

        bool streaming = false;
        bool initialized = false;


        int cameraFd;
        struct v4l2_requestbuffers requestBuffers;
        struct v4l2_buffer bufferInfo;
        char* bufferStart;

//        FrameCallback* frameCallback;

    public:
        void setupStream(PixelFormat pixelFormat, int width, int height, int fps);
        void setStreamMode(PixelFormat pixelFormat, int width, int height, int fps);
//        void setFrameCallback(FrameCallback* callback);
        void setAutoExposure(bool aeOn);

        void startStream();
        void stopStream();
        v4l2_format getFrameFormat();

        NetworkChunk* getFrameNC_BAD_TEMP_FUNC();

        NetworkManager* networkManager;

#ifdef LOCAL_STREAM
        std::queue<NetworkChunk*> frameQueue = std::queue<NetworkChunk*>();
#endif

        Camera(NetworkManager* networkManager);
        ~Camera();
    };

    void sendFrame(Frame *frame, void *camera);

#ifdef LOCAL_STREAM
    void queueFrame(Frame *frame, void *camera);
#endif

#ifdef USE_OPEN_CV
    cv::Mat frameToMat(Frame* frame, int matrixType);
    void saveFrame(Frame* frame, void* camera);
    void showFrame(Frame* frame, void* camera);
#endif

    void dummyCallback(Frame *frame, void *camera);

}

#endif //RCORE_RCAMERA_H