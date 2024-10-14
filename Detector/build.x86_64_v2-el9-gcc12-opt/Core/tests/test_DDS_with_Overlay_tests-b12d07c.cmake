add_test( [==[Detector.Core.DDS with overlay]==] /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/run /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/test_DDS_with_Overlay [==[DDS with overlay]==]  )
set_tests_properties( [==[Detector.Core.DDS with overlay]==] PROPERTIES WORKING_DIRECTORY /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector LABELS Detector LABELS Detector.Core)
set( test_DDS_with_Overlay_TESTS [==[Detector.Core.DDS with overlay]==])
