//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

#include "Detector/VP/DeVP.h"

#include "Core/Keys.h"

#include "Core/ConditionsRepository.h"

#include "DD4hep/Conditions.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"

#include "DDCond/ConditionsDataLoader.h"
#include "DDCond/ConditionsManager.h"
#include "DDCond/ConditionsSlice.h"

#include "TTimeStamp.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

static long test_dump_vp( dd4hep::Detector& description, int argc, char** argv ) {
  bool        help = false;
  std::string conditions, match;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) )
        help = true;
      else if ( 0 == ::strncmp( "-conditions", argv[i], 4 ) )
        conditions = argv[++i];
      else if ( 0 == ::strncmp( "-match", argv[i], 4 ) )
        match = argv[++i];
      else
        help = true;
    }
  }
  if ( help || conditions.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }
  if ( conditions.rfind( '/' ) != conditions.length() ) { conditions += '/'; }
  dd4hep::setPrintFormat( "%-24s %5s %s" );
  if ( match.empty() && conditions.find( ':' ) != std::string::npos ) {
    match = conditions.substr( 0, conditions.find( ':' ) + 1 );
  } else if ( match.empty() ) {
    match = "file:";
  }
  if ( conditions.find( match ) != 0 ) { conditions = match + conditions; }
  dd4hep::printout( dd4hep::INFO, "Conditions", "+++ Conditions directory: %s  [%s]", conditions.c_str(),
                    match.c_str() );

  /******************** Initialize the conditions manager *****************/
  // Now we instantiate the conditions manager
  dd4hep::cond::ConditionsManager manager( description, "DD4hep_ConditionsManager_Type1" );
  manager["PoolType"]                 = "DD4hep_ConditionsLinearPool";
  manager["UserPoolType"]             = "DD4hep_ConditionsMapUserPool";
  manager["UpdatePoolType"]           = "DD4hep_ConditionsLinearUpdatePool";
  manager["LoaderType"]               = "LHCb_ConditionsLoader";
  manager["OutputUnloadedConditions"] = true;
  manager["LoadConditions"]           = true;
  manager.initialize();
  const dd4hep::IOVType* iov_typ = manager.registerIOVType( 0, "run" ).second;
  if ( 0 == iov_typ ) { dd4hep::except( "ConditionsPrepare", "++ Unknown IOV type supplied." ); }
  dd4hep::cond::ConditionsDataLoader& loader = manager.loader();
  loader["ReaderType"]                       = "LHCb_ConditionsFileReader";
  loader["Directory"]                        = conditions;
  loader["Match"]                            = match;
  loader["IOVType"]                          = "run";
  loader.initialize();

  auto               content  = std::make_shared<LHCb::Detector::ConditionsRepository>();
  dd4hep::DetElement de       = description.detector( "VP" );
  const auto&        de_conds = *de.extension<std::shared_ptr<LHCb::Detector::ConditionsRepository>>();
  const auto& world_conds     = *description.world().extension<std::shared_ptr<LHCb::Detector::ConditionsRepository>>();
  content->merge( *world_conds );
  content->merge( *de_conds );

  dd4hep::IOV                             req_iov( iov_typ, 100 );
  auto                                    slice = std::make_shared<dd4hep::cond::ConditionsSlice>( manager, content );
  TTimeStamp                              start;
  dd4hep::cond::ConditionsManager::Result total = manager.load( req_iov, *slice );
  TTimeStamp                              comp;
  total += manager.compute( req_iov, *slice );
  TTimeStamp                                   stop;
  dd4hep::cond::ConditionsContent::Conditions& missing = slice->missingConditions();
  for ( const auto& m : missing ) {
    dd4hep::printout( dd4hep::ERROR, "TEST", "Failed to load condition [%016llX]: %s", m.first,
                      m.second->toString().c_str() );
  }
  dd4hep::printout( dd4hep::INFO, "Statistics",
                    "+  Created/Accessed a total of %ld conditions "
                    "(S:%6ld,L:%6ld,C:%6ld,M:%ld)  Load:%7.5f sec Compute:%7.5f sec",
                    total.total(), total.selected, total.loaded, total.computed, total.missing,
                    comp.AsDouble() - start.AsDouble(), stop.AsDouble() - comp.AsDouble() );

  std::stringstream    str;
  LHCb::Detector::DeVP vp = slice->get( de, LHCb::Detector::Keys::deKey );
  {
    for ( const auto& s : vp->sides ) {
      for ( const auto& m : s.modules ) {
        LHCb::Detector::DeVPModule module( &m );
        dd4hep::Alignment          align  = module->detectorAlignment(); // detector.nominal();
        dd4hep::Position           global = align.localToWorld( {0, 0, 0} );
        str.str( "" );
        for ( const auto& ladder : module->ladders ) {
          str << std::setw( 3 ) << to_unsigned( ladder.sensor.sensorNumber ) << " ";
        }
        auto& sens = module->ladders[0].sensor;
        // assign the result of operator* to avoid clang's -Wpotentially-evaluated-expression
        auto& module_held_object = *module.ptr();
        dd4hep::printout( dd4hep::INFO, "Module", "VP Module: %2d Sensors: %s Position: %8.3f %8.3f %8.3f [mm]  %s",
                          sens.moduleNumber, str.str().c_str(), global.X() / dd4hep::mm, global.Y() / dd4hep::mm,
                          global.Z() / dd4hep::mm, dd4hep::typeName( typeid( module_held_object ) ).c_str() );
      }
    }
  }
  {
    for ( const auto& s : vp->sides ) {
      for ( const auto& m : s.modules ) {
        LHCb::Detector::DeVPModule module( &m );
        dd4hep::Alignment          align  = module->detectorAlignment(); // detector.nominal();
        dd4hep::Position           global = align.localToWorld( {0, 0, 0} );
        str.str( "" );
        for ( const auto& l : module->ladders ) {
          LHCb::Detector::DeVPSensor sens( &( l.sensor ) );
          str << std::setw( 3 ) << to_unsigned( sens.sensorNumber() ) << " ";
        }
        auto& sensor0 = module->ladders[0].sensor;
        // assign the result of operator* to avoid clang's -Wpotentially-evaluated-expression
        auto& module_held_object = *module.ptr();
        dd4hep::printout( dd4hep::INFO, "Module",
                          "Station:%2d Module:%2d %-5s Sensors: %s %12s Pos:%6.3f %8.3f %8.3f [mm] %s",
                          sensor0.moduleNumber >> 1, sensor0.moduleNumber, sensor0.isLeft ? "LEFT" : "RIGHT",
                          str.str().c_str(), "", global.X() / dd4hep::mm, global.Y() / dd4hep::mm,
                          global.Z() / dd4hep::mm, dd4hep::typeName( typeid( module_held_object ) ).c_str() );
        for ( const auto& ladder : module->ladders ) {
          LHCb::Detector::DeVPSensor sens( &( ladder.sensor ) );
          align  = sens->detectorAlignment();
          global = sens.toGlobal( dd4hep::Position() );
          dd4hep::printout(
              dd4hep::INFO, "Sensor",
              "           Sensor: %2d Cols:%3d Rows:%3d Si(xyz) (%5.2f,%5.2f,%4.2f)  %8.3f %8.3f %8.3f [mm] %s",
              to_unsigned( sens.sensorNumber() ), sens.numColumns(), sens.numRows(), sens.activeSizeX() / dd4hep::mm,
              sens.activeSizeY() / dd4hep::mm, sens.siliconThickness(), global.X() / dd4hep::mm,
              global.Y() / dd4hep::mm, global.Z() / dd4hep::mm, sens.detector().path().c_str() );
#if 0
          Box box = sens.detector().volume().solid();
          Position pos;
          global = sens.toGlobal(pos=Position( box.x(),  box.y(),  box.z()));
          dd4hep::printout(dd4hep::INFO,"Sensor", "           local: %8.3f %8.3f %8.3f  global %8.3f %8.3f %8.3f",
                           pos.X()/dd4hep::mm, pos.Y()/dd4hep::mm, pos.Z()/dd4hep::mm,
                           global.X()/dd4hep::mm, global.Y()/dd4hep::mm, global.Z()/dd4hep::mm);
          global = sens.toGlobal(pos=Position(-box.x(), -box.y(),  box.z()));
          dd4hep::printout(dd4hep::INFO,"Sensor", "           local: %8.3f %8.3f %8.3f  global %8.3f %8.3f %8.3f",
                           pos.X()/dd4hep::mm, pos.Y()/dd4hep::mm, pos.Z()/dd4hep::mm,
                           global.X()/dd4hep::mm, global.Y()/dd4hep::mm, global.Z()/dd4hep::mm);
          global = sens.toGlobal(pos=Position( box.x(),  box.y(),  box.z()));
          dd4hep::printout(dd4hep::INFO,"Sensor", "           local: %8.3f %8.3f %8.3f  global %8.3f %8.3f %8.3f",
                           pos.X()/dd4hep::mm, pos.Y()/dd4hep::mm, pos.Z()/dd4hep::mm,
                           global.X()/dd4hep::mm, global.Y()/dd4hep::mm, global.Z()/dd4hep::mm);
          global = sens.toGlobal(pos=Position(-box.x(), -box.y(),  box.z()));
          dd4hep::printout(dd4hep::INFO,"Sensor", "           local: %8.3f %8.3f %8.3f  global %8.3f %8.3f %8.3f",
                           pos.X()/dd4hep::mm, pos.Y()/dd4hep::mm, pos.Z()/dd4hep::mm,
                           global.X()/dd4hep::mm, global.Y()/dd4hep::mm, global.Z()/dd4hep::mm);
#endif
        }
      }
    }
  }

  slice.reset();

  /// Clear it
  manager.clear();
  // Let's do the cleanup
  manager.destroy();
  return 1;

  return 1;
}

DECLARE_APPLY( LHCb_TEST_dump_VP, test_dump_vp )
