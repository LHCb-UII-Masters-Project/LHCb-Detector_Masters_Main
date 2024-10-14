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
/** SE 2022-01-10    **/
/*****************************************************************************/

#include "Detector/Rich2/DetElemAccess/DeRich2RadiatorGas.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich1/RichMatOPD.h"
#include "Detector/Rich1/RichMatPropData.h"
#include "Detector/Rich2/Rich2GeoUtil.h"

using namespace LHCb::Detector::detail;

DeRich2RadiatorGasObject::DeRich2RadiatorGasObject( const dd4hep::DetElement&             de,
                                                    dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeRichRadiatorObject( de, ctxt )
    , m_Rich2SphMirrorMasterDetElem{{{de.child(
                                          Rich2GeoUtil::getRich2GeoUtilInstance()->Rich2SphMirrorMasterDetName( 0 ) ),
                                      ctxt},
                                     {de.child(
                                          Rich2GeoUtil::getRich2GeoUtilInstance()->Rich2SphMirrorMasterDetName( 1 ) ),
                                      ctxt}}}
    , m_Rich2SecMirrorMasterDetElem{
          {{de.child( Rich2GeoUtil::getRich2GeoUtilInstance()->Rich2SecMirrorMasterDetName( 0 ) ), ctxt},
           {de.child( Rich2GeoUtil::getRich2GeoUtilInstance()->Rich2SecMirrorMasterDetName( 1 ) ), ctxt}}} {

  m_gasName = Rich::CF4;

  detail::init_param( m_Rich2NumSides, "Rh2NumSides" );
  detail::init_param( m_Presure_Nominal, "CF4Pressure" );
  detail::init_param( m_Temperature_Nominal, "CF4Temperature" );
  detail::init_param( m_PhotonMinEnergy, "PhotonCkvMinEnergyCF4" );
  detail::init_param( m_PhotonMaxEnergy, "PhotonCkvMaxEnergyCF4" );

  auto r2          = dd4hep::Detector::getInstance().detector( "Rich2" );
  m_refScalefactor = ctxt.condition( dd4hep::ConditionKey::KeyMaker( r2, "RefractivityScaleFactor" ).hash );
  m_gasParameters  = ctxt.condition( dd4hep::ConditionKey::KeyMaker( r2, "GasParameters" ).hash );

  // set nominal ref index
  // m_GasRefIndex = RetrieveNominalGasRefIndexInDeObjectFromDB( );

  // for illustration dummy transformation with nominal pressure and temperature.
  // Can be used with different pressure and temperature privided by conditions DB.
  // The line above is redundant when using the CurrentScaledRefIndexTab,
  // but kept for now, for illustration.
  m_GasRefIndex = CurrentScaledRefIndexInObjTab( m_Presure_Nominal, m_Temperature_Nominal );

  // Form nominal entry / exit plane z positions for volume intersections.
  // Note, values here are in mm to match the units passed in from the Gaudi/Rec side
  const auto meanZ = detail::dd4hep_param<double>( "Rich2MasterZ" );
  const auto sizeZ = detail::dd4hep_param<double>( "Rich2MasterZSize" );
  m_z_entry        = meanZ - ( 0.5 * sizeZ );
  m_z_exit         = meanZ + ( 0.5 * sizeZ );
  detail::init_param( m_x_size_entry, "Rh2MainGasSizeX1" );
  detail::init_param( m_y_size_entry, "Rh2MainGasSizeY1" );
  detail::init_param( m_x_size_exit, "Rh2MainGasSizeX2" );
  detail::init_param( m_y_size_exit, "Rh2MainGasSizeY2" );

  // form the entry / exit planes
  formPlanes();

  detail::print( *this );
}

//=================================================================================================//

void DeRich2RadiatorGasObject::PrintR2GasGeneralInfoFromDBInObj() const {

  // Printing to be improved using msg svc
  // Here an example of accessing general info of the gas radiator from DD4HEP structure is shown.
  // The ref index is accessed from the dd4hep structure for illustration. Other geometry parameters can also be
  // retrieved in a similar way from the detelement.
  // In the production version these illustrations will be suppressed.

  auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
  aRichMatOpd->PrintRichMatProperty( detector().volume().material(), RichMatPropType::CkvRndx );
}

//=================================================================================================//

void DeRich2RadiatorGasObject::RetrieveAndPrintDebugR2GasNominalRefIndexFromLocalInpInObj() {

  // Printing to be improved using msg svc
  // Here the full table of ref index is accessed locally from the inputs to dd4hep structure.
  // The values within the energy limits are printed for
  // illustration. This illustration will be suppressed in production version.

  dd4hep::printout( dd4hep::DEBUG, "DeRich2RadiatorGasObject",
                    "Debug Printing R2 Radiator gas Ref index from local input in DeRich2GasRadiator" );

  auto aRichMatPropData    = RichMatPropData::getRichMatPropDataInstance();
  auto aRichMatCF4EnRiFull = aRichMatPropData->RichMatCF4EnRi();

  for ( auto itH = aRichMatCF4EnRiFull.begin(); itH != aRichMatCF4EnRiFull.end(); ++itH ) {
    auto aEnValH = itH->first;
    if ( aEnValH >= m_PhotonMinEnergy && aEnValH <= m_PhotonMaxEnergy ) {
      auto aRiValH = itH->second;
      dd4hep::printout( dd4hep::DEBUG, "DeRich2RadiatorGasObject", "En Ri %lf %lf", aEnValH, aRiValH );
    }
  }
  dd4hep::printout( dd4hep::DEBUG, "DeRich2RadiatorGasObject",
                    "End of debug printing R2 Radiator gas Ref index from local input in DeRich2GasRadiator" );
}
//===================================================================================================//
