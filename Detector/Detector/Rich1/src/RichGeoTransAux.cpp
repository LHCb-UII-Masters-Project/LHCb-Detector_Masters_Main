//==========================================================================
//  LHCb Rich Detector geometry Transform Auxiliary class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-12-01
//
//==========================================================================//

#include "Detector/Rich1/RichGeoTransAux.h"
#include "Detector/Rich1/RichGeoUtil.h"
#include "Detector/Rich1/RichPmtGeoAux.h"
#include "Detector/Rich2/Rich2GeoUtil.h"
#include <cassert>
#include <map>
#include <memory>
#include <mutex>

//==========================================================================//

size_t RichGeoTransAux::loadRichTransforms( xml_h element, const std::string& tag,
                                            const std::string& RichDetectorName ) {

  size_t count = 0;
  for ( xml_coll_t c( element, Unicode( tag ) ); c; ++c ) {
    auto ref = c.attr<std::string>( _U( ref ) );
    printout( getDD4hepPrintLevel(), " RichGeoTransAux: ", "Now reading the File with name :   %s", ref.c_str() );
    if ( ref.find( RichDetectorName ) != std::string::npos ) {
      if ( ( ref.find( "Transform" ) != std::string::npos ) && ( ref.find( "Phys" ) != std::string::npos ) ) {
        auto docT = std::make_unique<dd4hep::xml::DocumentHolder>(
            dd4hep::xml::DocumentHandler().load( element, c.attr_value( _U( ref ) ) ) );
        auto trF = docT->root();

        printout( getDD4hepPrintLevel(),
                  "RichGeoTransAux: ", "++ Processing and loading Transforms from xml document %s .",
                  docT->uri().c_str() );
        if ( RichDetectorName.find( "Rich2" ) != std::string::npos ) {
          auto aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
          if ( aRich2GeoUtil->IsRich2SuperStrComp( ref ) ) {
            aRich2GeoUtil->build_Rich2SuperStrPhysTransforms( trF, ref );
          } else {
            aRich2GeoUtil->build_Rich2Transforms( trF );
          }
        } else if ( ( RichDetectorName.find( "RichPMT" ) != std::string::npos ) ||
                    ( RichDetectorName.find( "RichGrandPMT" ) != std::string::npos ) ) {
          auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
          aRichPmtGeoAux->build_RichPmtTransforms( trF );
        } else {

          auto aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();
          aRichGeoUtil->build_RichTransforms( trF );
        }
        ++count;
        printout( getDD4hepPrintLevel(), "RichGeoTransAux : ", " Finished loading transforms from file  %s for %s",
                  ref.c_str(), RichDetectorName.c_str() );
      }

    } // end test on RichDetector name

    printout( getDD4hepPrintLevel(), "RichGeoTransAux : ", " Finished with the file  %s", ref.c_str() );
  }

  return count;
}

//==========================================================================//

RichGeoTransAux* RichGeoTransAux::getRichGeoTransAuxInstance() {
  static std::once_flag                   alloc_instance_once;
  static std::unique_ptr<RichGeoTransAux> instance;
  std::call_once( alloc_instance_once, []() { instance = std::make_unique<RichGeoTransAux>(); } );
  assert( instance.get() );
  return instance.get();
}
//==========================================================================//
