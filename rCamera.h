//
// Created by Bryce Cater on 10/5/15.
//


#ifndef RCORE_RCAMERA_H
#define RCORE_RCAMERA_H

//#define USE_OPEN_CV
#include <libuvc/libuvc.h>
#include <string>
#ifdef USE_OPEN_CV
    #include <opencv2/opencv.hpp>
#endif
namespace RVR
{
    void processUvcErrorResult(uvc_error errorResult, std::string message);

    class Camera
    {
    private:
        int frameWidth;
        int frameHeight;

        int fps;
        uvc_frame_format frameFormat;

        bool streaming = false;
        bool initialized = false;


        uvc_context *context;
        uvc_device *device;
        uvc_device_handle *deviceHandle;
        uvc_stream_ctrl_t streamController;
        uvc_error errorResult;

        uvc_frame_callback_t* frameCallback;

    public:
        void setupStream(uvc_frame_format format, int width, int height, int fps);
        void setStreamMode(uvc_frame_format format, int width, int height, int fps);
        void setFrameCallback(uvc_frame_callback_t callback);
        void setAutoExposure(bool aeOn);

        void startStream();
        void stopStream();
        uvc_frame_format getFrameFormat();

        ~Camera();
    };

    void sendFrame(uvc_frame_t* frame, void* camera);

#ifdef USE_OPEN_CV
    cv::Mat frameToMat(uvc_frame* frame, int matrixType);
    void saveFrame(uvc_frame_t* frame, void* camera);
    void showFrame(uvc_frame_t* frame, void* camera);
#endif

    void dummyCallback(uvc_frame_t* frame, void* camera);

}

#endif //RCORE_RCAMERA_H