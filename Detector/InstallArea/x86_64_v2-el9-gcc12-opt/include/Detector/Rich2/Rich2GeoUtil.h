//==========================================================================
//  LHCb Rich Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2021-01-14
//
//==========================================================================

#pragma once

#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"
#include "Detector/Rich1/RichGeomVersion.h"
#include "XML/Utilities.h"
#include <type_traits>

enum class Rich2GeomTransformLabel : int {
  pvRich2GasEnclosureTN,
  pvRich2TubeTN,
  pvRich2GasContWallTopTN,
  pvRich2GasContWallBotTN,
  pvRich2GasContWallSide0TN,
  pvRich2GasContWallSide1TN,
  pvRich2EntryWindowSkinDnsTN,
  pvRich2EntryWindowSkinUpsTN,
  pvRich2EntryWindowPMITN,
  pvRich2EntryWinTubeLockTN,
  pvRich2ExitWindowSkinDnsTN,
  pvRich2ExitWindowSkinUpsTN,
  pvRich2ExitWindowPMITN,
  pvRich2ExitWinTubeLockTN,
  pvRich2SphMirrorMaster0TN,
  pvRich2SphMirrorMaster1TN,
  pvRich2SphMSupport0TN,
  pvRich2SphMSupport1TN,
  pvRich2SecMirrorMaster0TN,
  pvRich2SecMirrorMaster1TN,
  pvRich2SecMSupport0TN,
  pvRich2SecMSupport1TN,
  pvRich2QuartzWindow0TN,
  pvRich2QuartzWindow1TN,
  pvRich2MagShFront0TN,
  pvRich2MagShTop0TN,
  pvRich2MagShBottom0TN,
  pvRich2MagShSideBack0TN,
  pvRich2MagShSideFront0TN,
  pvRich2MagShFront1TN,
  pvRich2MagShTop1TN,
  pvRich2MagShBottom1TN,
  pvRich2MagShSideBack1TN,
  pvRich2MagShSideFront1TN,
  pvRich2PhDetGasEncl0TN,
  pvRich2PhDetGasEncl1TN,
  pvRich2PhDetPanel0TN,
  pvRich2PhDetPanel1TN,
  pvRichSystemTN,
  pvRich2SuperStrAirBoxTopTN,
  pvRich2SuperStrAirBoxBotTN,
  pvRich2SuperStrAirTrapRightTN,
  pvRich2SuperStrAirTrapLeftTN
};

class Rich2GeoUtil {
private:
  template <typename H>
  constexpr auto toInHe( H h ) const noexcept {
    return static_cast<std::underlying_type_t<H>>( h );
  }

public:
  void build_Rich2Transforms( xml_h tr_handle );
  void build_Rich2SuperStrPhysTransforms( xml_h tr_handle, const std::string aSupStrFileName );

  void InitRich2TransformsAndNames();

  void InitRich2FillTN( const Rich::Detector::NamedVector<int>& aRich2CompPvI );

  const dd4hep::Transform3D& getRich2GeomTransform( const std::string& aName );

  const auto& getRich2TransformName( Rich2GeomTransformLabel aLabel ) const {
    return m_Rich2TransformNames[toInHe( aLabel )];
  }

  const auto& getRich2PhysVolName( Rich2GeomTransformLabel aLabel ) const {
    return m_Rich2PhysVolNames[toInHe( aLabel )];
  }

  int getRich2PhysVolCopyNumber( Rich2GeomTransformLabel aLabel ) const {
    return m_Rich2PhysVolCopyNumbers[toInHe( aLabel )];
  }

  bool IsInRich2TransformList( const std::string& aTrName );
  bool IsRich2SuperStrComp( const std::string& aDetFileName );

  static Rich2GeoUtil* getRich2GeoUtilInstance();

  void setRh2DebugLevel( bool rLevel ) { m_Rich2GeomUtilDebug = rLevel; }

  int Rich2NumSides() const { return m_Rich2NumSides; }

  const auto& Rich2MasterLVName() const { return m_Rich2MasterLVName; }

  const auto& Rich2GasEnclosureLVName() const { return m_Rich2GasEnclosureLVName; }
  const auto& Rich2GasEnclosureDetName() const { return m_Rich2GasEnclosureDetName; }

  const auto& Rich2_BeamTubeLVName() const { return m_Rich2_BeamTubeLVName; }
  const auto& Rich2_BeamTubeDetName() const { return m_Rich2_BeamTubeDetName; }

  const auto& Rich2PhDetGasEnclLVName( int aSide ) const { return m_Rich2PhDetGasEnclLVName[aSide]; }
  const auto& Rich2PhDetPanelLVName( int kSide ) const { return m_Rich2PhDetPanelLVName[kSide]; }
  const auto& Rich2PhDetPanelDetName( int kSide ) const { return m_Rich2PhDetPanelDetName[kSide]; }

  const auto& Rich2RichSystemLVName() const { return m_Rich2RichSystemLVName; }
  const auto& Rich2RichSystemDetName() const { return m_Rich2RichSystemDetName; }

  const auto& Rich2SphMirrorMasterLVName( int pSide ) const { return m_Rich2SphMirrorMasterLVName[pSide]; }
  const auto& Rich2SecMirrorMasterLVName( int qSide ) const { return m_Rich2SecMirrorMasterLVName[qSide]; }
  const auto& Rich2SphMirrorMasterDetName( int pSide ) const { return m_Rich2SphMirrorMasterDetName[pSide]; }
  const auto& Rich2SecMirrorMasterDetName( int qSide ) const { return m_Rich2SecMirrorMasterDetName[qSide]; }

  const auto& Rich2SphMirrorSupportLVName() const { return m_Rich2SphMirrorSupportLVName; }
  const auto& Rich2SecMirrorSupportLVName() const { return m_Rich2SecMirrorSupportLVName; }
  const auto& Rich2SuperStrMasterLVName( int aLateralSide ) const { return m_Rich2SuperStrMasterLVNames[aLateralSide]; }
  const auto& Rich2SuperStrSectionName( int aLateralSide ) const { return m_Rich2SuperStrSectionNames[aLateralSide]; }
  int         Rich2NumSuperStrLateralSides() const { return m_Rich2NumSuperStrLateralSides; }
  Rich2GeomTransformLabel Rich2SuperStrMasterTN( int aLateralSide ) const {
    return m_Rich2SuperStrMasterTNs[aLateralSide];
  }

  const auto& Rich2SuperStrPhysVolName( int aLateralSide, int aIndex ) const {
    return m_Rich2SuperStrPhysVolNames[aLateralSide][aIndex];
  }
  const auto& Rich2SuperStrLogVolName( int aLateralSide, int aIndex ) const {
    return m_Rich2SuperStrLogVolNames[aLateralSide][aIndex];
  }
  const dd4hep::Transform3D& getRich2SuperStrTransform( int aLateralSide, int aIndex );
  int getNumRich2SuperStrPhysVols( int aLateralSide ) const { return m_Rich2SuperStrPhysVolNames[aLateralSide].size(); }

  const auto& Rich2GasQuartzWMatName() const { return m_Rich2GasQuartzWMatName; }
  const auto& Rich2NitrogenMatName() const { return m_Rich2NitrogenMatName; }
  const auto& Rich2CF4MatName() const { return m_Rich2CF4MatName; }
  const auto& Rich2RadiatorGasMatName() const { return m_Rich2RadiatorGasMatName; }
  const auto& Rich2CO2GasMatName() const { return m_Rich2CO2GasMatName; }
  void        setRich2GeometryVersion( RichGeomLabel aRG ) { m_Cur_Rich2GeomVersion = aRG; }
  const auto& Cur_Rich2GeomVersion() { return m_Cur_Rich2GeomVersion; }

  // Rich2GeoUtil() { InitRich2TransformsAndNames(); }
  Rich2GeoUtil() {}

private:
  int m_Rich2MaxNumGeomTransforms{};
  int m_Rich2NumSides{};
  int m_Rich2NumSuperStrLateralSides{};

  std::vector<std::string>                   m_Rich2TransformNames;
  std::map<std::string, dd4hep::Transform3D> m_Rich2GeomTransforms;
  std::vector<std::string>                   m_Rich2PhysVolNames;
  std::vector<int>                           m_Rich2PhysVolCopyNumbers;

  std::string m_Rich2MasterLVName;
  std::string m_Rich2GasEnclosureLVName;
  std::string m_Rich2GasEnclosureDetName;

  std::string m_Rich2_BeamTubeLVName;
  std::string m_Rich2_BeamTubeDetName;

  std::vector<std::string> m_Rich2PhDetGasEnclLVName;

  std::vector<std::string> m_Rich2PhDetPanelLVName;
  std::vector<std::string> m_Rich2PhDetPanelDetName;

  std::string m_Rich2RichSystemLVName;
  std::string m_Rich2RichSystemDetName;

  std::vector<std::string> m_Rich2SphMirrorMasterLVName;
  std::vector<std::string> m_Rich2SecMirrorMasterLVName;
  std::vector<std::string> m_Rich2SphMirrorMasterDetName;
  std::vector<std::string> m_Rich2SecMirrorMasterDetName;

  std::string                          m_Rich2SphMirrorSupportLVName;
  std::string                          m_Rich2SecMirrorSupportLVName;
  std::vector<std::string>             m_Rich2SuperStrMasterLVNames;
  std::vector<std::string>             m_Rich2SuperStrSectionNames;
  std::vector<Rich2GeomTransformLabel> m_Rich2SuperStrMasterTNs;

  std::vector<std::vector<std::string>>                   m_Rich2SuperStrPhysVolNames;
  std::vector<std::vector<std::string>>                   m_Rich2SuperStrLogVolNames;
  std::vector<std::map<std::string, dd4hep::Transform3D>> m_Rich2SuperStrTransforms;
  int                                                     m_Rich2MaxNumSuperStrTransforms{};

  // Parameters used in simulation

  std::string   m_Rich2GasQuartzWMatName;
  std::string   m_Rich2NitrogenMatName;
  std::string   m_Rich2CF4MatName;
  std::string   m_Rich2RadiatorGasMatName;
  std::string   m_Rich2CO2GasMatName;
  RichGeomLabel m_Cur_Rich2GeomVersion;
  bool          m_Rich2GeomUtilDebug{true};
};
