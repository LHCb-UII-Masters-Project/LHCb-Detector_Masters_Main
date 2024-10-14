###############################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################
#[========================================================================[.rst:
DetectorDependencies
--------------------

This file gather all the calls to find_package() used by Detector

#]========================================================================]

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

# Find ROOT forcing the CONFIG mode.
# This forces CMake to ignore the Gaudi FindROOT.cmake which does not
# create the targets appropriately
find_package(ROOT REQUIRED COMPONENTS Geom Gdml ROOTEve ROOTWebDisplay Graf CONFIG)

if(NOT TARGET GitCondDB::GitCondDB)
  find_package(GitCondDB 0.1.3 REQUIRED)
endif()
find_package(fmt 6.1.2 REQUIRED)
find_package(yaml-cpp 0.6.2 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost CONFIG REQUIRED program_options)
find_package(Vc REQUIRED)
find_package(Rangev3 REQUIRED)

# We need yaml-cpp > 0.6.3 to use a well defined yaml-cpp::yaml-cpp target
if(NOT TARGET yaml-cpp::yaml-cpp AND TARGET yaml-cpp)
  add_library(yaml-cpp::yaml-cpp INTERFACE IMPORTED GLOBAL)
  target_link_libraries(yaml-cpp::yaml-cpp INTERFACE yaml-cpp)
  target_include_directories(yaml-cpp::yaml-cpp SYSTEM INTERFACE ${YAML_CPP_INCLUDE_DIR})
endif()

find_package(DD4hep REQUIRED DDCore DDCond)

find_library(TBB_lib libtbb.so)

if(LCG_VERSION STREQUAL "LCG_96b")
  get_filename_component(DD4hep_ROOT "${DD4hep_DIR}" DIRECTORY)
  set(ENV{LD_LIBRARY_PATH} "${DD4hep_ROOT}/lib:$ENV{LD_LIBRARY_PATH}")
  set(ENV{PATH} "${DD4hep_ROOT}/bin:$ENV{PATH}")
  set(ENV{PYTHONPATH} "${DD4hep_ROOT}/lib/python2.7/site-packages:$ENV{PYTHONPATH}")
  set(ENV{ROOT_INCLUDE_PATH} "${DD4hep_ROOT}/include:$ENV{ROOT_INCLUDE_PATH}")
  # temporary hack
  set(ENV{ROOT_INCLUDE_PATH} "${DD4hep_ROOT}/DDCore/src:$ENV{ROOT_INCLUDE_PATH}")
endif()
