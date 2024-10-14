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
#include "Detector/Rich2/DetElemAccess/DeRich2BeamPipe.h"
#include "Detector/Rich2/DetElemAccess/DeRich2PhDetPanel.h"
#include "Detector/Rich2/DetElemAccess/DeRich2RadiatorGas.h"
#include <array>

namespace LHCb::Detector {

  namespace detail {
    /**
     *  Rich2 detector element data
     *  \author  Sajan Easo
     *  \date    2022-01-10
     *  \version  1.0
     */

    struct DeRich2Object : DeRichObject {
      DeRich2RadiatorGasObject m_Rich2RadiatorGasDetElem;
      DeRich2BeamPipeObject    m_Rich2BeamPipeDetElem;

      std::array<DeRich2PhDetPanelObject, Rich2::NumRich2Sides> m_Rich2PhDetPanel;

      DeRich2Object( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& panel : m_Rich2PhDetPanel ) { func( &panel ); };
        func( &m_Rich2RadiatorGasDetElem );
        func( &m_Rich2BeamPipeDetElem );
      }
    };

  } // End namespace detail

  struct DeRich2 : DeRichElement<detail::DeRich2Object> {

    // inherit base constructors
    using DeRichElement::DeRichElement;

    // Return the DetectorElement for the gas radiator
    inline auto radiatorGas() const { return DeRich2RadiatorGas( &( this->access()->m_Rich2RadiatorGasDetElem ) ); }

    // Return the DetectorElement for Rich2 beam pipe
    inline auto BeamPipe() const { return DeRich2BeamPipe( &( this->access()->m_Rich2BeamPipeDetElem ) ); }

    // Return the DetectorElement for the Photon detector panel in Rich2
    inline auto PhDetPanel( const Rich::Side side ) const noexcept {
      return DeRich2PhDetPanel( &( this->access()->m_Rich2PhDetPanel[side] ) );
    }

    /// Access the primary mirrors for the given side
    inline auto primaryMirrors( const Rich::Side side ) const noexcept {
      return radiatorGas().SphMM( side ).SphMirrorSegVect();
    }

    /// Access the secondary mirrors for the given side
    inline auto secondaryMirrors( const Rich::Side side ) const noexcept {
      return radiatorGas().SecMM( side ).SecMirrorSegVect();
    }

    /// All primary mirrors
    inline auto primaryMirrors() const noexcept {
      auto mirrs   = primaryMirrors( Rich::left );
      auto mirrs_r = primaryMirrors( Rich::right );
      mirrs.reserve( mirrs.size() + mirrs_r.size() );
      mirrs.insert( std::end( mirrs ), std::begin( mirrs_r ), std::end( mirrs_r ) );
      return mirrs;
    }

    /// All secondary mirrors
    inline auto secondaryMirrors() const noexcept {
      auto mirrs   = secondaryMirrors( Rich::left );
      auto mirrs_r = secondaryMirrors( Rich::right );
      mirrs.reserve( mirrs.size() + mirrs_r.size() );
      mirrs.insert( std::end( mirrs ), std::begin( mirrs_r ), std::end( mirrs_r ) );
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
