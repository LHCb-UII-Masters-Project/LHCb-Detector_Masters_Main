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

#include "Detector/FT/DeFTConditionCalls.h"
#include "Detector/FT/DeFTHalfLayer.h"
#include "Detector/FT/DeFTLayer.h"
#include "Detector/FT/DeFTMat.h"
#include "Detector/FT/DeFTModule.h"
#include "Detector/FT/DeFTQuarter.h"
#include "Detector/FT/DeFTStation.h"

//#include "DD4hep/DetectorProcessor.h"
//#include "DD4hep/detail/DetectorInterna.h"
#include "DD4hep/Detector.h"
#include "XML/XML.h"

dd4hep::Condition LHCb::Detector::DeFTConditionCall::operator()( const dd4hep::ConditionKey& /* key */,
                                                                 dd4hep::cond::ConditionUpdateContext& ctxt ) {
  return DeIOV( new DeFT::Object( dd4hep::Detector::getInstance().detector( "FT" ), ctxt ) );
}
