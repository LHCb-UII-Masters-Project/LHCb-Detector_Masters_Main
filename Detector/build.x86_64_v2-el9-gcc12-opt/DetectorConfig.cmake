cmake_policy(PUSH)
cmake_policy(VERSION 3.14)

cmake_policy(SET CMP0074 NEW)


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was DetectorConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set_and_check(Detector_PLUGINS_DIR "${PACKAGE_PREFIX_DIR}/lib")

include("${CMAKE_CURRENT_LIST_DIR}/DetectorDependencies.cmake")

# Import the targets
include("${CMAKE_CURRENT_LIST_DIR}/DetectorTargets.cmake")

# Add the installed dirs to the relevant PATH environment variables
set(ENV{LD_LIBRARY_PATH} "${Detector_PLUGINS_DIR}:$ENV{LD_LIBRARY_PATH}")
set(ENV{ROOT_INCLUDE_PATH} "${PACKAGE_PREFIX_DIR}/include:$ENV{ROOT_INCLUDE_PATH}")
set(ENV{PATH} "${PACKAGE_PREFIX_DIR}/bin:$ENV{PATH}")

# announce we found the project
if(NOT Detector_FIND_QUIETLY)
    message(STATUS "Found Detector: ${CMAKE_CURRENT_LIST_DIR} (found version 2.0)")
endif()

# Hide this file location to non-advanced users
mark_as_advanced(Detector_DIR)

include("${CMAKE_CURRENT_LIST_DIR}/DetectorMacros.cmake")

cmake_policy(POP)
