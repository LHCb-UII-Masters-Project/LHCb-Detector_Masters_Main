# Install script for directory: /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/InstallArea/x86_64_v2-el9-gcc12-opt")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/cvmfs/lhcb.cern.ch/lib/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/DetectorPlugins.components")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorPlugins.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorPlugins.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorPlugins.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE MODULE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/lib/libDetectorPlugins.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorPlugins.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorPlugins.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorPlugins.so"
         OLD_RPATH "/cvmfs/lhcb.cern.ch/lib/lcg/releases/GitCondDB/0.2.2-40070/x86_64-el9-gcc12-opt/lib64:/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/DD4hep/01.27.02-a9b45/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/tbb/2021.10.0-2a247/x86_64-el9-gcc12-opt/lib64:/cvmfs/lhcb.cern.ch/lib/lcg/releases/ROOT/6.30.04-dd2db/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/XercesC/3.2.4-9e637/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/yamlcpp/0.6.3-d05b2/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/fmt/10.0.0-f6609/x86_64-el9-gcc12-opt/lib64:/cvmfs/lhcb.cern.ch/lib/lcg/releases/libgit2/1.1.1-98d83/x86_64-el9-gcc12-opt/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/lhcb.cern.ch/lib/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorPlugins.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/CMakeFiles/DetectorPlugins.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/DetectorTestPlugins.components")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorTestPlugins.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorTestPlugins.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorTestPlugins.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE MODULE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/lib/libDetectorTestPlugins.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorTestPlugins.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorTestPlugins.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorTestPlugins.so"
         OLD_RPATH "/cvmfs/lhcb.cern.ch/lib/lcg/releases/GitCondDB/0.2.2-40070/x86_64-el9-gcc12-opt/lib64:/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/DD4hep/01.27.02-a9b45/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/ROOT/6.30.04-dd2db/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/yamlcpp/0.6.3-d05b2/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/tbb/2021.10.0-2a247/x86_64-el9-gcc12-opt/lib64:/cvmfs/lhcb.cern.ch/lib/lcg/releases/libgit2/1.1.1-98d83/x86_64-el9-gcc12-opt/lib64:/cvmfs/lhcb.cern.ch/lib/lcg/releases/XercesC/3.2.4-9e637/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/fmt/10.0.0-f6609/x86_64-el9-gcc12-opt/lib64:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/lhcb.cern.ch/lib/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorTestPlugins.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/CMakeFiles/DetectorTestPlugins.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/TestUtils/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/Core/tests/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/Detector/Calo/tests/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/Detector/Muon/tests/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/Detector/LHCb/tests/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE DIRECTORY FILES
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Core/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/VP/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/FT/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/Magnet/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/Rich/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/Rich1/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/Rich2/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/Calo/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/Muon/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/UT/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/FT/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/MP/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/LHCb/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/TV/include"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/Detector/UP/include"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE DIRECTORY FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/compact")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorLib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorLib.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorLib.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/lib/libDetectorLib.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorLib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorLib.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorLib.so"
         OLD_RPATH "/cvmfs/lhcb.cern.ch/lib/lcg/releases/DD4hep/01.27.02-a9b45/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/fmt/10.0.0-f6609/x86_64-el9-gcc12-opt/lib64:/cvmfs/lhcb.cern.ch/lib/lcg/releases/ROOT/6.30.04-dd2db/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/tbb/2021.10.0-2a247/x86_64-el9-gcc12-opt/lib64:/cvmfs/lhcb.cern.ch/lib/lcg/releases/XercesC/3.2.4-9e637/x86_64-el9-gcc12-opt/lib:/cvmfs/lhcb.cern.ch/lib/lcg/releases/yamlcpp/0.6.3-d05b2/x86_64-el9-gcc12-opt/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/cvmfs/lhcb.cern.ch/lib/lcg/releases/binutils/2.40-acaab/x86_64-el9/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDetectorLib.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Detector/DetectorTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Detector/DetectorTargets.cmake"
         "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/CMakeFiles/Export/9f607114fd88b24f4d308579a6c9de2a/DetectorTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Detector/DetectorTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Detector/DetectorTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Detector" TYPE FILE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/CMakeFiles/Export/9f607114fd88b24f4d308579a6c9de2a/DetectorTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Detector" TYPE FILE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/CMakeFiles/Export/9f607114fd88b24f4d308579a6c9de2a/DetectorTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/run_test_Detector.sh")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/thisDetector.sh")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/examples/Detector" TYPE DIRECTORY FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/compact" REGEX "/\\.svn$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/examples/Detector" TYPE DIRECTORY FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/scripts" REGEX "/\\.svn$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/examples/Detector" TYPE DIRECTORY FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/compact" REGEX "/\\.svn$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/run")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Detector" TYPE FILE FILES
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/DetectorConfig.cmake"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/DetectorConfigVersion.cmake"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/cmake/DetectorDependencies.cmake"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/cmake/FindRangev3.cmake"
    "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/cmake/DetectorMacros.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Detector" TYPE FILE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/.metadata.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/Detector.xenv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE FILES "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/manifest.xml")
endif()

if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
  file(WRITE "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
