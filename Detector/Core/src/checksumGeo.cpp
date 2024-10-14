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
#include <boost/program_options.hpp>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

#include "Core/Checksum.h"
#include "Core/GeometryTools.h"
#include "DD4hep/Detector.h"
#include "TGeoManager.h"
#include "TGeoNode.h"

namespace po = boost::program_options;

/**
 * @brief templated method to save/print the checksums, as the collection
 * can be a map<string, string> or a map<string, long>
 *
 * @tparam C typ of the checksum collection
 * @param checksums collections with the paths and their checksums
 * @param output_file output file name
 */
template <typename C>
void save_checksums( C checksums, std::string output_file ) {
  if ( output_file == "-" ) {
    for ( auto& e : checksums ) { std::cout << e.first << ":" << e.second << std::endl; }
  } else {
    YAML::Emitter yaml;
    yaml << checksums;

    std::ofstream outfile( output_file, std::ofstream::trunc );
    outfile << yaml.c_str();
    outfile.close();
  }
}

/*
 * Simple tool to checksum a geometry file
 */

int main( int argc, char* argv[] ) {

  try {
    po::options_description visible{"visible"};
    visible.add_options()( "help,h", "Help" )( "maxlevel", po::value<int>()->default_value( 9 ),
                                               "Max depth for checksums for checksum printout" )(
        "outfile,o", po::value<std::string>()->default_value( "-" ),
        "Filename for the output" )( "time", po::bool_switch()->default_value( false ), "Display timing statistics" )(
        "checksum,c", po::value<std::string>()->default_value( "crc" ), "Checksum to use: crc" );

    po::options_description hidden{"hidden"};
    hidden.add_options()( "input,i", po::value<std::string>(), "input file" );

    po::options_description opts{"options"};
    opts.add( visible ).add( hidden );

    po::positional_options_description pos_opts;
    pos_opts.add( "input", -1 );

    po::variables_map       vm;
    po::command_line_parser parser{argc, argv};
    parser.options( opts ).positional( pos_opts );
    po::parsed_options parsed = parser.run();
    po::store( parsed, vm );
    notify( vm );

    if ( vm.count( "help" ) ) {
      std::cout << "Usage: " << argv[0] << " [options] <compact file>" << std::endl;
      std::cout << visible << std::endl;
      return 0;
    }

    // Checking the possible algorithms
    std::vector<std::string> algorithms{"crc"};
    std::string              alg   = vm["checksum"].as<std::string>();
    bool                     found = false;
    for ( const auto& a : algorithms ) {
      if ( a == alg ) {
        found = true;
        break;
      }
    }
    if ( !found ) {
      std::cerr << "Unknow checksum:" << alg << std::endl;
      return 1;
    }

    if ( !vm.count( "input" ) ) {
      std::cerr << "Please specify the compact file to load" << std::endl;
      return 1;
    }

    std::string input_file  = vm["input"].as<std::string>();
    std::string output_file = vm["outfile"].as<std::string>();

    // Loading the geometry from XML
    dd4hep::Detector& desc  = dd4hep::Detector::getInstance();
    auto              start = std::chrono::high_resolution_clock::now();
    desc.fromXML( input_file );
    auto                        geometry_loaded = std::chrono::high_resolution_clock::now();
    decltype( geometry_loaded ) checksum_done;

    if ( alg == "crc" ) {
      auto checksums = lhcb::Detector::checksum::geometry_crc( gGeoManager->GetTopNode(), vm["maxlevel"].as<int>() );
      checksum_done  = std::chrono::high_resolution_clock::now();
      save_checksums( checksums, output_file );
    } else {
      std::cerr << "Unknow checksum algorithm:" << alg << std::endl;
      return 1;
    }

    if ( vm["time"].as<bool>() ) {
      std::chrono::duration<double> load_duration_s     = geometry_loaded - start;
      std::chrono::duration<double> checksum_duration_s = checksum_done - geometry_loaded;

      std::cout << "Geometry loading time  (s): " << load_duration_s.count() << '\n';
      std::cout << "Geometry checksum time (s): " << checksum_duration_s.count() << '\n';
    }

  } catch ( const po::error& ex ) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  return 0;
}
