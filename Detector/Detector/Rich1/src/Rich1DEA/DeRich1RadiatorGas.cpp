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
/**  DeRich1RadiatorGas detector element implementation in DD4HEP   **/
/** SE 2021-10-03    **/
/*****************************************************************************/

#include "Detector/Rich1/DetElemAccess/DeRich1RadiatorGas.h"
#include "Detector/Rich1/RichGeoUtil.h"
#include "Detector/Rich1/RichMatOPD.h"
#include "Detector/Rich1/RichMatPropData.h"

#include "DD4hep/Printout.h"

#include <array>

using namespace LHCb::Detector::detail;

DeRich1RadiatorGasObject::DeRich1RadiatorGasObject( const dd4hep::DetElement&             de,
                                                    dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichRadiatorObject( de, ctxt )
    , m_Rich1BeamPipeDetElem{de.child( RichGeoUtil::getRichGeoUtilInstance()->Rich1BeamPipeInSubMDetName() ), ctxt} {

  m_gasName = Rich::C4F10;

  detail::init_param( m_Presure_Nominal, "C4F10Pressure" );
  detail::init_param( m_Temperature_Nominal, "C4F10Temperature" );
  detail::init_param( m_PhotonMinEnergy, "PhotonCkvMinEnergyC4F10" );
  detail::init_param( m_PhotonMaxEnergy, "PhotonCkvMaxEnergyC4F10" );

  auto r1          = dd4hep::Detector::getInstance().detector( "Rich1" );
  m_refScalefactor = ctxt.condition( dd4hep::ConditionKey::KeyMaker( r1, "RefractivityScaleFactor" ).hash );
  m_gasParameters  = ctxt.condition( dd4hep::ConditionKey::KeyMaker( r1, "GasParameters" ).hash );

  // set nominal ref index
  // m_GasRefIndex = RetrieveNominalGasRefIndexInDeObjectFromDB();

  // for illustration dummy transformation with nominal pressure and temperature.
  // Can be used with different pressure and temperature privided by conditions DB.
  // The line above is redundant when using the CurrentScaledRefIndexTab,
  // but kept for now, for illustration.
  m_GasRefIndex = CurrentScaledRefIndexInObjTab( m_Presure_Nominal, m_Temperature_Nominal );

  // Form nominal entry / exit plane z positions for volume intersections.
  // Note, values here are in mm to match the units passed in from the Gaudi/Rec side
  detail::init_param( m_z_entry, "Rh1SubMasterUpstrZLim" );
  detail::init_param( m_z_exit, "Rh1SubMasterDnstrZLim" );
  detail::init_param( m_x_size_entry, "Rh1SubMasterXSize" );
  detail::init_param( m_y_size_entry, "Rh1SubMasterYSize" );
  detail::init_param( m_x_size_exit, "Rh1SubMasterXSize" );
  detail::init_param( m_y_size_exit, "Rh1SubMasterYSize" );

  // form the entry / exit planes
  formPlanes();

  detail::print( *this );
}

//=================================================================================================//

void DeRich1RadiatorGasObject::PrintR1GasGeneralInfoFromDBInObj() const {

  // Printing to be improved using msg svc
  // Here an example of accessing general info of the gas radiator from DD4HEP structure is shown.
  // The ref index is accessed from the dd4hep structure for illustration. Other geometry parameters can also be
  // retrieved in a similar way from the detelement.
  // In the production version these illustrations will be suppressed.

  auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
  aRichMatOpd->PrintRichMatProperty( detector().volume().material(), RichMatPropType::CkvRndx );

  // Other options to print are commented out
  // RichDD4HepAccessUtil* aRichDD4HepAccessUtil = RichDD4HepAccessUtil::getRichDD4HepAccessUtilInstance();
  // aRichDD4HepAccessUtil->PrintRichMatTabProp(RichMatNameWOP::R1RGas,RichMatPropType::CkvRndx);
  // std::string aR1RadiatorGas_RefIndexTableName = aRichMatOpd->GetTableName(
  // RichMatNameWOP::R1RGas,RichMatPropType::CkvRndx);
  // aRichDD4HepAccessUtil->PrintAMatTable(aR1RadiatorGas_RefIndexTableName);
}

//=================================================================================================//

void DeRich1RadiatorGasObject::RetrieveAndPrintDebugR1GasNominalRefIndexFromLocalInpInObj() {

  // Printing to be improved using msg svc
  // Here the full table of ref index is accessed locally from the inputs to dd4hep structure.
  // The values within the energy limits are printed for
  // illustration. This illustration will be suppressed in production version.

  dd4hep::printout( dd4hep::DEBUG, "DeRich1RadiatorGasObject",
                    "Debug Printing R1 Radiator gas Ref index from local input in DeRich1GasRadiator" );

  auto aRichMatPropData      = RichMatPropData::getRichMatPropDataInstance();
  auto aRichMatC4F10EnRiFull = aRichMatPropData->RichMatC4F10EnRi();

  for ( auto itH = aRichMatC4F10EnRiFull.begin(); itH != aRichMatC4F10EnRiFull.end(); ++itH ) {
    auto aEnValH = itH->first;
    if ( aEnValH >= m_PhotonMinEnergy && aEnValH <= m_PhotonMaxEnergy ) {
      auto aRiValH = itH->second;
      dd4hep::printout( dd4hep::DEBUG, "DeRich1RadiatorGasObject", "En Ri %lf %lf", aEnValH, aRiValH );
    }
  }
  dd4hep::printout( dd4hep::DEBUG, "DeRich1RadiatorGasObject",
                    "End of debug printing R1 Radiator gas Ref index from local input in DeRich1GasRadiator" );
}
//===================================================================================================//
