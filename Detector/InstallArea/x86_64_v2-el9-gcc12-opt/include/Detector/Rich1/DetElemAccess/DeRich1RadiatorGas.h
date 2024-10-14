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
#include "Detector/Rich/Types.h"
#include "Detector/Rich1/DetElemAccess/DeRich1BeamPipe.h"
#include <map>

namespace LHCb::Detector {

  namespace detail {
    /**
     *  Rich1 Radiator gas detector element data
     *  \author  Sajan Easo
     *  \date    2021-10-01
     *  \version  1.0
     */

    struct DeRich1RadiatorGasObject : DeRichRadiatorObject {

      DeRich1BeamPipeObject m_Rich1BeamPipeDetElem;

      void PrintR1GasGeneralInfoFromDBInObj() const;

      // For debug printing of the inputs
      void RetrieveAndPrintDebugR1GasNominalRefIndexFromLocalInpInObj();

      DeRich1RadiatorGasObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        func( &m_Rich1BeamPipeDetElem );
      }
    };

  } // End namespace detail
  //===============================================================================================//
  struct DeRich1RadiatorGas : detail::DeRichRadiatorElement<detail::DeRich1RadiatorGasObject> {
    using DeRichRadiatorElement::DeRichRadiatorElement;

    // Return the DetectorElement for Rich1 beam pipe
    inline auto BeamPipe() const { return DeRich1BeamPipe( &( this->access()->m_Rich1BeamPipeDetElem ) ); }
    auto        radiatorID() const noexcept { return Rich::Rich1Gas; }
    auto        rich() const noexcept { return Rich::Rich1; }

    auto CurrentScaledRefIndexTab( double aPressure, double aTemperature ) const {
      return this->access()->CurrentScaledRefIndexInObjTab( aPressure, aTemperature );
    }

    // The following methods are for test and debug in initial stages and may eventually be suppressed.

    // The implementation of the following in the DeRich1RadiatorGas.cpp would not link during build. Hence
    // accessing it from the  implementation done from the corresponding object.
    void PrintGasGeneralInfoFromDB() const { this->access()->PrintR1GasGeneralInfoFromDBInObj(); }

    // auto RetrieveNominalGasRefIndexFromDB() const {
    //   return this->access()->RetrieveNominalR1GasRefIndexInDeObjectFromDB();
    // }
    // For debug printing of the inputs
    void RetrieveAndPrintDebugGasNominalRefIndexFromLocalInp() const {
      this->access()->RetrieveAndPrintDebugR1GasNominalRefIndexFromLocalInpInObj();
    }

    // end of methods for debug and test
  };

  using DeRich1Gas = DeRich1RadiatorGas;
} // End namespace LHCb::Detector
