//==========================================================================
//  LHCb Rich Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2021-10-03
//
//==========================================================================

#include "Detector/Rich1/RichDD4HepAccessUtil.h"
#include "DD4hep/DetectorData.h"
#include "Detector/Rich1/RichMatOPD.h"
#include <memory>
#include <mutex>

//==========================================================================//
dd4hep::Detector& RichDD4HepAccessUtil::GetCurDD4HepDetectorInstance() {
  // In the future this may be improved, once the method is known.
  // For now using the same method used in Gaussino implementation.
  return ( dd4hep::Detector::getInstance() );
}
//==========================================================================//
TGDMLMatrix* RichDD4HepAccessUtil::GetGDMLTable( const std::string& aTableName ) {
  return ( GetCurDD4HepDetectorInstance().manager().GetGDMLMatrix( aTableName.c_str() ) );
}
//==========================================================================//
void RichDD4HepAccessUtil::PrintAMatTable( const std::string& aTableName ) {
  auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
  auto aTab        = GetGDMLTable( aTableName );
  aRichMatOpd->PrintAGDMLMatrix( aTab );
}
//==========================================================================//
dd4hep::Material::Property RichDD4HepAccessUtil::GetMatTabProp( const RichMatNameWOP&  aMatNameW,
                                                                const RichMatPropType& aPropType ) {
  auto        aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
  auto        aMatName    = aRichMatOpd->RichMatNameWithOptProp( aMatNameW );
  const auto& aRichMat    = GetCurDD4HepDetectorInstance().material( aMatName.c_str() );
  auto        aMPType     = aRichMatOpd->RichMatPropTypeName( aPropType );
  return aRichMat.property( aMPType.c_str() );
}
//==========================================================================//
void RichDD4HepAccessUtil::PrintRichMatTabProp( const RichMatNameWOP& aMatNameW, const RichMatPropType& aPropType ) {
  auto        aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
  auto        aMatName    = aRichMatOpd->RichMatNameWithOptProp( aMatNameW );
  const auto& aRichMat    = GetCurDD4HepDetectorInstance().material( aMatName.c_str() );
  aRichMatOpd->PrintRichMatProperty( aRichMat, aPropType );
}
//==========================================================================//
dd4hep::OpticalSurfaceManager RichDD4HepAccessUtil::GetOpticalSurfManager() {
  return ( GetCurDD4HepDetectorInstance().surfaceManager() );
}
//==========================================================================//
dd4hep::OpticalSurface RichDD4HepAccessUtil::GetAnOpticalSurface( const std::string& aSurfName ) {
  return ( GetOpticalSurfManager().opticalSurface( aSurfName ) );
}
//==========================================================================//
RichDD4HepAccessUtil* RichDD4HepAccessUtil::getRichDD4HepAccessUtilInstance() {
  static std::once_flag                        alloc_once;
  static std::unique_ptr<RichDD4HepAccessUtil> instance;
  std::call_once( alloc_once, []() { instance = std::make_unique<RichDD4HepAccessUtil>(); } );
  assert( instance.get() );
  return instance.get();
}
//==========================================================================//
