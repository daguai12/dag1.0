CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT(FiberLib)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置c++标准
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用测试功能
IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    INCLUDE(CTest)
ENDIF()

ADD_SUBDIRECTORY(fiber_lib)
ADD_SUBDIRECTORY(test)
