/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
/*

  Test that iterates through the DeFT DetElement tree
  To run:

  ./Detector/build.x86_64_v2-centos7-gcc11+dd4hep-dbg/bin/run geoPluginRun
      -input /home/lben/develop/geoinfo/Detector/compact/run3/trunk/LHCb.xml
      -plugin LHCb_TEST_DeFT_geo
      -conditions git:/cvmfs/lhcb.cern.ch/lib/lhcb/git-conddb/lhcb-conditions-database.git@master
      -fieldmappath /cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf

*/

#include "Core/DetectorDataService.h"
#include <Core/Keys.h>
#include <Core/MagneticFieldExtension.h>
#include <Core/MagneticFieldGridReader.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Factories.h>
#include <DD4hep/Printout.h>
#include <Detector/FT/DeFT.h>
#include <Detector/FT/FTChannelID.h>
#include <TGeoManager.h>
#include <exception>
#include <string>
#include <vector>

void det_info( dd4hep::DetElement det ) {

  std::cout << "Path:          " << det.path() << '\n';
  std::cout << "PlacementPath: " << det.placementPath() << '\n';
  std::cout << "Matrix: " << '\n';
  det.placement().matrix().Print();
  std::cout << '\n';

  if ( det.parent().isValid() ) { det_info( det.parent() ); }
}

void ft_mat_info( LHCb::Detector::DeFTMat mat ) {

  auto det = mat.detector();

  auto center = mat.toGlobal( ROOT::Math::XYZPoint{0, 0, 0} );
  auto x1     = mat.toGlobal( ROOT::Math::XYZPoint{1, 0, 0} );

  std::cout << "=======================\nProcessing: " << det.path() << ' ';
  std::cout << "Center: " << center << "\t\tX_unit_vector:" << ( x1 - center ) << '\n';

  det_info( det );

  // To find the related volumes in the ROOT TGeo:
  TGeoManager*   manager = det.volume()->GetGeoManager();
  TGeoNavigator* nav     = manager->GetCurrentNavigator();
  if ( nav->cd( det.placementPath().c_str() ) ) { std::cout << "TGeoNode path:" << nav->GetPath() << '\n'; }
}

/**
 * @brief Iterate over the DetElement tree to find the leaves
 *
 * N.B. We could just do a DeFT.applyToAllMats(easier), but the point of here was to double
 * check the tree...
 *
 * @param det The DetElement from the FT tree
 * @param ft the DeFT, used to get the DeFTMat from
 */
void process_detelement( const dd4hep::DetElement& det, const LHCb::Detector::DeFT& ft ) {

  // Processing this specific DetElement, print the position and orientation of the leaves of the tree

  if ( det.children().size() == 0 ) {
    // Finding the DetectorElement from the DeFT itself
    bool                    found = false;
    LHCb::Detector::DeFTMat mat;
    ft.applyToAllMats( [&det, &found, &mat]( LHCb::Detector::DeIOV child ) {
      if ( child.detector().path() == det.path() ) {
        found = true;
        mat   = child;
      }
    } );

    if ( found ) {
      ft_mat_info( mat );
    } else {
      std::cout << "NOT_FOUND ";
    }
    std::cout << '\n';

    // This does not work as the DeFTMat is not attached to the DetElement
    // const LHCb::Detector::DeFTMat& mat = slice->get(det, LHCb::Detector::Keys::deKey );
    // std::cout << mat.toGlobal(ROOT::Math::XYZPoint{0, 0, 0});
  }
  // To iterate over children, just un-comment
  // for ( const auto& c : det.children() ) { process_detelement( c.second, ft ); }
}

static long test_DeFT_geo( dd4hep::Detector& description, int argc, char** argv ) {

  // Boiler plate for the plugin, to be improved/factorized
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
                 "     name:   factory name     LHCb_TEST_load_deft_cond              \n"
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
  std::vector<std::string>            detector_list{"/world", "Magnet", "FT"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  auto                        slice = dds.get_slice( 100 );
  dd4hep::DetElement          ftdet = description.detector( "FT" );
  const LHCb::Detector::DeFT& ft    = slice->get( ftdet, LHCb::Detector::Keys::deKey );

  // Useful snippet to iterate over all conditions linked to the DetElement
  // const auto& conds = slice->get(det);
  //   for (const auto&c: conds) {
  //     if (!c.isValid()) {
  //       std::cout << "Invalid DetectorElement ";
  //     } else {
  //       std::cout << c.typeInfo().name() << "| ";
  //     }
  //   }

  process_detelement( ftdet, ft );

  // Finalizing the service and returning
  dds.finalize();
  return 1;
}
DECLARE_APPLY( LHCb_TEST_DeFT_geo, test_DeFT_geo )
