add_executable(echo echo.cc main.cc)
target_link_libraries(echo mouse_net glog)

add_executable(connector connector_test.cc)
target_link_libraries(connector mouse_net glog)
