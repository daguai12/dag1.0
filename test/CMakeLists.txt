# file(GLOB_RECURSE DAG_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*.cpp")
#
# foreach(dag_test_source ${DAG_TEST_SOURCES})
#     get_filename_component(dag_test_filename ${dag_test_source} NAME)
#     string(REPLACE ".cpp" "" dag_test_name ${dag_test_filename})
#     add_executable(${dag_test_name} EXCLUDE_FROM_ALL ${dag_test_source})
#     add_dependencies(build_tests ${dag_test_name})
#     add_dependencies(build_check ${dag_test_name})
#
#     target_link_libraries(${dag_test_name} ${PROJECT_NAME})
#     if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#         target_compile_options(${dag_test_name} PRIVATE -g)
#     endif()
#     if(ENABLE_COMPLIE_OPTIMIZE)
#         target_compile_options(${dag_test_name} PRIVATE -O3)
#     endif()
#
#     set_target_properties(${dag_test_name}
#         PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
#     )
#
#     string(REPLACE "test_" "" dag_test_command ${dag_test_name})
#     add_custom_target(build-${dag_test_command}
#         COMMAND echo "build ${dag_test_name} test..."
#         DEPENDS ${dag_test_name}
#         COMMENT "build ${dag_test_command} tests..."
#     )
#
#     add_custom_target(check-${dag_test_command}
#         COMMAND $<TARGET_FILE:${dag_test_name}>
#         DEPENDS ${dag_test_name}
#         COMMENT "Running ${dag_test_name} test..."
#     )
# endforeach()

# 在循环开始前，先创建两个空的自定义目标，用作“聚合目标”或“分组目标”
# 当你执行 make build_tests 时，所有依赖于它的目标都会被构建
add_custom_target(build_tests COMMENT "Building all tests")

# 当你执行 make check 时，所有依赖于它的目标都会被执行
add_custom_target(check COMMENT "Running all tests")


file(GLOB_RECURSE DAG_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*.cpp")

foreach(dag_test_source ${DAG_TEST_SOURCES})
    get_filename_component(dag_test_filename ${dag_test_source} NAME)
    string(REPLACE ".cpp" "" dag_test_name ${dag_test_filename})
    add_executable(${dag_test_name} EXCLUDE_FROM_ALL ${dag_test_source})

    # 将每个单独的测试可执行文件作为 build_tests 目标的依赖
    # 这样 `make build_tests` 就会构建所有的 ${dag_test_name}
    add_dependencies(build_tests ${dag_test_name})

    target_link_libraries(${dag_test_name} ${PROJECT_NAME})
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${dag_test_name} PRIVATE -g)
    endif()
    if(ENABLE_COMPLIE_OPTIMIZE)
        target_compile_options(${dag_test_name} PRIVATE -O3)
    endif()

    set_target_properties(${dag_test_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )

    string(REPLACE "test_" "" dag_test_command ${dag_test_name})

    # 这个目标用于构建单个测试，但我们已经有 build_tests 了，所以这一步可以省略或保留
    add_custom_target(build-${dag_test_command}
        COMMAND echo "build ${dag_test_name} test..."
        DEPENDS ${dag_test_name}
        COMMENT "build ${dag_test_command} tests..."
    )

    # 这个目标用于运行单个测试
    add_custom_target(check-${dag_test_command}
        COMMAND $<TARGET_FILE:${dag_test_name}>
        DEPENDS ${dag_test_name}
        COMMENT "Running ${dag_test_name} test..."
    )

    # 将每个单独的 check-* 目标作为总 check 目标的依赖
    # 这样 `make check` 就会执行所有的 check-* 目标，从而运行所有测试
    add_dependencies(check check-${dag_test_command})
endforeach()

# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_fiber test_fiber.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_fiber PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#         ${PROJECT_SOURCE_DIR}/dag/stream
#     )
#
#     SET_TARGET_PROPERTIES(test_fiber PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     TARGET_LINK_LIBRARIES(test_fiber PRIVATE fiber_shared)
#
# ENDIF()
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_timer test_timer.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_timer PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#         ${PROJECT_SOURCE_DIR}/dag/stream
#     )
#
#     SET_TARGET_PROPERTIES(test_timer PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     TARGET_LINK_LIBRARIES(test_timer PRIVATE fiber_shared)
#
# ENDIF()
#
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_hook test_hook.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_hook PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#         ${PROJECT_SOURCE_DIR}/dag/stream
#     )
#
#     SET_TARGET_PROPERTIES(test_hook PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     TARGET_LINK_LIBRARIES(test_hook PRIVATE fiber_shared)
#
# ENDIF()
#
#
#
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
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#     )
#
#     SET_TARGET_PROPERTIES(test_socket PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_socket PRIVATE fiber_shared)
#
#     #注册测试
#     ADD_TEST(NAME HookUnitTest COMMAND test_socket)
# ENDIF()
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_iomanager test_iomanager.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_iomanager PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#     )
#
#     SET_TARGET_PROPERTIES(test_iomanager PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_iomanager PRIVATE fiber_shared)
#
# ENDIF()
#
#
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_ioscheduler test_ioscheduler.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_ioscheduler PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#     )
#
#     SET_TARGET_PROPERTIES(test_ioscheduler PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_ioscheduler PRIVATE fiber_shared)
#
#     #注册测试
#     # ADD_TEST(NAME HookUnitTest COMMAND test_ioscheduler)
# ENDIF()
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_scheduler test_scheduler.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_scheduler PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#     )
#
#     SET_TARGET_PROPERTIES(test_scheduler PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_scheduler PRIVATE fiber_shared)
#
#     #注册测试
#     # ADD_TEST(NAME HookUnitTest COMMAND test_scheduler)
# ENDIF()
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_bytearray test_bytearray.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_bytearray PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#     )
#
#     SET_TARGET_PROPERTIES(test_bytearray PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_bytearray PRIVATE fiber_shared)
#
#     #注册测试
#     # ADD_TEST(NAME HookUnitTest COMMAND test_bytearray)
# ENDIF()
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_tcp_server test_tcp_server.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_tcp_server PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#         ${PROJECT_SOURCE_DIR}/dag/stream
#     )
#
#     SET_TARGET_PROPERTIES(test_tcp_server PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_tcp_server PRIVATE fiber_shared)
#
#     #注册测试
#     # ADD_TEST(NAME HookUnitTest COMMAND test_tcp_server)
# ENDIF()
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_socket_stream test_socket_stream.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_socket_stream PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#         ${PROJECT_SOURCE_DIR}/dag/stream
#     )
#
#     SET_TARGET_PROPERTIES(test_socket_stream PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_socket_stream PRIVATE fiber_shared)
#
#     #注册测试
#     # ADD_TEST(NAME HookUnitTest COMMAND test_socket_stream)
# ENDIF()
#
# IF(BUILD_TESTING)
#     SET(CMAKE_BUILD_TYPE Debug)
#
#     SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#
#     ADD_EXECUTABLE(test_socket_stream_client test_socket_stream_client.cpp)
#
#
#     TARGET_INCLUDE_DIRECTORIES(test_socket_stream_client PRIVATE
#         ${PROJECT_SOURCE_DIR}/dag
#         ${PROJECT_SOURCE_DIR}/dag/util
#         ${PROJECT_SOURCE_DIR}/dag/stream
#     )
#
#     SET_TARGET_PROPERTIES(test_socket_stream_client PROPERTIES
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
#     )
#
#     #链接到测试的库
#     TARGET_LINK_LIBRARIES(test_socket_stream_client PRIVATE fiber_shared)
#
#     #注册测试
#     # ADD_TEST(NAME HookUnitTest COMMAND test_socket_stream_client)
# ENDIF()
