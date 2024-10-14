/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*******************************************************************************/

#pragma once

#include "Detector/Rich/DeRichBeampipe.h"

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Rich2 Beam Pipe detector element data
     *  \author  Sajan Easo
     *  \date    2022-04-09
     *  \version  1.0
     */
    struct DeRich2BeamPipeObject : DeRichBeamPipeObject {
      DeRich2BeamPipeObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };

  } // End namespace detail

  struct DeRich2BeamPipe : DeRichBeamPipeElement<detail::DeRich2BeamPipeObject> {
    using DeRichBeamPipeElement::DeRichBeamPipeElement;
  };
} // End namespace LHCb::Detector
