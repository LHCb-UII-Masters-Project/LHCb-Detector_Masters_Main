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

static long test_load_deut( dd4hep::Detector& description, int argc, char** argv ) {

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
                 "     name:   factory name     LHCb_TEST_load_deut_detailed          \n"
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
  // [QUESTION] why including Muon?
  std::vector<std::string> detector_list{"/world", "Magnet", "Muon", "UT"};
  // std::vector<std::string>            detector_list{"UT"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  dd4hep::printout( dd4hep::INFO, "UT", "Creating DeUT" );
  // The fake run number 100 should give conditions with null alignment and motion
  auto                           slice = dds.get_slice( 100 );
  dd4hep::DetElement             utdet = description.detector( "UT" );
  const LHCb::Detector::UT::DeUT ut    = slice->get( utdet, LHCb::Detector::Keys::deKey );

  dd4hep::printout( dd4hep::INFO, "UT", "Created DeUT!" );

  bool allChecksOK{true};

  const ROOT::Math::XYZPoint UTGlobalZero{0.0, 0.0, 2485.0};

  // Check the global position of the UT
  auto diff = ut.toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ) - UTGlobalZero;
  if ( diff.r() > 1e-5 ) {
    std::cerr << "UT {0, 0, 2485.0} is " << ut.toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ) << ", expected "
              << UTGlobalZero << std::endl;
    allChecksOK = false;
  }
  dd4hep::printout( dd4hep::INFO, "UT", "Checked global position" );

  // Check the number of sectors
  unsigned int nSectors = 0;
  ut.applyToAllSectors( [&nSectors]( LHCb::Detector::UT::DeUTSector ) { ++nSectors; } );
  if ( nSectors != 1048 ) {
    dd4hep::printout( dd4hep::DEBUG, "UT", "Number of sectors: %d", static_cast<unsigned int>( nSectors ) );
    allChecksOK = false;
  }
  dd4hep::printout( dd4hep::INFO, "UT", "Checked total number of sectors" );

  // Check global position of the C-side
  const LHCb::Detector::UT::DeUTSide CSide = ut.sides()[0];
  if ( CSide.layers().size() != 4 ) {
    dd4hep::printout( dd4hep::DEBUG, "UT", "Number of layers on C-side: %d",
                      static_cast<unsigned int>( CSide.layers().size() ) );
    allChecksOK = false;
  }
  const ROOT::Math::XYZPoint CSideZero{dd4hep::_toDouble( "-1/2*UTLayerSideCorrectedWidth" ), 0.0,
                                       dd4hep::_toDouble( "-1/2*UTStationDz-1/2*UTLayerDz" )};
  diff = CSide.toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ) - ut.toGlobal( CSideZero );
  if ( diff.r() > 1e-5 ) {
    std::cerr << "C-side is " << CSide.toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ) << ", expected "
              << ut.toGlobal( CSideZero ) << std::endl;
    allChecksOK = false;
  }
  dd4hep::printout( dd4hep::INFO, "UT", "Checked global position of C-side" );

  // Check global position of the A-side
  const LHCb::Detector::UT::DeUTSide ASide = ut.sides()[1];
  if ( ASide.layers().size() != 4 ) {
    std::cerr << "Number of modules on left side = " << ASide.layers().size() << std::endl;
    allChecksOK = false;
  }
  const ROOT::Math::XYZPoint ASideZero{dd4hep::_toDouble( "1/2*UTLayerSideCorrectedWidth" ), 0.0,
                                       dd4hep::_toDouble( "-1/2*UTStationDz-1/2*UTLayerDz" )};
  diff = ASide.toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ) - ut.toGlobal( ASideZero );
  if ( diff.r() > 1e-5 ) {
    std::cerr << "A-side is " << ASide.toGlobal( ROOT::Math::XYZPoint{0.0, 0.0, 0.0} ) << ", expected "
              << ut.toGlobal( ASideZero ) << std::endl;
    allChecksOK = false;
  }
  dd4hep::printout( dd4hep::INFO, "UT", "Checked global position of A-side" );

  /////////////////////////////////
  /* check ChannelID vs VolumeID */
  /////////////////////////////////
  for ( const auto& side : ut.sides() )
    for ( const auto& layer : side.layers() )
      for ( const auto& stave : layer.staves() )
        for ( const auto& face : stave.faces() )
          for ( const auto& module : face.modules() )
            for ( const auto& sector : module.sectors() ) {
              auto chanID = sector.channelID();
              if ( ( static_cast<unsigned int>( side.detector().placement().volIDs()[0].second ) != chanID.side() ) ||
                   ( static_cast<unsigned int>( layer->detector().placement().volIDs()[0].second ) !=
                     chanID.layer() ) ||
                   ( static_cast<unsigned int>( stave->detector().placement().volIDs()[0].second ) !=
                     chanID.stave() ) ||
                   ( static_cast<unsigned int>( face->detector().placement().volIDs()[0].second ) != chanID.face() ) ||
                   ( static_cast<unsigned int>( module->detector().placement().volIDs()[0].second ) !=
                     chanID.module() ) ||
                   ( static_cast<unsigned int>( sector->detector().placement().volIDs()[0].second ) !=
                     chanID.sector() ) ) {
                std::cerr << "Mismatched UTChannelID: " << chanID << std::endl;
                allChecksOK = false;
              }
            }
  dd4hep::printout( dd4hep::INFO, "UT", "Checked ChannelID vs VolumeID for all sectors" );

  // testing accessibility of readout map from Condition DB
  std::vector<std::string> layer_names{"UTaU", "UTaX", "UTbX", "UTbV"};
  try {
    const auto& readoutMapCond = slice->get( utdet, LHCb::Detector::item_key( "ReadoutMap" ) ).get<nlohmann::json>();
    for ( int i = 0; i < 216; i++ ) {
      const auto& values = readoutMapCond["UTTell" + std::to_string( i ) + "Map"]; // implement for all layers
      for ( std::size_t i = 0; i < values.size(); i++ ) {
        dd4hep::printout( dd4hep::DEBUG, "UTCondition", "values[%d]: %.0f", i, values[i].get<double>() );
      }
    }
  } catch ( const char* msg ) { allChecksOK = false; }
  dd4hep::printout( dd4hep::INFO, "UTCondition", "Successfully accessed ReadoutMap condition" );

  ////////////////////////
  /* check getLayerGeom */
  ////////////////////////
  std::array<float, 4> z_values{{2328.15, 2383.15, 2593.15, 2648.15}};
  std::array<float, 4> invHalfSectorYSize_values{{0.0209205, 0.0210004, 0.0210004, 0.0209205}};
  std::array<float, 4> invHalfSectorXSize_values{{0.0209263, 0.0209166, 0.0209166, 0.0209263}};
  std::array<float, 4> dxDy_values{{0, 0.0874886, -0.0874886, 0}};
  for ( unsigned int i = 0; i < 4; i++ ) {
    auto layerGeom = ut.getLayerGeom( i );
    if ( layerGeom.z - z_values[i] > 0.01 ) {
      std::cerr << "layer " << layer_names[i] << " has z=" << layerGeom.z << ", expected z=" << z_values[i]
                << ", diff=" << layerGeom.z - z_values[i] << std::endl;
      allChecksOK = false;
    }
    if ( layerGeom.invHalfSectorYSize - invHalfSectorYSize_values[i] > 1e-5 ) {
      std::cerr << "layer " << layer_names[i] << " has invHalfSectorYSize=" << layerGeom.invHalfSectorYSize
                << ", expected invHalfSectorYSize=" << invHalfSectorYSize_values[i]
                << ", diff=" << layerGeom.invHalfSectorYSize - invHalfSectorYSize_values[i] << std::endl;
      allChecksOK = false;
    }
    if ( layerGeom.invHalfSectorXSize - invHalfSectorXSize_values[i] > 1e-5 ) {
      std::cerr << "layer " << layer_names[i] << " has invHalfSectorXSize=" << layerGeom.invHalfSectorXSize
                << ", expected invHalfSectorXSize=" << invHalfSectorXSize_values[i]
                << ", diff=" << layerGeom.invHalfSectorXSize - invHalfSectorXSize_values[i] << std::endl;
      allChecksOK = false;
    }
    if ( layerGeom.dxDy - dxDy_values[i] > 1e-5 ) {
      std::cerr << "layer " << layer_names[i] << "has dxDy=" << layerGeom.dxDy << ", expected dxDy=" << dxDy_values[i]
                << ", diff=" << layerGeom.dxDy - dxDy_values[i] << std::endl;
      allChecksOK = false;
    }
  }

  ////////////////////////////////////////////////
  /* sector and stave names based on GeoVersion */
  ////////////////////////////////////////////////
  std::vector<std::string> sector_names;
  std::vector<std::string> stave_names;
  if ( ut.version() == LHCb::Detector::UT::DeUT::GeoVersion::v1 ) {
    dd4hep::printout( dd4hep::INFO, "UT", "Using GeoVersion::v1" );
    stave_names = {
        "UTAsideUTaUStave0", "UTAsideUTaUStave1", "UTAsideUTaUStave2", "UTAsideUTaUStave3", "UTAsideUTaUStave4",
        "UTAsideUTaUStave5", "UTAsideUTaUStave6", "UTAsideUTaUStave7", "UTAsideUTaXStave0", "UTAsideUTaXStave1",
        "UTAsideUTaXStave2", "UTAsideUTaXStave3", "UTAsideUTaXStave4", "UTAsideUTaXStave5", "UTAsideUTaXStave6",
        "UTAsideUTaXStave7", "UTAsideUTbVStave0", "UTAsideUTbVStave1", "UTAsideUTbVStave2", "UTAsideUTbVStave3",
        "UTAsideUTbVStave4", "UTAsideUTbVStave5", "UTAsideUTbVStave6", "UTAsideUTbVStave7", "UTAsideUTbVStave8",
        "UTAsideUTbXStave0", "UTAsideUTbXStave1", "UTAsideUTbXStave2", "UTAsideUTbXStave3", "UTAsideUTbXStave4",
        "UTAsideUTbXStave5", "UTAsideUTbXStave6", "UTAsideUTbXStave7", "UTAsideUTbXStave8", "UTCsideUTaUStave0",
        "UTCsideUTaUStave1", "UTCsideUTaUStave2", "UTCsideUTaUStave3", "UTCsideUTaUStave4", "UTCsideUTaUStave5",
        "UTCsideUTaUStave6", "UTCsideUTaUStave7", "UTCsideUTaXStave0", "UTCsideUTaXStave1", "UTCsideUTaXStave2",
        "UTCsideUTaXStave3", "UTCsideUTaXStave4", "UTCsideUTaXStave5", "UTCsideUTaXStave6", "UTCsideUTaXStave7",
        "UTCsideUTbVStave0", "UTCsideUTbVStave1", "UTCsideUTbVStave2", "UTCsideUTbVStave3", "UTCsideUTbVStave4",
        "UTCsideUTbVStave5", "UTCsideUTbVStave6", "UTCsideUTbVStave7", "UTCsideUTbVStave8", "UTCsideUTbXStave0",
        "UTCsideUTbXStave1", "UTCsideUTbXStave2", "UTCsideUTbXStave3", "UTCsideUTbXStave4", "UTCsideUTbXStave5",
        "UTCsideUTbXStave6", "UTCsideUTbXStave7", "UTCsideUTbXStave8",
    };
    sector_names = {"UTAsideUTaUStave0Face0Module0Sector0", "UTAsideUTaUStave0Face0Module1Sector0",
                    "UTAsideUTaUStave0Face0Module2Sector0", "UTAsideUTaUStave0Face0Module2Sector1",
                    "UTAsideUTaUStave0Face0Module3Sector0", "UTAsideUTaUStave0Face0Module3Sector1",
                    "UTAsideUTaUStave0Face0Module4Sector0", "UTAsideUTaUStave0Face0Module4Sector1",
                    "UTAsideUTaUStave0Face0Module5Sector0", "UTAsideUTaUStave0Face0Module6Sector0",
                    "UTAsideUTaUStave0Face0Module7Sector0", "UTAsideUTaUStave0Face1Module0Sector0",
                    "UTAsideUTaUStave0Face1Module1Sector0", "UTAsideUTaUStave0Face1Module2Sector0",
                    "UTAsideUTaUStave0Face1Module3Sector0", "UTAsideUTaUStave0Face1Module3Sector1",
                    "UTAsideUTaUStave0Face1Module4Sector0", "UTAsideUTaUStave0Face1Module4Sector1",
                    "UTAsideUTaUStave0Face1Module5Sector0", "UTAsideUTaUStave0Face1Module5Sector1",
                    "UTAsideUTaUStave0Face1Module6Sector0", "UTAsideUTaUStave0Face1Module7Sector0",
                    "UTAsideUTaUStave1Face0Module0Sector0", "UTAsideUTaUStave1Face0Module1Sector0",
                    "UTAsideUTaUStave1Face0Module2Sector0", "UTAsideUTaUStave1Face0Module2Sector1",
                    "UTAsideUTaUStave1Face0Module4Sector0", "UTAsideUTaUStave1Face0Module4Sector1",
                    "UTAsideUTaUStave1Face0Module5Sector0", "UTAsideUTaUStave1Face0Module6Sector0",
                    "UTAsideUTaUStave1Face0Module7Sector0", "UTAsideUTaUStave1Face1Module0Sector0",
                    "UTAsideUTaUStave1Face1Module1Sector0", "UTAsideUTaUStave1Face1Module2Sector0",
                    "UTAsideUTaUStave1Face1Module3Sector0", "UTAsideUTaUStave1Face1Module3Sector1",
                    "UTAsideUTaUStave1Face1Module5Sector0", "UTAsideUTaUStave1Face1Module5Sector1",
                    "UTAsideUTaUStave1Face1Module6Sector0", "UTAsideUTaUStave1Face1Module7Sector0",
                    "UTAsideUTaUStave2Face0Module0Sector0", "UTAsideUTaUStave2Face0Module1Sector0",
                    "UTAsideUTaUStave2Face0Module2Sector0", "UTAsideUTaUStave2Face0Module4Sector0",
                    "UTAsideUTaUStave2Face0Module5Sector0", "UTAsideUTaUStave2Face0Module6Sector0",
                    "UTAsideUTaUStave2Face0Module7Sector0", "UTAsideUTaUStave2Face1Module0Sector0",
                    "UTAsideUTaUStave2Face1Module1Sector0", "UTAsideUTaUStave2Face1Module2Sector0",
                    "UTAsideUTaUStave2Face1Module3Sector0", "UTAsideUTaUStave2Face1Module5Sector0",
                    "UTAsideUTaUStave2Face1Module6Sector0", "UTAsideUTaUStave2Face1Module7Sector0",
                    "UTAsideUTaUStave3Face0Module0Sector0", "UTAsideUTaUStave3Face0Module1Sector0",
                    "UTAsideUTaUStave3Face0Module2Sector0", "UTAsideUTaUStave3Face0Module4Sector0",
                    "UTAsideUTaUStave3Face0Module5Sector0", "UTAsideUTaUStave3Face0Module6Sector0",
                    "UTAsideUTaUStave3Face0Module7Sector0", "UTAsideUTaUStave3Face1Module0Sector0",
                    "UTAsideUTaUStave3Face1Module1Sector0", "UTAsideUTaUStave3Face1Module2Sector0",
                    "UTAsideUTaUStave3Face1Module3Sector0", "UTAsideUTaUStave3Face1Module5Sector0",
                    "UTAsideUTaUStave3Face1Module6Sector0", "UTAsideUTaUStave3Face1Module7Sector0",
                    "UTAsideUTaUStave4Face0Module0Sector0", "UTAsideUTaUStave4Face0Module1Sector0",
                    "UTAsideUTaUStave4Face0Module2Sector0", "UTAsideUTaUStave4Face0Module4Sector0",
                    "UTAsideUTaUStave4Face0Module5Sector0", "UTAsideUTaUStave4Face0Module6Sector0",
                    "UTAsideUTaUStave4Face0Module7Sector0", "UTAsideUTaUStave4Face1Module0Sector0",
                    "UTAsideUTaUStave4Face1Module1Sector0", "UTAsideUTaUStave4Face1Module2Sector0",
                    "UTAsideUTaUStave4Face1Module3Sector0", "UTAsideUTaUStave4Face1Module5Sector0",
                    "UTAsideUTaUStave4Face1Module6Sector0", "UTAsideUTaUStave4Face1Module7Sector0",
                    "UTAsideUTaUStave5Face0Module0Sector0", "UTAsideUTaUStave5Face0Module1Sector0",
                    "UTAsideUTaUStave5Face0Module2Sector0", "UTAsideUTaUStave5Face0Module4Sector0",
                    "UTAsideUTaUStave5Face0Module5Sector0", "UTAsideUTaUStave5Face0Module6Sector0",
                    "UTAsideUTaUStave5Face0Module7Sector0", "UTAsideUTaUStave5Face1Module0Sector0",
                    "UTAsideUTaUStave5Face1Module1Sector0", "UTAsideUTaUStave5Face1Module2Sector0",
                    "UTAsideUTaUStave5Face1Module3Sector0", "UTAsideUTaUStave5Face1Module5Sector0",
                    "UTAsideUTaUStave5Face1Module6Sector0", "UTAsideUTaUStave5Face1Module7Sector0",
                    "UTAsideUTaUStave6Face0Module0Sector0", "UTAsideUTaUStave6Face0Module1Sector0",
                    "UTAsideUTaUStave6Face0Module2Sector0", "UTAsideUTaUStave6Face0Module4Sector0",
                    "UTAsideUTaUStave6Face0Module5Sector0", "UTAsideUTaUStave6Face0Module6Sector0",
                    "UTAsideUTaUStave6Face0Module7Sector0", "UTAsideUTaUStave6Face1Module0Sector0",
                    "UTAsideUTaUStave6Face1Module1Sector0", "UTAsideUTaUStave6Face1Module2Sector0",
                    "UTAsideUTaUStave6Face1Module3Sector0", "UTAsideUTaUStave6Face1Module5Sector0",
                    "UTAsideUTaUStave6Face1Module6Sector0", "UTAsideUTaUStave6Face1Module7Sector0",
                    "UTAsideUTaUStave7Face0Module0Sector0", "UTAsideUTaUStave7Face0Module1Sector0",
                    "UTAsideUTaUStave7Face0Module2Sector0", "UTAsideUTaUStave7Face0Module4Sector0",
                    "UTAsideUTaUStave7Face0Module5Sector0", "UTAsideUTaUStave7Face0Module6Sector0",
                    "UTAsideUTaUStave7Face0Module7Sector0", "UTAsideUTaUStave7Face1Module0Sector0",
                    "UTAsideUTaUStave7Face1Module1Sector0", "UTAsideUTaUStave7Face1Module2Sector0",
                    "UTAsideUTaUStave7Face1Module3Sector0", "UTAsideUTaUStave7Face1Module5Sector0",
                    "UTAsideUTaUStave7Face1Module6Sector0", "UTAsideUTaUStave7Face1Module7Sector0",
                    "UTAsideUTaXStave0Face0Module0Sector0", "UTAsideUTaXStave0Face0Module1Sector0",
                    "UTAsideUTaXStave0Face0Module2Sector0", "UTAsideUTaXStave0Face0Module2Sector1",
                    "UTAsideUTaXStave0Face0Module3Sector0", "UTAsideUTaXStave0Face0Module3Sector1",
                    "UTAsideUTaXStave0Face0Module4Sector0", "UTAsideUTaXStave0Face0Module4Sector1",
                    "UTAsideUTaXStave0Face0Module5Sector0", "UTAsideUTaXStave0Face0Module6Sector0",
                    "UTAsideUTaXStave0Face0Module7Sector0", "UTAsideUTaXStave0Face1Module0Sector0",
                    "UTAsideUTaXStave0Face1Module1Sector0", "UTAsideUTaXStave0Face1Module2Sector0",
                    "UTAsideUTaXStave0Face1Module3Sector0", "UTAsideUTaXStave0Face1Module3Sector1",
                    "UTAsideUTaXStave0Face1Module4Sector0", "UTAsideUTaXStave0Face1Module4Sector1",
                    "UTAsideUTaXStave0Face1Module5Sector0", "UTAsideUTaXStave0Face1Module5Sector1",
                    "UTAsideUTaXStave0Face1Module6Sector0", "UTAsideUTaXStave0Face1Module7Sector0",
                    "UTAsideUTaXStave1Face0Module0Sector0", "UTAsideUTaXStave1Face0Module1Sector0",
                    "UTAsideUTaXStave1Face0Module2Sector0", "UTAsideUTaXStave1Face0Module2Sector1",
                    "UTAsideUTaXStave1Face0Module4Sector0", "UTAsideUTaXStave1Face0Module4Sector1",
                    "UTAsideUTaXStave1Face0Module5Sector0", "UTAsideUTaXStave1Face0Module6Sector0",
                    "UTAsideUTaXStave1Face0Module7Sector0", "UTAsideUTaXStave1Face1Module0Sector0",
                    "UTAsideUTaXStave1Face1Module1Sector0", "UTAsideUTaXStave1Face1Module2Sector0",
                    "UTAsideUTaXStave1Face1Module3Sector0", "UTAsideUTaXStave1Face1Module3Sector1",
                    "UTAsideUTaXStave1Face1Module5Sector0", "UTAsideUTaXStave1Face1Module5Sector1",
                    "UTAsideUTaXStave1Face1Module6Sector0", "UTAsideUTaXStave1Face1Module7Sector0",
                    "UTAsideUTaXStave2Face0Module0Sector0", "UTAsideUTaXStave2Face0Module1Sector0",
                    "UTAsideUTaXStave2Face0Module2Sector0", "UTAsideUTaXStave2Face0Module4Sector0",
                    "UTAsideUTaXStave2Face0Module5Sector0", "UTAsideUTaXStave2Face0Module6Sector0",
                    "UTAsideUTaXStave2Face0Module7Sector0", "UTAsideUTaXStave2Face1Module0Sector0",
                    "UTAsideUTaXStave2Face1Module1Sector0", "UTAsideUTaXStave2Face1Module2Sector0",
                    "UTAsideUTaXStave2Face1Module3Sector0", "UTAsideUTaXStave2Face1Module5Sector0",
                    "UTAsideUTaXStave2Face1Module6Sector0", "UTAsideUTaXStave2Face1Module7Sector0",
                    "UTAsideUTaXStave3Face0Module0Sector0", "UTAsideUTaXStave3Face0Module1Sector0",
                    "UTAsideUTaXStave3Face0Module2Sector0", "UTAsideUTaXStave3Face0Module4Sector0",
                    "UTAsideUTaXStave3Face0Module5Sector0", "UTAsideUTaXStave3Face0Module6Sector0",
                    "UTAsideUTaXStave3Face0Module7Sector0", "UTAsideUTaXStave3Face1Module0Sector0",
                    "UTAsideUTaXStave3Face1Module1Sector0", "UTAsideUTaXStave3Face1Module2Sector0",
                    "UTAsideUTaXStave3Face1Module3Sector0", "UTAsideUTaXStave3Face1Module5Sector0",
                    "UTAsideUTaXStave3Face1Module6Sector0", "UTAsideUTaXStave3Face1Module7Sector0",
                    "UTAsideUTaXStave4Face0Module0Sector0", "UTAsideUTaXStave4Face0Module1Sector0",
                    "UTAsideUTaXStave4Face0Module2Sector0", "UTAsideUTaXStave4Face0Module4Sector0",
                    "UTAsideUTaXStave4Face0Module5Sector0", "UTAsideUTaXStave4Face0Module6Sector0",
                    "UTAsideUTaXStave4Face0Module7Sector0", "UTAsideUTaXStave4Face1Module0Sector0",
                    "UTAsideUTaXStave4Face1Module1Sector0", "UTAsideUTaXStave4Face1Module2Sector0",
                    "UTAsideUTaXStave4Face1Module3Sector0", "UTAsideUTaXStave4Face1Module5Sector0",
                    "UTAsideUTaXStave4Face1Module6Sector0", "UTAsideUTaXStave4Face1Module7Sector0",
                    "UTAsideUTaXStave5Face0Module0Sector0", "UTAsideUTaXStave5Face0Module1Sector0",
                    "UTAsideUTaXStave5Face0Module2Sector0", "UTAsideUTaXStave5Face0Module4Sector0",
                    "UTAsideUTaXStave5Face0Module5Sector0", "UTAsideUTaXStave5Face0Module6Sector0",
                    "UTAsideUTaXStave5Face0Module7Sector0", "UTAsideUTaXStave5Face1Module0Sector0",
                    "UTAsideUTaXStave5Face1Module1Sector0", "UTAsideUTaXStave5Face1Module2Sector0",
                    "UTAsideUTaXStave5Face1Module3Sector0", "UTAsideUTaXStave5Face1Module5Sector0",
                    "UTAsideUTaXStave5Face1Module6Sector0", "UTAsideUTaXStave5Face1Module7Sector0",
                    "UTAsideUTaXStave6Face0Module0Sector0", "UTAsideUTaXStave6Face0Module1Sector0",
                    "UTAsideUTaXStave6Face0Module2Sector0", "UTAsideUTaXStave6Face0Module4Sector0",
                    "UTAsideUTaXStave6Face0Module5Sector0", "UTAsideUTaXStave6Face0Module6Sector0",
                    "UTAsideUTaXStave6Face0Module7Sector0", "UTAsideUTaXStave6Face1Module0Sector0",
                    "UTAsideUTaXStave6Face1Module1Sector0", "UTAsideUTaXStave6Face1Module2Sector0",
                    "UTAsideUTaXStave6Face1Module3Sector0", "UTAsideUTaXStave6Face1Module5Sector0",
                    "UTAsideUTaXStave6Face1Module6Sector0", "UTAsideUTaXStave6Face1Module7Sector0",
                    "UTAsideUTaXStave7Face0Module0Sector0", "UTAsideUTaXStave7Face0Module1Sector0",
                    "UTAsideUTaXStave7Face0Module2Sector0", "UTAsideUTaXStave7Face0Module4Sector0",
                    "UTAsideUTaXStave7Face0Module5Sector0", "UTAsideUTaXStave7Face0Module6Sector0",
                    "UTAsideUTaXStave7Face0Module7Sector0", "UTAsideUTaXStave7Face1Module0Sector0",
                    "UTAsideUTaXStave7Face1Module1Sector0", "UTAsideUTaXStave7Face1Module2Sector0",
                    "UTAsideUTaXStave7Face1Module3Sector0", "UTAsideUTaXStave7Face1Module5Sector0",
                    "UTAsideUTaXStave7Face1Module6Sector0", "UTAsideUTaXStave7Face1Module7Sector0",
                    "UTAsideUTbVStave0Face0Module0Sector0", "UTAsideUTbVStave0Face0Module1Sector0",
                    "UTAsideUTbVStave0Face0Module2Sector0", "UTAsideUTbVStave0Face0Module2Sector1",
                    "UTAsideUTbVStave0Face0Module3Sector0", "UTAsideUTbVStave0Face0Module3Sector1",
                    "UTAsideUTbVStave0Face0Module4Sector0", "UTAsideUTbVStave0Face0Module4Sector1",
                    "UTAsideUTbVStave0Face0Module5Sector0", "UTAsideUTbVStave0Face0Module6Sector0",
                    "UTAsideUTbVStave0Face0Module7Sector0", "UTAsideUTbVStave0Face1Module0Sector0",
                    "UTAsideUTbVStave0Face1Module1Sector0", "UTAsideUTbVStave0Face1Module2Sector0",
                    "UTAsideUTbVStave0Face1Module3Sector0", "UTAsideUTbVStave0Face1Module3Sector1",
                    "UTAsideUTbVStave0Face1Module4Sector0", "UTAsideUTbVStave0Face1Module4Sector1",
                    "UTAsideUTbVStave0Face1Module5Sector0", "UTAsideUTbVStave0Face1Module5Sector1",
                    "UTAsideUTbVStave0Face1Module6Sector0", "UTAsideUTbVStave0Face1Module7Sector0",
                    "UTAsideUTbVStave1Face0Module0Sector0", "UTAsideUTbVStave1Face0Module1Sector0",
                    "UTAsideUTbVStave1Face0Module2Sector0", "UTAsideUTbVStave1Face0Module2Sector1",
                    "UTAsideUTbVStave1Face0Module4Sector0", "UTAsideUTbVStave1Face0Module4Sector1",
                    "UTAsideUTbVStave1Face0Module5Sector0", "UTAsideUTbVStave1Face0Module6Sector0",
                    "UTAsideUTbVStave1Face0Module7Sector0", "UTAsideUTbVStave1Face1Module0Sector0",
                    "UTAsideUTbVStave1Face1Module1Sector0", "UTAsideUTbVStave1Face1Module2Sector0",
                    "UTAsideUTbVStave1Face1Module3Sector0", "UTAsideUTbVStave1Face1Module3Sector1",
                    "UTAsideUTbVStave1Face1Module5Sector0", "UTAsideUTbVStave1Face1Module5Sector1",
                    "UTAsideUTbVStave1Face1Module6Sector0", "UTAsideUTbVStave1Face1Module7Sector0",
                    "UTAsideUTbVStave2Face0Module0Sector0", "UTAsideUTbVStave2Face0Module1Sector0",
                    "UTAsideUTbVStave2Face0Module2Sector0", "UTAsideUTbVStave2Face0Module4Sector0",
                    "UTAsideUTbVStave2Face0Module5Sector0", "UTAsideUTbVStave2Face0Module6Sector0",
                    "UTAsideUTbVStave2Face0Module7Sector0", "UTAsideUTbVStave2Face1Module0Sector0",
                    "UTAsideUTbVStave2Face1Module1Sector0", "UTAsideUTbVStave2Face1Module2Sector0",
                    "UTAsideUTbVStave2Face1Module3Sector0", "UTAsideUTbVStave2Face1Module5Sector0",
                    "UTAsideUTbVStave2Face1Module6Sector0", "UTAsideUTbVStave2Face1Module7Sector0",
                    "UTAsideUTbVStave3Face0Module0Sector0", "UTAsideUTbVStave3Face0Module1Sector0",
                    "UTAsideUTbVStave3Face0Module2Sector0", "UTAsideUTbVStave3Face0Module4Sector0",
                    "UTAsideUTbVStave3Face0Module5Sector0", "UTAsideUTbVStave3Face0Module6Sector0",
                    "UTAsideUTbVStave3Face0Module7Sector0", "UTAsideUTbVStave3Face1Module0Sector0",
                    "UTAsideUTbVStave3Face1Module1Sector0", "UTAsideUTbVStave3Face1Module2Sector0",
                    "UTAsideUTbVStave3Face1Module3Sector0", "UTAsideUTbVStave3Face1Module5Sector0",
                    "UTAsideUTbVStave3Face1Module6Sector0", "UTAsideUTbVStave3Face1Module7Sector0",
                    "UTAsideUTbVStave4Face0Module0Sector0", "UTAsideUTbVStave4Face0Module1Sector0",
                    "UTAsideUTbVStave4Face0Module2Sector0", "UTAsideUTbVStave4Face0Module4Sector0",
                    "UTAsideUTbVStave4Face0Module5Sector0", "UTAsideUTbVStave4Face0Module6Sector0",
                    "UTAsideUTbVStave4Face0Module7Sector0", "UTAsideUTbVStave4Face1Module0Sector0",
                    "UTAsideUTbVStave4Face1Module1Sector0", "UTAsideUTbVStave4Face1Module2Sector0",
                    "UTAsideUTbVStave4Face1Module3Sector0", "UTAsideUTbVStave4Face1Module5Sector0",
                    "UTAsideUTbVStave4Face1Module6Sector0", "UTAsideUTbVStave4Face1Module7Sector0",
                    "UTAsideUTbVStave5Face0Module0Sector0", "UTAsideUTbVStave5Face0Module1Sector0",
                    "UTAsideUTbVStave5Face0Module2Sector0", "UTAsideUTbVStave5Face0Module4Sector0",
                    "UTAsideUTbVStave5Face0Module5Sector0", "UTAsideUTbVStave5Face0Module6Sector0",
                    "UTAsideUTbVStave5Face0Module7Sector0", "UTAsideUTbVStave5Face1Module0Sector0",
                    "UTAsideUTbVStave5Face1Module1Sector0", "UTAsideUTbVStave5Face1Module2Sector0",
                    "UTAsideUTbVStave5Face1Module3Sector0", "UTAsideUTbVStave5Face1Module5Sector0",
                    "UTAsideUTbVStave5Face1Module6Sector0", "UTAsideUTbVStave5Face1Module7Sector0",
                    "UTAsideUTbVStave6Face0Module0Sector0", "UTAsideUTbVStave6Face0Module1Sector0",
                    "UTAsideUTbVStave6Face0Module2Sector0", "UTAsideUTbVStave6Face0Module4Sector0",
                    "UTAsideUTbVStave6Face0Module5Sector0", "UTAsideUTbVStave6Face0Module6Sector0",
                    "UTAsideUTbVStave6Face0Module7Sector0", "UTAsideUTbVStave6Face1Module0Sector0",
                    "UTAsideUTbVStave6Face1Module1Sector0", "UTAsideUTbVStave6Face1Module2Sector0",
                    "UTAsideUTbVStave6Face1Module3Sector0", "UTAsideUTbVStave6Face1Module5Sector0",
                    "UTAsideUTbVStave6Face1Module6Sector0", "UTAsideUTbVStave6Face1Module7Sector0",
                    "UTAsideUTbVStave7Face0Module0Sector0", "UTAsideUTbVStave7Face0Module1Sector0",
                    "UTAsideUTbVStave7Face0Module2Sector0", "UTAsideUTbVStave7Face0Module4Sector0",
                    "UTAsideUTbVStave7Face0Module5Sector0", "UTAsideUTbVStave7Face0Module6Sector0",
                    "UTAsideUTbVStave7Face0Module7Sector0", "UTAsideUTbVStave7Face1Module0Sector0",
                    "UTAsideUTbVStave7Face1Module1Sector0", "UTAsideUTbVStave7Face1Module2Sector0",
                    "UTAsideUTbVStave7Face1Module3Sector0", "UTAsideUTbVStave7Face1Module5Sector0",
                    "UTAsideUTbVStave7Face1Module6Sector0", "UTAsideUTbVStave7Face1Module7Sector0",
                    "UTAsideUTbVStave8Face0Module0Sector0", "UTAsideUTbVStave8Face0Module1Sector0",
                    "UTAsideUTbVStave8Face0Module2Sector0", "UTAsideUTbVStave8Face0Module4Sector0",
                    "UTAsideUTbVStave8Face0Module5Sector0", "UTAsideUTbVStave8Face0Module6Sector0",
                    "UTAsideUTbVStave8Face0Module7Sector0", "UTAsideUTbVStave8Face1Module0Sector0",
                    "UTAsideUTbVStave8Face1Module1Sector0", "UTAsideUTbVStave8Face1Module2Sector0",
                    "UTAsideUTbVStave8Face1Module3Sector0", "UTAsideUTbVStave8Face1Module5Sector0",
                    "UTAsideUTbVStave8Face1Module6Sector0", "UTAsideUTbVStave8Face1Module7Sector0",
                    "UTAsideUTbXStave0Face0Module0Sector0", "UTAsideUTbXStave0Face0Module1Sector0",
                    "UTAsideUTbXStave0Face0Module2Sector0", "UTAsideUTbXStave0Face0Module2Sector1",
                    "UTAsideUTbXStave0Face0Module3Sector0", "UTAsideUTbXStave0Face0Module3Sector1",
                    "UTAsideUTbXStave0Face0Module4Sector0", "UTAsideUTbXStave0Face0Module4Sector1",
                    "UTAsideUTbXStave0Face0Module5Sector0", "UTAsideUTbXStave0Face0Module6Sector0",
                    "UTAsideUTbXStave0Face0Module7Sector0", "UTAsideUTbXStave0Face1Module0Sector0",
                    "UTAsideUTbXStave0Face1Module1Sector0", "UTAsideUTbXStave0Face1Module2Sector0",
                    "UTAsideUTbXStave0Face1Module3Sector0", "UTAsideUTbXStave0Face1Module3Sector1",
                    "UTAsideUTbXStave0Face1Module4Sector0", "UTAsideUTbXStave0Face1Module4Sector1",
                    "UTAsideUTbXStave0Face1Module5Sector0", "UTAsideUTbXStave0Face1Module5Sector1",
                    "UTAsideUTbXStave0Face1Module6Sector0", "UTAsideUTbXStave0Face1Module7Sector0",
                    "UTAsideUTbXStave1Face0Module0Sector0", "UTAsideUTbXStave1Face0Module1Sector0",
                    "UTAsideUTbXStave1Face0Module2Sector0", "UTAsideUTbXStave1Face0Module2Sector1",
                    "UTAsideUTbXStave1Face0Module4Sector0", "UTAsideUTbXStave1Face0Module4Sector1",
                    "UTAsideUTbXStave1Face0Module5Sector0", "UTAsideUTbXStave1Face0Module6Sector0",
                    "UTAsideUTbXStave1Face0Module7Sector0", "UTAsideUTbXStave1Face1Module0Sector0",
                    "UTAsideUTbXStave1Face1Module1Sector0", "UTAsideUTbXStave1Face1Module2Sector0",
                    "UTAsideUTbXStave1Face1Module3Sector0", "UTAsideUTbXStave1Face1Module3Sector1",
                    "UTAsideUTbXStave1Face1Module5Sector0", "UTAsideUTbXStave1Face1Module5Sector1",
                    "UTAsideUTbXStave1Face1Module6Sector0", "UTAsideUTbXStave1Face1Module7Sector0",
                    "UTAsideUTbXStave2Face0Module0Sector0", "UTAsideUTbXStave2Face0Module1Sector0",
                    "UTAsideUTbXStave2Face0Module2Sector0", "UTAsideUTbXStave2Face0Module4Sector0",
                    "UTAsideUTbXStave2Face0Module5Sector0", "UTAsideUTbXStave2Face0Module6Sector0",
                    "UTAsideUTbXStave2Face0Module7Sector0", "UTAsideUTbXStave2Face1Module0Sector0",
                    "UTAsideUTbXStave2Face1Module1Sector0", "UTAsideUTbXStave2Face1Module2Sector0",
                    "UTAsideUTbXStave2Face1Module3Sector0", "UTAsideUTbXStave2Face1Module5Sector0",
                    "UTAsideUTbXStave2Face1Module6Sector0", "UTAsideUTbXStave2Face1Module7Sector0",
                    "UTAsideUTbXStave3Face0Module0Sector0", "UTAsideUTbXStave3Face0Module1Sector0",
                    "UTAsideUTbXStave3Face0Module2Sector0", "UTAsideUTbXStave3Face0Module4Sector0",
                    "UTAsideUTbXStave3Face0Module5Sector0", "UTAsideUTbXStave3Face0Module6Sector0",
                    "UTAsideUTbXStave3Face0Module7Sector0", "UTAsideUTbXStave3Face1Module0Sector0",
                    "UTAsideUTbXStave3Face1Module1Sector0", "UTAsideUTbXStave3Face1Module2Sector0",
                    "UTAsideUTbXStave3Face1Module3Sector0", "UTAsideUTbXStave3Face1Module5Sector0",
                    "UTAsideUTbXStave3Face1Module6Sector0", "UTAsideUTbXStave3Face1Module7Sector0",
                    "UTAsideUTbXStave4Face0Module0Sector0", "UTAsideUTbXStave4Face0Module1Sector0",
                    "UTAsideUTbXStave4Face0Module2Sector0", "UTAsideUTbXStave4Face0Module4Sector0",
                    "UTAsideUTbXStave4Face0Module5Sector0", "UTAsideUTbXStave4Face0Module6Sector0",
                    "UTAsideUTbXStave4Face0Module7Sector0", "UTAsideUTbXStave4Face1Module0Sector0",
                    "UTAsideUTbXStave4Face1Module1Sector0", "UTAsideUTbXStave4Face1Module2Sector0",
                    "UTAsideUTbXStave4Face1Module3Sector0", "UTAsideUTbXStave4Face1Module5Sector0",
                    "UTAsideUTbXStave4Face1Module6Sector0", "UTAsideUTbXStave4Face1Module7Sector0",
                    "UTAsideUTbXStave5Face0Module0Sector0", "UTAsideUTbXStave5Face0Module1Sector0",
                    "UTAsideUTbXStave5Face0Module2Sector0", "UTAsideUTbXStave5Face0Module4Sector0",
                    "UTAsideUTbXStave5Face0Module5Sector0", "UTAsideUTbXStave5Face0Module6Sector0",
                    "UTAsideUTbXStave5Face0Module7Sector0", "UTAsideUTbXStave5Face1Module0Sector0",
                    "UTAsideUTbXStave5Face1Module1Sector0", "UTAsideUTbXStave5Face1Module2Sector0",
                    "UTAsideUTbXStave5Face1Module3Sector0", "UTAsideUTbXStave5Face1Module5Sector0",
                    "UTAsideUTbXStave5Face1Module6Sector0", "UTAsideUTbXStave5Face1Module7Sector0",
                    "UTAsideUTbXStave6Face0Module0Sector0", "UTAsideUTbXStave6Face0Module1Sector0",
                    "UTAsideUTbXStave6Face0Module2Sector0", "UTAsideUTbXStave6Face0Module4Sector0",
                    "UTAsideUTbXStave6Face0Module5Sector0", "UTAsideUTbXStave6Face0Module6Sector0",
                    "UTAsideUTbXStave6Face0Module7Sector0", "UTAsideUTbXStave6Face1Module0Sector0",
                    "UTAsideUTbXStave6Face1Module1Sector0", "UTAsideUTbXStave6Face1Module2Sector0",
                    "UTAsideUTbXStave6Face1Module3Sector0", "UTAsideUTbXStave6Face1Module5Sector0",
                    "UTAsideUTbXStave6Face1Module6Sector0", "UTAsideUTbXStave6Face1Module7Sector0",
                    "UTAsideUTbXStave7Face0Module0Sector0", "UTAsideUTbXStave7Face0Module1Sector0",
                    "UTAsideUTbXStave7Face0Module2Sector0", "UTAsideUTbXStave7Face0Module4Sector0",
                    "UTAsideUTbXStave7Face0Module5Sector0", "UTAsideUTbXStave7Face0Module6Sector0",
                    "UTAsideUTbXStave7Face0Module7Sector0", "UTAsideUTbXStave7Face1Module0Sector0",
                    "UTAsideUTbXStave7Face1Module1Sector0", "UTAsideUTbXStave7Face1Module2Sector0",
                    "UTAsideUTbXStave7Face1Module3Sector0", "UTAsideUTbXStave7Face1Module5Sector0",
                    "UTAsideUTbXStave7Face1Module6Sector0", "UTAsideUTbXStave7Face1Module7Sector0",
                    "UTAsideUTbXStave8Face0Module0Sector0", "UTAsideUTbXStave8Face0Module1Sector0",
                    "UTAsideUTbXStave8Face0Module2Sector0", "UTAsideUTbXStave8Face0Module4Sector0",
                    "UTAsideUTbXStave8Face0Module5Sector0", "UTAsideUTbXStave8Face0Module6Sector0",
                    "UTAsideUTbXStave8Face0Module7Sector0", "UTAsideUTbXStave8Face1Module0Sector0",
                    "UTAsideUTbXStave8Face1Module1Sector0", "UTAsideUTbXStave8Face1Module2Sector0",
                    "UTAsideUTbXStave8Face1Module3Sector0", "UTAsideUTbXStave8Face1Module5Sector0",
                    "UTAsideUTbXStave8Face1Module6Sector0", "UTAsideUTbXStave8Face1Module7Sector0",
                    "UTCsideUTaUStave0Face0Module0Sector0", "UTCsideUTaUStave0Face0Module1Sector0",
                    "UTCsideUTaUStave0Face0Module2Sector0", "UTCsideUTaUStave0Face0Module2Sector1",
                    "UTCsideUTaUStave0Face0Module3Sector0", "UTCsideUTaUStave0Face0Module3Sector1",
                    "UTCsideUTaUStave0Face0Module4Sector0", "UTCsideUTaUStave0Face0Module4Sector1",
                    "UTCsideUTaUStave0Face0Module5Sector0", "UTCsideUTaUStave0Face0Module6Sector0",
                    "UTCsideUTaUStave0Face0Module7Sector0", "UTCsideUTaUStave0Face1Module0Sector0",
                    "UTCsideUTaUStave0Face1Module1Sector0", "UTCsideUTaUStave0Face1Module2Sector0",
                    "UTCsideUTaUStave0Face1Module3Sector0", "UTCsideUTaUStave0Face1Module3Sector1",
                    "UTCsideUTaUStave0Face1Module4Sector0", "UTCsideUTaUStave0Face1Module4Sector1",
                    "UTCsideUTaUStave0Face1Module5Sector0", "UTCsideUTaUStave0Face1Module5Sector1",
                    "UTCsideUTaUStave0Face1Module6Sector0", "UTCsideUTaUStave0Face1Module7Sector0",
                    "UTCsideUTaUStave1Face0Module0Sector0", "UTCsideUTaUStave1Face0Module1Sector0",
                    "UTCsideUTaUStave1Face0Module2Sector0", "UTCsideUTaUStave1Face0Module2Sector1",
                    "UTCsideUTaUStave1Face0Module4Sector0", "UTCsideUTaUStave1Face0Module4Sector1",
                    "UTCsideUTaUStave1Face0Module5Sector0", "UTCsideUTaUStave1Face0Module6Sector0",
                    "UTCsideUTaUStave1Face0Module7Sector0", "UTCsideUTaUStave1Face1Module0Sector0",
                    "UTCsideUTaUStave1Face1Module1Sector0", "UTCsideUTaUStave1Face1Module2Sector0",
                    "UTCsideUTaUStave1Face1Module3Sector0", "UTCsideUTaUStave1Face1Module3Sector1",
                    "UTCsideUTaUStave1Face1Module5Sector0", "UTCsideUTaUStave1Face1Module5Sector1",
                    "UTCsideUTaUStave1Face1Module6Sector0", "UTCsideUTaUStave1Face1Module7Sector0",
                    "UTCsideUTaUStave2Face0Module0Sector0", "UTCsideUTaUStave2Face0Module1Sector0",
                    "UTCsideUTaUStave2Face0Module2Sector0", "UTCsideUTaUStave2Face0Module4Sector0",
                    "UTCsideUTaUStave2Face0Module5Sector0", "UTCsideUTaUStave2Face0Module6Sector0",
                    "UTCsideUTaUStave2Face0Module7Sector0", "UTCsideUTaUStave2Face1Module0Sector0",
                    "UTCsideUTaUStave2Face1Module1Sector0", "UTCsideUTaUStave2Face1Module2Sector0",
                    "UTCsideUTaUStave2Face1Module3Sector0", "UTCsideUTaUStave2Face1Module5Sector0",
                    "UTCsideUTaUStave2Face1Module6Sector0", "UTCsideUTaUStave2Face1Module7Sector0",
                    "UTCsideUTaUStave3Face0Module0Sector0", "UTCsideUTaUStave3Face0Module1Sector0",
                    "UTCsideUTaUStave3Face0Module2Sector0", "UTCsideUTaUStave3Face0Module4Sector0",
                    "UTCsideUTaUStave3Face0Module5Sector0", "UTCsideUTaUStave3Face0Module6Sector0",
                    "UTCsideUTaUStave3Face0Module7Sector0", "UTCsideUTaUStave3Face1Module0Sector0",
                    "UTCsideUTaUStave3Face1Module1Sector0", "UTCsideUTaUStave3Face1Module2Sector0",
                    "UTCsideUTaUStave3Face1Module3Sector0", "UTCsideUTaUStave3Face1Module5Sector0",
                    "UTCsideUTaUStave3Face1Module6Sector0", "UTCsideUTaUStave3Face1Module7Sector0",
                    "UTCsideUTaUStave4Face0Module0Sector0", "UTCsideUTaUStave4Face0Module1Sector0",
                    "UTCsideUTaUStave4Face0Module2Sector0", "UTCsideUTaUStave4Face0Module4Sector0",
                    "UTCsideUTaUStave4Face0Module5Sector0", "UTCsideUTaUStave4Face0Module6Sector0",
                    "UTCsideUTaUStave4Face0Module7Sector0", "UTCsideUTaUStave4Face1Module0Sector0",
                    "UTCsideUTaUStave4Face1Module1Sector0", "UTCsideUTaUStave4Face1Module2Sector0",
                    "UTCsideUTaUStave4Face1Module3Sector0", "UTCsideUTaUStave4Face1Module5Sector0",
                    "UTCsideUTaUStave4Face1Module6Sector0", "UTCsideUTaUStave4Face1Module7Sector0",
                    "UTCsideUTaUStave5Face0Module0Sector0", "UTCsideUTaUStave5Face0Module1Sector0",
                    "UTCsideUTaUStave5Face0Module2Sector0", "UTCsideUTaUStave5Face0Module4Sector0",
                    "UTCsideUTaUStave5Face0Module5Sector0", "UTCsideUTaUStave5Face0Module6Sector0",
                    "UTCsideUTaUStave5Face0Module7Sector0", "UTCsideUTaUStave5Face1Module0Sector0",
                    "UTCsideUTaUStave5Face1Module1Sector0", "UTCsideUTaUStave5Face1Module2Sector0",
                    "UTCsideUTaUStave5Face1Module3Sector0", "UTCsideUTaUStave5Face1Module5Sector0",
                    "UTCsideUTaUStave5Face1Module6Sector0", "UTCsideUTaUStave5Face1Module7Sector0",
                    "UTCsideUTaUStave6Face0Module0Sector0", "UTCsideUTaUStave6Face0Module1Sector0",
                    "UTCsideUTaUStave6Face0Module2Sector0", "UTCsideUTaUStave6Face0Module4Sector0",
                    "UTCsideUTaUStave6Face0Module5Sector0", "UTCsideUTaUStave6Face0Module6Sector0",
                    "UTCsideUTaUStave6Face0Module7Sector0", "UTCsideUTaUStave6Face1Module0Sector0",
                    "UTCsideUTaUStave6Face1Module1Sector0", "UTCsideUTaUStave6Face1Module2Sector0",
                    "UTCsideUTaUStave6Face1Module3Sector0", "UTCsideUTaUStave6Face1Module5Sector0",
                    "UTCsideUTaUStave6Face1Module6Sector0", "UTCsideUTaUStave6Face1Module7Sector0",
                    "UTCsideUTaUStave7Face0Module0Sector0", "UTCsideUTaUStave7Face0Module1Sector0",
                    "UTCsideUTaUStave7Face0Module2Sector0", "UTCsideUTaUStave7Face0Module4Sector0",
                    "UTCsideUTaUStave7Face0Module5Sector0", "UTCsideUTaUStave7Face0Module6Sector0",
                    "UTCsideUTaUStave7Face0Module7Sector0", "UTCsideUTaUStave7Face1Module0Sector0",
                    "UTCsideUTaUStave7Face1Module1Sector0", "UTCsideUTaUStave7Face1Module2Sector0",
                    "UTCsideUTaUStave7Face1Module3Sector0", "UTCsideUTaUStave7Face1Module5Sector0",
                    "UTCsideUTaUStave7Face1Module6Sector0", "UTCsideUTaUStave7Face1Module7Sector0",
                    "UTCsideUTaXStave0Face0Module0Sector0", "UTCsideUTaXStave0Face0Module1Sector0",
                    "UTCsideUTaXStave0Face0Module2Sector0", "UTCsideUTaXStave0Face0Module2Sector1",
                    "UTCsideUTaXStave0Face0Module3Sector0", "UTCsideUTaXStave0Face0Module3Sector1",
                    "UTCsideUTaXStave0Face0Module4Sector0", "UTCsideUTaXStave0Face0Module4Sector1",
                    "UTCsideUTaXStave0Face0Module5Sector0", "UTCsideUTaXStave0Face0Module6Sector0",
                    "UTCsideUTaXStave0Face0Module7Sector0", "UTCsideUTaXStave0Face1Module0Sector0",
                    "UTCsideUTaXStave0Face1Module1Sector0", "UTCsideUTaXStave0Face1Module2Sector0",
                    "UTCsideUTaXStave0Face1Module3Sector0", "UTCsideUTaXStave0Face1Module3Sector1",
                    "UTCsideUTaXStave0Face1Module4Sector0", "UTCsideUTaXStave0Face1Module4Sector1",
                    "UTCsideUTaXStave0Face1Module5Sector0", "UTCsideUTaXStave0Face1Module5Sector1",
                    "UTCsideUTaXStave0Face1Module6Sector0", "UTCsideUTaXStave0Face1Module7Sector0",
                    "UTCsideUTaXStave1Face0Module0Sector0", "UTCsideUTaXStave1Face0Module1Sector0",
                    "UTCsideUTaXStave1Face0Module2Sector0", "UTCsideUTaXStave1Face0Module2Sector1",
                    "UTCsideUTaXStave1Face0Module4Sector0", "UTCsideUTaXStave1Face0Module4Sector1",
                    "UTCsideUTaXStave1Face0Module5Sector0", "UTCsideUTaXStave1Face0Module6Sector0",
                    "UTCsideUTaXStave1Face0Module7Sector0", "UTCsideUTaXStave1Face1Module0Sector0",
                    "UTCsideUTaXStave1Face1Module1Sector0", "UTCsideUTaXStave1Face1Module2Sector0",
                    "UTCsideUTaXStave1Face1Module3Sector0", "UTCsideUTaXStave1Face1Module3Sector1",
                    "UTCsideUTaXStave1Face1Module5Sector0", "UTCsideUTaXStave1Face1Module5Sector1",
                    "UTCsideUTaXStave1Face1Module6Sector0", "UTCsideUTaXStave1Face1Module7Sector0",
                    "UTCsideUTaXStave2Face0Module0Sector0", "UTCsideUTaXStave2Face0Module1Sector0",
                    "UTCsideUTaXStave2Face0Module2Sector0", "UTCsideUTaXStave2Face0Module4Sector0",
                    "UTCsideUTaXStave2Face0Module5Sector0", "UTCsideUTaXStave2Face0Module6Sector0",
                    "UTCsideUTaXStave2Face0Module7Sector0", "UTCsideUTaXStave2Face1Module0Sector0",
                    "UTCsideUTaXStave2Face1Module1Sector0", "UTCsideUTaXStave2Face1Module2Sector0",
                    "UTCsideUTaXStave2Face1Module3Sector0", "UTCsideUTaXStave2Face1Module5Sector0",
                    "UTCsideUTaXStave2Face1Module6Sector0", "UTCsideUTaXStave2Face1Module7Sector0",
                    "UTCsideUTaXStave3Face0Module0Sector0", "UTCsideUTaXStave3Face0Module1Sector0",
                    "UTCsideUTaXStave3Face0Module2Sector0", "UTCsideUTaXStave3Face0Module4Sector0",
                    "UTCsideUTaXStave3Face0Module5Sector0", "UTCsideUTaXStave3Face0Module6Sector0",
                    "UTCsideUTaXStave3Face0Module7Sector0", "UTCsideUTaXStave3Face1Module0Sector0",
                    "UTCsideUTaXStave3Face1Module1Sector0", "UTCsideUTaXStave3Face1Module2Sector0",
                    "UTCsideUTaXStave3Face1Module3Sector0", "UTCsideUTaXStave3Face1Module5Sector0",
                    "UTCsideUTaXStave3Face1Module6Sector0", "UTCsideUTaXStave3Face1Module7Sector0",
                    "UTCsideUTaXStave4Face0Module0Sector0", "UTCsideUTaXStave4Face0Module1Sector0",
                    "UTCsideUTaXStave4Face0Module2Sector0", "UTCsideUTaXStave4Face0Module4Sector0",
                    "UTCsideUTaXStave4Face0Module5Sector0", "UTCsideUTaXStave4Face0Module6Sector0",
                    "UTCsideUTaXStave4Face0Module7Sector0", "UTCsideUTaXStave4Face1Module0Sector0",
                    "UTCsideUTaXStave4Face1Module1Sector0", "UTCsideUTaXStave4Face1Module2Sector0",
                    "UTCsideUTaXStave4Face1Module3Sector0", "UTCsideUTaXStave4Face1Module5Sector0",
                    "UTCsideUTaXStave4Face1Module6Sector0", "UTCsideUTaXStave4Face1Module7Sector0",
                    "UTCsideUTaXStave5Face0Module0Sector0", "UTCsideUTaXStave5Face0Module1Sector0",
                    "UTCsideUTaXStave5Face0Module2Sector0", "UTCsideUTaXStave5Face0Module4Sector0",
                    "UTCsideUTaXStave5Face0Module5Sector0", "UTCsideUTaXStave5Face0Module6Sector0",
                    "UTCsideUTaXStave5Face0Module7Sector0", "UTCsideUTaXStave5Face1Module0Sector0",
                    "UTCsideUTaXStave5Face1Module1Sector0", "UTCsideUTaXStave5Face1Module2Sector0",
                    "UTCsideUTaXStave5Face1Module3Sector0", "UTCsideUTaXStave5Face1Module5Sector0",
                    "UTCsideUTaXStave5Face1Module6Sector0", "UTCsideUTaXStave5Face1Module7Sector0",
                    "UTCsideUTaXStave6Face0Module0Sector0", "UTCsideUTaXStave6Face0Module1Sector0",
                    "UTCsideUTaXStave6Face0Module2Sector0", "UTCsideUTaXStave6Face0Module4Sector0",
                    "UTCsideUTaXStave6Face0Module5Sector0", "UTCsideUTaXStave6Face0Module6Sector0",
                    "UTCsideUTaXStave6Face0Module7Sector0", "UTCsideUTaXStave6Face1Module0Sector0",
                    "UTCsideUTaXStave6Face1Module1Sector0", "UTCsideUTaXStave6Face1Module2Sector0",
                    "UTCsideUTaXStave6Face1Module3Sector0", "UTCsideUTaXStave6Face1Module5Sector0",
                    "UTCsideUTaXStave6Face1Module6Sector0", "UTCsideUTaXStave6Face1Module7Sector0",
                    "UTCsideUTaXStave7Face0Module0Sector0", "UTCsideUTaXStave7Face0Module1Sector0",
                    "UTCsideUTaXStave7Face0Module2Sector0", "UTCsideUTaXStave7Face0Module4Sector0",
                    "UTCsideUTaXStave7Face0Module5Sector0", "UTCsideUTaXStave7Face0Module6Sector0",
                    "UTCsideUTaXStave7Face0Module7Sector0", "UTCsideUTaXStave7Face1Module0Sector0",
                    "UTCsideUTaXStave7Face1Module1Sector0", "UTCsideUTaXStave7Face1Module2Sector0",
                    "UTCsideUTaXStave7Face1Module3Sector0", "UTCsideUTaXStave7Face1Module5Sector0",
                    "UTCsideUTaXStave7Face1Module6Sector0", "UTCsideUTaXStave7Face1Module7Sector0",
                    "UTCsideUTbVStave0Face0Module0Sector0", "UTCsideUTbVStave0Face0Module1Sector0",
                    "UTCsideUTbVStave0Face0Module2Sector0", "UTCsideUTbVStave0Face0Module2Sector1",
                    "UTCsideUTbVStave0Face0Module3Sector0", "UTCsideUTbVStave0Face0Module3Sector1",
                    "UTCsideUTbVStave0Face0Module4Sector0", "UTCsideUTbVStave0Face0Module4Sector1",
                    "UTCsideUTbVStave0Face0Module5Sector0", "UTCsideUTbVStave0Face0Module6Sector0",
                    "UTCsideUTbVStave0Face0Module7Sector0", "UTCsideUTbVStave0Face1Module0Sector0",
                    "UTCsideUTbVStave0Face1Module1Sector0", "UTCsideUTbVStave0Face1Module2Sector0",
                    "UTCsideUTbVStave0Face1Module3Sector0", "UTCsideUTbVStave0Face1Module3Sector1",
                    "UTCsideUTbVStave0Face1Module4Sector0", "UTCsideUTbVStave0Face1Module4Sector1",
                    "UTCsideUTbVStave0Face1Module5Sector0", "UTCsideUTbVStave0Face1Module5Sector1",
                    "UTCsideUTbVStave0Face1Module6Sector0", "UTCsideUTbVStave0Face1Module7Sector0",
                    "UTCsideUTbVStave1Face0Module0Sector0", "UTCsideUTbVStave1Face0Module1Sector0",
                    "UTCsideUTbVStave1Face0Module2Sector0", "UTCsideUTbVStave1Face0Module2Sector1",
                    "UTCsideUTbVStave1Face0Module4Sector0", "UTCsideUTbVStave1Face0Module4Sector1",
                    "UTCsideUTbVStave1Face0Module5Sector0", "UTCsideUTbVStave1Face0Module6Sector0",
                    "UTCsideUTbVStave1Face0Module7Sector0", "UTCsideUTbVStave1Face1Module0Sector0",
                    "UTCsideUTbVStave1Face1Module1Sector0", "UTCsideUTbVStave1Face1Module2Sector0",
                    "UTCsideUTbVStave1Face1Module3Sector0", "UTCsideUTbVStave1Face1Module3Sector1",
                    "UTCsideUTbVStave1Face1Module5Sector0", "UTCsideUTbVStave1Face1Module5Sector1",
                    "UTCsideUTbVStave1Face1Module6Sector0", "UTCsideUTbVStave1Face1Module7Sector0",
                    "UTCsideUTbVStave2Face0Module0Sector0", "UTCsideUTbVStave2Face0Module1Sector0",
                    "UTCsideUTbVStave2Face0Module2Sector0", "UTCsideUTbVStave2Face0Module4Sector0",
                    "UTCsideUTbVStave2Face0Module5Sector0", "UTCsideUTbVStave2Face0Module6Sector0",
                    "UTCsideUTbVStave2Face0Module7Sector0", "UTCsideUTbVStave2Face1Module0Sector0",
                    "UTCsideUTbVStave2Face1Module1Sector0", "UTCsideUTbVStave2Face1Module2Sector0",
                    "UTCsideUTbVStave2Face1Module3Sector0", "UTCsideUTbVStave2Face1Module5Sector0",
                    "UTCsideUTbVStave2Face1Module6Sector0", "UTCsideUTbVStave2Face1Module7Sector0",
                    "UTCsideUTbVStave3Face0Module0Sector0", "UTCsideUTbVStave3Face0Module1Sector0",
                    "UTCsideUTbVStave3Face0Module2Sector0", "UTCsideUTbVStave3Face0Module4Sector0",
                    "UTCsideUTbVStave3Face0Module5Sector0", "UTCsideUTbVStave3Face0Module6Sector0",
                    "UTCsideUTbVStave3Face0Module7Sector0", "UTCsideUTbVStave3Face1Module0Sector0",
                    "UTCsideUTbVStave3Face1Module1Sector0", "UTCsideUTbVStave3Face1Module2Sector0",
                    "UTCsideUTbVStave3Face1Module3Sector0", "UTCsideUTbVStave3Face1Module5Sector0",
                    "UTCsideUTbVStave3Face1Module6Sector0", "UTCsideUTbVStave3Face1Module7Sector0",
                    "UTCsideUTbVStave4Face0Module0Sector0", "UTCsideUTbVStave4Face0Module1Sector0",
                    "UTCsideUTbVStave4Face0Module2Sector0", "UTCsideUTbVStave4Face0Module4Sector0",
                    "UTCsideUTbVStave4Face0Module5Sector0", "UTCsideUTbVStave4Face0Module6Sector0",
                    "UTCsideUTbVStave4Face0Module7Sector0", "UTCsideUTbVStave4Face1Module0Sector0",
                    "UTCsideUTbVStave4Face1Module1Sector0", "UTCsideUTbVStave4Face1Module2Sector0",
                    "UTCsideUTbVStave4Face1Module3Sector0", "UTCsideUTbVStave4Face1Module5Sector0",
                    "UTCsideUTbVStave4Face1Module6Sector0", "UTCsideUTbVStave4Face1Module7Sector0",
                    "UTCsideUTbVStave5Face0Module0Sector0", "UTCsideUTbVStave5Face0Module1Sector0",
                    "UTCsideUTbVStave5Face0Module2Sector0", "UTCsideUTbVStave5Face0Module4Sector0",
                    "UTCsideUTbVStave5Face0Module5Sector0", "UTCsideUTbVStave5Face0Module6Sector0",
                    "UTCsideUTbVStave5Face0Module7Sector0", "UTCsideUTbVStave5Face1Module0Sector0",
                    "UTCsideUTbVStave5Face1Module1Sector0", "UTCsideUTbVStave5Face1Module2Sector0",
                    "UTCsideUTbVStave5Face1Module3Sector0", "UTCsideUTbVStave5Face1Module5Sector0",
                    "UTCsideUTbVStave5Face1Module6Sector0", "UTCsideUTbVStave5Face1Module7Sector0",
                    "UTCsideUTbVStave6Face0Module0Sector0", "UTCsideUTbVStave6Face0Module1Sector0",
                    "UTCsideUTbVStave6Face0Module2Sector0", "UTCsideUTbVStave6Face0Module4Sector0",
                    "UTCsideUTbVStave6Face0Module5Sector0", "UTCsideUTbVStave6Face0Module6Sector0",
                    "UTCsideUTbVStave6Face0Module7Sector0", "UTCsideUTbVStave6Face1Module0Sector0",
                    "UTCsideUTbVStave6Face1Module1Sector0", "UTCsideUTbVStave6Face1Module2Sector0",
                    "UTCsideUTbVStave6Face1Module3Sector0", "UTCsideUTbVStave6Face1Module5Sector0",
                    "UTCsideUTbVStave6Face1Module6Sector0", "UTCsideUTbVStave6Face1Module7Sector0",
                    "UTCsideUTbVStave7Face0Module0Sector0", "UTCsideUTbVStave7Face0Module1Sector0",
                    "UTCsideUTbVStave7Face0Module2Sector0", "UTCsideUTbVStave7Face0Module4Sector0",
                    "UTCsideUTbVStave7Face0Module5Sector0", "UTCsideUTbVStave7Face0Module6Sector0",
                    "UTCsideUTbVStave7Face0Module7Sector0", "UTCsideUTbVStave7Face1Module0Sector0",
                    "UTCsideUTbVStave7Face1Module1Sector0", "UTCsideUTbVStave7Face1Module2Sector0",
                    "UTCsideUTbVStave7Face1Module3Sector0", "UTCsideUTbVStave7Face1Module5Sector0",
                    "UTCsideUTbVStave7Face1Module6Sector0", "UTCsideUTbVStave7Face1Module7Sector0",
                    "UTCsideUTbVStave8Face0Module0Sector0", "UTCsideUTbVStave8Face0Module1Sector0",
                    "UTCsideUTbVStave8Face0Module2Sector0", "UTCsideUTbVStave8Face0Module4Sector0",
                    "UTCsideUTbVStave8Face0Module5Sector0", "UTCsideUTbVStave8Face0Module6Sector0",
                    "UTCsideUTbVStave8Face0Module7Sector0", "UTCsideUTbVStave8Face1Module0Sector0",
                    "UTCsideUTbVStave8Face1Module1Sector0", "UTCsideUTbVStave8Face1Module2Sector0",
                    "UTCsideUTbVStave8Face1Module3Sector0", "UTCsideUTbVStave8Face1Module5Sector0",
                    "UTCsideUTbVStave8Face1Module6Sector0", "UTCsideUTbVStave8Face1Module7Sector0",
                    "UTCsideUTbXStave0Face0Module0Sector0", "UTCsideUTbXStave0Face0Module1Sector0",
                    "UTCsideUTbXStave0Face0Module2Sector0", "UTCsideUTbXStave0Face0Module2Sector1",
                    "UTCsideUTbXStave0Face0Module3Sector0", "UTCsideUTbXStave0Face0Module3Sector1",
                    "UTCsideUTbXStave0Face0Module4Sector0", "UTCsideUTbXStave0Face0Module4Sector1",
                    "UTCsideUTbXStave0Face0Module5Sector0", "UTCsideUTbXStave0Face0Module6Sector0",
                    "UTCsideUTbXStave0Face0Module7Sector0", "UTCsideUTbXStave0Face1Module0Sector0",
                    "UTCsideUTbXStave0Face1Module1Sector0", "UTCsideUTbXStave0Face1Module2Sector0",
                    "UTCsideUTbXStave0Face1Module3Sector0", "UTCsideUTbXStave0Face1Module3Sector1",
                    "UTCsideUTbXStave0Face1Module4Sector0", "UTCsideUTbXStave0Face1Module4Sector1",
                    "UTCsideUTbXStave0Face1Module5Sector0", "UTCsideUTbXStave0Face1Module5Sector1",
                    "UTCsideUTbXStave0Face1Module6Sector0", "UTCsideUTbXStave0Face1Module7Sector0",
                    "UTCsideUTbXStave1Face0Module0Sector0", "UTCsideUTbXStave1Face0Module1Sector0",
                    "UTCsideUTbXStave1Face0Module2Sector0", "UTCsideUTbXStave1Face0Module2Sector1",
                    "UTCsideUTbXStave1Face0Module4Sector0", "UTCsideUTbXStave1Face0Module4Sector1",
                    "UTCsideUTbXStave1Face0Module5Sector0", "UTCsideUTbXStave1Face0Module6Sector0",
                    "UTCsideUTbXStave1Face0Module7Sector0", "UTCsideUTbXStave1Face1Module0Sector0",
                    "UTCsideUTbXStave1Face1Module1Sector0", "UTCsideUTbXStave1Face1Module2Sector0",
                    "UTCsideUTbXStave1Face1Module3Sector0", "UTCsideUTbXStave1Face1Module3Sector1",
                    "UTCsideUTbXStave1Face1Module5Sector0", "UTCsideUTbXStave1Face1Module5Sector1",
                    "UTCsideUTbXStave1Face1Module6Sector0", "UTCsideUTbXStave1Face1Module7Sector0",
                    "UTCsideUTbXStave2Face0Module0Sector0", "UTCsideUTbXStave2Face0Module1Sector0",
                    "UTCsideUTbXStave2Face0Module2Sector0", "UTCsideUTbXStave2Face0Module4Sector0",
                    "UTCsideUTbXStave2Face0Module5Sector0", "UTCsideUTbXStave2Face0Module6Sector0",
                    "UTCsideUTbXStave2Face0Module7Sector0", "UTCsideUTbXStave2Face1Module0Sector0",
                    "UTCsideUTbXStave2Face1Module1Sector0", "UTCsideUTbXStave2Face1Module2Sector0",
                    "UTCsideUTbXStave2Face1Module3Sector0", "UTCsideUTbXStave2Face1Module5Sector0",
                    "UTCsideUTbXStave2Face1Module6Sector0", "UTCsideUTbXStave2Face1Module7Sector0",
                    "UTCsideUTbXStave3Face0Module0Sector0", "UTCsideUTbXStave3Face0Module1Sector0",
                    "UTCsideUTbXStave3Face0Module2Sector0", "UTCsideUTbXStave3Face0Module4Sector0",
                    "UTCsideUTbXStave3Face0Module5Sector0", "UTCsideUTbXStave3Face0Module6Sector0",
                    "UTCsideUTbXStave3Face0Module7Sector0", "UTCsideUTbXStave3Face1Module0Sector0",
                    "UTCsideUTbXStave3Face1Module1Sector0", "UTCsideUTbXStave3Face1Module2Sector0",
                    "UTCsideUTbXStave3Face1Module3Sector0", "UTCsideUTbXStave3Face1Module5Sector0",
                    "UTCsideUTbXStave3Face1Module6Sector0", "UTCsideUTbXStave3Face1Module7Sector0",
                    "UTCsideUTbXStave4Face0Module0Sector0", "UTCsideUTbXStave4Face0Module1Sector0",
                    "UTCsideUTbXStave4Face0Module2Sector0", "UTCsideUTbXStave4Face0Module4Sector0",
                    "UTCsideUTbXStave4Face0Module5Sector0", "UTCsideUTbXStave4Face0Module6Sector0",
                    "UTCsideUTbXStave4Face0Module7Sector0", "UTCsideUTbXStave4Face1Module0Sector0",
                    "UTCsideUTbXStave4Face1Module1Sector0", "UTCsideUTbXStave4Face1Module2Sector0",
                    "UTCsideUTbXStave4Face1Module3Sector0", "UTCsideUTbXStave4Face1Module5Sector0",
                    "UTCsideUTbXStave4Face1Module6Sector0", "UTCsideUTbXStave4Face1Module7Sector0",
                    "UTCsideUTbXStave5Face0Module0Sector0", "UTCsideUTbXStave5Face0Module1Sector0",
                    "UTCsideUTbXStave5Face0Module2Sector0", "UTCsideUTbXStave5Face0Module4Sector0",
                    "UTCsideUTbXStave5Face0Module5Sector0", "UTCsideUTbXStave5Face0Module6Sector0",
                    "UTCsideUTbXStave5Face0Module7Sector0", "UTCsideUTbXStave5Face1Module0Sector0",
                    "UTCsideUTbXStave5Face1Module1Sector0", "UTCsideUTbXStave5Face1Module2Sector0",
                    "UTCsideUTbXStave5Face1Module3Sector0", "UTCsideUTbXStave5Face1Module5Sector0",
                    "UTCsideUTbXStave5Face1Module6Sector0", "UTCsideUTbXStave5Face1Module7Sector0",
                    "UTCsideUTbXStave6Face0Module0Sector0", "UTCsideUTbXStave6Face0Module1Sector0",
                    "UTCsideUTbXStave6Face0Module2Sector0", "UTCsideUTbXStave6Face0Module4Sector0",
                    "UTCsideUTbXStave6Face0Module5Sector0", "UTCsideUTbXStave6Face0Module6Sector0",
                    "UTCsideUTbXStave6Face0Module7Sector0", "UTCsideUTbXStave6Face1Module0Sector0",
                    "UTCsideUTbXStave6Face1Module1Sector0", "UTCsideUTbXStave6Face1Module2Sector0",
                    "UTCsideUTbXStave6Face1Module3Sector0", "UTCsideUTbXStave6Face1Module5Sector0",
                    "UTCsideUTbXStave6Face1Module6Sector0", "UTCsideUTbXStave6Face1Module7Sector0",
                    "UTCsideUTbXStave7Face0Module0Sector0", "UTCsideUTbXStave7Face0Module1Sector0",
                    "UTCsideUTbXStave7Face0Module2Sector0", "UTCsideUTbXStave7Face0Module4Sector0",
                    "UTCsideUTbXStave7Face0Module5Sector0", "UTCsideUTbXStave7Face0Module6Sector0",
                    "UTCsideUTbXStave7Face0Module7Sector0", "UTCsideUTbXStave7Face1Module0Sector0",
                    "UTCsideUTbXStave7Face1Module1Sector0", "UTCsideUTbXStave7Face1Module2Sector0",
                    "UTCsideUTbXStave7Face1Module3Sector0", "UTCsideUTbXStave7Face1Module5Sector0",
                    "UTCsideUTbXStave7Face1Module6Sector0", "UTCsideUTbXStave7Face1Module7Sector0",
                    "UTCsideUTbXStave8Face0Module0Sector0", "UTCsideUTbXStave8Face0Module1Sector0",
                    "UTCsideUTbXStave8Face0Module2Sector0", "UTCsideUTbXStave8Face0Module4Sector0",
                    "UTCsideUTbXStave8Face0Module5Sector0", "UTCsideUTbXStave8Face0Module6Sector0",
                    "UTCsideUTbXStave8Face0Module7Sector0", "UTCsideUTbXStave8Face1Module0Sector0",
                    "UTCsideUTbXStave8Face1Module1Sector0", "UTCsideUTbXStave8Face1Module2Sector0",
                    "UTCsideUTbXStave8Face1Module3Sector0", "UTCsideUTbXStave8Face1Module5Sector0",
                    "UTCsideUTbXStave8Face1Module6Sector0", "UTCsideUTbXStave8Face1Module7Sector0"};
  } else {
    dd4hep::printout( dd4hep::INFO, "UT", "Not using GeoVersion::v1" );
    stave_names = {
        "UTaULayerR1Module1", "UTaULayerR1Module2", "UTaULayerR1Module3", "UTaULayerR1Module4", "UTaULayerR1Module5",
        "UTaULayerR1Module6", "UTaULayerR2Module1", "UTaULayerR2Module2", "UTaULayerR2Module3", "UTaULayerR2Module4",
        "UTaULayerR3Module1", "UTaULayerR3Module2", "UTaULayerR3Module3", "UTaULayerR3Module4", "UTaULayerR3Module5",
        "UTaULayerR3Module6", "UTaXLayerR1Module1", "UTaXLayerR1Module2", "UTaXLayerR1Module3", "UTaXLayerR1Module4",
        "UTaXLayerR1Module5", "UTaXLayerR1Module6", "UTaXLayerR2Module1", "UTaXLayerR2Module2", "UTaXLayerR2Module3",
        "UTaXLayerR2Module4", "UTaXLayerR3Module1", "UTaXLayerR3Module2", "UTaXLayerR3Module3", "UTaXLayerR3Module4",
        "UTaXLayerR3Module5", "UTaXLayerR3Module6", "UTbVLayerR1Module1", "UTbVLayerR1Module2", "UTbVLayerR1Module3",
        "UTbVLayerR1Module4", "UTbVLayerR1Module5", "UTbVLayerR1Module6", "UTbVLayerR1Module7", "UTbVLayerR2Module1",
        "UTbVLayerR2Module2", "UTbVLayerR2Module3", "UTbVLayerR2Module4", "UTbVLayerR3Module1", "UTbVLayerR3Module2",
        "UTbVLayerR3Module3", "UTbVLayerR3Module4", "UTbVLayerR3Module5", "UTbVLayerR3Module6", "UTbVLayerR3Module7",
        "UTbXLayerR1Module1", "UTbXLayerR1Module2", "UTbXLayerR1Module3", "UTbXLayerR1Module4", "UTbXLayerR1Module5",
        "UTbXLayerR1Module6", "UTbXLayerR1Module7", "UTbXLayerR2Module1", "UTbXLayerR2Module2", "UTbXLayerR2Module3",
        "UTbXLayerR2Module4", "UTbXLayerR3Module1", "UTbXLayerR3Module2", "UTbXLayerR3Module3", "UTbXLayerR3Module4",
        "UTbXLayerR3Module5", "UTbXLayerR3Module6", "UTbXLayerR3Module7"};
    sector_names = {"UTaULayerR1Module1Sector1",  "UTaULayerR1Module1Sector10", "UTaULayerR1Module1Sector11",
                    "UTaULayerR1Module1Sector12", "UTaULayerR1Module1Sector13", "UTaULayerR1Module1Sector14",
                    "UTaULayerR1Module1Sector2",  "UTaULayerR1Module1Sector3",  "UTaULayerR1Module1Sector4",
                    "UTaULayerR1Module1Sector5",  "UTaULayerR1Module1Sector6",  "UTaULayerR1Module1Sector7",
                    "UTaULayerR1Module1Sector8",  "UTaULayerR1Module1Sector9",  "UTaULayerR1Module2Sector1",
                    "UTaULayerR1Module2Sector10", "UTaULayerR1Module2Sector11", "UTaULayerR1Module2Sector12",
                    "UTaULayerR1Module2Sector13", "UTaULayerR1Module2Sector14", "UTaULayerR1Module2Sector2",
                    "UTaULayerR1Module2Sector3",  "UTaULayerR1Module2Sector4",  "UTaULayerR1Module2Sector5",
                    "UTaULayerR1Module2Sector6",  "UTaULayerR1Module2Sector7",  "UTaULayerR1Module2Sector8",
                    "UTaULayerR1Module2Sector9",  "UTaULayerR1Module3Sector1",  "UTaULayerR1Module3Sector10",
                    "UTaULayerR1Module3Sector11", "UTaULayerR1Module3Sector12", "UTaULayerR1Module3Sector13",
                    "UTaULayerR1Module3Sector14", "UTaULayerR1Module3Sector2",  "UTaULayerR1Module3Sector3",
                    "UTaULayerR1Module3Sector4",  "UTaULayerR1Module3Sector5",  "UTaULayerR1Module3Sector6",
                    "UTaULayerR1Module3Sector7",  "UTaULayerR1Module3Sector8",  "UTaULayerR1Module3Sector9",
                    "UTaULayerR1Module4Sector1",  "UTaULayerR1Module4Sector10", "UTaULayerR1Module4Sector11",
                    "UTaULayerR1Module4Sector12", "UTaULayerR1Module4Sector13", "UTaULayerR1Module4Sector14",
                    "UTaULayerR1Module4Sector2",  "UTaULayerR1Module4Sector3",  "UTaULayerR1Module4Sector4",
                    "UTaULayerR1Module4Sector5",  "UTaULayerR1Module4Sector6",  "UTaULayerR1Module4Sector7",
                    "UTaULayerR1Module4Sector8",  "UTaULayerR1Module4Sector9",  "UTaULayerR1Module5Sector1",
                    "UTaULayerR1Module5Sector10", "UTaULayerR1Module5Sector11", "UTaULayerR1Module5Sector12",
                    "UTaULayerR1Module5Sector13", "UTaULayerR1Module5Sector14", "UTaULayerR1Module5Sector2",
                    "UTaULayerR1Module5Sector3",  "UTaULayerR1Module5Sector4",  "UTaULayerR1Module5Sector5",
                    "UTaULayerR1Module5Sector6",  "UTaULayerR1Module5Sector7",  "UTaULayerR1Module5Sector8",
                    "UTaULayerR1Module5Sector9",  "UTaULayerR1Module6Sector1",  "UTaULayerR1Module6Sector10",
                    "UTaULayerR1Module6Sector11", "UTaULayerR1Module6Sector12", "UTaULayerR1Module6Sector13",
                    "UTaULayerR1Module6Sector14", "UTaULayerR1Module6Sector2",  "UTaULayerR1Module6Sector3",
                    "UTaULayerR1Module6Sector4",  "UTaULayerR1Module6Sector5",  "UTaULayerR1Module6Sector6",
                    "UTaULayerR1Module6Sector7",  "UTaULayerR1Module6Sector8",  "UTaULayerR1Module6Sector9",
                    "UTaULayerR2Module1Sector1",  "UTaULayerR2Module1Sector10", "UTaULayerR2Module1Sector11",
                    "UTaULayerR2Module1Sector12", "UTaULayerR2Module1Sector13", "UTaULayerR2Module1Sector14",
                    "UTaULayerR2Module1Sector15", "UTaULayerR2Module1Sector16", "UTaULayerR2Module1Sector17",
                    "UTaULayerR2Module1Sector18", "UTaULayerR2Module1Sector2",  "UTaULayerR2Module1Sector3",
                    "UTaULayerR2Module1Sector4",  "UTaULayerR2Module1Sector5",  "UTaULayerR2Module1Sector6",
                    "UTaULayerR2Module1Sector7",  "UTaULayerR2Module1Sector8",  "UTaULayerR2Module1Sector9",
                    "UTaULayerR2Module2Sector1",  "UTaULayerR2Module2Sector10", "UTaULayerR2Module2Sector11",
                    "UTaULayerR2Module2Sector12", "UTaULayerR2Module2Sector13", "UTaULayerR2Module2Sector14",
                    "UTaULayerR2Module2Sector15", "UTaULayerR2Module2Sector16", "UTaULayerR2Module2Sector17",
                    "UTaULayerR2Module2Sector18", "UTaULayerR2Module2Sector19", "UTaULayerR2Module2Sector2",
                    "UTaULayerR2Module2Sector20", "UTaULayerR2Module2Sector21", "UTaULayerR2Module2Sector22",
                    "UTaULayerR2Module2Sector3",  "UTaULayerR2Module2Sector4",  "UTaULayerR2Module2Sector5",
                    "UTaULayerR2Module2Sector6",  "UTaULayerR2Module2Sector7",  "UTaULayerR2Module2Sector8",
                    "UTaULayerR2Module2Sector9",  "UTaULayerR2Module3Sector1",  "UTaULayerR2Module3Sector10",
                    "UTaULayerR2Module3Sector11", "UTaULayerR2Module3Sector12", "UTaULayerR2Module3Sector13",
                    "UTaULayerR2Module3Sector14", "UTaULayerR2Module3Sector15", "UTaULayerR2Module3Sector16",
                    "UTaULayerR2Module3Sector17", "UTaULayerR2Module3Sector18", "UTaULayerR2Module3Sector19",
                    "UTaULayerR2Module3Sector2",  "UTaULayerR2Module3Sector20", "UTaULayerR2Module3Sector21",
                    "UTaULayerR2Module3Sector22", "UTaULayerR2Module3Sector3",  "UTaULayerR2Module3Sector4",
                    "UTaULayerR2Module3Sector5",  "UTaULayerR2Module3Sector6",  "UTaULayerR2Module3Sector7",
                    "UTaULayerR2Module3Sector8",  "UTaULayerR2Module3Sector9",  "UTaULayerR2Module4Sector1",
                    "UTaULayerR2Module4Sector10", "UTaULayerR2Module4Sector11", "UTaULayerR2Module4Sector12",
                    "UTaULayerR2Module4Sector13", "UTaULayerR2Module4Sector14", "UTaULayerR2Module4Sector15",
                    "UTaULayerR2Module4Sector16", "UTaULayerR2Module4Sector17", "UTaULayerR2Module4Sector18",
                    "UTaULayerR2Module4Sector2",  "UTaULayerR2Module4Sector3",  "UTaULayerR2Module4Sector4",
                    "UTaULayerR2Module4Sector5",  "UTaULayerR2Module4Sector6",  "UTaULayerR2Module4Sector7",
                    "UTaULayerR2Module4Sector8",  "UTaULayerR2Module4Sector9",  "UTaULayerR3Module1Sector1",
                    "UTaULayerR3Module1Sector10", "UTaULayerR3Module1Sector11", "UTaULayerR3Module1Sector12",
                    "UTaULayerR3Module1Sector13", "UTaULayerR3Module1Sector14", "UTaULayerR3Module1Sector2",
                    "UTaULayerR3Module1Sector3",  "UTaULayerR3Module1Sector4",  "UTaULayerR3Module1Sector5",
                    "UTaULayerR3Module1Sector6",  "UTaULayerR3Module1Sector7",  "UTaULayerR3Module1Sector8",
                    "UTaULayerR3Module1Sector9",  "UTaULayerR3Module2Sector1",  "UTaULayerR3Module2Sector10",
                    "UTaULayerR3Module2Sector11", "UTaULayerR3Module2Sector12", "UTaULayerR3Module2Sector13",
                    "UTaULayerR3Module2Sector14", "UTaULayerR3Module2Sector2",  "UTaULayerR3Module2Sector3",
                    "UTaULayerR3Module2Sector4",  "UTaULayerR3Module2Sector5",  "UTaULayerR3Module2Sector6",
                    "UTaULayerR3Module2Sector7",  "UTaULayerR3Module2Sector8",  "UTaULayerR3Module2Sector9",
                    "UTaULayerR3Module3Sector1",  "UTaULayerR3Module3Sector10", "UTaULayerR3Module3Sector11",
                    "UTaULayerR3Module3Sector12", "UTaULayerR3Module3Sector13", "UTaULayerR3Module3Sector14",
                    "UTaULayerR3Module3Sector2",  "UTaULayerR3Module3Sector3",  "UTaULayerR3Module3Sector4",
                    "UTaULayerR3Module3Sector5",  "UTaULayerR3Module3Sector6",  "UTaULayerR3Module3Sector7",
                    "UTaULayerR3Module3Sector8",  "UTaULayerR3Module3Sector9",  "UTaULayerR3Module4Sector1",
                    "UTaULayerR3Module4Sector10", "UTaULayerR3Module4Sector11", "UTaULayerR3Module4Sector12",
                    "UTaULayerR3Module4Sector13", "UTaULayerR3Module4Sector14", "UTaULayerR3Module4Sector2",
                    "UTaULayerR3Module4Sector3",  "UTaULayerR3Module4Sector4",  "UTaULayerR3Module4Sector5",
                    "UTaULayerR3Module4Sector6",  "UTaULayerR3Module4Sector7",  "UTaULayerR3Module4Sector8",
                    "UTaULayerR3Module4Sector9",  "UTaULayerR3Module5Sector1",  "UTaULayerR3Module5Sector10",
                    "UTaULayerR3Module5Sector11", "UTaULayerR3Module5Sector12", "UTaULayerR3Module5Sector13",
                    "UTaULayerR3Module5Sector14", "UTaULayerR3Module5Sector2",  "UTaULayerR3Module5Sector3",
                    "UTaULayerR3Module5Sector4",  "UTaULayerR3Module5Sector5",  "UTaULayerR3Module5Sector6",
                    "UTaULayerR3Module5Sector7",  "UTaULayerR3Module5Sector8",  "UTaULayerR3Module5Sector9",
                    "UTaULayerR3Module6Sector1",  "UTaULayerR3Module6Sector10", "UTaULayerR3Module6Sector11",
                    "UTaULayerR3Module6Sector12", "UTaULayerR3Module6Sector13", "UTaULayerR3Module6Sector14",
                    "UTaULayerR3Module6Sector2",  "UTaULayerR3Module6Sector3",  "UTaULayerR3Module6Sector4",
                    "UTaULayerR3Module6Sector5",  "UTaULayerR3Module6Sector6",  "UTaULayerR3Module6Sector7",
                    "UTaULayerR3Module6Sector8",  "UTaULayerR3Module6Sector9",  "UTaXLayerR1Module1Sector1",
                    "UTaXLayerR1Module1Sector10", "UTaXLayerR1Module1Sector11", "UTaXLayerR1Module1Sector12",
                    "UTaXLayerR1Module1Sector13", "UTaXLayerR1Module1Sector14", "UTaXLayerR1Module1Sector2",
                    "UTaXLayerR1Module1Sector3",  "UTaXLayerR1Module1Sector4",  "UTaXLayerR1Module1Sector5",
                    "UTaXLayerR1Module1Sector6",  "UTaXLayerR1Module1Sector7",  "UTaXLayerR1Module1Sector8",
                    "UTaXLayerR1Module1Sector9",  "UTaXLayerR1Module2Sector1",  "UTaXLayerR1Module2Sector10",
                    "UTaXLayerR1Module2Sector11", "UTaXLayerR1Module2Sector12", "UTaXLayerR1Module2Sector13",
                    "UTaXLayerR1Module2Sector14", "UTaXLayerR1Module2Sector2",  "UTaXLayerR1Module2Sector3",
                    "UTaXLayerR1Module2Sector4",  "UTaXLayerR1Module2Sector5",  "UTaXLayerR1Module2Sector6",
                    "UTaXLayerR1Module2Sector7",  "UTaXLayerR1Module2Sector8",  "UTaXLayerR1Module2Sector9",
                    "UTaXLayerR1Module3Sector1",  "UTaXLayerR1Module3Sector10", "UTaXLayerR1Module3Sector11",
                    "UTaXLayerR1Module3Sector12", "UTaXLayerR1Module3Sector13", "UTaXLayerR1Module3Sector14",
                    "UTaXLayerR1Module3Sector2",  "UTaXLayerR1Module3Sector3",  "UTaXLayerR1Module3Sector4",
                    "UTaXLayerR1Module3Sector5",  "UTaXLayerR1Module3Sector6",  "UTaXLayerR1Module3Sector7",
                    "UTaXLayerR1Module3Sector8",  "UTaXLayerR1Module3Sector9",  "UTaXLayerR1Module4Sector1",
                    "UTaXLayerR1Module4Sector10", "UTaXLayerR1Module4Sector11", "UTaXLayerR1Module4Sector12",
                    "UTaXLayerR1Module4Sector13", "UTaXLayerR1Module4Sector14", "UTaXLayerR1Module4Sector2",
                    "UTaXLayerR1Module4Sector3",  "UTaXLayerR1Module4Sector4",  "UTaXLayerR1Module4Sector5",
                    "UTaXLayerR1Module4Sector6",  "UTaXLayerR1Module4Sector7",  "UTaXLayerR1Module4Sector8",
                    "UTaXLayerR1Module4Sector9",  "UTaXLayerR1Module5Sector1",  "UTaXLayerR1Module5Sector10",
                    "UTaXLayerR1Module5Sector11", "UTaXLayerR1Module5Sector12", "UTaXLayerR1Module5Sector13",
                    "UTaXLayerR1Module5Sector14", "UTaXLayerR1Module5Sector2",  "UTaXLayerR1Module5Sector3",
                    "UTaXLayerR1Module5Sector4",  "UTaXLayerR1Module5Sector5",  "UTaXLayerR1Module5Sector6",
                    "UTaXLayerR1Module5Sector7",  "UTaXLayerR1Module5Sector8",  "UTaXLayerR1Module5Sector9",
                    "UTaXLayerR1Module6Sector1",  "UTaXLayerR1Module6Sector10", "UTaXLayerR1Module6Sector11",
                    "UTaXLayerR1Module6Sector12", "UTaXLayerR1Module6Sector13", "UTaXLayerR1Module6Sector14",
                    "UTaXLayerR1Module6Sector2",  "UTaXLayerR1Module6Sector3",  "UTaXLayerR1Module6Sector4",
                    "UTaXLayerR1Module6Sector5",  "UTaXLayerR1Module6Sector6",  "UTaXLayerR1Module6Sector7",
                    "UTaXLayerR1Module6Sector8",  "UTaXLayerR1Module6Sector9",  "UTaXLayerR2Module1Sector1",
                    "UTaXLayerR2Module1Sector10", "UTaXLayerR2Module1Sector11", "UTaXLayerR2Module1Sector12",
                    "UTaXLayerR2Module1Sector13", "UTaXLayerR2Module1Sector14", "UTaXLayerR2Module1Sector15",
                    "UTaXLayerR2Module1Sector16", "UTaXLayerR2Module1Sector17", "UTaXLayerR2Module1Sector18",
                    "UTaXLayerR2Module1Sector2",  "UTaXLayerR2Module1Sector3",  "UTaXLayerR2Module1Sector4",
                    "UTaXLayerR2Module1Sector5",  "UTaXLayerR2Module1Sector6",  "UTaXLayerR2Module1Sector7",
                    "UTaXLayerR2Module1Sector8",  "UTaXLayerR2Module1Sector9",  "UTaXLayerR2Module2Sector1",
                    "UTaXLayerR2Module2Sector10", "UTaXLayerR2Module2Sector11", "UTaXLayerR2Module2Sector12",
                    "UTaXLayerR2Module2Sector13", "UTaXLayerR2Module2Sector14", "UTaXLayerR2Module2Sector15",
                    "UTaXLayerR2Module2Sector16", "UTaXLayerR2Module2Sector17", "UTaXLayerR2Module2Sector18",
                    "UTaXLayerR2Module2Sector19", "UTaXLayerR2Module2Sector2",  "UTaXLayerR2Module2Sector20",
                    "UTaXLayerR2Module2Sector21", "UTaXLayerR2Module2Sector22", "UTaXLayerR2Module2Sector3",
                    "UTaXLayerR2Module2Sector4",  "UTaXLayerR2Module2Sector5",  "UTaXLayerR2Module2Sector6",
                    "UTaXLayerR2Module2Sector7",  "UTaXLayerR2Module2Sector8",  "UTaXLayerR2Module2Sector9",
                    "UTaXLayerR2Module3Sector1",  "UTaXLayerR2Module3Sector10", "UTaXLayerR2Module3Sector11",
                    "UTaXLayerR2Module3Sector12", "UTaXLayerR2Module3Sector13", "UTaXLayerR2Module3Sector14",
                    "UTaXLayerR2Module3Sector15", "UTaXLayerR2Module3Sector16", "UTaXLayerR2Module3Sector17",
                    "UTaXLayerR2Module3Sector18", "UTaXLayerR2Module3Sector19", "UTaXLayerR2Module3Sector2",
                    "UTaXLayerR2Module3Sector20", "UTaXLayerR2Module3Sector21", "UTaXLayerR2Module3Sector22",
                    "UTaXLayerR2Module3Sector3",  "UTaXLayerR2Module3Sector4",  "UTaXLayerR2Module3Sector5",
                    "UTaXLayerR2Module3Sector6",  "UTaXLayerR2Module3Sector7",  "UTaXLayerR2Module3Sector8",
                    "UTaXLayerR2Module3Sector9",  "UTaXLayerR2Module4Sector1",  "UTaXLayerR2Module4Sector10",
                    "UTaXLayerR2Module4Sector11", "UTaXLayerR2Module4Sector12", "UTaXLayerR2Module4Sector13",
                    "UTaXLayerR2Module4Sector14", "UTaXLayerR2Module4Sector15", "UTaXLayerR2Module4Sector16",
                    "UTaXLayerR2Module4Sector17", "UTaXLayerR2Module4Sector18", "UTaXLayerR2Module4Sector2",
                    "UTaXLayerR2Module4Sector3",  "UTaXLayerR2Module4Sector4",  "UTaXLayerR2Module4Sector5",
                    "UTaXLayerR2Module4Sector6",  "UTaXLayerR2Module4Sector7",  "UTaXLayerR2Module4Sector8",
                    "UTaXLayerR2Module4Sector9",  "UTaXLayerR3Module1Sector1",  "UTaXLayerR3Module1Sector10",
                    "UTaXLayerR3Module1Sector11", "UTaXLayerR3Module1Sector12", "UTaXLayerR3Module1Sector13",
                    "UTaXLayerR3Module1Sector14", "UTaXLayerR3Module1Sector2",  "UTaXLayerR3Module1Sector3",
                    "UTaXLayerR3Module1Sector4",  "UTaXLayerR3Module1Sector5",  "UTaXLayerR3Module1Sector6",
                    "UTaXLayerR3Module1Sector7",  "UTaXLayerR3Module1Sector8",  "UTaXLayerR3Module1Sector9",
                    "UTaXLayerR3Module2Sector1",  "UTaXLayerR3Module2Sector10", "UTaXLayerR3Module2Sector11",
                    "UTaXLayerR3Module2Sector12", "UTaXLayerR3Module2Sector13", "UTaXLayerR3Module2Sector14",
                    "UTaXLayerR3Module2Sector2",  "UTaXLayerR3Module2Sector3",  "UTaXLayerR3Module2Sector4",
                    "UTaXLayerR3Module2Sector5",  "UTaXLayerR3Module2Sector6",  "UTaXLayerR3Module2Sector7",
                    "UTaXLayerR3Module2Sector8",  "UTaXLayerR3Module2Sector9",  "UTaXLayerR3Module3Sector1",
                    "UTaXLayerR3Module3Sector10", "UTaXLayerR3Module3Sector11", "UTaXLayerR3Module3Sector12",
                    "UTaXLayerR3Module3Sector13", "UTaXLayerR3Module3Sector14", "UTaXLayerR3Module3Sector2",
                    "UTaXLayerR3Module3Sector3",  "UTaXLayerR3Module3Sector4",  "UTaXLayerR3Module3Sector5",
                    "UTaXLayerR3Module3Sector6",  "UTaXLayerR3Module3Sector7",  "UTaXLayerR3Module3Sector8",
                    "UTaXLayerR3Module3Sector9",  "UTaXLayerR3Module4Sector1",  "UTaXLayerR3Module4Sector10",
                    "UTaXLayerR3Module4Sector11", "UTaXLayerR3Module4Sector12", "UTaXLayerR3Module4Sector13",
                    "UTaXLayerR3Module4Sector14", "UTaXLayerR3Module4Sector2",  "UTaXLayerR3Module4Sector3",
                    "UTaXLayerR3Module4Sector4",  "UTaXLayerR3Module4Sector5",  "UTaXLayerR3Module4Sector6",
                    "UTaXLayerR3Module4Sector7",  "UTaXLayerR3Module4Sector8",  "UTaXLayerR3Module4Sector9",
                    "UTaXLayerR3Module5Sector1",  "UTaXLayerR3Module5Sector10", "UTaXLayerR3Module5Sector11",
                    "UTaXLayerR3Module5Sector12", "UTaXLayerR3Module5Sector13", "UTaXLayerR3Module5Sector14",
                    "UTaXLayerR3Module5Sector2",  "UTaXLayerR3Module5Sector3",  "UTaXLayerR3Module5Sector4",
                    "UTaXLayerR3Module5Sector5",  "UTaXLayerR3Module5Sector6",  "UTaXLayerR3Module5Sector7",
                    "UTaXLayerR3Module5Sector8",  "UTaXLayerR3Module5Sector9",  "UTaXLayerR3Module6Sector1",
                    "UTaXLayerR3Module6Sector10", "UTaXLayerR3Module6Sector11", "UTaXLayerR3Module6Sector12",
                    "UTaXLayerR3Module6Sector13", "UTaXLayerR3Module6Sector14", "UTaXLayerR3Module6Sector2",
                    "UTaXLayerR3Module6Sector3",  "UTaXLayerR3Module6Sector4",  "UTaXLayerR3Module6Sector5",
                    "UTaXLayerR3Module6Sector6",  "UTaXLayerR3Module6Sector7",  "UTaXLayerR3Module6Sector8",
                    "UTaXLayerR3Module6Sector9",  "UTbVLayerR1Module1Sector1",  "UTbVLayerR1Module1Sector10",
                    "UTbVLayerR1Module1Sector11", "UTbVLayerR1Module1Sector12", "UTbVLayerR1Module1Sector13",
                    "UTbVLayerR1Module1Sector14", "UTbVLayerR1Module1Sector2",  "UTbVLayerR1Module1Sector3",
                    "UTbVLayerR1Module1Sector4",  "UTbVLayerR1Module1Sector5",  "UTbVLayerR1Module1Sector6",
                    "UTbVLayerR1Module1Sector7",  "UTbVLayerR1Module1Sector8",  "UTbVLayerR1Module1Sector9",
                    "UTbVLayerR1Module2Sector1",  "UTbVLayerR1Module2Sector10", "UTbVLayerR1Module2Sector11",
                    "UTbVLayerR1Module2Sector12", "UTbVLayerR1Module2Sector13", "UTbVLayerR1Module2Sector14",
                    "UTbVLayerR1Module2Sector2",  "UTbVLayerR1Module2Sector3",  "UTbVLayerR1Module2Sector4",
                    "UTbVLayerR1Module2Sector5",  "UTbVLayerR1Module2Sector6",  "UTbVLayerR1Module2Sector7",
                    "UTbVLayerR1Module2Sector8",  "UTbVLayerR1Module2Sector9",  "UTbVLayerR1Module3Sector1",
                    "UTbVLayerR1Module3Sector10", "UTbVLayerR1Module3Sector11", "UTbVLayerR1Module3Sector12",
                    "UTbVLayerR1Module3Sector13", "UTbVLayerR1Module3Sector14", "UTbVLayerR1Module3Sector2",
                    "UTbVLayerR1Module3Sector3",  "UTbVLayerR1Module3Sector4",  "UTbVLayerR1Module3Sector5",
                    "UTbVLayerR1Module3Sector6",  "UTbVLayerR1Module3Sector7",  "UTbVLayerR1Module3Sector8",
                    "UTbVLayerR1Module3Sector9",  "UTbVLayerR1Module4Sector1",  "UTbVLayerR1Module4Sector10",
                    "UTbVLayerR1Module4Sector11", "UTbVLayerR1Module4Sector12", "UTbVLayerR1Module4Sector13",
                    "UTbVLayerR1Module4Sector14", "UTbVLayerR1Module4Sector2",  "UTbVLayerR1Module4Sector3",
                    "UTbVLayerR1Module4Sector4",  "UTbVLayerR1Module4Sector5",  "UTbVLayerR1Module4Sector6",
                    "UTbVLayerR1Module4Sector7",  "UTbVLayerR1Module4Sector8",  "UTbVLayerR1Module4Sector9",
                    "UTbVLayerR1Module5Sector1",  "UTbVLayerR1Module5Sector10", "UTbVLayerR1Module5Sector11",
                    "UTbVLayerR1Module5Sector12", "UTbVLayerR1Module5Sector13", "UTbVLayerR1Module5Sector14",
                    "UTbVLayerR1Module5Sector2",  "UTbVLayerR1Module5Sector3",  "UTbVLayerR1Module5Sector4",
                    "UTbVLayerR1Module5Sector5",  "UTbVLayerR1Module5Sector6",  "UTbVLayerR1Module5Sector7",
                    "UTbVLayerR1Module5Sector8",  "UTbVLayerR1Module5Sector9",  "UTbVLayerR1Module6Sector1",
                    "UTbVLayerR1Module6Sector10", "UTbVLayerR1Module6Sector11", "UTbVLayerR1Module6Sector12",
                    "UTbVLayerR1Module6Sector13", "UTbVLayerR1Module6Sector14", "UTbVLayerR1Module6Sector2",
                    "UTbVLayerR1Module6Sector3",  "UTbVLayerR1Module6Sector4",  "UTbVLayerR1Module6Sector5",
                    "UTbVLayerR1Module6Sector6",  "UTbVLayerR1Module6Sector7",  "UTbVLayerR1Module6Sector8",
                    "UTbVLayerR1Module6Sector9",  "UTbVLayerR1Module7Sector1",  "UTbVLayerR1Module7Sector10",
                    "UTbVLayerR1Module7Sector11", "UTbVLayerR1Module7Sector12", "UTbVLayerR1Module7Sector13",
                    "UTbVLayerR1Module7Sector14", "UTbVLayerR1Module7Sector2",  "UTbVLayerR1Module7Sector3",
                    "UTbVLayerR1Module7Sector4",  "UTbVLayerR1Module7Sector5",  "UTbVLayerR1Module7Sector6",
                    "UTbVLayerR1Module7Sector7",  "UTbVLayerR1Module7Sector8",  "UTbVLayerR1Module7Sector9",
                    "UTbVLayerR2Module1Sector1",  "UTbVLayerR2Module1Sector10", "UTbVLayerR2Module1Sector11",
                    "UTbVLayerR2Module1Sector12", "UTbVLayerR2Module1Sector13", "UTbVLayerR2Module1Sector14",
                    "UTbVLayerR2Module1Sector15", "UTbVLayerR2Module1Sector16", "UTbVLayerR2Module1Sector17",
                    "UTbVLayerR2Module1Sector18", "UTbVLayerR2Module1Sector2",  "UTbVLayerR2Module1Sector3",
                    "UTbVLayerR2Module1Sector4",  "UTbVLayerR2Module1Sector5",  "UTbVLayerR2Module1Sector6",
                    "UTbVLayerR2Module1Sector7",  "UTbVLayerR2Module1Sector8",  "UTbVLayerR2Module1Sector9",
                    "UTbVLayerR2Module2Sector1",  "UTbVLayerR2Module2Sector10", "UTbVLayerR2Module2Sector11",
                    "UTbVLayerR2Module2Sector12", "UTbVLayerR2Module2Sector13", "UTbVLayerR2Module2Sector14",
                    "UTbVLayerR2Module2Sector15", "UTbVLayerR2Module2Sector16", "UTbVLayerR2Module2Sector17",
                    "UTbVLayerR2Module2Sector18", "UTbVLayerR2Module2Sector19", "UTbVLayerR2Module2Sector2",
                    "UTbVLayerR2Module2Sector20", "UTbVLayerR2Module2Sector21", "UTbVLayerR2Module2Sector22",
                    "UTbVLayerR2Module2Sector3",  "UTbVLayerR2Module2Sector4",  "UTbVLayerR2Module2Sector5",
                    "UTbVLayerR2Module2Sector6",  "UTbVLayerR2Module2Sector7",  "UTbVLayerR2Module2Sector8",
                    "UTbVLayerR2Module2Sector9",  "UTbVLayerR2Module3Sector1",  "UTbVLayerR2Module3Sector10",
                    "UTbVLayerR2Module3Sector11", "UTbVLayerR2Module3Sector12", "UTbVLayerR2Module3Sector13",
                    "UTbVLayerR2Module3Sector14", "UTbVLayerR2Module3Sector15", "UTbVLayerR2Module3Sector16",
                    "UTbVLayerR2Module3Sector17", "UTbVLayerR2Module3Sector18", "UTbVLayerR2Module3Sector19",
                    "UTbVLayerR2Module3Sector2",  "UTbVLayerR2Module3Sector20", "UTbVLayerR2Module3Sector21",
                    "UTbVLayerR2Module3Sector22", "UTbVLayerR2Module3Sector3",  "UTbVLayerR2Module3Sector4",
                    "UTbVLayerR2Module3Sector5",  "UTbVLayerR2Module3Sector6",  "UTbVLayerR2Module3Sector7",
                    "UTbVLayerR2Module3Sector8",  "UTbVLayerR2Module3Sector9",  "UTbVLayerR2Module4Sector1",
                    "UTbVLayerR2Module4Sector10", "UTbVLayerR2Module4Sector11", "UTbVLayerR2Module4Sector12",
                    "UTbVLayerR2Module4Sector13", "UTbVLayerR2Module4Sector14", "UTbVLayerR2Module4Sector15",
                    "UTbVLayerR2Module4Sector16", "UTbVLayerR2Module4Sector17", "UTbVLayerR2Module4Sector18",
                    "UTbVLayerR2Module4Sector2",  "UTbVLayerR2Module4Sector3",  "UTbVLayerR2Module4Sector4",
                    "UTbVLayerR2Module4Sector5",  "UTbVLayerR2Module4Sector6",  "UTbVLayerR2Module4Sector7",
                    "UTbVLayerR2Module4Sector8",  "UTbVLayerR2Module4Sector9",  "UTbVLayerR3Module1Sector1",
                    "UTbVLayerR3Module1Sector10", "UTbVLayerR3Module1Sector11", "UTbVLayerR3Module1Sector12",
                    "UTbVLayerR3Module1Sector13", "UTbVLayerR3Module1Sector14", "UTbVLayerR3Module1Sector2",
                    "UTbVLayerR3Module1Sector3",  "UTbVLayerR3Module1Sector4",  "UTbVLayerR3Module1Sector5",
                    "UTbVLayerR3Module1Sector6",  "UTbVLayerR3Module1Sector7",  "UTbVLayerR3Module1Sector8",
                    "UTbVLayerR3Module1Sector9",  "UTbVLayerR3Module2Sector1",  "UTbVLayerR3Module2Sector10",
                    "UTbVLayerR3Module2Sector11", "UTbVLayerR3Module2Sector12", "UTbVLayerR3Module2Sector13",
                    "UTbVLayerR3Module2Sector14", "UTbVLayerR3Module2Sector2",  "UTbVLayerR3Module2Sector3",
                    "UTbVLayerR3Module2Sector4",  "UTbVLayerR3Module2Sector5",  "UTbVLayerR3Module2Sector6",
                    "UTbVLayerR3Module2Sector7",  "UTbVLayerR3Module2Sector8",  "UTbVLayerR3Module2Sector9",
                    "UTbVLayerR3Module3Sector1",  "UTbVLayerR3Module3Sector10", "UTbVLayerR3Module3Sector11",
                    "UTbVLayerR3Module3Sector12", "UTbVLayerR3Module3Sector13", "UTbVLayerR3Module3Sector14",
                    "UTbVLayerR3Module3Sector2",  "UTbVLayerR3Module3Sector3",  "UTbVLayerR3Module3Sector4",
                    "UTbVLayerR3Module3Sector5",  "UTbVLayerR3Module3Sector6",  "UTbVLayerR3Module3Sector7",
                    "UTbVLayerR3Module3Sector8",  "UTbVLayerR3Module3Sector9",  "UTbVLayerR3Module4Sector1",
                    "UTbVLayerR3Module4Sector10", "UTbVLayerR3Module4Sector11", "UTbVLayerR3Module4Sector12",
                    "UTbVLayerR3Module4Sector13", "UTbVLayerR3Module4Sector14", "UTbVLayerR3Module4Sector2",
                    "UTbVLayerR3Module4Sector3",  "UTbVLayerR3Module4Sector4",  "UTbVLayerR3Module4Sector5",
                    "UTbVLayerR3Module4Sector6",  "UTbVLayerR3Module4Sector7",  "UTbVLayerR3Module4Sector8",
                    "UTbVLayerR3Module4Sector9",  "UTbVLayerR3Module5Sector1",  "UTbVLayerR3Module5Sector10",
                    "UTbVLayerR3Module5Sector11", "UTbVLayerR3Module5Sector12", "UTbVLayerR3Module5Sector13",
                    "UTbVLayerR3Module5Sector14", "UTbVLayerR3Module5Sector2",  "UTbVLayerR3Module5Sector3",
                    "UTbVLayerR3Module5Sector4",  "UTbVLayerR3Module5Sector5",  "UTbVLayerR3Module5Sector6",
                    "UTbVLayerR3Module5Sector7",  "UTbVLayerR3Module5Sector8",  "UTbVLayerR3Module5Sector9",
                    "UTbVLayerR3Module6Sector1",  "UTbVLayerR3Module6Sector10", "UTbVLayerR3Module6Sector11",
                    "UTbVLayerR3Module6Sector12", "UTbVLayerR3Module6Sector13", "UTbVLayerR3Module6Sector14",
                    "UTbVLayerR3Module6Sector2",  "UTbVLayerR3Module6Sector3",  "UTbVLayerR3Module6Sector4",
                    "UTbVLayerR3Module6Sector5",  "UTbVLayerR3Module6Sector6",  "UTbVLayerR3Module6Sector7",
                    "UTbVLayerR3Module6Sector8",  "UTbVLayerR3Module6Sector9",  "UTbVLayerR3Module7Sector1",
                    "UTbVLayerR3Module7Sector10", "UTbVLayerR3Module7Sector11", "UTbVLayerR3Module7Sector12",
                    "UTbVLayerR3Module7Sector13", "UTbVLayerR3Module7Sector14", "UTbVLayerR3Module7Sector2",
                    "UTbVLayerR3Module7Sector3",  "UTbVLayerR3Module7Sector4",  "UTbVLayerR3Module7Sector5",
                    "UTbVLayerR3Module7Sector6",  "UTbVLayerR3Module7Sector7",  "UTbVLayerR3Module7Sector8",
                    "UTbVLayerR3Module7Sector9",  "UTbXLayerR1Module1Sector1",  "UTbXLayerR1Module1Sector10",
                    "UTbXLayerR1Module1Sector11", "UTbXLayerR1Module1Sector12", "UTbXLayerR1Module1Sector13",
                    "UTbXLayerR1Module1Sector14", "UTbXLayerR1Module1Sector2",  "UTbXLayerR1Module1Sector3",
                    "UTbXLayerR1Module1Sector4",  "UTbXLayerR1Module1Sector5",  "UTbXLayerR1Module1Sector6",
                    "UTbXLayerR1Module1Sector7",  "UTbXLayerR1Module1Sector8",  "UTbXLayerR1Module1Sector9",
                    "UTbXLayerR1Module2Sector1",  "UTbXLayerR1Module2Sector10", "UTbXLayerR1Module2Sector11",
                    "UTbXLayerR1Module2Sector12", "UTbXLayerR1Module2Sector13", "UTbXLayerR1Module2Sector14",
                    "UTbXLayerR1Module2Sector2",  "UTbXLayerR1Module2Sector3",  "UTbXLayerR1Module2Sector4",
                    "UTbXLayerR1Module2Sector5",  "UTbXLayerR1Module2Sector6",  "UTbXLayerR1Module2Sector7",
                    "UTbXLayerR1Module2Sector8",  "UTbXLayerR1Module2Sector9",  "UTbXLayerR1Module3Sector1",
                    "UTbXLayerR1Module3Sector10", "UTbXLayerR1Module3Sector11", "UTbXLayerR1Module3Sector12",
                    "UTbXLayerR1Module3Sector13", "UTbXLayerR1Module3Sector14", "UTbXLayerR1Module3Sector2",
                    "UTbXLayerR1Module3Sector3",  "UTbXLayerR1Module3Sector4",  "UTbXLayerR1Module3Sector5",
                    "UTbXLayerR1Module3Sector6",  "UTbXLayerR1Module3Sector7",  "UTbXLayerR1Module3Sector8",
                    "UTbXLayerR1Module3Sector9",  "UTbXLayerR1Module4Sector1",  "UTbXLayerR1Module4Sector10",
                    "UTbXLayerR1Module4Sector11", "UTbXLayerR1Module4Sector12", "UTbXLayerR1Module4Sector13",
                    "UTbXLayerR1Module4Sector14", "UTbXLayerR1Module4Sector2",  "UTbXLayerR1Module4Sector3",
                    "UTbXLayerR1Module4Sector4",  "UTbXLayerR1Module4Sector5",  "UTbXLayerR1Module4Sector6",
                    "UTbXLayerR1Module4Sector7",  "UTbXLayerR1Module4Sector8",  "UTbXLayerR1Module4Sector9",
                    "UTbXLayerR1Module5Sector1",  "UTbXLayerR1Module5Sector10", "UTbXLayerR1Module5Sector11",
                    "UTbXLayerR1Module5Sector12", "UTbXLayerR1Module5Sector13", "UTbXLayerR1Module5Sector14",
                    "UTbXLayerR1Module5Sector2",  "UTbXLayerR1Module5Sector3",  "UTbXLayerR1Module5Sector4",
                    "UTbXLayerR1Module5Sector5",  "UTbXLayerR1Module5Sector6",  "UTbXLayerR1Module5Sector7",
                    "UTbXLayerR1Module5Sector8",  "UTbXLayerR1Module5Sector9",  "UTbXLayerR1Module6Sector1",
                    "UTbXLayerR1Module6Sector10", "UTbXLayerR1Module6Sector11", "UTbXLayerR1Module6Sector12",
                    "UTbXLayerR1Module6Sector13", "UTbXLayerR1Module6Sector14", "UTbXLayerR1Module6Sector2",
                    "UTbXLayerR1Module6Sector3",  "UTbXLayerR1Module6Sector4",  "UTbXLayerR1Module6Sector5",
                    "UTbXLayerR1Module6Sector6",  "UTbXLayerR1Module6Sector7",  "UTbXLayerR1Module6Sector8",
                    "UTbXLayerR1Module6Sector9",  "UTbXLayerR1Module7Sector1",  "UTbXLayerR1Module7Sector10",
                    "UTbXLayerR1Module7Sector11", "UTbXLayerR1Module7Sector12", "UTbXLayerR1Module7Sector13",
                    "UTbXLayerR1Module7Sector14", "UTbXLayerR1Module7Sector2",  "UTbXLayerR1Module7Sector3",
                    "UTbXLayerR1Module7Sector4",  "UTbXLayerR1Module7Sector5",  "UTbXLayerR1Module7Sector6",
                    "UTbXLayerR1Module7Sector7",  "UTbXLayerR1Module7Sector8",  "UTbXLayerR1Module7Sector9",
                    "UTbXLayerR2Module1Sector1",  "UTbXLayerR2Module1Sector10", "UTbXLayerR2Module1Sector11",
                    "UTbXLayerR2Module1Sector12", "UTbXLayerR2Module1Sector13", "UTbXLayerR2Module1Sector14",
                    "UTbXLayerR2Module1Sector15", "UTbXLayerR2Module1Sector16", "UTbXLayerR2Module1Sector17",
                    "UTbXLayerR2Module1Sector18", "UTbXLayerR2Module1Sector2",  "UTbXLayerR2Module1Sector3",
                    "UTbXLayerR2Module1Sector4",  "UTbXLayerR2Module1Sector5",  "UTbXLayerR2Module1Sector6",
                    "UTbXLayerR2Module1Sector7",  "UTbXLayerR2Module1Sector8",  "UTbXLayerR2Module1Sector9",
                    "UTbXLayerR2Module2Sector1",  "UTbXLayerR2Module2Sector10", "UTbXLayerR2Module2Sector11",
                    "UTbXLayerR2Module2Sector12", "UTbXLayerR2Module2Sector13", "UTbXLayerR2Module2Sector14",
                    "UTbXLayerR2Module2Sector15", "UTbXLayerR2Module2Sector16", "UTbXLayerR2Module2Sector17",
                    "UTbXLayerR2Module2Sector18", "UTbXLayerR2Module2Sector19", "UTbXLayerR2Module2Sector2",
                    "UTbXLayerR2Module2Sector20", "UTbXLayerR2Module2Sector21", "UTbXLayerR2Module2Sector22",
                    "UTbXLayerR2Module2Sector3",  "UTbXLayerR2Module2Sector4",  "UTbXLayerR2Module2Sector5",
                    "UTbXLayerR2Module2Sector6",  "UTbXLayerR2Module2Sector7",  "UTbXLayerR2Module2Sector8",
                    "UTbXLayerR2Module2Sector9",  "UTbXLayerR2Module3Sector1",  "UTbXLayerR2Module3Sector10",
                    "UTbXLayerR2Module3Sector11", "UTbXLayerR2Module3Sector12", "UTbXLayerR2Module3Sector13",
                    "UTbXLayerR2Module3Sector14", "UTbXLayerR2Module3Sector15", "UTbXLayerR2Module3Sector16",
                    "UTbXLayerR2Module3Sector17", "UTbXLayerR2Module3Sector18", "UTbXLayerR2Module3Sector19",
                    "UTbXLayerR2Module3Sector2",  "UTbXLayerR2Module3Sector20", "UTbXLayerR2Module3Sector21",
                    "UTbXLayerR2Module3Sector22", "UTbXLayerR2Module3Sector3",  "UTbXLayerR2Module3Sector4",
                    "UTbXLayerR2Module3Sector5",  "UTbXLayerR2Module3Sector6",  "UTbXLayerR2Module3Sector7",
                    "UTbXLayerR2Module3Sector8",  "UTbXLayerR2Module3Sector9",  "UTbXLayerR2Module4Sector1",
                    "UTbXLayerR2Module4Sector10", "UTbXLayerR2Module4Sector11", "UTbXLayerR2Module4Sector12",
                    "UTbXLayerR2Module4Sector13", "UTbXLayerR2Module4Sector14", "UTbXLayerR2Module4Sector15",
                    "UTbXLayerR2Module4Sector16", "UTbXLayerR2Module4Sector17", "UTbXLayerR2Module4Sector18",
                    "UTbXLayerR2Module4Sector2",  "UTbXLayerR2Module4Sector3",  "UTbXLayerR2Module4Sector4",
                    "UTbXLayerR2Module4Sector5",  "UTbXLayerR2Module4Sector6",  "UTbXLayerR2Module4Sector7",
                    "UTbXLayerR2Module4Sector8",  "UTbXLayerR2Module4Sector9",  "UTbXLayerR3Module1Sector1",
                    "UTbXLayerR3Module1Sector10", "UTbXLayerR3Module1Sector11", "UTbXLayerR3Module1Sector12",
                    "UTbXLayerR3Module1Sector13", "UTbXLayerR3Module1Sector14", "UTbXLayerR3Module1Sector2",
                    "UTbXLayerR3Module1Sector3",  "UTbXLayerR3Module1Sector4",  "UTbXLayerR3Module1Sector5",
                    "UTbXLayerR3Module1Sector6",  "UTbXLayerR3Module1Sector7",  "UTbXLayerR3Module1Sector8",
                    "UTbXLayerR3Module1Sector9",  "UTbXLayerR3Module2Sector1",  "UTbXLayerR3Module2Sector10",
                    "UTbXLayerR3Module2Sector11", "UTbXLayerR3Module2Sector12", "UTbXLayerR3Module2Sector13",
                    "UTbXLayerR3Module2Sector14", "UTbXLayerR3Module2Sector2",  "UTbXLayerR3Module2Sector3",
                    "UTbXLayerR3Module2Sector4",  "UTbXLayerR3Module2Sector5",  "UTbXLayerR3Module2Sector6",
                    "UTbXLayerR3Module2Sector7",  "UTbXLayerR3Module2Sector8",  "UTbXLayerR3Module2Sector9",
                    "UTbXLayerR3Module3Sector1",  "UTbXLayerR3Module3Sector10", "UTbXLayerR3Module3Sector11",
                    "UTbXLayerR3Module3Sector12", "UTbXLayerR3Module3Sector13", "UTbXLayerR3Module3Sector14",
                    "UTbXLayerR3Module3Sector2",  "UTbXLayerR3Module3Sector3",  "UTbXLayerR3Module3Sector4",
                    "UTbXLayerR3Module3Sector5",  "UTbXLayerR3Module3Sector6",  "UTbXLayerR3Module3Sector7",
                    "UTbXLayerR3Module3Sector8",  "UTbXLayerR3Module3Sector9",  "UTbXLayerR3Module4Sector1",
                    "UTbXLayerR3Module4Sector10", "UTbXLayerR3Module4Sector11", "UTbXLayerR3Module4Sector12",
                    "UTbXLayerR3Module4Sector13", "UTbXLayerR3Module4Sector14", "UTbXLayerR3Module4Sector2",
                    "UTbXLayerR3Module4Sector3",  "UTbXLayerR3Module4Sector4",  "UTbXLayerR3Module4Sector5",
                    "UTbXLayerR3Module4Sector6",  "UTbXLayerR3Module4Sector7",  "UTbXLayerR3Module4Sector8",
                    "UTbXLayerR3Module4Sector9",  "UTbXLayerR3Module5Sector1",  "UTbXLayerR3Module5Sector10",
                    "UTbXLayerR3Module5Sector11", "UTbXLayerR3Module5Sector12", "UTbXLayerR3Module5Sector13",
                    "UTbXLayerR3Module5Sector14", "UTbXLayerR3Module5Sector2",  "UTbXLayerR3Module5Sector3",
                    "UTbXLayerR3Module5Sector4",  "UTbXLayerR3Module5Sector5",  "UTbXLayerR3Module5Sector6",
                    "UTbXLayerR3Module5Sector7",  "UTbXLayerR3Module5Sector8",  "UTbXLayerR3Module5Sector9",
                    "UTbXLayerR3Module6Sector1",  "UTbXLayerR3Module6Sector10", "UTbXLayerR3Module6Sector11",
                    "UTbXLayerR3Module6Sector12", "UTbXLayerR3Module6Sector13", "UTbXLayerR3Module6Sector14",
                    "UTbXLayerR3Module6Sector2",  "UTbXLayerR3Module6Sector3",  "UTbXLayerR3Module6Sector4",
                    "UTbXLayerR3Module6Sector5",  "UTbXLayerR3Module6Sector6",  "UTbXLayerR3Module6Sector7",
                    "UTbXLayerR3Module6Sector8",  "UTbXLayerR3Module6Sector9",  "UTbXLayerR3Module7Sector1",
                    "UTbXLayerR3Module7Sector10", "UTbXLayerR3Module7Sector11", "UTbXLayerR3Module7Sector12",
                    "UTbXLayerR3Module7Sector13", "UTbXLayerR3Module7Sector14", "UTbXLayerR3Module7Sector2",
                    "UTbXLayerR3Module7Sector3",  "UTbXLayerR3Module7Sector4",  "UTbXLayerR3Module7Sector5",
                    "UTbXLayerR3Module7Sector6",  "UTbXLayerR3Module7Sector7",  "UTbXLayerR3Module7Sector8",
                    "UTbXLayerR3Module7Sector9"};
  }

  // testing accessibility of noise values
  try {
    const auto& noiseValueCond = slice->get( utdet, LHCb::Detector::item_key( "NoiseValues" ) ).get<nlohmann::json>();
    // std::cout << noiseValueCond << std::endl;
    for ( const std::string& sector_name : sector_names ) {
      const auto& values = noiseValueCond[sector_name]["SectorNoise"];
      for ( std::size_t i = 0; i < values.size(); i++ ) {
        dd4hep::printout( dd4hep::DEBUG, sector_name + "SectorNoise", "values[%d]: %f", i, values[i].get<double>() );
      }
    }
  } catch ( const char* msg ) { allChecksOK = false; }
  dd4hep::printout( dd4hep::INFO, "UTCondition", "Successfully accessed NoiseValues condition" );

  ///////////////////////////////
  /* Test DeUTSector functions */
  ///////////////////////////////
  const auto aSector = ut.sides()[0].layers()[0].staves()[0].faces()[0].modules()[0].sectors()[0];

  // test set status
  int initialStatus =
      aSector.statusCondition()->get<nlohmann::json>()[aSector.nickname()]["SectorStatus"].get<double>();
  dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Initial sector status: %d", initialStatus );
  if ( initialStatus == 0 ) {
    aSector.setStatusCondition( "SectorStatus", 0, LHCb::Detector::UT::Status::Open );
  } else if ( initialStatus == 1 ) {
    aSector.setStatusCondition( "SectorStatus", 0, LHCb::Detector::UT::Status::OK );
  }
  int updatedStatus =
      aSector.statusCondition()->get<nlohmann::json>()[aSector.nickname()]["SectorStatus"].get<double>();
  dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Updated sector status: %d", updatedStatus );
  if ( initialStatus == updatedStatus ) { allChecksOK = false; }
  dd4hep::printout( dd4hep::INFO, "DeUTSector", "Tested setStatusCondition" );
  // test reading beetle status
  for ( int i = 0; i < 4; i++ ) {
    dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Beetle %d: %d", i, int( aSector.beetleStatus( i ) ) );
  }
  dd4hep::printout( dd4hep::INFO, "DeUTSector", "Tested beetleStatus( int )" );
  // test reading strip status
  for ( int i = 0; i < 10; i++ ) {
    dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Strip %d: %d", i, int( aSector.stripStatus()[i] ) );
  }
  dd4hep::printout( dd4hep::INFO, "DeUTSector", "Tested stripStatus()" );
  // test set beetle status
  std::vector<int> initialBeetleStatus =
      aSector.statusCondition()->get<nlohmann::json>()[aSector.nickname()]["SALTStatus"].get<std::vector<int>>();
  dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Initial beetle status: %d %d %d %d", initialBeetleStatus[0],
                    initialBeetleStatus[1], initialBeetleStatus[2], initialBeetleStatus[3] );
  if ( initialBeetleStatus[0] == 0 ) {
    aSector.setBeetleStatus( 0, LHCb::Detector::UT::Status::Open );
  } else if ( initialBeetleStatus[0] == 1 ) {
    aSector.setBeetleStatus( 0, LHCb::Detector::UT::Status::OK );
  }
  std::vector<int> updatedBeetleStatus =
      aSector.statusCondition()->get<nlohmann::json>()[aSector.nickname()]["SALTStatus"].get<std::vector<int>>();
  dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Updated beetle status: %d %d %d %d", updatedBeetleStatus[0],
                    updatedBeetleStatus[1], updatedBeetleStatus[2], updatedBeetleStatus[3] );
  if ( initialBeetleStatus[0] == updatedBeetleStatus[0] ) { allChecksOK = false; }
  dd4hep::printout( dd4hep::INFO, "DeUTSector", "Tested setBeetleStatus" );
  // test set sector status
  int initialSectorStatus =
      aSector.statusCondition()->get<nlohmann::json>()[aSector.nickname()]["SectorStatus"].get<int>();
  dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Initial sector status: %d", initialSectorStatus );
  if ( initialSectorStatus == 0 ) {
    dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Set sector status to Open" );
    aSector.setSectorStatus( LHCb::Detector::UT::Status::Open );
  } else if ( initialSectorStatus == 1 ) {
    dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Set sector status to OK" );
    aSector.setSectorStatus( LHCb::Detector::UT::Status::OK );
  }
  int updatedSectorStatus =
      aSector.statusCondition()->get<nlohmann::json>()[aSector.nickname()]["SectorStatus"].get<int>();
  dd4hep::printout( dd4hep::DEBUG, "DeUTSector", "Updated sector status: %d", updatedSectorStatus );
  if ( initialSectorStatus == updatedSectorStatus ) { allChecksOK = false; }
  dd4hep::printout( dd4hep::INFO, "DeUTSector", "Tested setSectorStatus" );

  // test reading measured eff
  dd4hep::printout( dd4hep::INFO, "DeUTSector", "Measured eff: %.3f", aSector.measEff() );
  // test reading noise condition
  if ( aSector.updateNoiseCondition() ) {
    dd4hep::printout( dd4hep::INFO, "DeUTSector", "Tested updateNoiseCondition" );
  } else {
    allChecksOK = false;
    dd4hep::printout( dd4hep::ERROR, "DeUTSector", "Failed updateNoiseCondition" );
  }
  // test updating status condition
  if ( aSector.updateStatusCondition() ) {
    dd4hep::printout( dd4hep::INFO, "DeUTSector", "Tested updateStatusCondition" );
  } else {
    allChecksOK = false;
    dd4hep::printout( dd4hep::ERROR, "DeUTSector", "Failed updateStatusCondition" );
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
DECLARE_APPLY( LHCb_TEST_load_deut_detailed, test_load_deut )
