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
#pragma once

#include "Core/DeIOV.h"
#include "Core/Keys.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeMPObject : DeIOVObject {
      DeMPObject( dd4hep::DetElement de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& /*func*/ ) const override{
          // for ( auto& station : m_faces ) { //func( &station ); };
      };
    };

  }; // End namespace detail

  struct DeMP : DeIOVElement<detail::DeMPObject> {
    using DeIOVElement::DeIOVElement;
  };

} // End namespace LHCb::Detector
