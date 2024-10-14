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

#include "Detector/Rich2/DeRich2ConditionCalls.h"
#include "DD4hep/Detector.h"
#include "DD4hep/DetectorProcessor.h"
#include "Math/Transform3D.h"
#include "XML/XML.h"

dd4hep::Condition LHCb::Detector::DeRich2ConditionCall::operator()( const dd4hep::ConditionKey& /* key */,
                                                                    dd4hep::cond::ConditionUpdateContext& ctxt ) {
  ctxt.condition( Keys::alignmentsComputedKey );
  const auto& rich2 = dd4hep::Detector::getInstance().detector( "Rich2" );

  return DeIOV( new detail::DeRich2Object( rich2, ctxt ) );
}
