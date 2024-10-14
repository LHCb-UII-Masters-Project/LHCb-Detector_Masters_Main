add_test( Detector.Core.scope_test /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/run /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/test_scope scope_test  )
set_tests_properties( Detector.Core.scope_test PROPERTIES WORKING_DIRECTORY /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector LABELS Scope LABELS Detector.Core)
set( test_scope_TESTS Detector.Core.scope_test)
