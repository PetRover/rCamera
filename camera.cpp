//
// Created by Bryce Cater on 10/5/15.
//

#include "rCamera.h"

const int CAMERA_RES_HEIGHT = 480;
const int CAMERA_RES_WIDTH = 640;

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
        VLOG(2) << "[ DONE ] fd opened\n";

        this->setStreamMode(format, width, height, fps);

        this->streaming = false;

        this->initialized = true;
        this->frameNumber = 0;
    }

    void Camera::setStreamMode(PixelFormat pixelFormat, int width, int height, int fps)
    {
        VLOG(2) << "Negotiating camera stream mode...";
        struct v4l2_format frameFormat;
        frameFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        VLOG(3) << "frameFormat.type set to: " << frameFormat.type;
        frameFormat.fmt.pix.pixelformat = pixelFormat;
        VLOG(3) << "frameFormat.pixelformat set to: " << frameFormat.fmt.pix.pixelformat;
        frameFormat.fmt.pix.width = (uint32_t)width;
        VLOG(3) << "frameFormat.width set to: " << frameFormat.fmt.pix.width;
        frameFormat.fmt.pix.height = (uint32_t)height;
        VLOG(3) << "frameFormat.height set to: " << frameFormat.fmt.pix.height;

        VLOG(3) << "Making the IOCTL call to set the frame format...";
        VLOG(3) << "IOCTL REQUEST = " << VIDIOC_S_FMT;
        if (ioctl(this->cameraFd, VIDIOC_S_FMT, &frameFormat) < 0)
        {
            LOG(ERROR) << "Failed to set the stream mode of the camera";
            throw std::runtime_error("failed to set stream mode");
        }
        VLOG(3) << "The IOCTL call did not fail :)";
        VLOG(2) << "[ DONE ] stream negotiated\n";

        this->frameFormat = frameFormat;
        this->frameWidth = width;
        this->frameHeight = height;
        this->fps = fps; // TODO either implement fps handling or remove the parameter


        VLOG(2) << "Negotiating buffer memory with camera...";
        struct v4l2_requestbuffers bufrequest;
        bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        VLOG(3) << "Buffer request type is: " << bufrequest.type;
        bufrequest.memory = V4L2_MEMORY_MMAP;
        VLOG(3) << "Buffer request memory is: " << bufrequest.memory;
        bufrequest.count = 1;
        VLOG(3) << "Buffer request count is: " << bufrequest.count;
        VLOG(3) << "Making the IOCTL call to request a buffer...";
        VLOG(3) << "IOCTL REQUEST = " << VIDIOC_REQBUFS;
        if (ioctl(this->cameraFd, VIDIOC_REQBUFS, &bufrequest) < 0)
        {
            LOG(ERROR) << "Failed to request the buffer for the camera";
            throw std::runtime_error("failed to request camera buffer");
        }
        VLOG(3) << "The IOCTL call did not fail :)";

        VLOG(3) << "Clearing data out of bufferInfo";
        memset(&this->bufferInfo, 0, sizeof(this->bufferInfo));

        VLOG(3) << "Setting the properties of the frame buffer";
        this->bufferInfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        VLOG(3) << "Frame buffer type is: " << this->bufferInfo.type;
        this->bufferInfo.memory = V4L2_MEMORY_MMAP;
        VLOG(3) << "Frame buffer memory is: " << this->bufferInfo.memory;
        this->bufferInfo.index = 0;
        VLOG(3) << "Frame buffer index is: " << this->bufferInfo.index;

        VLOG(3) << "Making the IOCTL to have the camera do its part of populating bufferInfo (with size and such)...";
        VLOG(3) << "IOCTL REQUEST = " << VIDIOC_QUERYBUF;
        if (ioctl(this->cameraFd, VIDIOC_QUERYBUF, &this->bufferInfo) < 0)
        {
            LOG(ERROR) << "Failed to query the buffer for the camera";
            throw std::runtime_error("failed to query camera buffer");
        }
        VLOG(3) << "The IOCTL call did not fail :)";

        VLOG(3) << "Getting the actual buffer memory";
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

        VLOG(3) << "Zeroing out the buffer";
        memset(this->bufferStart, 0, this->bufferInfo.length);

        VLOG(2) << "[ DONE ] buffers negotiated\n";
    }

    void Camera::startStream()
    {
        VLOG(2) << "Starting camera stream...";
        int type = this->bufferInfo.type;

        VLOG(3) << "Making the IOCTL to have the camera start streaming...";
        VLOG(3) << "IOCTL REQUEST = " << VIDIOC_STREAMON;
        if (ioctl(this->cameraFd, VIDIOC_STREAMON, &type) < 0)
        {
            LOG(ERROR) << "Failed to start the camera stream";
            throw std::runtime_error("failed to start stream");
        }
        VLOG(3) << "The IOCTL call did not fail :)";
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

    NetworkChunk *Camera::getFrameNC_BAD_TEMP_FUNC()
    {
        VLOG(3) << "GETTING FRAME!";
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

            NetworkChunk* nc = new NetworkChunk(DataType::CAMERA, this->bufferInfo.length, this->bufferStart);
            VLOG(3) << "[ DONE ] GOT FRAME";
            return nc;
        }
        else
        {
            NetworkChunk* nc = new NetworkChunk(DataType::NONE, 0, NULL);
            VLOG(3) << "[ DONE ] NO FRAME TO GET";
            return nc;
        }
    }

}