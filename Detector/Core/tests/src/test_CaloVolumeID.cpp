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
#include <Detector/Calo/DeCalorimeter.h>

#include <Detector/Calo/CaloCellID.h>
#include <TGeoNavigator.h>
#include <exception>
#include <sstream>
#include <string>
#include <vector>

/*
 * Loop on all DetElements to find a Calo cell (i.e the bottomo of the DetElement hierarchy).
 * When a Calo cell is found, retrieve the Calo::CellID starting from:
 * 1) the cell position;
 * 2) the dd4hep::DDsegmentation::VolumeID of the cell.
 * The two CellIDs must be equal
 */
void _iterate_and_test( const dd4hep::DetElement& det, const LHCb::Detector::Calo::DeCalorimeter& calo,
                        bool& testStatus ) {
  auto children = det.children();
  if ( children.size() == 0 ) { // det is a Calo cell
    // Perform the test
    dd4hep::Position     posLocal{0.0, 0.0, 0.0};
    dd4hep::Position     posGlobal = det.nominal().localToWorld( posLocal );
    ROOT::Math::XYZPoint pGlobal( posGlobal.x(), posGlobal.y(), posGlobal.z() );
    ROOT::Math::XYZPoint pointGlobal = LHCb::Detector::detail::toLHCbLengthUnits( pGlobal );
    dd4hep::printout( dd4hep::DEBUG, "test_decalorimeter_volumeid", "test global point: %.3f %.3f %.3f\n",
                      pointGlobal.x(), pointGlobal.y(), pointGlobal.z() );
    LHCb::Detector::Calo::CellID     cell_id1 = calo.Cell( pointGlobal );
    dd4hep::DDSegmentation::VolumeID vol_id   = det.volumeID();
    LHCb::Detector::Calo::CellID     cell_id2 = calo.cellIDfromVolumeID( vol_id );
    if ( cell_id1 != cell_id2 ) { // Failed test
      testStatus = false;
      dd4hep::printout( dd4hep::ERROR, "test_decalorimeter_volumeid",
                        "Mismatch for %s\n"
                        "\tTested global position: x = %.4f, y = %.4f, z = %.4\n"
                        "\tDeCalorimeter::Cell(x,y,z)                  returned: region %u, row %u, col: %u \n"
                        "\tvolumeID: %llu\n"
                        "\tDeCalorimeter::cellIDfromVolumeID(volumeID) returned: region %u, row %u, col: %u \n",
                        det.placementPath().data(), pointGlobal.x(), pointGlobal.y(), pointGlobal.z(), cell_id1.area(),
                        cell_id1.row(), cell_id1.col(), vol_id, cell_id2.area(), cell_id2.row(), cell_id2.col() );

    } else { // Passed test
      dd4hep::printout( dd4hep::DEBUG, "test_decalorimeter_volumeid",
                        "Match for %s\n"
                        "\tTested global position: x = %.4f, y = %.4f, z = %.4\n"
                        "\tvolumeID: %llu\n"
                        "\tregion %u, row %u, col: %u \n",
                        det.placementPath().data(), pointGlobal.x(), pointGlobal.y(), pointGlobal.z(), vol_id,
                        cell_id1.area(), cell_id1.row(), cell_id1.col() );
    }
  } else {                                   // det is not a Calo cell
    for ( const auto& c : det.children() ) { // Loop on the subDetElements of det and look for a Calo cell
      _iterate_and_test( c.second, calo, testStatus );
    }
  }
}

/*
 *  This plugin iterates of the Calo volumes and checks their volume IDs.
 *  To run it the -volmgr option is needed, e.g.
 *
 *  ./Detector/build.x86_64_v2-centos7-gcc11-dbg/bin/run geoPluginRun -volmgr
 *  -input ./Detector/compact/trunk/LHCb.xml -plugin LHCb_TEST_decalorimeter_volumeid
 *  -detector [Ecal,Hcal]
 *  -conditions file:./Detector/tests/ConditionsIOV/
 *  -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf
 */
static long test_decalorimeter_volumeid( dd4hep::Detector& description, int argc, char** argv ) {
  bool        help = false;
  std::string detector;
  std::string conditions     = "";
  std::string field_map_path = "";
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) )
        help = true;
      else if ( 0 == ::strncmp( "-detector", argv[i], 9 ) )
        detector = argv[++i];
      else if ( 0 == ::strncmp( "-conditions", argv[i], 11 ) )
        conditions = argv[++i];
      else if ( 0 == ::strncmp( "-fieldmappath", argv[i], 13 ) ) {
        field_map_path = argv[++i];
      } else
        help = true;
    }
  }
  if ( help || detector.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_decalorimeter_volumeid      \n"
                 "     -detector   <name>       Name of the sub-detector to analyze [Ecal, Hcal].  \n"
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
  std::vector<std::string>            detector_list{"/world", "Magnet", detector};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );
  auto                                       slice      = dds.get_slice( 100 );
  dd4hep::DetElement                         calodet    = description.detector( detector.data() );
  const LHCb::Detector::Calo::DeCalorimeter& calo       = slice->get( calodet, LHCb::Detector::Keys::deKey );
  bool                                       testStatus = true;
  _iterate_and_test( calodet, calo, testStatus );
  dds.finalize();
  return testStatus ? 1 : 0;
}
DECLARE_APPLY( LHCb_TEST_decalorimeter_volumeid, test_decalorimeter_volumeid )
