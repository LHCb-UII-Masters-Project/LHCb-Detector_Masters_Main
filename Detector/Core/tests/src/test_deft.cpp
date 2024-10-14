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

#include "Detector/FT/DeFT.h"
#include "Detector/FT/FTChannelID.h"

#include <exception>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

static long test_load_DeFT( dd4hep::Detector& description, int argc, char** argv ) {

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

  auto               slice   = dds.get_slice( 100 );
  dd4hep::DetElement ftdet   = description.detector( "FT" );
  const auto&        cond    = slice->get( ftdet, LHCb::Detector::item_key( "ReadoutMap" ) ).get<nlohmann::json>();
  const auto&        version = cond["version"];
  std::cout << version << '\n';
  if ( version.get<int>() != 1 ) { throw std::runtime_error( "Readout map version should be 1" ); }

  // Finalizing the service and returning
  dds.finalize();
  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_load_deft_cond, test_load_DeFT )
