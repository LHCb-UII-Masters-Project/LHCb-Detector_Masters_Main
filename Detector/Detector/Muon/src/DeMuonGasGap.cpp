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

#include "Detector/Muon/DeMuonGasGap.h"
#include "Core/DeIOV.h"
#include "Core/Units.h"
#include "Core/yaml_converters.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"

#include <array>
#include <iostream>
#include <regex>
#include <string>

using DeMuonGasGap = LHCb::Detector::detail::DeMuonGasGapObject;

LHCb::Detector::detail::DeMuonGasGapObject::DeMuonGasGapObject( const dd4hep::DetElement&             de,
                                                                dd4hep::cond::ConditionUpdateContext& ctxt,
                                                                unsigned int                          gID )
    : DeIOVObject( de, ctxt, 11009 ) {

  //
  ROOT::Math::XYZPoint globalPoint = toGlobal( ( ROOT::Math::XYZPoint( 0., 0., 0. ) ) );

  m_Xgap  = globalPoint.x();
  m_Ygap  = globalPoint.y();
  m_Zgap  = globalPoint.z();
  m_gapID = gID;
  //
  //
  TGeoVolume* vol   = de.volume();
  TGeoShape*  shape = vol->GetShape();
  // Because we know it's a box....
  TGeoBBox* box = (TGeoBBox*)shape;
  m_DXgap       = toLHCbLengthUnits( ( box->GetDX() ) );
  m_DYgap       = toLHCbLengthUnits( ( box->GetDY() ) );
  m_DZgap       = toLHCbLengthUnits( ( box->GetDZ() ) );
  //
  //  std::cout << "gap position " << m_Z << " " << std::endl;
}

ROOT::Math::XYZPoint DeMuonGasGap::getPosition() const { return ROOT::Math::XYZPoint( m_Xgap, m_Ygap, m_Zgap ); }
