cmake_minimum_required(VERSION 3.20)

add_library(miv_containers INTERFACE)
add_library(miv::containers ALIAS miv_containers)

target_include_directories(miv_containers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(miv_containers INTERFACE cxx_std_23)
