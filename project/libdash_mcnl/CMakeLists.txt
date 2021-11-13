cmake_minimum_required(VERSION 2.8)


file(GLOB_RECURSE libdash_mcnl_source *.cpp)

add_executable(libdash_mcnl_test ${libdash_mcnl_source})
target_link_libraries(libdash_mcnl_test dash)
