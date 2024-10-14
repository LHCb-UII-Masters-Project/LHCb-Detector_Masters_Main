/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\******************************************************************************/

#pragma once

#include "Detector/Rich/DeRichMapmt.h"

namespace LHCb::Detector {

  namespace detail {
    /**
     *  Rich2 Mapmt  data
     *  \author  Sajan Easo
     *  \date    2022-02-10
     *  \version  1.0
     */
    struct DeRich2StdMapmtObject : DeRichMapmtObject {
      DeRich2StdMapmtObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };
    struct DeRich2GrandMapmtObject : DeRichMapmtObject {
      DeRich2GrandMapmtObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };
  } // End namespace detail

  struct DeRich2StdMapmt : detail::DeRichMapmtElement<detail::DeRich2StdMapmtObject> {
    using DeRichMapmtElement::DeRichMapmtElement;
  };

  struct DeRich2GrandMapmt : detail::DeRichMapmtElement<detail::DeRich2GrandMapmtObject> {
    using DeRichMapmtElement::DeRichMapmtElement;
  };
} // namespace LHCb::Detector
