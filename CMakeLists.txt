cmake_minimum_required(VERSION 3.3)

SET(USE_OPEN_CV FALSE)

if (USE_OPEN_CV)
    FIND_PACKAGE(OpenCV REQUIRED core imgproc highgui imgcodecs)
endif()

#FIND_PACKAGE(libuvc REQUIRED)

#INCLUDE_DIRECTORIES(SYSTEM ${libuvc_INCLUDE_DIRS})
INCLUDE_DIRECTORIES("./../rWifi")
INCLUDE_DIRECTORIES("./../rCore")

ADD_LIBRARY(rCamera STATIC camera.cpp rCamera.h)

if (USE_OPEN_CV)
    TARGET_LINK_LIBRARIES(rCamera ${OpenCV_LIBS})
endif()
#TARGET_LINK_LIBRARIES(rCamera ${libuvc_LIBRARIES})
TARGET_LINK_LIBRARIES(rCamera rWifi)
