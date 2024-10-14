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

#include "Detector/Muon/TileID.h"

#include "Core/DeIOV.h"

#include <string>

namespace LHCb::Detector::Muon {

  namespace detail {

    struct ODEBoardObject : LHCb::Detector::detail::DeIOVObject {
      ODEBoardObject( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    };

  } // namespace detail

  /**
   *  @author Alessia Satta
   *  @date   2004-01-05
   */
  struct ODEBoard : DeIOVElement<detail::ODEBoardObject> {
    using DeIOVElement::DeIOVElement;

    bool        addTSName( std::string ) { throw NotImplemented(); }
    long        getODESerialNumber() { throw NotImplemented(); }
    long        getTSLayoutX() { throw NotImplemented(); }
    long        getTSLayoutY() { throw NotImplemented(); }
    long        getTSNumber() { throw NotImplemented(); }
    long        getTSGridX( int ) { throw NotImplemented(); }
    long        getTSGridY( int ) { throw NotImplemented(); }
    long        getTSQuadrant( int ) { throw NotImplemented(); }
    std::string getTSName( int ) { throw NotImplemented(); }
    void        setRegion( long ) { throw NotImplemented(); }
    long        region() { throw NotImplemented(); }
    void        setQuadrants() { throw NotImplemented(); }
    bool        isQuadrantContained( long ) { throw NotImplemented(); }
    bool        isTSContained( TileID ) { throw NotImplemented(); }
    void        setECSName( std::string ) { throw NotImplemented(); }

    inline std::string ECSName() { throw NotImplemented(); }
  };
} // End namespace LHCb::Detector::Muon
