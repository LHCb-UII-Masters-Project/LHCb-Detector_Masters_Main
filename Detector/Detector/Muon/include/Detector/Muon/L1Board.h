/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
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

#include <string>
#include <vector>

namespace LHCb::Detector::Muon {

  namespace detail {

    struct L1BoardObject : LHCb::Detector::detail::DeIOVObject {
      L1BoardObject( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };

  } // namespace detail

  /**
   *  @author Alessia Satta
   *  @date   2004-01-05
   */
  struct L1Board : DeIOVElement<detail::L1BoardObject> {
    using DeIOVElement::DeIOVElement;

    long L1Number() { throw NotImplemented(); }
    void setL1Number( long ) { throw NotImplemented(); }
    void setL1Station( long ) { throw NotImplemented(); }

    long numberOfODE() { throw NotImplemented(); }
    void setNumberOfODE( long ) { throw NotImplemented(); }
    void setLinkConnection( long, long ) { throw NotImplemented(); }
    long getLinkConnection( long ) { throw NotImplemented(); }
    bool isLinkConnected( long ) { throw NotImplemented(); }

    void        addODE( long, std::string ) { throw NotImplemented(); }
    void        addLayout( long, long, long ) { throw NotImplemented(); }
    std::string getODEName( int ) { throw NotImplemented(); }
    long        getTSLayoutX( long ) { throw NotImplemented(); }
    long        getTSLayoutY( long ) { throw NotImplemented(); }
    long        getODEPosition( long, bool = true ) { throw NotImplemented(); }
    int         getStation() { throw NotImplemented(); }
  };
} // End namespace LHCb::Detector::Muon
