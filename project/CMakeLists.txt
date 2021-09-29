cmake_minimum_required(VERSION 3.18)

# Libdash
include_directories(libxml2/include)
include_directories(libdash/include)
include_directories(sdl/include)
include_directories(zlib/include)
include_directories(iconv/include)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(libdash)
add_subdirectory(libdash_mcnl)
add_subdirectory(Main)

##project(Open3DCMakeFindPackage LANGUAGES C CXX)

# The options need to be the same as Open3D's default
# If Open3D is configured and built with custom options, you'll also need to
# specify the same custom options.
option(STATIC_WINDOWS_RUNTIME "Use static (MT/MTd) Windows runtime" ON)
if(STATIC_WINDOWS_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()


# Find installed Open3D, which exports Open3D::Open3D
##find_package(Open3D REQUIRED)


# On Windows if BUILD_SHARED_LIBS is enabled, copy .dll files to the executable directory
if(WIN32)
    get_target_property(open3d_type Open3D::Open3D TYPE)
    if(open3d_type STREQUAL "SHARED_LIBRARY")
        message(STATUS "Copying Open3D.dll to ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
		add_custom_command(TARGET Main POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy
                                   ${CMAKE_INSTALL_PREFIX}/bin/Open3D.dll
                                   ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    endif()
endif()

##project(Open3DCMakeFindPackage LANGUAGES C CXX)

# The options need to be the same as Open3D's default
# If Open3D is configured and built with custom options, you'll also need to
# specify the same custom options.
##option(STATIC_WINDOWS_RUNTIME "Use static (MT/MTd) Windows runtime" ON)
##if(STATIC_WINDOWS_RUNTIME)
##    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
##else()
##    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
##endif()


# Find installed Open3D, which exports Open3D::Open3D
##find_package(Open3D REQUIRED)

##add_executable(Main)
##target_sources(Main PRIVATE Main.cpp)
##target_link_libraries(Main PRIVATE Open3D::Open3D)
##target_link_libraries(Main PRIVATE -lstdc++fs)
# target_link_libraries(Main PUBLIC ${CMAKE_SOURCE_DIR}/../../../include/libdash_networkpart/source)

# On Windows if BUILD_SHARED_LIBS is enabled, copy .dll files to the executable directory
##if(WIN32)
##    get_target_property(open3d_type Open3D::Open3D TYPE)
##    if(open3d_type STREQUAL "SHARED_LIBRARY")
##        message(STATUS "Copying Open3D.dll to ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
##        add_custom_command(TARGET Draw POST_BUILD
##                           COMMAND ${CMAKE_COMMAND} -E copy
##                                   ${CMAKE_INSTALL_PREFIX}/bin/Open3D.dll
##                                   ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
##    endif()
##endif()

## Libdash
#include_directories(libxml2/include)
#include_directories(libdash/include)
#include_directories(sdl/include)
#include_directories(zlib/include)
#include_directories(iconv/include)

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

#add_subdirectory(libdash)
#add_subdirectory(Main)
