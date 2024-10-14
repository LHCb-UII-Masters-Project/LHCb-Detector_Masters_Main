/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Detector/UP/DeUPSector.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Printout.h"

namespace LHCb::Detector {

  std::string toString( Status status ) {
    auto i = s_map.find( status );
    return i == s_map.end() ? "UnknownStatus" : i->second;
  }

  Status toStatus( std::string_view str ) {
    auto i = std::find_if( s_map.begin(), s_map.end(), [&]( const auto& i ) { return i.second == str; } );
    return i == s_map.end() ? Status::UnknownStatus : i->first;
  }

  detail::DeUPSectorObject::DeUPSectorObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt )
      : DeIOVObject( de, ctxt, 9320 ) {}

} // namespace LHCb::Detector
