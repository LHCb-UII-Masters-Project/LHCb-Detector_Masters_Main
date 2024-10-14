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
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(Detector VERSION 2.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

#---------------------------------------------------------------------------------
# General configuration
foreach(_language CXX C)
  set(CMAKE_${_language}_FLAGS "${CMAKE_${_language}_FLAGS} -fdiagnostics-color")
endforeach()

# Set the CMP0074 policy that dictates how the packages are looked up
if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# Added to make sure the LHCb DD4hep HEAD include dirs are before the LCG view
# one. THis is just neeed for test setup, not mandatory once we hve a clean
# production setup
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

#---------------------------------------------------------------------------------
# Depency locations has been externalized
include(cmake/DetectorDependencies.cmake)
# Needed macros and functions
include(cmake/DetectorMacros.cmake)

set(CMAKE_INSTALL_LIBDIR "lib" CACHE STRING "Install libraries in <prefix>/\${CMAKE_INSTALL_LIBDIR}")
#-----------------------------------------------------------------------------------

# Prepare the Detector library
include_directories(Core/include
                    Detector/VP/include
                    Detector/FT/include
                    Detector/MP/include
                    Detector/Rich1/include
                    Detector/Rich2/include
                    Detector/Magnet/include
		    Detector/Muon/include)

dd4hep_configure_output (INSTALL ${CMAKE_INSTALL_PREFIX})

add_library(DetectorLib SHARED
    Core/src/ConditionIdentifier.cpp
    Core/src/ConditionsLoader.cpp
    Core/src/ConditionsReader.cpp
    Core/src/ConditionsRepository.cpp
    Core/src/DeAlignmentCall.cpp
    Core/src/DeConditionCall.cpp
    Core/src/DeHandles.cpp
    Core/src/DeIOV.cpp
    Core/src/DetectorDataService.cpp
    Core/src/Keys.cpp
    Core/src/ParameterMap.cpp
    Core/src/UpgradeTags.cpp
    Core/src/Align_cond.cpp
    Core/src/GdmlImport.cpp
    Core/src/PrintHelpers.cpp
    Core/src/yaml_converters.cpp
    Core/src/GeometryTools.cpp
    Core/src/LHCbCompactTags.cpp
    Core/src/ShapeUtilities.cpp
    Core/src/Utils.cpp
    Detector/Calo/src/DeCaloHandles.cpp
    Detector/Calo/src/DeCalorimeter.cpp
    Detector/FT/src/DeFT.cpp
    Detector/FT/src/DeFTConditionCalls.cpp
    Detector/FT/src/DeFTHandles.cpp
    Detector/FT/src/DeFTMat.cpp
    Detector/MP/src/DeMP.cpp
    Detector/MP/src/DeMPConditionCalls.cpp
    Detector/MP/src/DeMPHandles.cpp
    Detector/VP/src/DeVP.cpp
    Detector/VP/src/DeVPConditionCalls.cpp
    Detector/VP/src/DeVPHandles.cpp
    Detector/VP/src/DeVPSensor.cpp
    Detector/VP/src/VPChannelID.cpp
    Detector/VP/src/VPVolumeID.cpp
    Detector/TV/src/DeTV.cpp
    Detector/TV/src/DeTVConditionCalls.cpp
    Detector/TV/src/DeTVHandles.cpp
    Detector/TV/src/DeTVSensor.cpp
    Detector/TV/src/TVChannelID.cpp
    Detector/TV/src/TVVolumeID.cpp
    Detector/UT/src/ChannelID.cpp
    Detector/UT/src/DeUTHandles.cpp
    Detector/UT/src/DeUT.cpp
    Detector/UT/src/DeUTConditionCalls.cpp
    Detector/UP/src/UPChannelID.cpp
    Detector/UP/src/DeUPHandles.cpp
    Detector/UP/src/DeUP.cpp
    Detector/UP/src/DeUPFace.cpp
    Detector/UP/src/DeUPLayer.cpp
    Detector/UP/src/DeUPModule.cpp
    Detector/UP/src/DeUPSector.cpp
    Detector/UP/src/DeUPSensor.cpp
    Detector/UP/src/DeUPSide.cpp
    Detector/UP/src/DeUPStave.cpp
    Detector/UP/src/DeUPConditionCalls.cpp
    Detector/Magnet/src/DeMagnet.cpp
    Detector/Magnet/src/DeMagnetHandles.cpp
    Detector/Muon/src/MuonChamberGrid.cpp
    Detector/Muon/src/muon_geo.cpp
    Core/src/MagneticFieldGrid.cpp
    Core/src/MagneticFieldGridReader.cpp
    Core/src/MagneticFieldExtension.cpp
    Detector/LHCb/src/DeLHCb.cpp
    Detector/LHCb/src/DeLHCbHandles.cpp
    Detector/LHCb/src/InteractionRegion.cpp
    Detector/LHCb/src/SMOGInfo.cpp
    Detector/LHCb/src/Tell40Links.cpp
    Detector/LHCb/src/LHCInfo.cpp
    Detector/Rich/src/DeRich.cpp
    Detector/Rich/src/DeRichRadiator.cpp
    Detector/Rich/src/DeRichMapmt.cpp
    Detector/Rich1/src/RichGeomVersion.cpp
    Detector/Rich1/src/RichDescUtilSrc/RichMatPropData.cpp
    Detector/Rich1/src/RichDescUtilSrc/RichMatOPD.cpp
    Detector/Rich1/src/RichDescUtilSrc/RichGeoUtil.cpp
    Detector/Rich1/src/RichDescUtilSrc/RichPmtGeoAux.cpp
    Detector/Rich1/src/RichDescUtilSrc/RichSurfaceUtil.cpp
    Detector/Rich2/src/Rich2DescUtilSrc/Rich2GeoUtil.cpp
    Detector/Rich1/src/RichGeoTransAux.cpp
    Detector/Rich2/src/Rich2DescUtilSrc/Rich2MirrorGeoAux.cpp
    Detector/Rich1/src/RichDescUtilSrc/RichDD4HepAccessUtil.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1BeamPipe.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1RadiatorGas.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1Mirror1.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1Mirror2.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1PhDetPanel.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1PDModule.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1Mapmt.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1.cpp
    Detector/Rich1/src/DeRich1ConditionCalls.cpp
    Detector/Rich1/src/Rich1DEA/DeRich1Handles.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2RadiatorGas.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2SphMirror.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2SecMirror.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2Mapmt.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2PDModule.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2PhDetPanel.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2BeamPipe.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2.cpp
    Detector/Rich2/src/DeRich2ConditionCalls.cpp
    Detector/Rich2/src/Rich2DEA/DeRich2Handles.cpp
    Detector/Muon/src/Muon_cond.cpp
    Detector/Muon/src/TileID.cpp
    Detector/Muon/src/Layout.cpp
    Detector/Muon/src/DeMuon.cpp
    Detector/Muon/src/DeMuonChamber.cpp
    Detector/Muon/src/DeMuonGasGap.cpp
    Detector/Muon/src/DeMuonHandles.cpp
    Detector/Muon/src/TSMap.cpp
    Detector/Muon/src/DAQHelper.cpp
    Detector/Muon/src/NODEBoard.cpp
    Detector/Muon/src/ReadoutCond.cpp
    Detector/Muon/src/ReadoutCondJson.cpp
    Detector/Muon/src/Tell40PCIBoard.cpp
)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16)
  # Needed to work around a floating point exception in clang 16 when SLP optimizaiton is used
  # See clang bug report : https://github.com/llvm/llvm-project/issues/77492
  set_source_files_properties(Core/src/MagneticFieldGridReader.cpp PROPERTIES COMPILE_FLAGS -fno-tree-slp-vectorize)
  set_source_files_properties(Detector/FT/src/DeFT.cpp PROPERTIES COMPILE_FLAGS -fno-tree-slp-vectorize)
endif()

target_include_directories(DetectorLib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/Calo/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/Muon/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/FT/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/Magnet/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/Rich/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/Rich1/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/Rich2/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/VP/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/TV/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/MP/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/UT/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/UP/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Detector/LHCb/include/>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(DetectorLib
  DD4hep::DDCond
  DD4hep::DDCore
  fmt::fmt
  Rangev3::rangev3
  ROOT::Core
  ROOT::Gdml
  ROOT::Geom
  ROOT::Graf
  Vc::Vc
  yaml-cpp::yaml-cpp
  nlohmann_json::nlohmann_json
  ${TBB_lib}
)
add_library(Detector::DetectorLib ALIAS DetectorLib)

add_executable(checksumGeo Core/src/checksumGeo.cpp)
target_link_libraries(checksumGeo DetectorLib ROOT::Geom ROOT::Gdml DD4hep::DDCore DD4hep::DDCond ${TBB_lib} Boost::program_options )
set_target_properties(checksumGeo PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)

add_executable(nodeInfo Core/src/nodeInfo.cpp)
target_link_libraries(nodeInfo DetectorLib ROOT::Geom ROOT::Gdml DD4hep::DDCore DD4hep::DDCond ${TBB_lib} Boost::program_options )
set_target_properties(nodeInfo PROPERTIES CROSSCOMPILING_EMULATOR ${PROJECT_BINARY_DIR}/bin/run)

#-----------------------------------------------------------------------------------
# Prepare the Detector Plugins
add_detector_plugin(DetectorPlugins
    Core/src/ConditionsFileReader.cpp
    Core/src/ConditionsGitReader.cpp
    Core/src/GeometryChecker.cpp
    Detector/BeamPipe/src/BeamPipe_geo.cpp
    Detector/BeamPipe/src/Pipe_Section1_volume.cpp
    Detector/BeamPipe/src/Pipe_VMA_geo.cpp
    Detector/FT/src/FT_cond.cpp
    Detector/FT/src/FT_geo.cpp
    Detector/MP/src/MP_cond.cpp
    Detector/MP/src/MP_geo.cpp
    Detector/Magnet/src/Magnet_cond.cpp
    Detector/LHCb/src/LHCb_cond.cpp
    Detector/UT/src/UT_cond.cpp
    Detector/UT/src/UT_geo.cpp
    Detector/UP/src/UP_cond.cpp
    Detector/UP/src/UP_geo.cpp
    Detector/VP/src/VP_cond.cpp
    Detector/VP/src/VP_geo.cpp
    Detector/VP/src/VP_geo_dump.cpp
    Detector/TV/src/TV_cond.cpp
    Detector/TV/src/TV_geo.cpp
    Detector/TV/src/TV_geo_dump.cpp
    Detector/Rich1/src/src_geo/Rich1_geo_RUN3.cpp
    Detector/Rich2/src/src_geo/Rich2_geo_RUN3.cpp
    Detector/Rich1/src/Rich1DEA/Rich1_dump_detelem.cpp
    Detector/Rich1/src/Rich1_cond.cpp
    Detector/Rich2/src/Rich2DEA/Rich2_dump_detelem.cpp
    Detector/Rich2/src/Rich2_cond.cpp
    Detector/Muon/src/Muon_cond.cpp
    Detector/ECAL/src/ECAL_geo.cpp
    Detector/ECAL/src/ECAL_cond.cpp
    Detector/HCAL/src/HCAL_geo.cpp
    Detector/HCAL/src/HCAL_cond.cpp
    Detector/Infrastructure/src/BCM_geo.cpp
    Detector/Infrastructure/src/BLS_geo.cpp
    Detector/Infrastructure/src/DetectorRegion.cpp
    Detector/Infrastructure/src/MBXW_geo.cpp
)
set(FS_LIBS stdc++fs)
target_link_libraries(DetectorPlugins GitCondDB::GitCondDB ${FS_LIB} DetectorLib)


# Prepare the Detector tests Plugins
add_detector_plugin(DetectorTestPlugins
    Core/tests/src/test_DetectorDataService.cpp
    Core/tests/src/test_magnet.cpp
    Core/tests/src/test_delhcb.cpp
    Core/tests/src/test_detelements.cpp
    Core/tests/src/test_demuon.cpp
    Core/tests/src/test_devp.cpp
    Core/tests/src/test_deut_detailed.cpp
    Core/tests/src/test_deut_stave_positions.cpp
    Core/tests/src/test_deft.cpp
    Core/tests/src/test_deft_geo.cpp
    Core/tests/src/test_demuon.cpp
    Core/tests/src/test_CaloVolumeID.cpp
    Core/tests/src/AlignmentUseCase.cpp
    Core/tests/src/DumpConditions.cpp
    Detector/TestScope/scope_geo.cpp
    Detector/TestScope/scope_cond.cpp
    Core/tests/src/test_load_scope.cpp
)
target_link_libraries(DetectorTestPlugins GitCondDB::GitCondDB TestUtils ${FS_LIB} DetectorLib)


#--------------------------------------------------------------------------
# Testing
include(CTest)
if(BUILD_TESTING)
  # Locate testing frameworks
  find_package(Boost CONFIG REQUIRED unit_test_framework)
  find_package(Catch2 REQUIRED)
  include(Catch)
  # Helpers for writing tests
  add_subdirectory(TestUtils)
  # Adding the test subdirectory
  add_subdirectory(Core/tests)
  add_subdirectory(Detector/Calo/tests)
  add_subdirectory(Detector/Muon/tests)
  add_subdirectory(Detector/LHCb/tests)
  #add_test (NAME MyTest COMMAND Test)
endif()

#--------------------------------------------------------------------------
# Install the appropriate files
install(DIRECTORY Core/include Detector/VP/include Detector/FT/include Detector/Magnet/include
        Detector/Rich/include Detector/Rich1/include Detector/Rich2/include
        Detector/Calo/include Detector/Muon/include
	Detector/UT/include Detector/FT/include Detector/MP/include Detector/LHCb/include Detector/TV/include Detector/UP/include
        DESTINATION .)
install(DIRECTORY compact DESTINATION .)

install(TARGETS DetectorLib EXPORT DetectorTargets)

export(EXPORT DetectorTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/DetectorTargets.cmake"
       NAMESPACE Detector::)
install(EXPORT DetectorTargets FILE DetectorTargets.cmake NAMESPACE Detector::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)
    # FIXME:
    # Hide "possibly dangling reference to a temporary" warnings coming from
    # the fact that gcc 13 seems not to be able to detect that
    # `some_cond.get<json>()` returns a reference to the object stored inside
    # an `OpaqueDataBlock` (it probably gets confused by the DD4hep
    # reimplementation of `dynamic_cast` invoked in `OpaqueData::as()`)
    set_source_files_properties(
        Core/src/MagneticFieldExtension.cpp
        Core/tests/src/AlignmentUseCase.cpp
        Core/tests/src/test_deft.cpp
        Core/tests/src/test_DetectorDataService.cpp
        Core/tests/src/test_deut_detailed.cpp
        Core/tests/src/test_magnet.cpp
        Detector/Calo/src/DeCalorimeter.cpp
        Detector/Muon/src/DAQHelper.cpp
        Detector/Rich1/src/Rich1DEA/Rich1_dump_detelem.cpp
        Detector/Rich2/src/Rich2DEA/Rich2_dump_detelem.cpp
        PROPERTIES
            COMPILE_OPTIONS -Wno-dangling-reference
    )
endif()
#--------------------------------------------------------------------------
# Generating the environment the DD4hep way for compatibility

dd4hep_configure_scripts ( Detector DEFAULT_SETUP WITH_TESTS )

# Generating our setenv file for development
if (EXISTS ${CMAKE_BINARY_DIR}/toolchain/wrapper)
  file(READ ${CMAKE_BINARY_DIR}/toolchain/wrapper _run_data)
  string(REGEX REPLACE "exec " "
export PATH=${PROJECT_BINARY_DIR}/bin:${CMAKE_BINARY_DIR}/toolchain:${DD4hep_DIR}/bin:\$PATH
export LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}/lib:${DD4hep_DIR}/lib:\$LD_LIBRARY_PATH
export PYTHONPATH=${DD4hep_DIR}/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages:\$PYTHONPATH

export ROOT_INCLUDE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/Detector/UT/include:${CMAKE_CURRENT_SOURCE_DIR}/Detector/VP/include:${CMAKE_CURRENT_SOURCE_DIR}/Detector/FT/include:${CMAKE_CURRENT_SOURCE_DIR}/Detector/MP/include:${CMAKE_CURRENT_SOURCE_DIR}/Detector/TV/include:${CMAKE_CURRENT_SOURCE_DIR}/Detector/MT/include:${CMAKE_CURRENT_SOURCE_DIR}/Detector/UP/include:${CMAKE_CURRENT_SOURCE_DIR}/Core/include:${GitCondDB_DIR}/../../../include:${DD4hep_DIR}/include:\$ROOT_INCLUDE_PATH

exec " _run_data "${_run_data}")
  file(WRITE ${PROJECT_BINARY_DIR}/bin/run "${_run_data}")

  # Using the same trick as in GaudiProjectConfig.cmake, need a more portable solution
  execute_process(COMMAND chmod a+x ${PROJECT_BINARY_DIR}/bin/run)
  install(FILES ${PROJECT_BINARY_DIR}/bin/run DESTINATION bin
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # we are in a subproject, no need to install, just make sure that LHCb picks up
  # the right runtime environment
  set(Detector_PLUGINS_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib CACHE INTERNAL "")
  return()
endif()
#--------------------------------------------------------------------------
# Now generating the DetectorConfig.cmake that will be used by LHCb
# to compile against the released version

# Needed for configure_package_config_file
include(CMakePackageConfigHelpers)

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/DetectorConfig.cmake.in
    ${CMAKE_BINARY_DIR}/DetectorConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  PATH_VARS CMAKE_INSTALL_LIBDIR
)

write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/DetectorConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
    ${CMAKE_BINARY_DIR}/DetectorConfig.cmake
    ${CMAKE_BINARY_DIR}/DetectorConfigVersion.cmake
    ${PROJECT_SOURCE_DIR}/cmake/DetectorDependencies.cmake
    ${PROJECT_SOURCE_DIR}/cmake/FindRangev3.cmake
    ${PROJECT_SOURCE_DIR}/cmake/DetectorMacros.cmake
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# make sure the right DD4hep shows in the .xenv file
set(ENV{PATH} "${DD4hep_DIR}/bin:$ENV{PATH}")
set(ENV{PYTHONPATH} "${DD4hep_DIR}/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages:$ENV{PYTHONPATH}")
set(ENV{ROOT_INCLUDE_PATH} "${GitCondDB_DIR}/../../../include:${DD4hep_DIR}/include:$ENV{ROOT_INCLUDE_PATH}")

# Optionally enable compatibility with old-style CMake configurations, via helper module
option(GAUDI_LEGACY_CMAKE_SUPPORT "Enable compatibility with old-style CMake builds" "$ENV{GAUDI_LEGACY_CMAKE_SUPPORT}")
if(GAUDI_LEGACY_CMAKE_SUPPORT)
  find_file(legacy_cmake_config_support NAMES LegacyGaudiCMakeSupport.cmake)
  if(legacy_cmake_config_support)
    include(${legacy_cmake_config_support})
  else()
    message(FATAL_ERROR "GAUDI_LEGACY_CMAKE_SUPPORT set to TRUE, but cannot find LegacyGaudiCMakeSupport.cmake")
  endif()
endif()
