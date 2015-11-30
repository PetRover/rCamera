//
// Created by Bryce Cater on 10/5/15.
//


#ifndef RCORE_RCAMERA_H
#define RCORE_RCAMERA_H

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
#include "../rCore/easylogging++.h"

extern const int CAMERA_RES_HEIGHT;
extern const int CAMERA_RES_WIDTH;

namespace RVR
{
    typedef uint32_t PixelFormat;


    class Camera
    {
    private:
        int frameNumber;
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

    public:
        void setupStream(PixelFormat pixelFormat, int width, int height, int fps);
        void setStreamMode(PixelFormat pixelFormat, int width, int height, int fps);
        void setAutoExposure(bool aeOn);

        void startStream();
        void stopStream();
        v4l2_format getFrameFormat();

        NetworkChunk* getFrameNC_BAD_TEMP_FUNC();

        NetworkManager* networkManager;

        Camera(NetworkManager* networkManager);
        ~Camera();
    };
}

#endif //RCORE_RCAMERA_H