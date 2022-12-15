file(GLOB_RECURSE SRC_FILES *.cpp)
file(GLOB_RECURSE HDR_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

message("${SRC_FILES} ${HDR_FILES}")

add_library(infinity STATIC
    infinity.h
    ${SRC_FILES}
)

set_target_properties(infinity
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${RDMA_SOURCE_DIR}/release"
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(infinity
    PRIVATE ${RDMA_SOURCE_DIR}/src
)

target_link_libraries(infinity
    ibverbs
)

add_custom_target(release_hdr
    COMMAND mkdir -p ${RDMA_SOURCE_DIR}/release/include/infinity
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && cp --parents ${HDR_FILES} ${RDMA_SOURCE_DIR}/release/include/infinity
    DEPENDS ${HDR_FILES})

add_dependencies(infinity release_hdr)
