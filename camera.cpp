//
// Created by Bryce Cater on 10/5/15.
//

#include "rCamera.h"
#include "../rCore/easylogging++.h"

#ifdef USE_OPEN_CV
    #include <opencv2/opencv.hpp>
#endif


namespace RVR
{

// ==============================================================
// Camera Class Member functions
// ==============================================================


    Camera::Camera(NetworkManager *networkManager)
    {
        this->networkManager = networkManager;
    }

    void Camera::setupStream(PixelFormat format, int width, int height, int fps)
    {
        VLOG(1) << "Setting up camera.";

        VLOG(2) << "Opening file descriptor for camera...";
        if ((this->cameraFd = open("/dev/video0", O_RDWR)) < 0)
        {
            LOG(ERROR) << "Failed to obtain file descriptor for the camera";
            throw std::runtime_error("failed to open camera");
        }
        VLOG(2) << "[ DONE ] fd ope\n";

        this->setStreamMode(format, width, height, fps);

        this->streaming = false;
//        this->setFrameCallback(dummyCallback);

        this->initialized = true;
    }

    void Camera::setStreamMode(PixelFormat pixelFormat, int width, int height, int fps)
    {
        VLOG(2) << "Negotiating camera stream mode...";
        struct v4l2_format frameFormat;
        frameFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        frameFormat.fmt.pix.pixelformat = pixelFormat;
        frameFormat.fmt.pix.width = width;
        frameFormat.fmt.pix.height = height;

        if (ioctl(this->cameraFd, VIDIOC_S_FMT, &frameFormat) < 0)
        {
            LOG(ERROR) << "Failed to set the stream mode of the camera";
            throw std::runtime_error("failed to set stream mode");
        }
        VLOG(2) << "[ DONE ] stream negotiated\n";

        this->frameFormat = frameFormat;
        this->frameWidth = width;
        this->frameHeight = height;
        this->fps = fps; // TODO either implement fps handling or remove the parameter

        VLOG(2) << "Negotiating buffer memory with camera...";
        struct v4l2_requestbuffers bufrequest;
        bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufrequest.memory = V4L2_MEMORY_MMAP;
        bufrequest.count = 1;

        if (ioctl(this->cameraFd, VIDIOC_REQBUFS, &bufrequest) < 0)
        {
            LOG(ERROR) << "Failed to request the buffer for the camera";
            throw std::runtime_error("failed to request camera buffer");
        }

        this->bufferInfo;
        memset(&this->bufferInfo, 0, sizeof(this->bufferInfo));

        this->bufferInfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        this->bufferInfo.memory = V4L2_MEMORY_MMAP;
        this->bufferInfo.index = 0;

        if (ioctl(this->cameraFd, VIDIOC_QUERYBUF, &this->bufferInfo) < 0)
        {
            LOG(ERROR) << "Failed to query the buffer for the camera";
            throw std::runtime_error("failed to query camera buffer");
        }

        this->bufferStart = (char *) mmap(
                NULL,
                this->bufferInfo.length,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                this->cameraFd,
                this->bufferInfo.m.offset
        );

        if (this->bufferStart == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }

        memset(this->bufferStart, 0, this->bufferInfo.length);

        VLOG(2) << "[ DONE ] buffers negotiated\n";
    }

//    void Camera::setFrameCallback(FrameCallback *callback)
//    {
//        this->frameCallback = callback;
//    }

    void Camera::startStream()
    {
        VLOG(2) << "Starting camera stream...";
        int type = this->bufferInfo.type;
        if (ioctl(this->cameraFd, VIDIOC_STREAMON, &type) < 0)
        {
            LOG(ERROR) << "Failed to start the camera stream";
            throw std::runtime_error("failed to start stream");
        }
        VLOG(2) << "[ DONE ] stream started\n";

        this->streaming = true;
    }

    void Camera::stopStream()
    {
        VLOG(2) << "Stopping camera stream...";
        int type = this->bufferInfo.type;
        if(ioctl(this->cameraFd, VIDIOC_STREAMOFF, &type) < 0){
            LOG(ERROR) << "Failed to stop the camera stream";
            throw std::runtime_error("failed to stop stream");
        }
        VLOG(2) << "[ DONE ] stream stopped\n";

        this->streaming = false;
    }

    v4l2_format Camera::getFrameFormat()
    {
        return this->frameFormat;
    }

    void Camera::setAutoExposure(bool aeOn)
    {
        VLOG(2) << "Settign Auto-Exposure ON to: " << aeOn;
        LOG(WARNING) << "NOT IMPLEMENTED... DOING NOTHING";
        // TODO Implement this
        VLOG(2) << "[ DONE ]\n";
    }


    Camera::~Camera()
    {
        VLOG(2) << "Destroying Camera object";
        this->stopStream();

        close(this->cameraFd);
        VLOG(2) << "[ DONE ] camera obj destroyed";
    }
//
//    void sendFrame(Frame *frame, void *camera)
//    {
//        NetworkChunk nc = NetworkChunk(DataType::CAMERA, frame->data_bytes, (char*)frame->data);
//
//        Camera* cam = (Camera*)camera;
//        cam->networkManager->sendData("CAMERA",&nc);
//    }
//
//#ifdef LOCAL_STREAM
//
//    void queueFrame(Frame *frame, void *camera)
//    {
//        NetworkChunk* nc = new NetworkChunk(DataType::CAMERA, frame->data_bytes, (char*)frame->data);
//
//        Camera* cam = (Camera*)camera;
//        cam->frameQueue.push(nc);
//    }
//
//#endif
//
//#ifdef USE_OPEN_CV
//    cv::Mat frameToMat(Frame *frame, int matrixType)
//    {
//        uvc_frame * tempFrame;
//        if (frame->frame_format != UVC_FRAME_FORMAT_BGR)
//        {
//            uvc_error errorResult;
//            tempFrame = uvc_allocate_frame(frame->width * frame->height * 3);
//            errorResult = uvc_any2bgr(frame, tempFrame);
//            processUvcErrorResult(errorResult, "Failed to convert frame to rgb");
//        }
//        else
//        {
//            tempFrame = frame;
//        }
//
//        cv::Mat img = cv::Mat::Mat(tempFrame->height, tempFrame->width, CV_8UC3, tempFrame->data, tempFrame->step);
//        uvc_free_frame(tempFrame);
//        return img;
//    }
//
//    void saveFrame(Frame *frame, void *camera)
//    {
//        Camera* cam = (Camera*)camera;
//
//
//        char filename[100];
//        snprintf(filename, sizeof(filename), "%i.jpg", frame->sequence);
//        VLOG_EVERY_N(30,2) << "Saving image file" << filename << "...";
//        cv::Mat img = frameToMat(frame, CV_8UC3);
//        cv::imwrite( filename, img );
//        img.release();
//        VLOG_EVERY_N(30,2) << "[ DONE ]";
//    }
//
//    void showFrame(Frame *frame, void *camera)
//    {
//        cv::waitKey(1);
//        cv::Mat img = frameToMat(frame, CV_8UC3);
////        cv::Mat image;
////        image = cv::imread("1.jpg", cv::IMREAD_COLOR);
////        cv::namedWindow( "window", cv::WINDOW_AUTOSIZE );
//        cv::imshow( "frame", img );
//
//        img.release();
//
//    }
//#endif
    void dummyCallback(Frame *frame, Camera *camera) {}

    NetworkChunk *Camera::getFrameNC_BAD_TEMP_FUNC()
    {
        VLOG(2) << "GETTING FRAME!";
        if (this->streaming)
        {
            if(ioctl(this->cameraFd, VIDIOC_QBUF, &(this->bufferInfo)) < 0){
                perror("VIDIOC_QBUF");
                exit(1);
            }

            // The buffer's waiting in the outgoing queue.
            if(ioctl(this->cameraFd, VIDIOC_DQBUF, &(this->bufferInfo)) < 0){
                perror("VIDIOC_QBUF");
                exit(1);
            }
            int jpgfile;
            if((jpgfile = open("/tmp/myimage.jpeg", O_WRONLY | O_CREAT, 0660)) < 0){
                perror("open");
                exit(1);
            }

            NetworkChunk* nc = new NetworkChunk(DataType::CAMERA, this->bufferInfo.length, this->bufferStart);
            VLOG(2) << "[ DONE ] GOT FRAME";
            return nc;
        }
        else
        {
            NetworkChunk* nc = new NetworkChunk(DataType::NONE, 0, NULL);
            VLOG(2) << "[ DONE ] NO FRAME TO GET";
            return nc;
        }
    }

}