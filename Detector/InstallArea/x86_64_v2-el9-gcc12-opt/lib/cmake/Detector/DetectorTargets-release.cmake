#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Detector::DetectorLib" for configuration "Release"
set_property(TARGET Detector::DetectorLib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Detector::DetectorLib PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libDetectorLib.so"
  IMPORTED_SONAME_RELEASE "libDetectorLib.so"
  )

list(APPEND _cmake_import_check_targets Detector::DetectorLib )
list(APPEND _cmake_import_check_files_for_Detector::DetectorLib "${_IMPORT_PREFIX}/lib/libDetectorLib.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
