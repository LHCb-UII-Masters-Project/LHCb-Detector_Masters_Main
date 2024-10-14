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

#include "Detector/UP/DeUPModule.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Printout.h"

LHCb::Detector::detail::DeUPModuleObject::DeUPModuleObject( const dd4hep::DetElement&             de,
                                                            dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9312 ), id( de.id() ) {
  for ( auto iter : de.children() ) {
    // m_sectors.emplace_back(iter.second, ctxt);
  }
}
