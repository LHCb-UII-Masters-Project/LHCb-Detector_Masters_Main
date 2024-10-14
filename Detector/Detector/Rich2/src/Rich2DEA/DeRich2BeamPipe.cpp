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
/**  DeRich2BeamPipe detector element implementation in DD4HEP   **/
/** SE 2022-04-09    **/
/*****************************************************************************/

#include "Detector/Rich2/DetElemAccess/DeRich2BeamPipe.h"
#include "DD4hep/Printout.h"

using namespace LHCb::Detector::detail;

DeRich2BeamPipeObject::DeRich2BeamPipeObject( const dd4hep::DetElement&             de, //
                                              dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichBeamPipeObject( de, ctxt ) {
  // nothing yet ..
}
