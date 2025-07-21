# IF(BUILD_TESTING)
#     ADD_EXECUTABLE(scheduler_test main.cpp)
#
#     TARGET_INCLUDE_DIRECTORIES(scheduler_test PRIVATE
#         ${PROJECT_SOURCE_DIR}/fiber_lib
#     )
#
#     SET_TARGET_PROPERTIES(scheduler_test PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(scheduler_test PRIVATE fiber_shared)
#
#     #注册测试
#     ADD_TEST(NAME SchedulerUnitTest COMMAND scheduler_test)
# ENDIF()
#

# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_socket test_socket.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_socket PRIVATE
#         ${PROJECT_SOURCE_DIR}/fiber_lib
#         ${PROJECT_SOURCE_DIR}/fiber_lib/util
#     )
#
#     SET_TARGET_PROPERTIES(test_socket PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_socket PRIVATE fiber_shared)
#
#     #注册测试
#     ADD_TEST(NAME HookUnitTest COMMAND test_socket)
# ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_socket test_socket.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_socket PRIVATE
        ${PROJECT_SOURCE_DIR}/fiber_lib
        ${PROJECT_SOURCE_DIR}/fiber_lib/util
    )

    SET_TARGET_PROPERTIES(test_socket PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_socket PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_socket)
ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_address test_address.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_address PRIVATE
        ${PROJECT_SOURCE_DIR}/fiber_lib
        ${PROJECT_SOURCE_DIR}/fiber_lib/util
    )

    SET_TARGET_PROPERTIES(test_address PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_address PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_address)
ENDIF()

IF(BUILD_TESTING)
    SET(CMAKE_BUILD_TYPE Debug)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

    ADD_EXECUTABLE(test_bytearray test_bytearray.cpp)


    TARGET_INCLUDE_DIRECTORIES(test_bytearray PRIVATE
        ${PROJECT_SOURCE_DIR}/fiber_lib
        ${PROJECT_SOURCE_DIR}/fiber_lib/util
    )

    SET_TARGET_PROPERTIES(test_bytearray PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    #链接到测试的库
    TARGET_LINK_LIBRARIES(test_bytearray PRIVATE fiber_shared)

    #注册测试
    # ADD_TEST(NAME HookUnitTest COMMAND test_bytearray)
ENDIF()
