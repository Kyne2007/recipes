set(base_SRCS
    timestamp.cc
    )

add_library(mouse_base ${base_SRCS})
target_link_libraries(mouse_base pthread rt)

