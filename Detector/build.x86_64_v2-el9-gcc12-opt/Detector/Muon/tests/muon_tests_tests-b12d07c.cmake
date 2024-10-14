add_test( Detector.Muon.DeMuon /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/run /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector/build.x86_64_v2-el9-gcc12-opt/bin/muon_tests DeMuon  )
set_tests_properties( Detector.Muon.DeMuon PROPERTIES WORKING_DIRECTORY /disk/moose/general/djdt/lhcbUII_masters/DetectorPlayground/Detector LABELS Detector LABELS Detector.Muon)
set( muon_tests_TESTS Detector.Muon.DeMuon)
