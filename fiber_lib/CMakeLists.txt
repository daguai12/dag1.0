FIND_PACKAGE(OpenSSL REQUIRED)
IF(OPENSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
ELSE()
    message(FATAL_ERROR "OpenSSL not found! Please install OpenSSL first (libssl-dev on Ubuntu).")
ENDIF()



INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/utils)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(fiber_shared SHARED
            fiber.cpp scheduler.cpp thread.cpp timer.cpp
            ioscheduler.cpp fd_manager.cpp hook.cpp logger.cpp 
            socket.cpp address.cpp
            utils/util.cpp )

ADD_LIBRARY(fiber_static STATIC
            fiber.cpp scheduler.cpp thread.cpp timer.cpp
            ioscheduler.cpp fd_manager.cpp hook.cpp logger.cpp
            socket.cpp address.cpp
            utils/util.cpp)

TARGET_LINK_LIBRARIES(fiber_shared PRIVATE OpenSSL::SSL OpenSSL::Crypto)


TARGET_LINK_LIBRARIES(fiber_static PRIVATE OpenSSL::SSL OpenSSL::Crypto)

SET_TARGET_PROPERTIES(fiber_shared PROPERTIES
    VERSION 1.0.0
    OUTPUT_NAME "myfiber"
    CLEAN_DIRECT_OUTPUT 1
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
)



SET_TARGET_PROPERTIES(fiber_static PROPERTIES
    VERSION 1.0.0
    OUPUT_NAME "myfiber"
    CLEAN_DIRECT_OUTPUT 1
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
)















# 如果启用了测试
# IF(BUILD_TESTING)
#     #添加一个测试用的可执行文件
#     ADD_EXECUTABLE(fiber_test fiber_test.cpp)
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(fiber_test PRIVATE fiber_shared)
#     #注册测试
#     ADD_TEST(NAME FiberUnitTest COMMAND fiber_test)
# ENDIF()

# 测试多线程环境下
# IF(BUILD_TESTING)
#     #添加一个测试用的可执行文件
#     ADD_EXECUTABLE(fiber_test more_thread_fiber_test.cpp)
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(fiber_test PRIVATE fiber_shared)
#     #注册测试
#     ADD_TEST(NAME FiberUnitTest COMMAND fiber_test)
# ENDIF()


# IF(BUILD_TESTING)
#     #添加一个测试用的可执行文件
#     ADD_EXECUTABLE(scheduler_test main.cpp)
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(scheduler_test PRIVATE fiber_shared)
#     #注册测试
#     ADD_TEST(NAME SchedulerUnitTest COMMAND scheduler_test)
# ENDIF()

