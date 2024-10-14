//==========================================================================
//  LHCb Rich Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-11-09
//
//==========================================================================//

#include "Detector/Rich1/RichSurfaceUtil.h"
#include "Detector/Rich1/RichGeoUtil.h"
#include "Detector/Rich2/Rich2MirrorGeoAux.h"
#include <cassert>
#include <memory>
#include <mutex>

//==========================================================================//
void RichSurfaceUtil::PrintRichSurfProperty( const dd4hep::OpticalSurface& aSurf, RichSurfPropType aSprop ) {
  const auto aPropType    = RichSurfacePropTypeName( aSprop );
  const auto aSurfaceName = aSurf->GetName();

  const auto  UC          = aSurf.property( aPropType.c_str() );
  const auto  aNumColC    = (int)( UC->GetCols() );
  const auto  aNumRowC    = (int)( UC->GetRows() );
  const char* aSurfTitleC = UC->GetName();
  std::cout << " Printing Surface Property Table: using PhotonEnergy in eV" << std::endl;
  std::cout << "Current Surface Name PropertyType NumColc NumRowC: " << aSurfaceName << "  " << aSurfTitleC << "  "
            << aPropType << "    " << aNumColC << "   " << aNumRowC << std::endl;
  std::cout << "Surface: PhotonEnergy  PropertyValue " << std::endl;

  for ( size_t row = 0; row < (size_t)aNumRowC; ++row ) {
    printf( "   " );
    for ( size_t col = 0; col < (size_t)aNumColC; ++col ) { printf( "%15.8g", UC->Get( row, col ) ); }
    printf( "\n" );
  }
  std::cout << " End of the Surface Table PrintOut for " << aSurfTitleC << std::endl;
}
//==========================================================================//
RichSurfaceUtil::TabData RichSurfaceUtil::RetrieveRichSurfProperty( const dd4hep::OpticalSurface& aSurf,
                                                                    RichSurfPropType              aSprop ) const {
  const auto aPropType = RichSurfacePropTypeName( aSprop );
  // std::string aSurfaceName = aSurf->GetName();
  TabData aSurfTab;

  auto UC = aSurf.property( aPropType.c_str() );
  // int                              aNumColC    = (int)( UC->GetCols() );
  auto aNumRowC = (int)( UC->GetRows() );

  for ( size_t row = 0; row < (size_t)aNumRowC; ++row ) {
    const auto aEnValF = UC->Get( row, 0 );
    const auto aSPVal  = UC->Get( row, 1 );
    aSurfTab.emplace( aEnValF, aSPVal );
  }
  return aSurfTab;
}
//==========================================================================//
void RichSurfaceUtil::InitRichGeneralSurfNames() {

  m_RichSurfacePropTypeNames.clear();
  m_NumRichSurfProp = 3; // For the three properties REFLECTIVITY and EFFICIENCY and TRANSMITTANCE
  m_RichSurfacePropTypeNames.resize( m_NumRichSurfProp );
  m_RichSurfacePropTypeNames[toInTs( RichSurfPropType::Refl )]  = "REFLECTIVITY";
  m_RichSurfacePropTypeNames[toInTs( RichSurfPropType::Effic )] = "EFFICIENCY";
  m_RichSurfacePropTypeNames[toInTs( RichSurfPropType::Trans )] = "TRANSMITTANCE";

  m_RichMapmtQWSurfaceName    = "RichPMTQuartzWindowSurface";
  m_RichMapmtPCSurfaceName    = "RichPMTQWPhCathodeSurface";
  m_RichMapmtMTSurfaceName    = "RichPMTEnvSideTubeMetalSurface";
  m_RichMapmtAnodeSurfaceName = "RichPMTAnodeSurface";
}
//==========================================================================//
void RichSurfaceUtil::InitRich1SurfNames() {
  m_Rich1Mirror1SurfaceNames.clear();
  m_Rich1Mirror2SurfaceNames.clear();
  m_Rich1Mirror1BorderSurfaceNames.clear();
  m_Rich1Mirror2BorderSurfaceNames.clear();

  m_Rich1GasQuartzWindowSurfaceName        = "Rich1GasQuartzWindowSurface";
  m_Rich1GasQuartzWindowBorderSurfaceNames = {"Rich1QuartzWindowBorderSurfaceTopH0",
                                              "Rich1QuartzWindowBorderSurfaceBotH1"};

  m_Rich1GasQuartzWindowBorderBackSurfaceNames = {"Rich1QuartzWindowBorderBackSurfaceTopH0",
                                                  "Rich1QuartzWindowBorderBackSurfaceBotH1"};

  //------------------------------------------------------------//
  const auto aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();

  const auto aR1M1QuadNames = aRichGeoUtil->Rich1Mirror1QuadrantNames();

  m_NumRich1MirrorQuad = aRichGeoUtil->Rich1NumMirror1Seg();

  m_Rich1Mirror1SurfaceNames.resize( m_NumRich1MirrorQuad );
  m_Rich1Mirror1BorderSurfaceNames.resize( m_NumRich1MirrorQuad );

  const std::string aR1M1SurfPref       = "Rich1Mirror1SurfaceQuad";
  const std::string aR1M1BorderSurfPref = "Rich1Mirror1BorderSurfaceQuad";
  for ( int iq = 0; iq < m_NumRich1MirrorQuad; iq++ ) {
    m_Rich1Mirror1SurfaceNames[iq]       = aR1M1SurfPref + aR1M1QuadNames[iq];
    m_Rich1Mirror1BorderSurfaceNames[iq] = aR1M1BorderSurfPref + aR1M1QuadNames[iq];
  }

  const auto aR1M2SegNames      = aRichGeoUtil->Rich1Mirror2SegNames();
  m_NumRich1Mirror2SurfaceNames = (int)aR1M2SegNames.size();
  m_Rich1Mirror2SurfaceNames.resize( m_NumRich1Mirror2SurfaceNames );
  m_Rich1Mirror2BorderSurfaceNames.resize( m_NumRich1Mirror2SurfaceNames );

  const auto kqg = aRichGeoUtil->Rich1NumMirror2SegInAQuad();

  const std::string aR1M2SurfPref       = "Rich1Mirror2Surface";
  const std::string aR1M2BorderSurfPref = "Rich1Mirror2BorderSurface";
  const std::string aR1M2SurfSuf        = "Seg";

  for ( int jq = 0; jq < m_NumRich1MirrorQuad; jq++ ) {
    auto cq = jq;
    if ( jq == 2 ) cq = 3;
    if ( jq == 3 ) cq = 2;
    for ( int ig = 0; ig < kqg; ig++ ) {
      const auto cg                  = ( jq * kqg ) + ig;
      m_Rich1Mirror2SurfaceNames[cg] = aR1M2SurfPref + aR1M1QuadNames[cq] + aR1M2SurfSuf + aR1M2SegNames[cg];
      m_Rich1Mirror2BorderSurfaceNames[cg] =
          aR1M2BorderSurfPref + aR1M1QuadNames[cq] + aR1M2SurfSuf + aR1M2SegNames[cg];
    }
  }

  m_Rich1Mirror1NominalSurfaceName = "Rich1Mirror1SurfaceNominal";
  m_Rich1Mirror2NominalSurfaceName = "Rich1Mirror2SurfaceNominal";
}
//========================================================================== //
void RichSurfaceUtil::InitRich2SurfNames() {

  m_Rich2Mirror1SurfaceNames.clear();
  m_Rich2Mirror2SurfaceNames.clear();
  m_Rich2Mirror1BorderSurfaceNames.clear();
  m_Rich2Mirror2BorderSurfaceNames.clear();

  m_Rich2GasQuartzWindowBorderSurfaceNames.clear();
  m_Rich2GasQuartzWindowBorderBackSurfaceNames.clear();

  m_Rich2GasQuartzWindowSurfaceName        = "Rich2GasQuartzWindowSurface";
  m_Rich2GasQuartzWindowBorderSurfaceNames = {"Rich2QuartzWindowBorderSurfaceLeftH0",
                                              "Rich2QuartzWindowBorderSurfaceRightH1"};

  m_Rich2GasQuartzWindowBorderBackSurfaceNames = {"Rich2QuartzWindowBorderBackSurfaceLeftH0",
                                                  "Rich2QuartzWindowBorderBackSurfaceRightH1"};

  const auto aRich2MirrorGeoAux = Rich2MirrorGeoAux::getRich2MirrorGeoAuxInstance();
  const auto aNumR2M1Seg        = aRich2MirrorGeoAux->NumRich2SphMirrSeg();
  const auto aNumR2M2Seg        = aRich2MirrorGeoAux->NumRich2SecMirrSeg();
  m_Rich2Mirror1SurfaceNames.resize( aNumR2M1Seg );
  m_Rich2Mirror1BorderSurfaceNames.resize( aNumR2M1Seg );
  m_Rich2Mirror2SurfaceNames.resize( aNumR2M2Seg );
  m_Rich2Mirror2BorderSurfaceNames.resize( aNumR2M2Seg );
  const std::string aR2M1Pref       = "Rich2SphMirrorSurface";
  const std::string aR2M1BorderPref = "Rich2SphMirrorBorderSurface";

  const std::string aHalfHexS = "HalfHexSeg";
  const std::string aHexS     = "HexSeg";

  for ( int iM = 0; iM < aNumR2M1Seg; iM++ ) {
    const auto aH                        = ( aRich2MirrorGeoAux->IsRich2SphHalfSeg( iM ) ) ? aHalfHexS : aHexS;
    const auto sInd                      = aRich2MirrorGeoAux->Rich2SphMIndexStr( iM );
    m_Rich2Mirror1SurfaceNames[iM]       = aR2M1Pref + aH + sInd;
    m_Rich2Mirror1BorderSurfaceNames[iM] = aR2M1BorderPref + aH + sInd;
  }

  const std::string aR2M2Pref       = "Rich2SecMirrorSurfaceSeg";
  const std::string aR2M2BorderPref = "Rich2SecMirrorBorderSurfaceSeg";
  for ( int iC = 0; iC < aNumR2M2Seg; iC++ ) {
    const auto cInd                      = aRich2MirrorGeoAux->Rich2SecMIndexStr( iC );
    m_Rich2Mirror2SurfaceNames[iC]       = aR2M2Pref + cInd;
    m_Rich2Mirror2BorderSurfaceNames[iC] = aR2M2BorderPref + cInd;
  }

  m_Rich2Mirror1NominalSurfaceName = "Rich2Mirror1SurfaceNominal";
  m_Rich2Mirror2NominalSurfaceName = "Rich2Mirror2SurfaceNominal";
}

//========================================================================== //
/// Initializing constructor

RichSurfaceUtil::RichSurfaceUtil() { InitRichGeneralSurfNames(); }
//==================================================================================//

RichSurfaceUtil* RichSurfaceUtil::getRichSurfaceUtilInstance() {
  static std::once_flag                   alloc_instance_once;
  static std::unique_ptr<RichSurfaceUtil> instance;
  std::call_once( alloc_instance_once, []() { instance = std::make_unique<RichSurfaceUtil>(); } );
  assert( instance.get() );
  return instance.get();
}
//==================================================================================//
