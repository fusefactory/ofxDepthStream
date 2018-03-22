# This script locates the Astra library
# ------------------------------------
#
# Usage
# -----
# find_package(Astra REQUIRED)
#
# Output
# ------
#
# This script defines the following variables:
# - ASTRA_LIBRARIES:    the list of all core Astra libraries
# - ASTRA_FOUND:        true if all the required modules are found
# - ASTRA_INCLUDE_DIR:  the path where Astra headers are located (the directory containing the astra_core/Astra.h file)
# - ASTRA_LIB_DIR:      the path where Astra libs are located
# - ASTRA_PLUGIN_LIBRARIES: the list of all plugin libraries
# - ASTRA_TOML:         a list TOML configuration files for Astra
# - ASTRA_PLUGIN_TOML:  a list of TOML configuration files for plugins
#
# example:
#   find_package(Astra REQUIRED)
#   include_directories(${ASTRA_INCLUDE_DIR})
#   add_executable(myapp ...)
#   target_link_libraries(myapp ${ASTRA_LIBRARIES})

# define the list of search paths for headers and libraries
set(FIND_ASTRA_PATHS
  ${ASTRA_ROOT}
  $ENV{ASTRA_ROOT}
  ..
  AstraSDK
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt)

# find the required libraries
set(ASTRA_FOUND TRUE) # will be set to false if one of the required modules is not found

if (NOT ASTRA_SDK_BUILD)
  # find the Astra include directory
  find_path(ASTRA_INCLUDE_DIR astra_core/astra_core.hpp
    PATH_SUFFIXES include
    PATHS ${FIND_ASTRA_PATHS})

  set(ASTRA_FIND_COMPONENTS
    astra_core
    astra_core_api
    astra
    )

  MARK_AS_ADVANCED(ASTRA_LIB_DIR)

  foreach(FIND_ASTRA_COMPONENT ${ASTRA_FIND_COMPONENTS})
    string(TOUPPER ${FIND_ASTRA_COMPONENT} FIND_ASTRA_COMPONENT_UPPER)

    # dynamic release library
    find_library(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE
      NAMES ${FIND_ASTRA_COMPONENT}
      PATH_SUFFIXES lib64 lib
      PATHS ${FIND_ASTRA_PATHS})

    # dynamic debug library
    find_library(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG
      NAMES ${FIND_ASTRA_COMPONENT}-d
      PATH_SUFFIXES lib64 lib
      PATHS ${FIND_ASTRA_PATHS})

    if (ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG OR ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE)
      # library found

      set(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_FOUND TRUE)

      # if both are found, set ASTRA_XXX_LIBRARY to contain both
      if (ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG AND ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE)
        set(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY debug     ${ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG}
          optimized ${ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE})
      endif()

      # if only one debug/release variant is found, set the other to be equal to the found one
      if (ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG AND NOT ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE)
        # debug and not release
        set(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE ${ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG})
        set(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY         ${ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG})
      endif()
      if (ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE AND NOT ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG)
        # release and not debug
        set(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG ${ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE})
        set(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY       ${ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE})
      endif()

      if (NOT ASTRA_LIB_DIR)
          get_filename_component(ASTRA_LIB_DIR ${ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE} DIRECTORY)
      endif()
    else()
      # library not found
      set(ASTRA_FOUND FALSE)
      set(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_FOUND FALSE)
      set(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY "")
      set(FIND_ASTRA_MISSING "${FIND_ASTRA_MISSING} ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY")
    endif()

    # mark as advanced
    MARK_AS_ADVANCED(ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY
      ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_RELEASE
      ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY_DEBUG)

    # add to the global list of libraries
    set(ASTRA_LIBRARIES ${ASTRA_LIBRARIES} "${ASTRA_${FIND_ASTRA_COMPONENT_UPPER}_LIBRARY}")
  endforeach()

  if (ASTRA_FOUND)
    FILE(GLOB ASTRA_PLUGIN_LIBRARIES   "${ASTRA_LIB_DIR}/Plugins/*.dll"
                                          "${ASTRA_LIB_DIR}/Plugins/*.dylib"
                                          "${ASTRA_LIB_DIR}/Plugins/*.so")
    FILE(GLOB ASTRA_PLUGIN_TOML   "${ASTRA_LIB_DIR}/Plugins/*.toml")
    FILE(GLOB ASTRA_TOML   "${ASTRA_LIB_DIR}/*.toml")

  endif()
endif()

# handle errors
if(NOT ASTRA_FOUND)
  # include directory or library not found
  set(FIND_ASTRA_ERROR "Could NOT find Astra (missing: ${FIND_ASTRA_MISSING})")
  if(ASTRA_FIND_REQUIRED)
    # fatal error
    message(FATAL_ERROR ${FIND_ASTRA_ERROR})
  elseif(NOT ASTRA_FIND_QUIETLY)
    # error but continue
    message("${FIND_ASTRA_ERROR}")
  endif()
endif()

# handle success
if(ASTRA_FOUND)
  message(STATUS "Found Astra includes in ${ASTRA_INCLUDE_DIR}")
  message(STATUS "Found Astra lib dir: ${ASTRA_LIB_DIR}")
  message(STATUS "Found Astra libraries: ${ASTRA_LIBRARIES}")
  message(STATUS "Found Astra TOML: ${ASTRA_TOML}")
  message(STATUS "Found Astra plugins: ${ASTRA_PLUGIN_LIBRARIES}")
  message(STATUS "Found Astra plugin TOML: ${ASTRA_PLUGIN_TOML}")
endif()
