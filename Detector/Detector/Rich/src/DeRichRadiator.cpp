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

#include "Detector/Rich/DeRichRadiator.h"
#include "DD4hep/DD4hepUnits.h"
#include "Detector/Rich1/RichMatOPD.h"
#include "Detector/Rich1/RichMatPropData.h"
#include <nlohmann/json.hpp>

using namespace LHCb::Detector::detail;

DeRichRadiatorObject::TabData //
DeRichRadiatorObject::CurrentScaledRefIndexInObjTab() {
  assert( m_refScalefactor.isValid() );
  assert( m_gasParameters.isValid() );
  auto& params = m_gasParameters.get<nlohmann::json>();
  // Convert pressue value read from condition in millibar to internal units (MeV/mm^3)
  const auto CurP = params["Pressure"].get<double>() * 0.001 * dd4hep::bar;
  const auto CurT = params["Temperature"].get<double>() * dd4hep::kelvin;
  // current ref. index scale factor
  const auto ScaleF = CurrentRefIndexScaleFactor();
  // name
  const auto radname = ( Rich::C4F10 == m_gasName ? "DeRich1RadiatorGas" : "DeRich2RadiatorGas" );
  dd4hep::printout( dd4hep::INFO, radname, "Current GasParameters : P=%f T=%f Scale=%f", //
                    CurP, CurT, ScaleF );
  return CurrentScaledRefIndexInObjTab( CurP, CurT, ScaleF );
}

DeRichRadiatorObject::TabData                                                   //
DeRichRadiatorObject::CurrentScaledRefIndexInObjTab( const double aPressure,    //
                                                     const double aTemperature, //
                                                     const double ScaleF ) const {
  assert( Rich::InvalidRadiator != m_gasName );
  const auto matprop = RichMatPropData::getRichMatPropDataInstance();
  return matprop->CreateRefIndxTable( m_gasName, aPressure, aTemperature, ScaleF );
}

DeRichRadiatorObject::TabData DeRichRadiatorObject::RetrieveNominalGasRefIndexInDeObjectFromDB() const {
  auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
  return aRichMatOpd->GetRichMatProperty( detector().volume().material(), RichMatPropType::CkvRndx );
}
