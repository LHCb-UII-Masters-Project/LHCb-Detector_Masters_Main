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
#include "DD4hep/DetElement.h"
#include "DD4hep/Objects.h"
#include "DD4hep/PropertyTable.h"

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Rich Beam Pipe detector element data base class
     *  \author  Sajan Easo
     *  \date    2022-04-09
     *  \version  1.0
     */
    struct DeRichBeamPipeObject : DeIOVObject {
      using DeIOVObject::DeIOVObject;
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {
        // no children
      }
    };

  } // End namespace detail

  template <typename ObjectType>
  struct DeRichBeamPipeElement : DeIOVElement<ObjectType> {
    using DeIOVElement<ObjectType>::DeIOVElement;
  };

  using DeRichBeamPipe = DeRichBeamPipeElement<detail::DeRichBeamPipeObject>;

} // End namespace LHCb::Detector
