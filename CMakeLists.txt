cmake_minimum_required(VERSION 3.15)
# PROJECT(FiberLib)
project(dag
    VERSION 1.0
    LANGUAGES CXX
)

# 检查是否创建build目录
get_filename_component(BUILD_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
string(TOLOWER "${BUILD_DIR_NAME}" BUILD_DIR_NAME_LOWER)
if(NOT BUILD_DIR_NAME_LOWER MATCHES "build")
    message(FATAL_ERROR
        "The build directory must be named include 'build'.\n"
        "Current build directory: ${CMAKE_BINARY_DIR}\n"
        "Please create a build directory with the correct name and run CMake again."
    )
endif()

# 设置c++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include(GNUInstallDirs)
include(CMakeDependentOption)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})


set(SRC_INCLUDE_DIR
    "${PROJECT_SOURCE_DIR}/dag"
    "${PROJECT_SOURCE_DIR}/util"
    "${PROJECT_SOURCE_DIR}/stream"
    "${PROJECT_BINARY_DIR}/dag")

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
option(ENABLE_DEBUG_MODE "Enable debug mode" ON)
option(ENABLE_BUILD_SHARED_LIBS "Enable build shared libs" OFF)
cmake_dependent_option(ENABLE_COMPILE_OPTIMIZE "Enable compile options -O3" ON "NOT ENABLE_DEBUG_MODE" OFF)

set(
    BUILD_SHARED_LIBS
    ${ENABLE_BUILD_SHARED_LIBS}
    CACHE INTERNAL ""
)

if(NOT CMAKE_BUILD_TYPE)
    if(ENABLE_DEBUG_MODE)
        set(CMAKE_BUILD_TYPE Debug)
    else()
        set(CMAKE_BUILD_TYPE Release)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ENABLE_DEBUG_MODE ON)
    set(ENABLE_COMPILE_OPTIMIZE OFF)
endif()

message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")
message(STATUS "Complier path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")
message(STATUS "Enable debug mode: ${ENABLE_DEBUG_MODE}")
message(STATUS "Enable build shared libs: ${ENABLE_BUILD_SHARED_LIBS}")
message(STATUS "Enable compile options -O3: ${ENABLE_COMPILE_OPTIMIZE}")

add_subdirectory(test)
# add_subdirectory(third_party)
add_subdirectory(dag)
add_subdirectory(benchmark)

add_library(${PROJECT_NAME} ${DAG_SOURCE_FILES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        LINKER_LANGUAGE CXX
        PREFIX ""
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE "-g")
endif()
if(ENABLE_COMPILE_OPTIMIZE)
    target_compile_options(${PROJECT_NAME} PUBLIC -O3)
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        "$<$<CONFIG:DEBUG>:DDEBUG>"
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        "$<$<CONFIG:RELEASE>:RELEASE>"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${SRC_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE pthread)

if(ENABLE_UNIT_TESTS)
    enable_testing()
endif()

