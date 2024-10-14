/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "DD4hep/Detector.h"
#include "DD4hep/Factories.h"
#include "DD4hep/Printout.h"

#include "Core/DetectorDataService.h"
#include "Core/Keys.h"
#include "Core/MagneticFieldExtension.h"
#include "Core/MagneticFieldGridReader.h"
#include "Core/yaml_converters.h"

#include "Detector/UT/DeUT.h"

#include <exception>
#include <string>
#include <vector>

static long test_ut_stave_positions( dd4hep::Detector& description, int argc, char** argv ) {

  // XXX Boiler plate for the plugin, to be improved/factorized
  bool        help = false;
  std::string conditions;
  std::string field_map_path = "";
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) )
        help = true;
      else if ( 0 == ::strncmp( "-conditions", argv[i], 11 ) )
        conditions = argv[++i];
      else if ( 0 == ::strncmp( "-fieldmappath", argv[i], 13 ) ) {
        field_map_path = argv[++i];
      } else
        help = true;
    }
  }
  if ( help || conditions.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_ut_stave_positions          \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "     -fieldmappath <directory>  Directory containing the filed map files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << '\n';
    ::exit( EINVAL );
  }

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, field_map_path );
  std::vector<std::string>            detector_list{"/world", "UT"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  dd4hep::printout( dd4hep::INFO, "UT", "Creating DeUT" );
  // The fake run number 100 should give conditions with null alignment and motion
  auto                            slice = dds.get_slice( 100 );
  dd4hep::DetElement              utdet = description.detector( "UT" );
  const LHCb::Detector::UT::DeUT& ut    = slice->get( utdet, LHCb::Detector::Keys::deKey );
  dd4hep::printout( dd4hep::INFO, "UT", "Creating DeUT...Success" );

  bool allChecksOK{true};

  std::map<std::string, ROOT::Math::XYZPoint> stave_positions{
      {"UTCsideUTaUStave1", ROOT::Math::XYZPoint{-47.0, 4.096, 2365}},
      {"UTCsideUTaUStave2", ROOT::Math::XYZPoint{-143.3, 0.0, 2380}},
      {"UTCsideUTaUStave3", ROOT::Math::XYZPoint{-237.6, 0.0, 2365}},
      {"UTCsideUTaUStave4", ROOT::Math::XYZPoint{-335.2, 0.0, 2380}},
      {"UTCsideUTaUStave5", ROOT::Math::XYZPoint{-428.4, 0.0, 2365}},
      {"UTCsideUTaUStave6", ROOT::Math::XYZPoint{-527.1, 0.0, 2380}},
      {"UTCsideUTaUStave7", ROOT::Math::XYZPoint{-619.2, 0.0, 2365}},
      {"UTCsideUTaUStave8", ROOT::Math::XYZPoint{-719.0, 0.0, 2380}},
      {"UTAsideUTaUStave1", ROOT::Math::XYZPoint{47.0, -4.096, 2380}},
      {"UTAsideUTaUStave2", ROOT::Math::XYZPoint{142.1, 0.0, 2365}},
      {"UTAsideUTaUStave3", ROOT::Math::XYZPoint{239, 0.0, 2380}},
      {"UTAsideUTaUStave4", ROOT::Math::XYZPoint{332.9, 0.0, 2365}},
      {"UTAsideUTaUStave5", ROOT::Math::XYZPoint{431.1, 0.0, 2380}},
      {"UTAsideUTaUStave6", ROOT::Math::XYZPoint{523.7, 0.0, 2365}},
      {"UTAsideUTaUStave7", ROOT::Math::XYZPoint{623.1, 0.0, 2380}},
      {"UTAsideUTaUStave8", ROOT::Math::XYZPoint{714.6, 0.0, 2365}},
      {"UTCsideUTaXStave1", ROOT::Math::XYZPoint{-47.0, 0.0, 2310}},
      {"UTCsideUTaXStave2", ROOT::Math::XYZPoint{-143.3, 0.0, 2325}},
      {"UTCsideUTaXStave3", ROOT::Math::XYZPoint{-237.6, 0.0, 2310}},
      {"UTCsideUTaXStave4", ROOT::Math::XYZPoint{-335.2, 0.0, 2325}},
      {"UTCsideUTaXStave5", ROOT::Math::XYZPoint{-428.4, 0.0, 2310}},
      {"UTCsideUTaXStave6", ROOT::Math::XYZPoint{-527.1, 0.0, 2325}},
      {"UTCsideUTaXStave7", ROOT::Math::XYZPoint{-619.1, 0.0, 2310}},
      {"UTCsideUTaXStave8", ROOT::Math::XYZPoint{-719.0, 0.0, 2325}},
      {"UTAsideUTaXStave1", ROOT::Math::XYZPoint{47.0, 0.0, 2325}},
      {"UTAsideUTaXStave2", ROOT::Math::XYZPoint{142.1, 0.0, 2310}},
      {"UTAsideUTaXStave3", ROOT::Math::XYZPoint{239.1, 0.0, 2325}},
      {"UTAsideUTaXStave4", ROOT::Math::XYZPoint{332.9, 0.0, 2310}},
      {"UTAsideUTaXStave5", ROOT::Math::XYZPoint{431.1, 0.0, 2325}},
      {"UTAsideUTaXStave6", ROOT::Math::XYZPoint{523.8, 0.0, 2310}},
      {"UTAsideUTaXStave7", ROOT::Math::XYZPoint{623.2, 0.0, 2325}},
      {"UTAsideUTaXStave8", ROOT::Math::XYZPoint{714.6, 0.0, 2310}},
      {"UTCsideUTbVStave1", ROOT::Math::XYZPoint{-47, -4.096, 2590}},
      {"UTCsideUTbVStave2", ROOT::Math::XYZPoint{-143.3, 0.0, 2605}},
      {"UTCsideUTbVStave3", ROOT::Math::XYZPoint{-237.9, 0.0, 2590}},
      {"UTCsideUTbVStave4", ROOT::Math::XYZPoint{-335.2, 0.0, 2605}},
      {"UTCsideUTbVStave5", ROOT::Math::XYZPoint{-428.7, 0.0, 2590}},
      {"UTCsideUTbVStave6", ROOT::Math::XYZPoint{-527.2, 0.0, 2605}},
      {"UTCsideUTbVStave7", ROOT::Math::XYZPoint{-619.6, 0.0, 2590}},
      {"UTCsideUTbVStave8", ROOT::Math::XYZPoint{-719.2, 0.0, 2605}},
      {"UTCsideUTbVStave9", ROOT::Math::XYZPoint{-810.5, 0.0, 2590}},
      {"UTAsideUTbVStave1", ROOT::Math::XYZPoint{47, 4.096, 2605}},
      {"UTAsideUTbVStave2", ROOT::Math::XYZPoint{142.3, 0.0, 2590}},
      {"UTAsideUTbVStave3", ROOT::Math::XYZPoint{239, 0.0, 2605}},
      {"UTAsideUTbVStave4", ROOT::Math::XYZPoint{333, 0.0, 2590}},
      {"UTAsideUTbVStave5", ROOT::Math::XYZPoint{431, 0.0, 2605}},
      {"UTAsideUTbVStave6", ROOT::Math::XYZPoint{523.9, 0.0, 2590}},
      {"UTAsideUTbVStave7", ROOT::Math::XYZPoint{622.9, 0.0, 2605}},
      {"UTAsideUTbVStave8", ROOT::Math::XYZPoint{714.8, 0.0, 2590}},
      {"UTAsideUTbVStave9", ROOT::Math::XYZPoint{814.9, 0.0, 2605}},
      {"UTCsideUTbXStave1", ROOT::Math::XYZPoint{-47, 0.0, 2645}},
      {"UTCsideUTbXStave2", ROOT::Math::XYZPoint{-143.3, 0.0, 2660}},
      {"UTCsideUTbXStave3", ROOT::Math::XYZPoint{-237.9, 0.0, 2645}},
      {"UTCsideUTbXStave4", ROOT::Math::XYZPoint{-335.2, 0.0, 2660}},
      {"UTCsideUTbXStave5", ROOT::Math::XYZPoint{-428.8, 0.0, 2645}},
      {"UTCsideUTbXStave6", ROOT::Math::XYZPoint{-527.2, 0.0, 2660}},
      {"UTCsideUTbXStave7", ROOT::Math::XYZPoint{-619.7, 0.0, 2645}},
      {"UTCsideUTbXStave8", ROOT::Math::XYZPoint{-719.2, 0.0, 2660}},
      {"UTCsideUTbXStave9", ROOT::Math::XYZPoint{-810.6, 0.0, 2645}},
      {"UTAsideUTbXStave1", ROOT::Math::XYZPoint{47, 0.0, 2660}},
      {"UTAsideUTbXStave2", ROOT::Math::XYZPoint{142.3, 0.0, 2645}},
      {"UTAsideUTbXStave3", ROOT::Math::XYZPoint{239, 0.0, 2660}},
      {"UTAsideUTbXStave4", ROOT::Math::XYZPoint{333.1, 0.0, 2645}},
      {"UTAsideUTbXStave5", ROOT::Math::XYZPoint{431, 0.0, 2660}},
      {"UTAsideUTbXStave6", ROOT::Math::XYZPoint{524, 0.0, 2645}},
      {"UTAsideUTbXStave7", ROOT::Math::XYZPoint{623, 0.0, 2660}},
      {"UTAsideUTbXStave8", ROOT::Math::XYZPoint{714.9, 0.0, 2645}},
      {"UTAsideUTbXStave9", ROOT::Math::XYZPoint{814.9, 0.0, 2660}}};

  std::map<std::string, std::string> stave_names{
      {"UTCsideUTaXStave1", "UTaXLayerR2Stave2"}, {"UTCsideUTaXStave2", "UTaXLayerR2Stave1"},
      {"UTCsideUTaXStave3", "UTaXLayerR1Stave7"}, {"UTCsideUTaXStave4", "UTaXLayerR1Stave6"},
      {"UTCsideUTaXStave5", "UTaXLayerR1Stave5"}, {"UTCsideUTaXStave6", "UTaXLayerR1Stave4"},
      {"UTCsideUTaXStave7", "UTaXLayerR1Stave3"}, {"UTCsideUTaXStave8", "UTaXLayerR1Stave2"},
      {"UTCsideUTaUStave1", "UTaULayerR2Stave2"}, {"UTCsideUTaUStave2", "UTaULayerR2Stave1"},
      {"UTCsideUTaUStave3", "UTaULayerR1Stave7"}, {"UTCsideUTaUStave4", "UTaULayerR1Stave6"},
      {"UTCsideUTaUStave5", "UTaULayerR1Stave5"}, {"UTCsideUTaUStave6", "UTaULayerR1Stave4"},
      {"UTCsideUTaUStave7", "UTaULayerR1Stave3"}, {"UTCsideUTaUStave8", "UTaULayerR1Stave2"},
      {"UTCsideUTbVStave1", "UTbVLayerR2Stave2"}, {"UTCsideUTbVStave2", "UTbVLayerR2Stave1"},
      {"UTCsideUTbVStave3", "UTbVLayerR1Stave7"}, {"UTCsideUTbVStave4", "UTbVLayerR1Stave6"},
      {"UTCsideUTbVStave5", "UTbVLayerR1Stave5"}, {"UTCsideUTbVStave6", "UTbVLayerR1Stave4"},
      {"UTCsideUTbVStave7", "UTbVLayerR1Stave3"}, {"UTCsideUTbVStave8", "UTbVLayerR1Stave2"},
      {"UTCsideUTbVStave9", "UTbVLayerR1Stave1"}, {"UTCsideUTbXStave1", "UTbXLayerR2Stave2"},
      {"UTCsideUTbXStave2", "UTbXLayerR2Stave1"}, {"UTCsideUTbXStave3", "UTbXLayerR1Stave7"},
      {"UTCsideUTbXStave4", "UTbXLayerR1Stave6"}, {"UTCsideUTbXStave5", "UTbXLayerR1Stave5"},
      {"UTCsideUTbXStave6", "UTbXLayerR1Stave4"}, {"UTCsideUTbXStave7", "UTbXLayerR1Stave3"},
      {"UTCsideUTbXStave8", "UTbXLayerR1Stave2"}, {"UTCsideUTbXStave9", "UTbXLayerR1Stave1"},
      {"UTAsideUTaXStave1", "UTaXLayerR2Stave3"}, {"UTAsideUTaXStave2", "UTaXLayerR2Stave4"},
      {"UTAsideUTaXStave3", "UTaXLayerR3Stave1"}, {"UTAsideUTaXStave4", "UTaXLayerR3Stave2"},
      {"UTAsideUTaXStave5", "UTaXLayerR3Stave3"}, {"UTAsideUTaXStave6", "UTaXLayerR3Stave4"},
      {"UTAsideUTaXStave7", "UTaXLayerR3Stave5"}, {"UTAsideUTaXStave8", "UTaXLayerR3Stave6"},
      {"UTAsideUTaUStave1", "UTaULayerR2Stave3"}, {"UTAsideUTaUStave2", "UTaULayerR2Stave4"},
      {"UTAsideUTaUStave3", "UTaULayerR3Stave1"}, {"UTAsideUTaUStave4", "UTaULayerR3Stave2"},
      {"UTAsideUTaUStave5", "UTaULayerR3Stave3"}, {"UTAsideUTaUStave6", "UTaULayerR3Stave4"},
      {"UTAsideUTaUStave7", "UTaULayerR3Stave5"}, {"UTAsideUTaUStave8", "UTaULayerR3Stave6"},
      {"UTAsideUTbVStave1", "UTbVLayerR2Stave3"}, {"UTAsideUTbVStave2", "UTbVLayerR2Stave4"},
      {"UTAsideUTbVStave3", "UTbVLayerR3Stave1"}, {"UTAsideUTbVStave4", "UTbVLayerR3Stave2"},
      {"UTAsideUTbVStave5", "UTbVLayerR3Stave3"}, {"UTAsideUTbVStave6", "UTbVLayerR3Stave4"},
      {"UTAsideUTbVStave7", "UTbVLayerR3Stave5"}, {"UTAsideUTbVStave8", "UTbVLayerR3Stave6"},
      {"UTAsideUTbVStave9", "UTbVLayerR3Stave7"}, {"UTAsideUTbXStave1", "UTbXLayerR2Stave3"},
      {"UTAsideUTbXStave2", "UTbXLayerR2Stave4"}, {"UTAsideUTbXStave3", "UTbXLayerR3Stave1"},
      {"UTAsideUTbXStave4", "UTbXLayerR3Stave2"}, {"UTAsideUTbXStave5", "UTbXLayerR3Stave3"},
      {"UTAsideUTbXStave6", "UTbXLayerR3Stave4"}, {"UTAsideUTbXStave7", "UTbXLayerR3Stave5"},
      {"UTAsideUTbXStave8", "UTbXLayerR3Stave6"}, {"UTAsideUTbXStave9", "UTbXLayerR3Stave7"}};

  for ( const auto& side : ut.sides() )
    for ( const auto& layer : side.layers() )
      for ( const auto& stave : layer.staves() ) {
        ROOT::Math::XYZPoint stavePos     = stave.toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} );
        ROOT::Math::XYZPoint staveTruePos = stave_positions[stave.nickname()];
        auto                 diff         = staveTruePos - stavePos;
        if ( diff.r() > 1e-3 ) {
          allChecksOK = false;
          dd4hep::printout(
              dd4hep::WARNING, "UT",
              fmt::format( "{stavePath} Pos: {x:>6.1f} {y:>4.1f} {z:>6.1f}, True: {x_true:>6.1f} {y_true:>4.1f} "
                           "{z_true:>6.1f}, Diff: {x_diff:>6.4f} {y_diff:>4.4f} {z_diff:>6.4f}",
                           fmt::arg( "stavePath", stave.nickname() ), fmt::arg( "x", stavePos.x() ),
                           fmt::arg( "y", stavePos.y() ), fmt::arg( "z", stavePos.z() ),
                           fmt::arg( "x_true", staveTruePos.x() ), fmt::arg( "y_true", staveTruePos.y() ),
                           fmt::arg( "z_true", staveTruePos.z() ), fmt::arg( "x_diff", diff.x() ),
                           fmt::arg( "y_diff", diff.y() ), fmt::arg( "z_diff", diff.z() ) ) );
        }
      }

  for ( const auto& side : ut.sides() )
    for ( const auto& layer : side.layers() )
      for ( const auto& stave : layer.staves() ) {
        if ( stave.nicknameOld() != stave_names[stave.nickname()] ) {
          allChecksOK = false;
          dd4hep::printout( dd4hep::WARNING, "UT",
                            fmt::format( "Stave old nickname {stavePathOld}, expected {stavePathOldTrue}",
                                         fmt::arg( "stavePathOld", stave.nicknameOld() ),
                                         fmt::arg( "stavePathOldTrue", stave_names[stave.nickname()] ) ) );
        }
      }

  // Finalizing the service and returning
  dds.finalize();

  if ( allChecksOK ) {
    std::cout << "All checks of DeUT succeeded!" << std::endl;
  } else {
    ::exit( EINVAL );
  }

  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_ut_stave_positions, test_ut_stave_positions )
