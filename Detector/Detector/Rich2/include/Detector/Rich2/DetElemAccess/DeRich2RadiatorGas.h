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
#include "DD4hep/DetElement.h"
#include "DD4hep/Objects.h"
#include "DD4hep/PropertyTable.h"
#include "Detector/Rich/DeRichRadiator.h"
#include "Detector/Rich/Rich_Geom_Constants_De.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich2/DetElemAccess/DeRich2SecMirror.h"
#include "Detector/Rich2/DetElemAccess/DeRich2SphMirror.h"
#include <cstdint>
#include <map>

namespace LHCb::Detector {

  namespace detail {
    /**
     *  Rich2 Radiator gas detector element data
     *  \author  Sajan Easo
     *  \date    2022-01-10
     *  \version  1.0
     */

    struct DeRich2RadiatorGasObject : DeRichRadiatorObject {

      std::int32_t m_Rich2NumSides = -1;

      // using hard coded value for now, to declare the following.
      std::array<DeRich2SphMirrorMasterObject, Rich2::NumRich2Sides> m_Rich2SphMirrorMasterDetElem;
      // using hard coded value for now, to declare the following.
      std::array<DeRich2SecMirrorMasterObject, Rich2::NumRich2Sides> m_Rich2SecMirrorMasterDetElem;

      void PrintR2GasGeneralInfoFromDBInObj() const;

      // For debug printing of the inputs
      void RetrieveAndPrintDebugR2GasNominalRefIndexFromLocalInpInObj();

      DeRich2RadiatorGasObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& master : m_Rich2SphMirrorMasterDetElem ) { func( &master ); };
        for ( auto& master : m_Rich2SecMirrorMasterDetElem ) { func( &master ); };
      }
    };

  } // End namespace detail

  struct DeRich2RadiatorGas : detail::DeRichRadiatorElement<detail::DeRich2RadiatorGasObject> {
    using DeRichRadiatorElement::DeRichRadiatorElement;

    auto radiatorID() const noexcept { return Rich::Rich2Gas; }
    auto rich() const noexcept { return Rich::Rich2; }

    auto CurrentScaledRefIndexTab( double aPressure, double aTemperature ) const {
      return this->access()->CurrentScaledRefIndexInObjTab( aPressure, aTemperature );
    }

    // The following methods are for test and debug in initial stages and may eventually be suppressed.

    void PrintGasGeneralInfoFromDB() const { this->access()->PrintR2GasGeneralInfoFromDBInObj(); }

    // auto RetrieveNominalGasRefIndexFromDB() const {
    //   return this->access()->RetrieveNominalR2GasRefIndexInDeObjectFromDB();
    // }
    // For debug printing of the inputs
    void RetrieveAndPrintDebugGasNominalRefIndexFromLocalInp() const {
      this->access()->RetrieveAndPrintDebugR2GasNominalRefIndexFromLocalInpInObj();
    }

    // end of methods for debug and test

    auto NumSides() const noexcept { return this->access()->m_Rich2NumSides; }

    // Return the Detector element for the Rich2 Sph mirror master
    auto SphMM( int aSide ) const noexcept {
      return DeRich2SphMirrorMaster( &( this->access()->m_Rich2SphMirrorMasterDetElem[aSide] ) );
    }
    // Return the Detector element for the Rich2 Sec mirror master
    auto SecMM( int aSide ) const noexcept {
      return DeRich2SecMirrorMaster( &( this->access()->m_Rich2SecMirrorMasterDetElem[aSide] ) );
    }
  };

  using DeRich2Gas = DeRich2RadiatorGas;

} // End namespace LHCb::Detector
