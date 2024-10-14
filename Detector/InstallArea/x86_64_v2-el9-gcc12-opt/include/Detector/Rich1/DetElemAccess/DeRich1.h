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

#include "Core/DeIOV.h"
#include "Detector/Rich/DeRich.h"
#include "Detector/Rich/Rich_Geom_Constants_De.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich1/DetElemAccess/DeRich1Mirror1.h"
#include "Detector/Rich1/DetElemAccess/DeRich1Mirror2.h"
#include "Detector/Rich1/DetElemAccess/DeRich1PhDetPanel.h"
#include "Detector/Rich1/DetElemAccess/DeRich1RadiatorGas.h"
#include <array>

namespace LHCb::Detector {

  namespace detail {
    /**
     *  Rich1 detector element data
     *  \author  Sajan Easo
     *  \date    2021-10-03
     *  \version  1.0
     */

    struct DeRich1Object : DeRichObject {

      int m_Rich1Mirror1NumSeg  = -1;
      int m_Rich1Mirror2NumSets = -1;

      DeRich1RadiatorGasObject m_Rich1RadiatorGasDetElem;

      // There are 4 Mirror1Segments.
      std::array<DeRich1Mirror1EnvObject, Rich1::NumRich1Mirror1Segments> m_Rich1Mirror1EnvDetElem;
      // Now for the Rich1Mirror2 Master
      std::array<DeRich1Mirror2MasterObject, Rich1::NumRich1Mirror2Sets> m_Rich1Mirror2MasterDetElem;
      // Now for the Rich1PhDetPanel
      Rich::PanelArray<DeRich1PhDetPanelObject> m_Rich1PhDetPanel;

      DeRich1Object( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        func( &m_Rich1RadiatorGasDetElem );
        for ( auto& mirror : m_Rich1Mirror1EnvDetElem ) { func( &mirror ); };
        for ( auto& mirror : m_Rich1Mirror2MasterDetElem ) { func( &mirror ); };
        for ( auto& mirror : m_Rich1PhDetPanel ) { func( &mirror ); };
      }
    };

  } // End namespace detail

  struct DeRich1 : DeRichElement<detail::DeRich1Object> {

    // inherit base constructors
    using DeRichElement::DeRichElement;

    /// Number of mirror segments in primary spherical mirror system
    inline auto nPrimaryMirrors() const noexcept { return ( this->access()->m_Rich1Mirror1NumSeg ); }
    // Number of mirror sets for Secondary mirror system.
    inline auto nSecondaryMirrors() const noexcept { return ( this->access()->m_Rich1Mirror2NumSets ); }

    /// Return the DetectorElement for the gas radiator
    inline auto radiatorGas() const { return DeRich1RadiatorGas( &( this->access()->m_Rich1RadiatorGasDetElem ) ); }

    /// Return DetectorElement for Rich1Mirror1Env
    inline auto Mirror1Env( const int aQ ) const {
      return DeRich1Mirror1Env( &( this->access()->m_Rich1Mirror1EnvDetElem[aQ] ) );
    }

    /// Return DetectorElement for Rich1Mirror2Master
    inline auto Mirror2Master( const int aS ) const {
      return DeRich1Mirror2Master( &( this->access()->m_Rich1Mirror2MasterDetElem[aS] ) );
    }

    /// Return DetectorElement for Rich1PhDetPanel for given side
    inline auto PhDetPanel( const Rich::Side side ) const noexcept {
      return DeRich1PhDetPanel( &( this->access()->m_Rich1PhDetPanel[side] ) );
    }

    /// vector of all primary mirror detector elements
    inline auto primaryMirrors() const noexcept {
      std::vector<DeRich1Mirror1Seg> aR1M1SegVect;
      aR1M1SegVect.reserve( nPrimaryMirrors() );
      for ( int i = 0; i < nPrimaryMirrors(); ++i ) { aR1M1SegVect.emplace_back( Mirror1Env( i ).MirrorSeg() ); }
      return aR1M1SegVect;
    }

    /// Access the primary mirrors for the given side
    inline auto primaryMirrors( const Rich::Side side ) const noexcept {
      auto                  all_mirrs = primaryMirrors();
      decltype( all_mirrs ) side_mirrs;
      for ( auto m : all_mirrs ) {
        assert( m.mirrorSide() != Rich::InvalidSide );
        if ( m.mirrorSide() == side ) { side_mirrs.emplace_back( m ); }
      }
      return side_mirrs;
    }

    /// Secondary mirrors for given side
    inline auto secondaryMirrors( const Rich::Side side ) const noexcept {
      return Mirror2Master( side ).Mirror2SegVect();
    }

    /// All secondary mirrors
    inline auto secondaryMirrors() const noexcept {
      auto mirrs        = secondaryMirrors( Rich::top );
      auto mirrs_bottom = secondaryMirrors( Rich::bottom );
      mirrs.reserve( mirrs.size() + mirrs_bottom.size() );
      mirrs.insert( std::end( mirrs ), std::begin( mirrs_bottom ), std::end( mirrs_bottom ) );
      return mirrs;
    }
  };

  // rules for detector element methods :
  //        1: Any method created in a templated element like this one must be implemented in the same .h file
  //        and not in the .cpp file. This is to avoid linking problems during make.
  //        2: However the methods created in the correspondig 'object' can be implemented in the cpp file.
  //        3: Therefore to acccess method from the templated element, just make a 'one line method' which access a
  //           method of the corresponding object, which in turn can be implemented in the cpp file.
  //        4: When accessing the element of the child element, remember to add the '&' as shown in the example above
  //        for
  //            radiatorGas() method.  For accesing a simple parameter this '&' is not needed.
  //
  //

} // End namespace LHCb::Detector
