cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)
set (_projname "OrbbecAstraTransmitter")

# if (ANDROID)
#   set(ASTRA_ANDROID TRUE)
# elseif (WIN32)
#   set(ASTRA_WINDOWS TRUE)
# elseif (APPLE)
#   set(ASTRA_OSX TRUE)
# elseif (UNIX) #not android
#   set(ASTRA_UNIX TRUE)
# endif()

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#
# if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#   set(ASTRA_64 ON)
# else()
#   set(ASTRA_32 ON)
# endif()

function(add_astra_sdk_dependency _targetname)
  if(NOT TARGET ${_targetname})
    message("add_sdk_dependency: There is no target named '${_targetname}'")
    return()
  endif()
  if(TARGET ${SDK_DEPENDENT_TARGET})
    add_dependencies(${_targetname} ${SDK_DEPENDENT_TARGET})
  endif()
endfunction()


FILE (GLOB ${_projname}_SOURCES src/*.cpp ../../libs/DepthStream/src/*.cpp)
add_executable(${_projname} ${${_projname}_SOURCES})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# You should probably just set the ASTRA_ROOT environment variable in the command line, but you can also set it here;
# set(ASTRA_ROOT /path/to/AstraSDK-0.5.0-20160426T102621Z-darwin-x64)

find_package(Astra REQUIRED)
target_link_libraries(${_projname} ${ASTRA_LIBRARIES} z)
add_astra_sdk_dependency(${_projname})
include_directories(${ASTRA_INCLUDE_DIR})
