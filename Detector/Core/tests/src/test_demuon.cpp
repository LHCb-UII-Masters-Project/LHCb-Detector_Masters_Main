/***************************************************************************** \
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <DD4hep/Detector.h>
#include <DD4hep/Factories.h>
#include <DD4hep/Printout.h>
#include <DD4hep/VolumeManager.h>

#include <Core/DetectorDataService.h>
#include <Core/Keys.h>
#include <Core/MagneticFieldExtension.h>
#include <Core/MagneticFieldGridReader.h>
#include <Core/yaml_converters.h>
#include <Detector/Muon/DeMuon.h>
#include <Detector/Muon/DeMuonChamber.h>
#include <Detector/Muon/MuonConstants.h>
#include <Detector/Muon/MuonVolumeID.h>

#include <TGeoNavigator.h>
#include <exception>
#include <sstream>
#include <string>
#include <vector>

/*
 * Iterate over the DeMuon children and print all the GasGaps, useful for debugging
 */
void _iterate_muon( const LHCb::Detector::DeMuon& muon ) {

  std::array<std::string, LHCb::Detector::Muon::nStations>   StationName = {"M2", "M3", "M4", "M5"};
  std::array<std::string, LHCb::Detector::Muon::nPartitions> RegionName  = {
      "M2R1", "M2R2", "M2R3", "M2R4", "M3R1", "M3R2", "M3R3", "M3R4",
      "M4R1", "M4R2", "M4R3", "M4R4", "M5R1", "M5R2", "M5R3", "M5R4"};
  //
  std::array<unsigned int, LHCb::Detector::Muon::nRegions> ChPerRegion = {12, 24, 48, 192};

  const std::array<LHCb::Detector::DeMuonStation, 4> test_all_station = muon.getStations();
  //
  dd4hep::printout( dd4hep::INFO, "test_demuon", "station 2 Z position %f", test_all_station[0].ZPos() );
  dd4hep::printout( dd4hep::INFO, "test_demuon", "station 3 Z position %f", test_all_station[1].ZPos() );
  dd4hep::printout( dd4hep::INFO, "test_demuon", "station 4 Z position %f", test_all_station[2].ZPos() );
  dd4hep::printout( dd4hep::INFO, "test_demuon", "station 5 Z position %f", test_all_station[3].ZPos() );
  //
  //
  for ( const auto& istation : test_all_station ) {

    int iSta = istation.stationID() - LHCb::Detector::Muon::firstStation;

    if ( muon.getStationName( iSta ) != StationName[iSta] ) {
      dd4hep::printout( dd4hep::ERROR, "test_demuon", "problems with getStationName %i %s %s", iSta,
                        muon.getStationName( iSta ).c_str(), StationName[iSta].c_str() );
      return;
    }
    dd4hep::printout( dd4hep::DEBUG, "test_demuon", "station X, Y, Z position s% %f %f %f", StationName[iSta].c_str(),
                      istation.XPos(), istation.YPos(), istation.ZPos() );
    //
    const std::array<LHCb::Detector::DeMuonSide, 2> test_all_sides = istation.sides();

    for ( const auto& iside : test_all_sides ) {

      int iSid = iside.sideID();

      dd4hep::printout( dd4hep::DEBUG, "test_demuon", "side X, Y, Z position %f %f %f", iside.XPos(), iside.YPos(),
                        iside.ZPos() );

      const std::array<LHCb::Detector::DeMuonRegion, 4> test_all_regions = iside.regions();

      for ( const auto& iregion : test_all_regions ) {

        int iReg = iregion.regionID() - LHCb::Detector::Muon::firstRegion;

        if ( muon.getRegionName( iSta, iReg ) != RegionName[iSta * 4 + iReg] ) {
          dd4hep::printout( dd4hep::ERROR, "test_demuon", "problems with getRegionName %i %i %s %s", iSta, iReg,
                            muon.getRegionName( iSta, iReg ).c_str(), RegionName[iSta * 4 + iReg].c_str() );
          return;
        }
        //
        // test layout, tile and related code (one pad per station, region, quadrant)
        int                          nx = 1;
        int                          ny = 1;
        LHCb::Detector::Muon::Layout layout( LHCb::Detector::Muon::grid[iReg + 1].first,
                                             LHCb::Detector::Muon::grid[iReg + 1].second );
        // iSid == 0 ==> A side, quadrants = 0, 1
        if ( iSid == 0 ) {
          LHCb::Detector::Muon::TileID tile0( iSta, layout, iReg, 0, nx, ny );
          LHCb::Detector::Muon::TileID tile1( iSta, layout, iReg, 1, nx, ny );
        }
        // iSid == 1 ==> C side, quadrants = 3, 2
        else if ( iSid == 1 ) {
          LHCb::Detector::Muon::TileID tile3( iSta, layout, iReg, 3, nx, ny );
          LHCb::Detector::Muon::TileID tile2( iSta, layout, iReg, 2, nx, ny );
        }

        //
        dd4hep::printout( dd4hep::DEBUG, "test_demuon", "region X, Y, Z position %s %f %f %f",
                          RegionName[iSta * 4 + iReg].c_str(), iregion.XPos(), iregion.YPos(), iregion.ZPos() );
        //
        dd4hep::printout( dd4hep::DEBUG, "test_demuon", "number of chambers installed in this region %i",
                          iregion.chambersNumber() );
        //
        if ( ( iregion.chambersNumber() ) * 2 != ChPerRegion[iReg] ) {
          dd4hep::printout( dd4hep::ERROR, "test_demuon", "problems with chamber numbers %i %i %i %i", iSta, iReg,
                            ( iregion.chambersNumber() ) * 2, ChPerRegion[iReg] );
          return;
        }

        for ( unsigned int ich = 0; ich < iregion.chambersNumber(); ich++ ) {

          auto det_ch = iregion.getChamber( ich );

          // test listOfPhysChannels and related code
          const std::array<LHCb::Detector::DeMuonGasGap, 4> test_all_gaps = det_ch.gaps();
          unsigned int                                      i             = 0;
          double                                            zin = 0., zout = 0.;

          for ( const auto& igap : test_all_gaps ) {
            i++;
            if ( i == 2 ) {
              zin  = ( igap.ZPos() + 0.05 );
              zout = ( igap.ZPos() - 0.05 );
            }

            ROOT::Math::XYZPoint point{igap.XPos(), igap.YPos(), igap.ZPos()};
            auto                 sensid = muon.sensitiveVolumeID( point );
            std::string          cmp    = "ERROR";
            if ( sensid >= 0 ) { cmp = "OK"; }
            std::stringstream out;
            out << "CCC GasGap " << muon.getStationName( iSta ) << "/" << muon.getRegionName( iSta, iReg ) << "/"
                << det_ch.chamberNumber() << "/" << i << " " << igap.XPos() << " " << igap.YPos() << " " << igap.ZPos()
                << " " << igap.DX() << " " << igap.DY() << " " << igap.DZ() << " " << sensid << " " << cmp << '\n';
            dd4hep::printout( dd4hep::INFO, "test_demuon", out.str().c_str() );
          }
          ROOT::Math::XYZPoint entry = ROOT::Math::XYZPoint( det_ch.XPos(), det_ch.YPos(), zin );
          ROOT::Math::XYZPoint exit  = ROOT::Math::XYZPoint( det_ch.XPos(), det_ch.YPos(), zout );

          //
          auto listFE = muon.listOfPhysChannels( entry, exit, 0, 0 );
        }
      }
    }
  }
}

void _iterate( const dd4hep::DetElement& det, std::map<std::string, dd4hep::DetElement>& map ) {
  map[det.placementPath()] = det;
  dd4hep::printout( dd4hep::DEBUG, "test_demuon", "_iterate det.PlacementPath %s", det.placementPath().c_str() );
  for ( const auto& c : det.children() ) { _iterate( c.second, map ); }
}

/*
 * get GasGaps from the DetElement tree
 */
void _get_gas_gaps( const dd4hep::DetElement& det, std::map<std::string, dd4hep::DetElement>& map ) {
  if ( strcmp( det.name(), "GasGapLayer" ) == 0 ) { map[det.path()] = det; }
  for ( const auto& c : det.children() ) { _get_gas_gaps( c.second, map ); }
}

bool check_layer( const dd4hep::DetElement& det, const LHCb::Detector::DeMuon& muon,
                  std::map<std::string, dd4hep::DetElement>& map, [[maybe_unused]] int do_header ) {

  // Find the position of the center of the DetElement
  TGeoNavigator* nav = gGeoManager->GetCurrentNavigator();
  bool           ret = nav->cd( det.placementPath().c_str() );
  if ( !ret ) {
    dd4hep::printout( dd4hep::ERROR, "test_demuon", "Could not cd to %s", det.placementPath().c_str() );
    return 0; // Actually means error for plugins
  }
  double zero[3] = {0, 0, 0};
  double pos[3]  = {0, 0, 0};

  nav->LocalToMaster( zero, pos );
  ROOT::Math::XYZPoint point{pos[0], pos[1], pos[2]};

  std::stringstream out;
  out << "============================================\n\n";
  out << "Node " << nav->GetCurrentNode()->GetName() << '\n';
  out << "DetElement:" << det.path() << '\n';
  out << "Placement :" << det.placementPath() << '\n';
  out << "Position  :" << point << '\n';

  int packed = muon.sensitiveVolumeID( point );

  out << "Test of DeMuon sensitiveVolumeID\n";
  out << "--------------------------------\n\n";
  out << "Looking up muon.sensitiveVolumeID(): " << point << '\n';
  out << "Found: " << packed << " 0x" << std::hex << packed << "\n\n";
  out << std::dec;
  out << " DeMuon sensitiveVolumeID gapID     : " << muon.gapID( packed ) << '\n';
  out << " DeMuon sensitiveVolumeID chamberID : " << muon.chamberID( packed ) << '\n';
  out << " DeMuon sensitiveVolumeID regionID  : " << muon.regionID( packed ) << '\n';
  out << " DeMuon sensitiveVolumeID stationID : " << muon.stationID( packed ) << '\n';
  out << " DeMuon sensitiveVolumeID quadrantID: " << muon.quadrantID( packed ) << '\n';
  out << '\n';

  out << "Test of DD4hep volumeID()\n";
  out << "---------------------------------\n\n";
  auto n = nav->FindNode( point.x(), point.y(), point.z() );
  auto p = nav->GetPath();
  out << "TGeo Node path for point: " << point << " <" << p << "> is " << n << "\n";
  out << "path to det map size: " << map.size() << '\n';
  const dd4hep::DetElement& found = map[p];
  out << "DetElement path " << found.path() << '\n';
  out << "DD4Hep VolumeID " << found.volumeID() << " " << std::hex << found.volumeID() << '\n';
  unsigned int v = found.volumeID();
  out << std::dec;
  LHCb::Detector::MuonVolumeID mvid{v};
  out << "MuonVolumeID valid      : " << mvid.isValid() << '\n';
  out << "DeMuon dd4hep gapID     : " << mvid.gap().value() << '\n';
  out << "DeMuon dd4hep chamberID : " << mvid.chamber().value() << '\n';
  out << "DeMuon dd4hep regionID  : " << mvid.region().value() << '\n';
  out << "DeMuon dd4hep stationID : " << mvid.station().value() << '\n';
  out << "DeMuon dd4hep quadrantID: " << mvid.quadrant().value() << '\n';
  out << "DeMuon dd4hep sideID    : " << mvid.side().value() << '\n';

  // Uncomment to dump a CSV will all the gasGap info
  // {
  //   auto s = ',';
  //   if ( do_header == 1 ) {
  //     std::ofstream o( "detelements.csv", std::ios::trunc );
  //     o << "DetDelement path" << s << "x" << s << "y" << s << "z" << s << "sensitiveVolumeID" << s;
  //     o << "gapID" << s << "chamberID" << s << "regionID" << s << "stationID" << s << "quadrantID" << s;
  //     o << "DD4hep volumeID" << s << "DD4hep gapID" << s << "DD4hep chamberID" << s << "DD4hep regionID" << s
  //       << "DD4hep stationID" << s << "DD4hep quadrantID" << s << "DD4hep sideID" << s;
  //     o << "placementPath" << '\n';
  //   } else {
  //     std::ofstream o( "detelements.csv", std::ios::app );
  //     o << det.path() << s << point.x() << s << point.y() << s << point.z() << s << packed << s;
  //     o << muon.gapID( packed ) << s << muon.chamberID( packed ) << s << muon.regionID( packed ) << s
  //       << muon.stationID( packed ) << s << muon.quadrantID( packed ) << s;
  //     o << found.volumeID() << s << h.gapID << s << h.chamberID << s << h.regionID << s << h.stationID << s
  //       << h.quadrantID << s << h.sideID << s;
  //     o << det.placementPath() << '\n';
  //   }
  // }
  dd4hep::printout( dd4hep::INFO, "test_demuon", out.str().c_str() );
  bool retval = true;
  if ( ( muon.gapID( packed ) != mvid.gap().value() ) || ( muon.chamberID( packed ) != mvid.chamber().value() ) ||
       ( muon.regionID( packed ) != mvid.region().value() ) || ( muon.stationID( packed ) != mvid.station().value() ) ||
       ( muon.quadrantID( packed ) != mvid.quadrant().value() ) ) {
    retval = false;
  }
  return retval;
}

static long test_load_demuon( dd4hep::Detector& description, int argc, char** argv ) {

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
                 "     name:   factory name     LHCb_TEST_Magnet                      \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "     -fieldmappath <directory>  Directory containing the filed map files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << '\n';
    ::exit( EINVAL );
  }
  //
  std::array<std::string, LHCb::Detector::Muon::nStations>   StationName = {"M2", "M3", "M4", "M5"};
  std::array<std::string, LHCb::Detector::Muon::nPartitions> RegionName  = {
      "M2R1", "M2R2", "M2R3", "M2R4", "M3R1", "M3R2", "M3R3", "M3R4",
      "M4R1", "M4R2", "M4R3", "M4R4", "M5R1", "M5R2", "M5R3", "M5R4"};
  //
  std::array<unsigned int, LHCb::Detector::Muon::nRegions> ChPerRegion = {12, 24, 48, 192};

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, field_map_path );
  std::vector<std::string>            detector_list{"/world", "Magnet", "Muon"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  auto                          slice   = dds.get_slice( 100 );
  dd4hep::DetElement            muondet = description.detector( "Muon" );
  const LHCb::Detector::DeMuon& muon    = slice->get( muondet, LHCb::Detector::Keys::deKey );
  //
  //
  const std::array<LHCb::Detector::DeMuonStation, 4> test_all_station = muon.getStations();
  //
  dd4hep::printout( dd4hep::DEBUG, "test_demuon", "station 2 Z position %f", test_all_station[0].ZPos() );
  dd4hep::printout( dd4hep::DEBUG, "test_demuon", "station 3 Z position %f", test_all_station[1].ZPos() );
  dd4hep::printout( dd4hep::DEBUG, "test_demuon", "station 4 Z position %f", test_all_station[2].ZPos() );
  dd4hep::printout( dd4hep::DEBUG, "test_demuon", "station 5 Z position %f", test_all_station[3].ZPos() );
  //
  //
  for ( const auto& istation : test_all_station ) {

    int iSta = istation.stationID() - LHCb::Detector::Muon::firstStation;

    if ( muon.getStationName( iSta ) != StationName[iSta] ) {
      dd4hep::printout( dd4hep::ERROR, "test_demuon", "problems with getStationName %i %s %s", iSta,
                        muon.getStationName( iSta ).c_str(), StationName[iSta].c_str() );
      return 0;
    }
    dd4hep::printout( dd4hep::DEBUG, "test_demuon", "station X, Y, Z position s% %f %f %f", StationName[iSta].c_str(),
                      istation.XPos(), istation.YPos(), istation.ZPos() );
    const std::array<LHCb::Detector::DeMuonSide, 2> test_all_sides = istation.sides();

    for ( const auto& iside : test_all_sides ) {

      int iSid = iside.sideID();

      dd4hep::printout( dd4hep::DEBUG, "test_demuon", "side X, Y, Z position %f %f %f", iside.XPos(), iside.YPos(),
                        iside.ZPos() );
      const std::array<LHCb::Detector::DeMuonRegion, 4> test_all_regions = iside.regions();

      for ( const auto& iregion : test_all_regions ) {

        int iReg = iregion.regionID() - LHCb::Detector::Muon::firstRegion;

        if ( muon.getRegionName( iSta, iReg ) != RegionName[iSta * 4 + iReg] ) {
          dd4hep::printout( dd4hep::ERROR, "test_demuon", "problems with getRegionName %i %i %s %s", iSta, iReg,
                            muon.getRegionName( iSta, iReg ).c_str(), RegionName[iSta * 4 + iReg].c_str() );
          return 0;
        }
        //
        // test layout, tile and related code (one pad per station, region, quadrant)
        int                          nx = 1;
        int                          ny = 1;
        LHCb::Detector::Muon::Layout layout( LHCb::Detector::Muon::grid[iReg + 1].first,
                                             LHCb::Detector::Muon::grid[iReg + 1].second );
        // iSid == 0 ==> A side, quadrants = 0, 1
        if ( iSid == 0 ) {
          LHCb::Detector::Muon::TileID tile0( iSta, layout, iReg, 0, nx, ny );
          LHCb::Detector::Muon::TileID tile1( iSta, layout, iReg, 1, nx, ny );
        }
        // iSid == 1 ==> C side, quadrants = 3, 2
        else if ( iSid == 1 ) {
          LHCb::Detector::Muon::TileID tile3( iSta, layout, iReg, 3, nx, ny );
          LHCb::Detector::Muon::TileID tile2( iSta, layout, iReg, 2, nx, ny );
        }

        //
        dd4hep::printout( dd4hep::DEBUG, "test_demuon", "region X, Y, Z position %s %f %f %f",
                          RegionName[iSta * 4 + iReg].c_str(), iregion.XPos(), iregion.YPos(), iregion.ZPos() );
        dd4hep::printout( dd4hep::DEBUG, "test_demuon", "number of chambers installed in this region %i",
                          iregion.chambersNumber() );
        if ( ( iregion.chambersNumber() ) * 2 != ChPerRegion[iReg] ) {
          dd4hep::printout( dd4hep::ERROR, "test_demuon", "problems with chamber numbers %i %i %i %i", iSta, iReg,
                            ( iregion.chambersNumber() ) * 2, ChPerRegion[iReg] );
          return 0;
        }

        for ( unsigned int ich = 0; ich < iregion.chambersNumber(); ich++ ) {

          auto det_ch = iregion.getChamber( ich );

          // test listOfPhysChannels and related code
          const std::array<LHCb::Detector::DeMuonGasGap, 4> test_all_gaps = det_ch.gaps();
          unsigned int                                      i             = 0;
          double                                            zin = 0., zout = 0.;

          for ( const auto& igap : test_all_gaps ) {
            i++;
            if ( i == 2 ) {
              zin  = ( igap.ZPos() + 0.05 );
              zout = ( igap.ZPos() - 0.05 );
            }
          }
          ROOT::Math::XYZPoint entry = ROOT::Math::XYZPoint( det_ch.XPos(), det_ch.YPos(), zin );
          ROOT::Math::XYZPoint exit  = ROOT::Math::XYZPoint( det_ch.XPos(), det_ch.YPos(), zout );
          //
          auto listFE = muon.listOfPhysChannels( entry, exit, 0, 0 );
        }
      }
    }
  }

  // Finalizing the service and returning
  dds.finalize();
  return 1; // DD4hep plugins have to return 1 in case of success
}
DECLARE_APPLY( LHCb_TEST_load_demuon_detailed, test_load_demuon )

/**
 *  This plugin iterates of the Muon GasGaps and checks their volume IDs.
 * To run it the -volmgr option is needed, e.g.
 *
 * ./Detector/build.x86_64_v2-centos7-gcc11-dbg/bin/run geoPluginRun -volmgr
 * -input ./Detector/compact/run3/trunk/LHCb.xml -plugin LHCb_TEST_demuon_volumeid
 * -conditions file:./Detector/tests/ConditionsIOV/
 * -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf
 *
 */
static long test_demuon_volumeid( dd4hep::Detector& description, int argc, char** argv ) {

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
                 "     name:   factory name     LHCb_TEST_Magnet                      \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
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
  std::vector<std::string>            detector_list{"/world", "Magnet", "Muon"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  auto               slice   = dds.get_slice( 100 );
  dd4hep::DetElement muondet = description.detector( "Muon" );

  // Build the physical path to DetElement map
  std::map<std::string, dd4hep::DetElement> map;
  _iterate( muondet, map );

  const LHCb::Detector::DeMuon& muon = slice->get( muondet, LHCb::Detector::Keys::deKey );

  // Uncomment to Dump the list of GasGaps getting them by iterating on the DeMuon children and checking the ID
  //_iterate_muon( muon );

  ROOT::Math::XYZPoint puntoErika = ROOT::Math::XYZPoint( -2756.919678, -3948.525879, 18800.011719 );
  int                  packed     = muon.sensitiveVolumeID( puntoErika );

  std::stringstream out;
  out << "Test of DeMuon sensitiveVolumeID\n";
  out << "================================\n\n";
  out << "Looking up muon.sensitiveVolumeID(): " << puntoErika << '\n';
  out << "Found: " << packed << " 0x" << std::hex << packed << "\n\n";
  out << std::dec;
  out << " DeMuon sensitiveVolumeID gapID     : " << muon.gapID( packed ) << '\n';
  out << " DeMuon sensitiveVolumeID chamberID : " << muon.chamberID( packed ) << '\n';
  out << " DeMuon sensitiveVolumeID regionID  : " << muon.regionID( packed ) << '\n';
  out << " DeMuon sensitiveVolumeID stationID : " << muon.stationID( packed ) << '\n';
  out << " DeMuon sensitiveVolumeID quadrantID: " << muon.quadrantID( packed ) << '\n';
  out << '\n';

  out << "Test of DD4hep volumeID()\n";
  out << "================================\n\n";
  auto nav = gGeoManager->GetCurrentNavigator();
  auto n   = nav->FindNode( puntoErika.x(), puntoErika.y(), puntoErika.z() );
  auto p   = nav->GetPath();
  out << "TGeo Node path for point: " << puntoErika << " <" << p << "> is " << n << "\n";
  out << "path to det map size: " << map.size() << '\n';
  //
  dd4hep::DetElement found = map[p];
  out << "DetElement path " << found.path() << '\n';
  //
  out << "DD4Hep VolumeID " << found.volumeID() << " " << std::hex << found.volumeID() << '\n';
  //
  unsigned int                 v = found.volumeID();
  LHCb::Detector::MuonVolumeID mvid{v};
  out << std::dec;
  out << "DeMuon dd4hep gapID     : " << mvid.gap().value() << '\n';
  out << "DeMuon dd4hep chamberID : " << mvid.chamber().value() << '\n';
  out << "DeMuon dd4hep regionID  : " << mvid.region().value() << '\n';
  out << "DeMuon dd4hep stationID : " << mvid.station().value() << '\n';
  out << "DeMuon dd4hep sideID    : " << mvid.side().value() << '\n';
  //
  dd4hep::printout( dd4hep::INFO, "test_demuon", out.str().c_str() );

  // Getting all the gas gaps
  std::map<std::string, dd4hep::DetElement> gasgapsmaps;
  _get_gas_gaps( muondet, gasgapsmaps );

  // Now iterating on all GasGaps to check that the
  // DD4hep ID matches the output of sensitiveVolumeID
  int  do_header = 1;
  bool allok     = true;
  for ( auto const& [p, d] : gasgapsmaps ) {
    bool ret = check_layer( d, muon, map, do_header++ );
    if ( !ret ) {
      allok = false;
      dd4hep::printout( dd4hep::INFO, "test_demuon", "Error with GapGap: %s", d.path().c_str() );
    }
  }

  // Finalizing the service and returning
  dds.finalize();
  return allok ? 1 : 0; // DD4hep plugins have to return 1 in case of success
}
DECLARE_APPLY( LHCb_TEST_demuon_volumeid, test_demuon_volumeid )
