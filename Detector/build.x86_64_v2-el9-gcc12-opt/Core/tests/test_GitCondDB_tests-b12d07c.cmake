add_test( Detector.Core.GitCondDB /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/toolchain/wrapper /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/test_GitCondDB GitCondDB  )
set_tests_properties( Detector.Core.GitCondDB PROPERTIES WORKING_DIRECTORY /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector LABELS Detector LABELS Detector.Core)
set( test_GitCondDB_TESTS Detector.Core.GitCondDB)
