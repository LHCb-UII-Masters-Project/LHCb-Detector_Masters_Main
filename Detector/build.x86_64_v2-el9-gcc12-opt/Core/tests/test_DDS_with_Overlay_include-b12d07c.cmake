if(EXISTS "/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/Core/tests/test_DDS_with_Overlay_tests-b12d07c.cmake")
  include("/disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/Core/tests/test_DDS_with_Overlay_tests-b12d07c.cmake")
else()
  add_test(test_DDS_with_Overlay_NOT_BUILT-b12d07c test_DDS_with_Overlay_NOT_BUILT-b12d07c)
endif()