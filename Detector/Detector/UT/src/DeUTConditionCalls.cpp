/*****************************************************************************\
* (c) Copyright 2000-2022 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Detector/UT/DeUTConditionCalls.h"
#include "DD4hep/ConditionDerived.h"
#include "DD4hep/Detector.h"

dd4hep::Condition LHCb::Detector::DeUTConditionCall::operator()( const dd4hep::ConditionKey& /* key */,
                                                                 dd4hep::cond::ConditionUpdateContext& ctxt ) {
  return LHCb::Detector::DeIOV(
      new LHCb::Detector::UT::detail::DeUTObject( dd4hep::Detector::getInstance().detector( "UT" ), ctxt ) );
}
