cmake_minimum_required(VERSION 3.16.0)
list(APPEND EXTRA_COMPONENT_DIRS "../../common/esp32-camera")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(camera_example)
