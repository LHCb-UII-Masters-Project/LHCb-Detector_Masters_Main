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

# Tests not requiring DetectorLib or DD4hep
add_executable(muon_code_tests
    src/test_FrontEndID.cpp
    src/test_ReadoutCond.cpp ../src/ReadoutCond.cpp ../src/ReadoutCondJson.cpp
    src/test_MuonVolumeID.cpp
    src/test_Types.cpp
)
target_link_libraries(muon_code_tests
    Catch2::Catch2WithMain
    fmt::fmt
    nlohmann_json::nlohmann_json
    yaml-cpp
    DetectorLib
    ${TBB_lib}
)
catch_discover_tests(muon_code_tests
    TEST_PREFIX Detector.Muon.
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.Muon"
)

# Tests requiring DetectorLib and DD4hep
add_executable(muon_tests
    src/test_DeMuon.cpp
)
target_link_libraries(muon_tests
    Catch2::Catch2WithMain
    DetectorLib
    TestUtils
    ${TBB_lib}
)
# FIXME: this is too specific to the way lcg-toolchains and Gaudi projects work
set_target_properties(muon_tests PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
# This is to ensure that we have everything we need if we want to build and run only muon_tests
add_dependencies(muon_tests DetectorPlugins)

catch_discover_tests(muon_tests
    TEST_PREFIX Detector.Muon.
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.Muon"
)
