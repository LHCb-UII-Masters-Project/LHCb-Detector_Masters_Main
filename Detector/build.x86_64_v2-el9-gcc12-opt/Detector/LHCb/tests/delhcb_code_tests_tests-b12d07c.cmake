add_test( Detector.LHCb.Tell40Links /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/toolchain/wrapper /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/delhcb_code_tests Tell40Links  )
set_tests_properties( Detector.LHCb.Tell40Links PROPERTIES WORKING_DIRECTORY /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/Detector/LHCb/tests LABELS Detector LABELS Detector.LHCb)
set( delhcb_code_tests_TESTS Detector.LHCb.Tell40Links)
