/*****************************************************************************\
* (c) Copyright 2021-2022 CERN for the benefit of the LHCb Collaboration      *
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

#include "Detector/FT/DeFT.h"
#include "Detector/Muon/DeMuon.h"
#include "Detector/Rich1/DetElemAccess/DeRich1.h"
#include "Detector/Rich2/DetElemAccess/DeRich2.h"
#include "Detector/UT/DeUT.h"
#include "Detector/VP/DeVP.h"

#include <exception>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

void _parse_args( int argc, char** argv, std::string plugin_name, std::string& conditions,
                  std::string& field_map_path ) {
  // Boiler plate for the plugin, to be improved/factorized

  bool help = false;

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
    std::cout << "Usage: -plugin <name> -arg [-arg] \n"
                 "     name:   factory name  "
              << plugin_name
              << "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "     -fieldmappath <directory>  Directory containing the filed map files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << "\n";
    ::exit( EINVAL );
  }
}

/*
 Map containing the detector names based on type.
*/
namespace {
  std::map<std::type_index, std::string> typename_map = {
      {std::type_index( typeid( LHCb::Detector::DeVP ) ), std::string( "VP" )},
      {std::type_index( typeid( LHCb::Detector::DeFT ) ), std::string( "FT" )},
      {std::type_index( typeid( LHCb::Detector::UT::DeUT ) ), std::string( "UT" )},
      {std::type_index( typeid( LHCb::Detector::DeRich1 ) ), std::string( "Rich1" )},
      {std::type_index( typeid( LHCb::Detector::DeRich2 ) ), std::string( "Rich2" )},
      {std::type_index( typeid( LHCb::Detector::DeMuon ) ), std::string( "Muon" )}};
}

template <typename DE>
static long test_load_detelement( dd4hep::Detector& description, int argc, char** argv ) {

  std::string conditions;
  std::string field_map_path = "";
  _parse_args( argc, argv, "LHCb_TEST_load_detelement", conditions, field_map_path );

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, field_map_path );
  std::vector<std::string>            detector_list{"/world", "Magnet", typename_map[std::type_index( typeid( DE ) )]};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  int                slice1_id = 100;
  auto               slice     = dds.get_slice( slice1_id );
  dd4hep::DetElement det       = description.detector( typename_map[std::type_index( typeid( DE ) )] );
  const DE&          detelem   = slice->get( det, LHCb::Detector::Keys::deKey );

  std::cout << typename_map[std::type_index( typeid( DE ) )]
            << " (0, 0, 0) position in global ref.: " << detelem.toGlobal( ROOT::Math::XYZPoint( 0, 0, 0 ) ) << "\n";

  // Finalizing the service and returning
  dds.finalize();
  return 1; // Plugins return 1 when they are successful
}

DECLARE_APPLY( LHCb_TEST_load_deft, (test_load_detelement<LHCb::Detector::DeFT>))
DECLARE_APPLY( LHCb_TEST_load_devp, (test_load_detelement<LHCb::Detector::DeVP>))
DECLARE_APPLY( LHCb_TEST_load_deut, (test_load_detelement<LHCb::Detector::UT::DeUT>))
DECLARE_APPLY( LHCb_TEST_load_derich1, (test_load_detelement<LHCb::Detector::DeRich1>))
DECLARE_APPLY( LHCb_TEST_load_derich2, (test_load_detelement<LHCb::Detector::DeRich2>))
DECLARE_APPLY( LHCb_TEST_load_demuon, (test_load_detelement<LHCb::Detector::DeMuon>))
