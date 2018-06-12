#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(RealSenseTransmitter)

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()


SET(CMAKE_INSTALL_NAME_DIR @executable_path)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)


file(GLOB DEPTH_STREAM_SOURCES "../../libs/DepthStream/src/*.cpp")
file(GLOB LIBRARIES "lib/*.dylib")
find_package(ZLIB)

add_executable(RealSenseTransmitter src/RealSenseTransmitter.cpp ${DEPTH_STREAM_SOURCES})
include_directories(RealSenseTransmitter include)
target_link_libraries(RealSenseTransmitter ${DEPENDENCIES} ${ZLIB_LIBRARIES})

target_link_libraries(RealSenseTransmitter ${LIBRARIES})

install(
    TARGETS
    RealSenseTransmitter

    RUNTIME DESTINATION
    ${CMAKE_INSTALL_PREFIX}/bin
)
