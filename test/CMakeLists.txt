IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_fiber test_fiber.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_fiber PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
        ${PROJECT_SOURCE_DIR}/dag/stream
    )

    SET_TARGET_PROPERTIES(test_fiber PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    TARGET_LINK_LIBRARIES(test_fiber PRIVATE fiber_shared)

ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_timer test_timer.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_timer PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
        ${PROJECT_SOURCE_DIR}/dag/stream
    )

    SET_TARGET_PROPERTIES(test_timer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    TARGET_LINK_LIBRARIES(test_timer PRIVATE fiber_shared)

ENDIF()


IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_hook test_hook.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_hook PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
        ${PROJECT_SOURCE_DIR}/dag/stream
    )

    SET_TARGET_PROPERTIES(test_hook PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    TARGET_LINK_LIBRARIES(test_hook PRIVATE fiber_shared)

ENDIF()




IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_socket test_socket.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_socket PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
    )

    SET_TARGET_PROPERTIES(test_socket PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_socket PRIVATE fiber_shared)

    #注册测试
    ADD_TEST(NAME HookUnitTest COMMAND test_socket)
ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_iomanager test_iomanager.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_iomanager PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
    )

    SET_TARGET_PROPERTIES(test_iomanager PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_iomanager PRIVATE fiber_shared)

ENDIF()



IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_ioscheduler test_ioscheduler.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_ioscheduler PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
    )

    SET_TARGET_PROPERTIES(test_ioscheduler PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_ioscheduler PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_ioscheduler)
ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_scheduler test_scheduler.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_scheduler PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
    )

    SET_TARGET_PROPERTIES(test_scheduler PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_scheduler PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_scheduler)
ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_bytearray test_bytearray.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_bytearray PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
    )

    SET_TARGET_PROPERTIES(test_bytearray PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_bytearray PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_bytearray)
ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_tcp_server test_tcp_server.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_tcp_server PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
        ${PROJECT_SOURCE_DIR}/dag/stream
    )

    SET_TARGET_PROPERTIES(test_tcp_server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_tcp_server PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_tcp_server)
ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_socket_stream test_socket_stream.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_socket_stream PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
        ${PROJECT_SOURCE_DIR}/dag/stream
    )

    SET_TARGET_PROPERTIES(test_socket_stream PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_socket_stream PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_socket_stream)
ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_socket_stream_client test_socket_stream_client.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_socket_stream_client PRIVATE
        ${PROJECT_SOURCE_DIR}/dag
        ${PROJECT_SOURCE_DIR}/dag/util
        ${PROJECT_SOURCE_DIR}/dag/stream
    )

    SET_TARGET_PROPERTIES(test_socket_stream_client PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_socket_stream_client PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_socket_stream_client)
ENDIF()
