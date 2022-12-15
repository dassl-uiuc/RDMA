link_libraries(
    infinity
    ibverbs
    Threads::Threads)

add_executable(read-write-send read-write-send.cpp)
add_executable(send-performance send-performance.cpp)
add_executable(custom-read-write custom-read-write.cpp)
add_executable(multi-read-write multi-read-write.cpp)
add_executable(sequencer sequencer.cpp)

include_directories(${RDMA_SOURCE_DIR}/release/include)

set_target_properties(read-write-send send-performance custom-read-write multi-read-write sequencer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${RDMA_SOURCE_DIR}/release/examples"
)
