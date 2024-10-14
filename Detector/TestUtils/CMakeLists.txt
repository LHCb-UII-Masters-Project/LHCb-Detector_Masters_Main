###############################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################

# Library with helper classes to simplify writing tests
add_library(TestUtils SHARED src/Fixture.cpp)
target_include_directories(TestUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(TestUtils
  PRIVATE
    fmt::fmt
  PUBLIC
    ROOT::Core
    DD4hep::DDCore
)
