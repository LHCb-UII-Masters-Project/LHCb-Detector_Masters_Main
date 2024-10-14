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
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include <yaml-cpp/yaml.h>

#include <DD4hep/Detector.h>
#include <DD4hep/Printout.h>

#include <Core/Checksum.h>
#include <Core/GeometryTools.h>

#include <TGeoManager.h>
#include <TGeoNode.h>

namespace po = boost::program_options;

/*
 * Simple tool to checksum a geometry file
 */
int main( int argc, char* argv[] ) {

  try {
    po::options_description visible{"visible"};
    visible.add_options()( "help,h", "Help" )( "path,p", po::value<std::string>()->default_value( "-" ),
                                               "Process only specific  path" );

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

    if ( !vm.count( "input" ) ) {
      std::cerr << "Please specify the compact file to load" << std::endl;
      return 1;
    }

    std::string input_file = vm["input"].as<std::string>();
    std::string path       = vm["path"].as<std::string>();

    dd4hep::Detector& desc = dd4hep::Detector::getInstance();
    desc.fromXML( input_file );

    std::cout << "Looking for node: " << path << std::endl;
    gGeoManager->cd( path.c_str() );
    auto node = gGeoManager->GetCurrentNode();

    std::cout << '\n' << lhcb::geometrytools::toString( node ) << std::endl;
    lhcb::Detector::checksum::AccumulatorStr a;
    lhcb::Detector::checksum::checksum_node( a, node );
    std::cout << '\n' << a.result() << '\n';

  } catch ( const po::error& ex ) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  return 0;
}
