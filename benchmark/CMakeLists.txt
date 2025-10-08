file(GLOB_RECURSE DAG_BENCH_SOURCES "${PROJECT_SOURCE_DIR}/benchmark/*/*.cpp")

add_custom_target(build-bench COMMAND echo "Building benchmark case...")

add_custom_target(build-benchtools
    COMMAND cargo run --release --manifest-path=${PROJECT_SOURCE_DIR}/third_party/rust_echo_bench/Cargo.toml -- --help
)

foreach(dag_bench_source ${DAG_BENCH_SOURCES})
    get_filename_component(dag_bench_filename ${dag_bench_source} NAME)
    string(REPLACE ".cpp" "" dag_bench_name ${dag_bench_filename})
    add_executable(${dag_bench_name} EXCLUDE_FROM_ALL ${dag_bench_source})
    add_dependencies(build-bench ${dag_bench_name})

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${dag_bench_name} PRIVATE "-g")
    endif()
    if(ENABLE_COMPILE_OPTIMIZE)
        target_compile_options(${dag_bench_name} PUBLIC -O3)
    endif()
    target_link_libraries(${dag_bench_name} ${PROJECT_NAME})

    set_target_properties(${dag_bench_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmark"
    )

    string(REPLACE "_bench" "" dag_bench_command ${dag_bench_name})
    add_custom_target(bench_${dag_bench_command}
        COMMAND $<TARGET_FILE:${dag_bench_name}>
        DEPENDS ${dag_bench_name}
        COMMENT "Running ${tinycoro_bench_command} bench..."
    )

endforeach()
