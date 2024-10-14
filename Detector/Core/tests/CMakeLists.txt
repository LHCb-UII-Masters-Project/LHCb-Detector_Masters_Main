###############################################################################
# (c) Copyright 2020-2023 CERN for the benefit of the LHCb Collaboration      #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################

add_executable(test_GitCondDB src/test_GitCondDB.cpp)
target_link_libraries(test_GitCondDB
    Catch2::Catch2WithMain
    GitCondDB::GitCondDB
    yaml-cpp
)
catch_discover_tests(test_GitCondDB
    TEST_PREFIX Detector.Core.
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.Core"
)

add_executable(test_DDS
    src/test_DDS.cpp
    src/test_DDS_limit_IOV.cpp
    src/test_DDS_tell40links.cpp
    src/test_DDS_interactionregion.cpp
    src/test_DDS_lhcinfo.cpp
    src/test_conddb_schema_handling.cpp
    src/test_condition_git_reader.cpp
    src/test_DDS_SMOG.cpp
)
target_link_libraries(test_DDS
    Catch2::Catch2WithMain
    DetectorLib
    GitCondDB::GitCondDB
    TestUtils
    yaml-cpp
    ${TBB_lib}
)

# FIXME: this is too specific to the way lcg-toolchains and Gaudi projects work
set_target_properties(test_DDS PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
# This is to ensure that we have everything we need if we want to build and run only test_DDS
add_dependencies(test_DDS DetectorPlugins)

catch_discover_tests(test_DDS
    TEST_PREFIX Detector.Core.
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.Core"
)

# install the test wrapper script
install(FILES scripts/runTest.sh DESTINATION ${CMAKE_BINARY_DIR}/bin/
  PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

set(TEST_SCRIPT ${PROJECT_SOURCE_DIR}/Core/tests/scripts/runTest.sh)

add_test(NAME test_load_all
         COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -dry-run)

foreach(subdetector IN ITEMS VP UT FT Rich1 Rich2 Muon Ecal Hcal UP MP TV)
    add_test(NAME test_load_${subdetector}_debug
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/components/debug/${subdetector}.xml -dry-run)
endforeach()

add_test(NAME test_find_intersection
         COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin find_intersections 0 0 0 5 5 5)

add_test(NAME test_load_conditions
         COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_DetectorDataService -conditions)

# Disable this test for now. See discussion in issue
# https://gitlab.cern.ch/lhcb/Detector/-/issues/22
#add_test(NAME test_vp_position
#         COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_VPPosition -conditions)

add_test(NAME test_vp_ladder_position
         COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_VPLadders_conditions -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/)


add_test(NAME test_vp_motion
         COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_VP_motion -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/)

add_test(NAME test_vp_sensor_position
         COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_VPSensors_conditions -conditions)


add_test(NAME test_Magnet_load_condition
         COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_Magnet_load_condition -conditions)

add_test(NAME test_Magnet_load_field
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_Magnet_load_field -conditions -fieldmappath)

add_test(NAME test_Magnet_load_demagnet
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_Magnet_load_demagnet -conditions -fieldmappath)

add_test(NAME test_load_deft
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_deft -conditions -fieldmappath)

add_test(NAME test_load_deft_cond
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_deft_cond -conditions -fieldmappath)

add_test(NAME test_deft_geo
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_DeFT_geo -conditions -fieldmappath)

add_test(NAME test_load_deut
        COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_deut -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/ -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)

add_test(NAME test_load_deut_detailed
        COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_deut_detailed -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/ -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)

add_test(NAME test_ut_stave_positions
        COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_ut_stave_positions -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/ -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)

add_test(NAME test_load_devp
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_devp -conditions -fieldmappath)

add_test(NAME test_load_derich1
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_derich1 -conditions -fieldmappath)

add_test(NAME test_load_derich2
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_derich2 -conditions -fieldmappath)

add_test(NAME test_load_demuon
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_demuon -conditions -fieldmappath)

add_test(NAME test_load_delhcb
        COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_delhcb -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/ -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)

add_test(NAME test_load_devp_detailed
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_devp_detailed -conditions -fieldmappath)

add_test(NAME test_load_demuon_detailed
        COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_load_demuon_detailed -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/ -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)

add_test(NAME test_demuon_volumeid
        COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -volmgr -plugin LHCb_TEST_demuon_volumeid -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/ -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)

add_test(NAME test_decalorimeter_volumeid_Ecal
        COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -volmgr -plugin LHCb_TEST_decalorimeter_volumeid -detector Ecal -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/ -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)

add_test(NAME test_decalorimeter_volumeid_Hcal
        COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -volmgr -plugin LHCb_TEST_decalorimeter_volumeid -detector Hcal -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/ -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf)

add_test(NAME AlignmentUseCase
         COMMAND ${PROJECT_BINARY_DIR}/bin/run geoPluginRun -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_AlignmentUseCase -conditions file:${PROJECT_SOURCE_DIR}/tests/ConditionsIOV/)



add_executable(test_loadDetector src/test_loadDetector.cpp)
target_link_libraries(test_loadDetector DetectorLib ROOT::Geom ROOT::Gdml DD4hep::DDCore DD4hep::DDCond ${TBB_lib})
set_target_properties(test_loadDetector PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
add_test(NAME test_loadDetector
         COMMAND test_loadDetector
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(test_RichIntersections src/test_RichIntersections.cpp)
target_link_libraries(test_RichIntersections DetectorLib ROOT::Geom ROOT::Gdml DD4hep::DDCore DD4hep::DDCond ${TBB_lib})
set_target_properties(test_RichIntersections PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
add_test(NAME test_RichIntersections
        COMMAND test_RichIntersections
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(test_Subtraction src/test_Subtraction.cpp)
target_link_libraries(test_Subtraction DetectorLib ROOT::Geom ROOT::Gdml DD4hep::DDCore DD4hep::DDCond ${TBB_lib})
set_target_properties(test_Subtraction PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
add_test(NAME test_Subtraction
        COMMAND test_Subtraction
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(test_checkChecksum src/test_checkChecksum.cpp)
target_link_libraries(test_checkChecksum DetectorLib ROOT::Geom ROOT::Gdml DD4hep::DDCore DD4hep::DDCond ${TBB_lib})
set_target_properties(test_checkChecksum PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
add_test(NAME test_checkChecksum
         COMMAND test_checkChecksum
         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(test_multipleTGeo src/test_multipleTGeo.cpp)
target_link_libraries(test_multipleTGeo DetectorLib ROOT::Geom ROOT::Gdml DD4hep::DDCore DD4hep::DDCond ${TBB_lib})
set_target_properties(test_multipleTGeo PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
add_test(NAME test_multipleTGeo
        COMMAND test_multipleTGeo
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(test_ConditionsOverlay src/test_ConditionsOverlay.cpp)
target_link_libraries(test_ConditionsOverlay
    yaml-cpp
    stdc++fs
    Catch2::Catch2WithMain
    TestUtils
    ${TBB_lib}
)
catch_discover_tests(test_ConditionsOverlay
    TEST_PREFIX Detector.Core.ConditionsOverlay.
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.Core"
)

add_test(NAME test_dump_Rich1
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_dump_Rich1 -conditions)

add_test(NAME test_dump_Rich2
        COMMAND ${TEST_SCRIPT} -input ${PROJECT_SOURCE_DIR}/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_dump_Rich2 -conditions)

add_executable(test_alignment_parsing
    src/test_alignment_parsing.cpp
)
target_link_libraries(test_alignment_parsing
    yaml-cpp
    Catch2::Catch2WithMain
    DetectorLib
    ${TBB_lib}
)
catch_discover_tests(test_alignment_parsing
    TEST_PREFIX Detector.Core.
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.Core"
)

add_executable(test_DDS_with_Overlay
    src/test_DDS_with_Overlay.cpp
)
target_link_libraries(test_DDS_with_Overlay
    yaml-cpp
    Catch2::Catch2WithMain
    DetectorLib
    TestUtils
    ${TBB_lib}
)

# FIXME: this is too specific to the way lcg-toolchains and Gaudi projects work
set_target_properties(test_DDS_with_Overlay PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
# This is to ensure that we have everything we need if we want to build and run only muon_tests
add_dependencies(test_DDS_with_Overlay DetectorPlugins)

catch_discover_tests(test_DDS_with_Overlay
    TEST_PREFIX Detector.Core.
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.Core"
)

add_executable(test_scope
    src/test_scope.cpp
)
target_link_libraries(test_scope
    yaml-cpp
    Catch2::Catch2WithMain
    DetectorLib
    ${TBB_lib}
)
# FIXME: this is too specific to the way lcg-toolchains and Gaudi projects work
set_target_properties(test_scope PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
# This is to ensure that we have everything we need if we want to build and run only muon_tests
add_dependencies(test_scope DetectorPlugins)
catch_discover_tests(test_scope
    TEST_PREFIX Detector.Core.
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    PROPERTIES
        LABELS "Scope"
        LABELS "Detector.Core"
)

add_executable(test_vpvolumeid
    src/test_vpvolumeid.cpp
)
target_link_libraries(test_vpvolumeid
    Catch2::Catch2WithMain
    DetectorLib
    TestUtils
    yaml-cpp
)
# FIXME: this is too specific to the way lcg-toolchains and Gaudi projects work
set_target_properties(test_vpvolumeid PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
# This is to ensure that we have everything we need if we want to build and run only muon_tests
add_dependencies(test_vpvolumeid DetectorPlugins)
catch_discover_tests(test_vpvolumeid
    TEST_PREFIX Detector.VP.
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.VP"
)


## Checking the validity of the Detector elements LHCb
add_executable(test_DetElements_validity
    src/test_DetElements_validity.cpp
)
target_link_libraries(test_DetElements_validity
    Catch2::Catch2WithMain
    DetectorLib
    TestUtils
    yaml-cpp
)
# FIXME: this is too specific to the way lcg-toolchains and Gaudi projects work
set_target_properties(test_DetElements_validity PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)
# This is to ensure that we have everything we need if we want to build and run only muon_tests
add_dependencies(test_DetElements_validity DetectorPlugins)
catch_discover_tests(test_DetElements_validity
    TEST_PREFIX Detector.LHCb
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    PROPERTIES
        LABELS "Detector"
        LABELS "Detector.LHCb"
)
