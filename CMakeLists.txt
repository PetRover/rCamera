cmake_minimum_required(VERSION 3.3)

INCLUDE_DIRECTORIES("./../rWifi")
INCLUDE_DIRECTORIES("./../rCore")
INCLUDE_DIRECTORIES("LinuxHeaders" SYSTEM)

ADD_LIBRARY(rCamera STATIC camera.cpp rCamera.h)

TARGET_LINK_LIBRARIES(rCamera rWifi)