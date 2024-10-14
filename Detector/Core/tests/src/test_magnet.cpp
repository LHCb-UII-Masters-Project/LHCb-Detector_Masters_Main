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

#include "Detector/Magnet/DeMagnet.h"

#include <exception>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

static long test_magnet_load_condition( dd4hep::Detector& description, int argc, char** argv ) {

  bool        help = false;
  std::string conditions;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) )
        help = true;
      else if ( 0 == ::strncmp( "-conditions", argv[i], 4 ) )
        conditions = argv[++i];
      else
        help = true;
    }
  }
  if ( help || conditions.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_Magnet                      \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  LHCb::Magnet::setup_magnetic_field_extension( description, "/cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf" );
  std::vector<std::string>            detector_list{"/world", "Magnet"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  int                slice1_id = 100;
  auto               slice     = dds.get_slice( slice1_id );
  dd4hep::DetElement magnetdet = description.detector( "Magnet" );
  // const LHCb::Detector::DeMagnet& magnet    = slice->get( magnetdet, LHCb::Detector::Keys::deKey );
  const auto& cond   = slice->get( magnetdet, LHCb::Detector::item_key( "FieldMapFilesDown" ) ).get<nlohmann::json>();
  const auto& values = cond["Files"];
  for ( std::size_t i = 0; i < values.size(); i++ ) {
    dd4hep::printout( dd4hep::INFO, "test_magnet_load_condition", "values[%d]: %s", i,
                      values[i].get<std::string>().c_str() );
  }
  std::string file1 = values[0].get<std::string>();

  // Finalizing the service and returning
  dds.finalize();
  return file1 == "field.v5r0.c1.down.cdf";
}

DECLARE_APPLY( LHCb_TEST_Magnet_load_condition, test_magnet_load_condition )

static long test_magnet_load_field( dd4hep::Detector& description, int argc, char** argv ) {

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
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, field_map_path );
  std::vector<std::string>            detector_list{"/world", "Magnet"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  int                slice1_id = 100;
  auto               slice     = dds.get_slice( slice1_id );
  dd4hep::DetElement magnetdet = description.detector( "Magnet" );
  // const LHCb::Detector::DeMagnet& magnet    = slice->get( magnetdet, LHCb::Detector::Keys::deKey );

  using nlohmann::json;

  // Loading the FieldMap file
  const auto& cond   = slice->get( magnetdet, LHCb::Detector::item_key( "FieldMapFilesDown" ) ).get<json>();
  const auto& values = cond["Files"];
  for ( std::size_t i = 0; i < values.size(); i++ ) {
    dd4hep::printout( dd4hep::INFO, "test_Magnet", "values[%d]: %s", i, values[i].get<std::string>().c_str() );
  }
  const auto filenames = values.get<std::vector<std::string>>();

  // Loading the current value
  const auto& online_cond = slice->get( magnetdet, LHCb::Detector::item_key( "Magnet" ) ).get<json>();
  dd4hep::printout( dd4hep::INFO, "test_Magnet", "Current value: %f", online_cond["Current"].get<double>() );

  // Now that we have the filenames, load the field itself
  LHCb::Magnet::MagneticFieldGridReader reader{field_map_path};
  LHCb::Magnet::MagneticFieldGrid       fieldgrid;
  const auto sc = ( filenames.size() == 1 ? reader.readDC06File( filenames.front(), fieldgrid )
                                          : reader.readFiles( filenames, fieldgrid ) );

  if ( !sc ) {
    dd4hep::printout( dd4hep::ERROR, "test_magnet", "Error loading magnetic field map" );
    ::exit( EINVAL );
  }

  double maxdiff = 1e-5;
  auto   check   = [&maxdiff]( double val, double ref, std::string name ) {
    if ( abs( ( val - ref ) / val ) > maxdiff ) {
      dd4hep::printout( dd4hep::ERROR, "test_magnet", "Error with %s", name.c_str() );
      ::exit( EINVAL );
    }
  };

  const auto f = fieldgrid.fieldVectorLinearInterpolation( ROOT::Math::XYZPoint{-400, 0, 0} );
  // Field should be -1.18417e-07,-6.66836e-06,1.30723e-06 at that point...
  std::cout << "field:" << f << std::endl;
  check( f.X(), -1.18417e-07, "field map X component" );
  check( f.Y(), -6.66836e-06, "field map Y component" );
  check( f.Z(), 1.30723e-06, "field map Z component" );

  // Finalizing the service and returning
  dds.finalize();
  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_Magnet_load_field, test_magnet_load_field )

static long test_magnet_load_demagnet( dd4hep::Detector& description, int argc, char** argv ) {

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
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, field_map_path );
  std::vector<std::string>            detector_list{"/world", "Magnet"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  int                             slice1_id = 100;
  auto                            slice     = dds.get_slice( slice1_id );
  dd4hep::DetElement              magnetdet = description.detector( "Magnet" );
  const LHCb::Detector::DeMagnet& magnet    = slice->get( magnetdet, LHCb::Detector::Keys::deKey );

  double maxdiff = 1e-5;
  auto   check   = [&maxdiff]( double val, double ref, std::string name ) {
    if ( abs( ( val - ref ) / val ) > maxdiff ) {
      dd4hep::printout( dd4hep::ERROR, "test_magnet", "Error with %s", name.c_str() );
      ::exit( EINVAL );
    }
  };

  const auto f = magnet.fieldVector( ROOT::Math::XYZPoint{-400, 0, 0} );
  // Field should be -1.18417e-07,-6.66836e-06,1.30723e-06 at that point...
  check( f.X(), -1.18417e-07, "field map X component" );
  check( f.Y(), -6.66836e-06, "field map Y component" );
  check( f.Z(), 1.30723e-06, "field map Z component" );

  // Finalizing the service and returning
  dds.finalize();
  return 1; // Plugins return 1 when they are successful
}
DECLARE_APPLY( LHCb_TEST_Magnet_load_demagnet, test_magnet_load_demagnet )
