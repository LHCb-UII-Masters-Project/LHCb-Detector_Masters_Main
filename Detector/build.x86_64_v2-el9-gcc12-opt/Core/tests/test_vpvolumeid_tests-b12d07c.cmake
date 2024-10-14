add_test( Detector.VP.VPVolumeID /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/run /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/test_vpvolumeid VPVolumeID  )
set_tests_properties( Detector.VP.VPVolumeID PROPERTIES WORKING_DIRECTORY /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector LABELS Detector LABELS Detector.VP)
set( test_vpvolumeid_TESTS Detector.VP.VPVolumeID)
