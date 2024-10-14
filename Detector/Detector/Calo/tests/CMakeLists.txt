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
add_executable(test_DeCalorimeter src/test_DeCalorimeter.cpp)
target_link_libraries(test_DeCalorimeter Catch2::Catch2 DetectorLib  ${TBB_lib})
# FIXME: this is too specific to the way lcg-toolchains and Gaudi projects work
set_target_properties(test_DeCalorimeter PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
# This is to ensure that we have everything we need if we want to build and run only test_DeCalorimeter
add_dependencies(test_DeCalorimeter DetectorPlugins)

catch_discover_tests(test_DeCalorimeter
    TEST_PREFIX Detector.Calo.
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.Calo"
)
