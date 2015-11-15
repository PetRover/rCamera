//
// Created by Bryce Cater on 10/5/15.
//

#include "rCamera.h"
#include "../rCore/easylogging++.h"
#include "rWifi.h"
#include <stdexcept>

#ifdef USE_OPEN_CV
    #include <opencv2/opencv.hpp>
#endif


namespace RVR
{
    void processUvcErrorResult(uvc_error errorResult, std::string message)
    {
        if (errorResult < 0)
        {
            LOG(ERROR) << "[ FAILURE ] " << uvc_strerror(errorResult);
            throw std::runtime_error(message);
        }
    }

// ==============================================================
// Camera Class Member functions
// ==============================================================


    Camera::Camera(NetworkManager *networkManager)
    {
        this->networkManager = networkManager;
    }

    void Camera::setupStream(uvc_frame_format format, int width, int height, int fps)
    {
        VLOG(1) << "Setting up camera.";

        VLOG(2) << "Creating context object for UVC device...";
        this->errorResult = uvc_init(&this->context, NULL);
        processUvcErrorResult(this->errorResult, "Failed to create UVC context");
        VLOG(2) << "[ DONE ]\n";

        VLOG(2) << "Finding UVC camera device...";
        this->errorResult = uvc_find_device(this->context, &this->device, 0, 0, NULL);
        processUvcErrorResult(this->errorResult, "Failed to find UVC camera device");
        VLOG(2) << "[ DONE ]\n";

        VLOG(2) << "Opening UVC camera device...";
        this->errorResult = uvc_open(this->device, &this->deviceHandle);
        processUvcErrorResult(this->errorResult, "Failed to open UVC camera");
        VLOG(2) << "[ DONE ]\n";

        if (VLOG_IS_ON(3))
        {
            uvc_print_diag(this->deviceHandle, stdout);
        }

        this->setStreamMode(format, width, height, fps);

        this->streaming = false;
        this->setFrameCallback(dummyCallback);

        this->initialized = true;
    }

    void Camera::setStreamMode(uvc_frame_format format, int width, int height, int fps)
    {
        VLOG(2) << "Negotiating UVC stream mode...";
        this->errorResult = uvc_get_stream_ctrl_format_size(this->deviceHandle, &this->streamController,
                                                            format, width, height, fps);
        processUvcErrorResult(this->errorResult, "Failed to negotiate stream mode");
        if (VLOG_IS_ON(3))
        {
            uvc_print_stream_ctrl(&this->streamController, stdout);
        }
        VLOG(2) << "[ DONE ]\n";

        this->frameFormat = format;
        this->frameWidth = width;
        this->frameHeight = height;
        this->fps = fps;

    }

    void Camera::setFrameCallback(uvc_frame_callback_t callback)
    {
        this->frameCallback = callback;
    }

    void Camera::startStream()
    {
        VLOG(2) << "Starting UVC stream...";
        this->errorResult = uvc_start_streaming(this->deviceHandle, &this->streamController, this->frameCallback, this, 0);
        processUvcErrorResult(this->errorResult, "Failed start UVC camera stream");
        VLOG(2) << "[ DONE ]\n";

        this->streaming = true;
    }

    void Camera::stopStream()
    {
        VLOG(2) << "Stopping UVC stream...";
        try
        {
            uvc_stop_streaming(this->deviceHandle);
        }
        catch (...)
        {
            LOG(ERROR) << "Failed to stop stream";
            throw;
        }
        VLOG(2) << "[ DONE ]\n";

        this->streaming = false;
    }

    uvc_frame_format Camera::getFrameFormat()
    {
        return this->frameFormat;
    }

    void Camera::setAutoExposure(bool aeOn)
    {
        VLOG(2) << "Settign Auto-Exposure ON to: " << aeOn;
        uvc_set_ae_mode(this->deviceHandle, 1);
        VLOG(2) << "[ DONE ]\n";
    }


    Camera::~Camera()
    {
        this->stopStream();
        uvc_close(this->deviceHandle);
        uvc_unref_device(this->device);
        uvc_exit(this->context);
    }

    void sendFrame(uvc_frame *frame, void *camera)
    {
        NetworkChunk nc = NetworkChunk(DataType::CAMERA, frame->data_bytes, (char*)frame->data);

        Camera* cam = (Camera*)camera;
        cam->networkManager->sendData("CAMERA",&nc);
    }

#ifdef USE_OPEN_CV
    cv::Mat frameToMat(uvc_frame *frame, int matrixType)
    {
        uvc_frame * tempFrame;
        if (frame->frame_format != UVC_FRAME_FORMAT_BGR)
        {
            uvc_error errorResult;
            tempFrame = uvc_allocate_frame(frame->width * frame->height * 3);
            errorResult = uvc_any2bgr(frame, tempFrame);
            processUvcErrorResult(errorResult, "Failed to convert frame to rgb");
        }
        else
        {
            tempFrame = frame;
        }

        cv::Mat img = cv::Mat::Mat(tempFrame->height, tempFrame->width, CV_8UC3, tempFrame->data, tempFrame->step);
        uvc_free_frame(tempFrame);
        return img;
    }

    void saveFrame(uvc_frame *frame, void *camera)
    {
        Camera* cam = (Camera*)camera;


        char filename[100];
        snprintf(filename, sizeof(filename), "%i.jpg", frame->sequence);
        VLOG_EVERY_N(30,2) << "Saving image file" << filename << "...";
        cv::Mat img = frameToMat(frame, CV_8UC3);
        cv::imwrite( filename, img );
        img.release();
        VLOG_EVERY_N(30,2) << "[ DONE ]";
    }

    void showFrame(uvc_frame *frame, void *camera)
    {
        cv::waitKey(1);
        cv::Mat img = frameToMat(frame, CV_8UC3);
//        cv::Mat image;
//        image = cv::imread("1.jpg", cv::IMREAD_COLOR);
//        cv::namedWindow( "window", cv::WINDOW_AUTOSIZE );
        cv::imshow( "frame", img );

        img.release();

    }
#endif
    void dummyCallback(uvc_frame* frame, void* camera) {}
}