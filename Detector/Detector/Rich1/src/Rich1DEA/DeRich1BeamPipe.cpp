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
/**  DeRich1BeamPipe detector element implementation in DD4HEP   **/
/** SE 2022-04-09    **/
/*****************************************************************************/

#include "Detector/Rich1/DetElemAccess/DeRich1BeamPipe.h"
#include "DD4hep/Printout.h"

using namespace LHCb::Detector::detail;

DeRich1BeamPipeObject::DeRich1BeamPipeObject( const dd4hep::DetElement&             de, //
                                              dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichBeamPipeObject( de, ctxt ) {
  // nothing yet ..
}
