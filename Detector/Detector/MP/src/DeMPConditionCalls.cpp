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

#include "Detector/MP/DeMPConditionCalls.h"
#include "Detector/MP/DeMP.h"

#include "DD4hep/Detector.h"
#include "XML/XML.h"

dd4hep::Condition LHCb::Detector::DeMPConditionCall::operator()( const dd4hep::ConditionKey& /* key */,
                                                                 dd4hep::cond::ConditionUpdateContext& ctxt ) {
  ctxt.condition( Keys::alignmentsComputedKey );
  return DeIOV( new DeMP::Object( dd4hep::Detector::getInstance().detector( "MP" ), ctxt ) );
}
