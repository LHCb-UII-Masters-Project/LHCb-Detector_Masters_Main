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

#include "Detector/UP/DeUPStave.h"
#include "Core/PrintHelpers.h"

#include "DD4hep/Printout.h"

LHCb::Detector::detail::DeUPStaveObject::DeUPStaveObject( const dd4hep::DetElement&             de,
                                                          dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt, 9310 ) {

  for ( auto iter : de.children() ) {
    // m_faces.emplace_back( iter.second, ctxt );
  }
}
