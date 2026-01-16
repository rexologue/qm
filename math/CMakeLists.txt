cmake_minimum_required(VERSION 3.20)

add_library(miv_math INTERFACE)
add_library(miv::math ALIAS miv_math)

target_include_directories(miv_math INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(miv_math INTERFACE miv::containers)
target_compile_features(miv_math INTERFACE cxx_std_23)
