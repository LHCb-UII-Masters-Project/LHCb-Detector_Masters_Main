###############################################################################
# (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################

# Tests not requiring DetectorLib or DD4hep
add_executable(delhcb_code_tests
    src/test_Tell40Links.cpp
)
target_link_libraries(delhcb_code_tests
    Catch2::Catch2WithMain
    fmt::fmt
    nlohmann_json::nlohmann_json
    DetectorLib
)
catch_discover_tests(delhcb_code_tests
    TEST_PREFIX Detector.LHCb.
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.LHCb"
)
