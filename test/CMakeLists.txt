add_custom_target(build_tests COMMENT "Building all tests")

add_custom_target(check COMMENT "Running all tests")


file(GLOB_RECURSE DAG_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*.cpp")

foreach(dag_test_source ${DAG_TEST_SOURCES})
    get_filename_component(dag_test_filename ${dag_test_source} NAME)
    string(REPLACE ".cpp" "" dag_test_name ${dag_test_filename})
    add_executable(${dag_test_name} EXCLUDE_FROM_ALL ${dag_test_source})

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

    add_custom_target(build-${dag_test_command}
        COMMAND echo "build ${dag_test_name} test..."
        DEPENDS ${dag_test_name}
        COMMENT "build ${dag_test_command} tests..."
    )

    add_custom_target(check-${dag_test_command}
        COMMAND $<TARGET_FILE:${dag_test_name}>
        DEPENDS ${dag_test_name}
        COMMENT "Running ${dag_test_name} test..."
    )

    add_dependencies(check check-${dag_test_command})
endforeach()

