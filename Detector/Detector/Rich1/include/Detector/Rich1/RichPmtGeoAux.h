//==========================================================================
//  LHCb Rich Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-08-28
//
//========================================================================== //

#pragma once

#include "Core/DeIOV.h"
#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich/Rich_Geom_Constants_De.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"
#include "XML/Utilities.h"
#include <array>
#include <cstdint>
#include <type_traits>
#include <vector>

enum class RichPmtGeomTransformLabel : int {
  pvRichPMTSMasterTN,
  pvRichPMTAnode0000TN,
  pvRichPMTQuartzTN,
  pvRichPMTPhCathodeTN,
  pvRichPMTSideEnvelopeTN,
  pvRichPMTBackEnvelopeTN,
  pvRichPMTFrontRingTN,
  pvRichPmtSingleShieldingLongPlateTN,
  pvRichPmtSingleShieldingSemiPlateTopTN,
  pvRichPmtSingleShieldingSemiPlateBottomTN,
  pvRichGrandPMTSMasterTN,
  pvRichPMTAnode0001TN,
  pvRichGrandPMTQuartzTN,
  pvRichGrandPMTPhCathodeTN,
  pvRichGrandPMTSideEnvelopeTN,
  pvRichGrandPMTBackEnvelopeTN,
  pvRichGrandPMTFrontRingTN
};
enum class RichStdPmtCompLabel : int {
  stdPmtAnode,
  stdPmtQW,
  stdPmtSideEnv,
  stdPmtBackEnv,
  stdPmtPhc,
  stdPmtFR,
  stdPmtSM
};
enum class RichGrandPmtCompLabel : int { grPmtAnode, grPmtQW, grPmtSideEnv, grPmtBackEnv, grPmtPhc, grPmtFR, grPmtSM };
//========================================================================== //

class RichPmtGeoAux {
public:
  void               initRich1PmtModuleTypes();
  void               loadRich1PmtArraySpecParam();
  void               initRich1PmtStructureLabels();
  void               initRichPmtGeneralStructureLabels();
  void               initRichPmtComponentLabels();
  void               initRich1PmtStructures();
  void               initRich2PmtStructures();
  void               InitRichPmtFillTN( const Rich::Detector::NamedVector<int>& aRichPmtCompPvI );
  void               CreateRich1PmtModuleTransforms();
  void               initRich1PmtECTypes( int aModuleNum, const std::string& aModuleLabel );
  void               CreateRich1PmtECRTransforms();
  void               initRich1PmtMasterTypes( int bModuleNum, const std::string& bModuleLabel );
  void               CreateRich1PmtMasterTransforms();
  std::array<int, 2> get_Rich1EcrRange( int iModule ) const;
  void               loadRich1PmtArrayExtraUserParam();

  auto Rh1NumPMTModulesInRow() const noexcept { return m_Rh1NumPMTModulesInRow; }
  auto Rh1NumPMTModulesInColumn() const noexcept { return m_Rh1NumPMTModulesInColumn; }
  auto Rh1TotalNumPmtModulesInPanel() const noexcept { return m_Rh1TotalNumPmtModulesInPanel; }
  auto Rh1NumPanels() const noexcept { return m_Rh1NumPanels; }
  auto Rich1TotalNumPmtModules() const noexcept { return m_Rh1TotalNumPmtModulesInRich1; }
  auto Rich1ModuleSideFromCopyNum( int aCopyNum ) const noexcept {
    return ( aCopyNum < m_Rh1TotalNumPmtModulesInPanel ) ? 0 : 1;
  }
  auto Rich1ModuleNumInPanelFromCopyNum( int aCopyNum ) const noexcept {
    return ( aCopyNum < m_Rh1TotalNumPmtModulesInPanel ) ? aCopyNum : ( aCopyNum - m_Rh1TotalNumPmtModulesInPanel );
  }
  auto Rich1PmtSideFromCopyNum( int aPmtCopyNum ) const noexcept {
    return ( aPmtCopyNum < m_Rich1MaxNumberOfPMTInPanel ) ? 0 : 1;
  }

  auto RhMaxNumECRInModule() const noexcept { return m_RhMaxNumECRInModule; }
  auto RhMaxNumECHInGrandModule() const noexcept { return m_RhMaxNumECHInGrandModule; }

  auto RhNumPMTInECR() const noexcept { return m_RhNumPMTInECR; }
  auto RhNumPmtInStdModule() const noexcept { return m_RhNumPmtInStdModule; }

  auto Rich2NumGrandPmtInECH() const noexcept { return m_RhNumGrandPmtInECH; }
  auto RhNumGrandPmtInGrandModule() const noexcept { return m_RhNumGrandPmtInGrandModule; }

  auto Rh2NumECInCurrentModule( int iM ) const noexcept {
    //  return ( IsRich2GrandPmtModule( iM ) ) ? ( RhMaxNumECHInGrandModule() ) : ( RhNumPMTInECR() );
    return ( IsRich2GrandPmtModule( iM ) ) ? ( RhMaxNumECHInGrandModule() ) : ( RhMaxNumECRInModule() );
  }
  auto Rh2NumPmtInCurrentEC( int iM ) const noexcept {
    return ( IsRich2GrandPmtModule( iM ) ) ? Rich2NumGrandPmtInECH() : RhNumPMTInECR();
  }

  auto RhPmtAnodeDetIdShiftFactor() const noexcept { return m_RhPmtAnodeDetIdShiftFactor; }
  auto RhPmtPhCathodeDetIdShiftFactor() const noexcept { return m_RhPmtPhCathodeDetIdShiftFactor; }

  auto Rh1NumPmtStdModulesInPanel() const noexcept { return m_Rh1NumPmtStdModulesInPanel; }
  auto Rh1NumPmtEdgeNonOuterCornerModulesInPanel() const noexcept {
    return m_Rh1NumPmtEdgeNonOuterCornerModulesInPanel;
  }
  auto Rh1NumPmtOuterCornerModulesInPanel() const noexcept { return m_Rh1NumPmtOuterCornerModulesInPanel; }

  const auto& Rich1PmtModuleLogVolName( int aModuleNum ) const noexcept {
    return m_Rich1PmtModuleLogVolNames[aModuleNum];
  }

  const auto& Rich1PmtModulePhysVolName( int aModuleNum ) const noexcept {
    return m_Rich1PmtModulePhysVolNames[aModuleNum];
  }

  const auto& Rich1PmtModuleShapeName( int aModNum ) const noexcept { return m_Rich1PmtModuleShapeNames[aModNum]; }

  const auto& getRich1PmtModuleTransform( int aModule ) const noexcept { return m_Rich1PmtModuleTransforms[aModule]; }
  const auto& getRich2PmtModuleTransform( int aModule ) const noexcept { return m_Rich2PmtModuleTransforms[aModule]; }
  const auto& Rich1PmtModuleDetName( int aModuleNum ) const noexcept { return m_Rich1PmtModuleDetNames[aModuleNum]; }
  const auto& Rich1PmtModuleCommonDetNamePref() const noexcept { return m_Rich1PmtModuleCommonDetNamePref; }

  std::string getRich1PhDetSupDetNameFromModuleNum( int iM ) const;
  std::string getRich2PhDetPanelDetNameFromModuleNum( int iM ) const;

  auto Rich1PmtModuleTypeIndex( int aM ) const noexcept { return m_Rich1PmtModuleTypeIndex[aM]; }

  const auto& Rich1PmtECRLogVolName( int aModuleNum, int aECRNum ) const noexcept {
    return m_Rich1PmtECRLogVolNames[aModuleNum][aECRNum];
  }
  const auto& Rich1PmtECRPhysVolName( int aModuleNum, int aECRNum ) const noexcept {
    return m_Rich1PmtECRPhysVolNames[aModuleNum][aECRNum];
  }
  const auto& Rich1PmtECRShapeName( int aModuleNum, int aECRNum ) const noexcept {
    return m_Rich1PmtECRShapeNames[aModuleNum][aECRNum];
  }

  const auto& Rich1PmtECRDetName( int aModuleNum, int aECRNum ) const noexcept {
    return m_Rich1PmtECRDetNames[aModuleNum][aECRNum];
  }
  const auto& Rich1PmtECRCommonDetNamePref() const noexcept { return m_Rich1PmtECRCommonDetNamePref; }

  bool Rich1PmtECRValid( int aModuleNum, int aECRNum ) const noexcept {
    return m_Rich1PmtECRValid[aModuleNum][aECRNum];
  }
  void setRich1PmtECRValid( int aModuleNum, int aECRNum ) { m_Rich1PmtECRValid[aModuleNum][aECRNum] = true; }

  const auto& getRich1PmtECRTransform( int aEcr ) const noexcept { return m_Rich1PmtECRTransforms[aEcr]; }
  const auto& getRich2PmtECRTransform( int aEcr ) const noexcept { return m_Rich2PmtECRTransforms[aEcr]; }
  const auto& getRich2PmtECHTransform( int aEch ) const noexcept { return m_Rich2PmtECHTransforms[aEch]; }

  bool Rich2PmtECRHValid( int aR2ModuleNum, int aECRHNum ) const noexcept {
    return m_Rich2PmtECRHValid[aR2ModuleNum][aECRHNum];
  }
  void setRich2PmtECRHValid( int aR2ModuleNum, int aECRHNum ) { m_Rich2PmtECRHValid[aR2ModuleNum][aECRHNum] = true; }

  const auto& Rich1PmtMasterLogVolName( int aModuleNum, int aECRNum, int aPmtNum ) const noexcept {
    return m_Rich1PmtMasterLogVolNames[aModuleNum][( aECRNum * m_RhNumPMTInECR ) + aPmtNum];
  }
  const auto& Rich1PmtMasterPhysVolName( int aModuleNum, int aECRNum, int aPmtNum ) const noexcept {
    return m_Rich1PmtMasterPhysVolNames[aModuleNum][( aECRNum * m_RhNumPMTInECR ) + aPmtNum];
  }
  const auto& Rich1PmtMasterShapeName( int aModuleNum, int aECRNum, int aPmtNum ) const noexcept {
    return m_Rich1PmtMasterShapeNames[aModuleNum][( aECRNum * m_RhNumPMTInECR ) + aPmtNum];
  }

  const auto& Rich1PmtMasterDetName( int aModuleNum, int aECRNum, int aPmtNum ) const noexcept {
    return m_Rich1PmtMasterDetNames[aModuleNum][( aECRNum * m_RhNumPMTInECR ) + aPmtNum];
  }
  const auto& Rich1PmtMPDetName( int aModuleNum, int aPmtNumInModule ) const noexcept {
    return m_Rich1PmtMasterDetNames[aModuleNum][aPmtNumInModule];
  }
  const auto& Rich1PmtMasterCommonDetNamePref() const noexcept { return m_Rich1PmtMasterCommonDetNamePref; }

  bool Rich1PmtMasterValid( int aModuleNum, int aECRNum, int aPmtNum ) const noexcept {
    return m_Rich1PmtMasterValid[aModuleNum][( aECRNum * m_RhNumPMTInECR ) + aPmtNum];
  }

  bool Rich1PmtModulePmtValid( int aModuleNum, int aPmtNumInModule ) const noexcept {
    return m_Rich1PmtMasterValid[aModuleNum][aPmtNumInModule];
  }

  void setRich1PmtMasterValid( int aModuleNum, int aECRNum, int aPmtNum ) {
    m_Rich1PmtMasterValid[aModuleNum][( aECRNum * m_RhNumPMTInECR ) + aPmtNum] = true;
  }

  bool Rich2PmtMasterValid( int aR2ModuleNum, int aECNum, int aPmtNum ) const noexcept {
    return m_Rich2PmtMasterValid[aR2ModuleNum][( aECNum * m_RhNumPMTInECR ) + aPmtNum];
  }
  void setRich2PmtMasterValid( int aR2ModuleNum, int aECNum, int aPmtNum ) {
    m_Rich2PmtMasterValid[aR2ModuleNum][( aECNum * m_RhNumPMTInECR ) + aPmtNum] = true;
  }

  const auto& getRich1PmtMasterTransform( int aP ) const noexcept { return m_Rich1PmtMasterTransforms[aP]; }
  const auto& getRich2PmtMasterTransform( int aP ) const noexcept { return m_Rich2PmtMasterTransforms[aP]; }

  const auto& Rich2GrandPmtMasterTransform() const noexcept { return m_Rich2GrandPmtMasterTransform; }

  const auto& RichMaPmtSubMasterLVName() const noexcept { return m_RichMaPmtSubMasterLVName; }
  const auto& RichMaPmtStdAnodeLVName() const noexcept { return m_RichMaPmtStdAnodeLVName; }
  const auto& RichMaPmtQuartzLVName() const noexcept { return m_RichMaPmtQuartzLVName; }
  const auto& RichMaPmtPhCathodeLVName() const noexcept { return m_RichMaPmtPhCathodeLVName; }
  const auto& RichMaPmtSideEnvelopeLVName() const noexcept { return m_RichMaPmtSideEnvelopeLVName; }
  const auto& RichMaPmtBackEnvelopeLVName() const noexcept { return m_RichMaPmtBackEnvelopeLVName; }
  const auto& RichMaPmtFrontRingLVName() const noexcept { return m_RichMaPmtFrontRingLVName; }

  const auto& Rich2MaPmtSubMasterLVName() const noexcept { return m_Rich2MaPmtSubMasterLVName; }
  const auto& Rich2MaPmtStdAnodeLVName() const noexcept { return m_Rich2MaPmtStdAnodeLVName; }
  const auto& Rich2MaPmtQuartzLVName() const noexcept { return m_Rich2MaPmtQuartzLVName; }
  const auto& Rich2MaPmtPhCathodeLVName() const noexcept { return m_Rich2MaPmtPhCathodeLVName; }
  const auto& Rich2MaPmtSideEnvelopeLVName() const noexcept { return m_Rich2MaPmtSideEnvelopeLVName; }
  const auto& Rich2MaPmtBackEnvelopeLVName() const noexcept { return m_Rich2MaPmtBackEnvelopeLVName; }
  const auto& Rich2MaPmtFrontRingLVName() const noexcept { return m_Rich2MaPmtFrontRingLVName; }

  const auto& RichGrandMaPmtSubMasterLVName() const noexcept { return m_RichGrandMaPmtSubMasterLVName; }
  const auto& RichGrandMaPmtAnodeLVName() const noexcept { return m_RichGrandMaPmtAnodeLVName; }
  const auto& RichGrandMaPmtQuartzLVName() const noexcept { return m_RichGrandMaPmtQuartzLVName; }
  const auto& RichGrandMaPmtPhCathodeLVName() const noexcept { return m_RichGrandMaPmtPhCathodeLVName; }
  const auto& RichGrandMaPmtSideEnvelopeLVName() const noexcept { return m_RichGrandMaPmtSideEnvelopeLVName; }
  const auto& RichGrandMaPmtBackEnvelopeLVName() const noexcept { return m_RichGrandMaPmtBackEnvelopeLVName; }
  const auto& RichGrandMaPmtFrontRingLVName() const noexcept { return m_RichGrandMaPmtFrontRingLVName; }

  const auto& RichMaPmtAnodeDetName() const noexcept { return m_RichMaPmtAnodeDetName; }
  const auto& RichGrandMaPmtAnodeDetName() const noexcept { return m_RichGrandMaPmtAnodeDetName; }

  const auto& RichMaPmtPhCathodeDetName() const noexcept { return m_RichMaPmtPhCathodeDetName; }
  const auto& RichGrandMaPmtPhCathodeDetName() const noexcept { return m_RichGrandMaPmtPhCathodeDetName; }

  void loadRich2PmtArraySpecParam();
  void initRich2PmtStructureLabels();
  void initRich2PmtModuleTypes();
  void initRich2PmtECTypes( int aR2ModuleNum, int irow, const std::string& aModuleLabel );
  void initRich2PmtMasterTypes( int curR2ModuleNum, int irow, const std::string& aModuleNumSuf );

  void CreateRich2PmtModuleTransforms();
  void CreateRich2PmtECRHTransforms();
  void CreateRich2PmtMasterTransforms();

  auto IsRich2StdPmtModuleRow( int aRow ) const noexcept {
    return ( std::find( m_Rh2StdModuleRowIndex.begin(), m_Rh2StdModuleRowIndex.end(), aRow ) !=
             m_Rh2StdModuleRowIndex.end() );
  }

  auto IsRich2GrandPmtTopRow( int aRow ) const noexcept {
    return ( std::find( m_Rh2GrandModuleTopRowIndex.begin(), m_Rh2GrandModuleTopRowIndex.end(), aRow ) !=
             m_Rh2GrandModuleTopRowIndex.end() );
  }
  auto IsRich2GrandPmtBotRow( int aRow ) const noexcept {
    return ( std::find( m_Rh2GrandModuleBotRowIndex.begin(), m_Rh2GrandModuleBotRowIndex.end(), aRow ) !=
             m_Rh2GrandModuleBotRowIndex.end() );
  }

  auto Rich2StdPmtModuleRowBeg() const noexcept { return m_Rh2StdModuleRowIndex[0]; }
  auto Rich2GrandPmtModuleTopRowBeg() const noexcept { return m_Rh2GrandModuleTopRowIndex[0]; }
  auto Rich2GrandPmtModuleBotRowBeg() const noexcept { return m_Rh2GrandModuleBotRowIndex[0]; }

  auto Rh2DefLiveNumPmtModulesInRow() const noexcept { return m_Rh2DefLiveNumPmtModulesInRow; }
  auto Rh2PmtModuleDefLiveStartCol() const noexcept { return m_Rh2PmtModuleDefLiveStartCol; }
  auto Rh2PmtModuleDefLiveEndCol() const noexcept { return m_Rh2PmtModuleDefLiveEndCol; }

  bool IsRich2ActiveModule( int aR2ModuleNum ) const noexcept { return m_IsRich2ActiveModule[aR2ModuleNum]; }

  const auto& Rich2PmtModuleLogVolName( int aR2Mod ) const noexcept { return m_Rich2PmtModuleLogVolNames[aR2Mod]; }
  const auto& Rich2PmtModulePhysVolName( int aR2Mod ) const noexcept { return m_Rich2PmtModulePhysVolNames[aR2Mod]; }
  const auto& Rich2PmtModuleShapeName( int aR2Mod ) const noexcept { return m_Rich2PmtModuleShapeNames[aR2Mod]; }
  const auto& Rich2PmtModuleDetName( int aR2Mod ) const noexcept { return m_Rich2PmtModuleDetNames[aR2Mod]; }
  const auto& Rich2PmtModuleCommonDetNamePref() const noexcept { return m_Rich2PmtModuleCommonDetNamePref; }

  auto Rich2PmtModuleCopyNumber( int aR2Mod ) const noexcept { return m_Rich2PmtModuleCopyNumbers[aR2Mod]; }
  bool IsRich2GrandPmtModule( int aR2Mod ) const noexcept { return m_IsRich2GrandPmtModule[aR2Mod]; }

  auto Rich2TotalNumPmtModules() const noexcept { return m_Rh2TotalNumPmtModulesInRich2; }
  auto Rich2TotalNumPmtDefLiveModules() const noexcept { return m_Rh2TotalNumPmtDefLiveModulesInRich2; }
  auto Rh2TotalNumPmtModulesInPanel() const noexcept { return m_Rh2TotalNumPmtModulesInPanel; }
  auto Rh2TotalNumPmtDefLiveModulesInPanel() const noexcept { return m_Rh2TotalNumPmtDefLiveModulesInPanel; }
  auto RhTotalNumPmtModulesInRich() const noexcept { return m_RhTotalNumPmtModulesInRich; }
  auto RhTotalNumPmtDefLiveModulesInRich() const noexcept { return m_RhTotalNumPmtDefLiveModulesInRich; }
  std::vector<int> Rich2DefLiveModuleCopyNumRange( int aR2Side ) const noexcept {
    return m_Rich2DefLiveModuleCopyNumRange[aR2Side];
  }
  auto Rich1PmtStructureCreated() const noexcept { return m_Rich1PmtStructureCreated; }
  auto Rich2PmtStructureCreated() const noexcept { return m_Rich2PmtStructureCreated; }

  auto Rich2ModuleSideFromR2ModuleNum( int aR2ModuleNum ) const noexcept {
    return ( aR2ModuleNum < Rh2TotalNumPmtModulesInPanel() ) ? 0 : 1;
  }

  auto Rich2ModuleNumInPanelFromR2ModuleNum( int aR2ModuleNum ) const noexcept {
    return ( aR2ModuleNum < Rh2TotalNumPmtModulesInPanel() ) ? aR2ModuleNum
                                                             : ( aR2ModuleNum - Rh2TotalNumPmtModulesInPanel() );
  }

  auto Rich2ModuleSideFromCopyNum( int aModuleCopyNum ) const noexcept {
    auto aMInRich2 = aModuleCopyNum - m_Rh1TotalNumPmtModulesInRich1;
    return ( aMInRich2 < Rh2TotalNumPmtModulesInPanel() ) ? 0 : 1;
  }

  auto Rich2ModuleNumInPanelFromCopyNum( int aModuleCopyNum ) const noexcept {
    auto aMInRich2 = aModuleCopyNum - m_Rh1TotalNumPmtModulesInRich1;
    return ( aMInRich2 < Rh2TotalNumPmtModulesInPanel() ? aMInRich2 : ( aMInRich2 - Rh2TotalNumPmtModulesInPanel() ) );
  }

  auto Rich2ModuleNumInRich2FromCopyNum( int aModuleCopyNum ) const noexcept {
    return ( aModuleCopyNum - m_Rh1TotalNumPmtModulesInRich1 );
  }

  auto Rich2SideFromRich2PmtCopyNum( int aCopyNum ) const noexcept {
    auto aModuleCopyNum = aCopyNum / RhNumPmtInStdModule();
    return ( Rich2ModuleSideFromCopyNum( aModuleCopyNum ) );
  }

  const auto& Rich2PmtECRHLogVolName( int iR2Module, int iec ) const noexcept {
    return m_Rich2PmtECRHLogVolNames[iR2Module][iec];
  }
  const auto& Rich2PmtECRHPhysVolName( int iR2Module, int iec ) const noexcept {
    return m_Rich2PmtECRHPhysVolNames[iR2Module][iec];
  }
  const auto& Rich2PmtECRHShapeName( int iR2Module, int iec ) const noexcept {
    return m_Rich2PmtECRHShapeNames[iR2Module][iec];
  }
  const auto& Rich2PmtECRHDetName( int iR2Module, int iec ) const noexcept {
    return m_Rich2PmtECRHDetNames[iR2Module][iec];
  }

  const auto& Rich2PmtECRHCommonDetNamePref() const noexcept { return m_Rich2PmtECRHCommonDetNamePref; }

  const auto& Rich2PmtMasterLogVolName( int aModuleNum, int aECNum, int aPmtNum ) const noexcept {
    auto aNumPmtMasterInEc = ( IsRich2GrandPmtModule( aModuleNum ) ) ? m_RhNumGrandPmtInECH : m_RhNumPMTInECR;
    auto ipc               = ( aECNum * aNumPmtMasterInEc ) + aPmtNum;
    return m_Rich2PmtMasterLogVolNames[aModuleNum][ipc];
  }
  const auto& Rich2PmtMasterPhysVolName( int aModuleNum, int aECNum, int aPmtNum ) const noexcept {
    auto aNumPmtMasterInEc = ( IsRich2GrandPmtModule( aModuleNum ) ) ? m_RhNumGrandPmtInECH : m_RhNumPMTInECR;
    auto ipc               = ( aECNum * aNumPmtMasterInEc ) + aPmtNum;
    return m_Rich2PmtMasterPhysVolNames[aModuleNum][ipc];
  }
  const auto& Rich2PmtMasterShapeName( int aModuleNum, int aECNum, int aPmtNum ) const noexcept {
    auto aNumPmtMasterInEc = ( IsRich2GrandPmtModule( aModuleNum ) ) ? m_RhNumGrandPmtInECH : m_RhNumPMTInECR;
    auto ipc               = ( aECNum * aNumPmtMasterInEc ) + aPmtNum;
    return m_Rich2PmtMasterShapeNames[aModuleNum][ipc];
  }
  const auto& Rich2PmtMasterDetName( int aModuleNum, int aECNum, int aPmtNum ) const noexcept {
    auto aNumPmtMasterInEc = ( IsRich2GrandPmtModule( aModuleNum ) ) ? m_RhNumGrandPmtInECH : m_RhNumPMTInECR;
    auto ipc               = ( aECNum * aNumPmtMasterInEc ) + aPmtNum;
    return m_Rich2PmtMasterDetNames[aModuleNum][ipc];
  }

  std::string Rich2PmtMasterCommonDetNamePref() const noexcept { return m_Rich2PmtMasterCommonDetNamePref; }

  void setRichNumStdPmtCompUploaded( size_t aValue ) { m_RichNumStdPmtCompUploaded = aValue; }
  void setRichNumGrandPmtCompUploaded( size_t aValue ) { m_RichNumGrandPmtCompUploaded = aValue; }

  auto IsRichStdPmtCompUploaded() const noexcept { return ( m_RichNumStdPmtCompUploaded > 0 ); }
  auto IsRichGrandPmtCompUploaded() const noexcept { return ( m_RichNumGrandPmtCompUploaded > 0 ); }

  auto RichNumStdPmtCompUploaded() const noexcept { return m_RichNumStdPmtCompUploaded; }
  auto RichNumGrandPmtCompUploaded() const noexcept { return m_RichNumGrandPmtCompUploaded; }

  auto IsInRichPmtTransformList( const std::string& aTransformName ) const noexcept {
    return ( std::find( m_RichPmtCompTransformNames.begin(), m_RichPmtCompTransformNames.end(), aTransformName ) !=
             m_RichPmtCompTransformNames.end() );
  }

  void build_RichPmtTransforms( xml_h tr_handle );

  template <typename P>
  constexpr auto toInTg( const P g ) const noexcept {
    return static_cast<std::underlying_type_t<P>>( g );
  }

  const auto& getRichPmtCompTransformName( RichPmtGeomTransformLabel aLabel ) const noexcept {
    return m_RichPmtCompTransformNames[toInTg( aLabel )];
  }

  dd4hep::Transform3D getRichPmtCompGeomTransform( const std::string& aName ) const;

  auto getRich1PmtMasterCopyNumber( int aModule, int aEca, int aPmt ) const noexcept {
    auto aMe = RhNumPmtInStdModule();
    auto aPe = RhNumPMTInECR();
    return ( aModule * aMe ) + ( aEca * aPe ) + aPmt;
  }
  auto getRich2PmtMasterCopyNumber( int aModule, int aEca, int aPmt ) const noexcept {
    auto aMe = RhNumPmtInStdModule();
    // auto aPe = RhNumPMTInECR();
    auto aPe = Rh2NumPmtInCurrentEC( aModule );
    return ( ( Rich2PmtModuleCopyNumber( aModule ) ) * aMe ) + ( aEca * aPe ) + aPmt;
  }

  int getRichPmtNumInStdModuleFromPmtCopyNum( int aPmtCopyNum ) const noexcept {
    return ( aPmtCopyNum % ( RhNumPmtInStdModule() ) );
  }
  int getRichPmtNumInGrandModuleFromPmtCopyNum( int aPmtCopyNum ) const noexcept {
    // return getRichPmtNumInStdModuleFromPmtCopyNum( aPmtCopyNum ) / Rich2::MaxNumRich2MapmtsInGrandModule;
    return getRichPmtNumInStdModuleFromPmtCopyNum( aPmtCopyNum );
  }

  auto getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel aLabel ) const noexcept {
    return m_RichPmtCompPhysVolCopyNumbers[toInTg( aLabel )];
  }

  const auto& getRichPmtCompPhysVolName( RichPmtGeomTransformLabel aLabel ) const noexcept {
    return m_RichPmtCompPhysVolNames[toInTg( aLabel )];
  }

  auto NumRichPmtCompForSurf() const noexcept { return m_NumRichPmtCompForSurf; }

  static RichPmtGeoAux* getRichPmtGeoAuxInstance();

  void setPmtDebugLevel( bool aLevel ) { m_RichPmtGeoAuxDebug = aLevel; }

  const auto& RichPmtQuartzMatName() const noexcept { return m_RichPmtQuartzMatName; }
  const auto& RichPmtPhCathMatName() const noexcept { return m_RichPmtPhCathMatName; }
  const auto& RichPmtQWMatName() const noexcept { return m_RichPmtQWMatName; }
  const auto& RichPmtVacMatName() const noexcept { return m_RichPmtVacMatName; }

  using DEList = std::vector<std::pair<dd4hep::DetElement, dd4hep::cond::ConditionUpdateContext&>>;

  DEList Rich1PDModuleListDE( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, int aSide,
                              int aMType ) const;

  DEList Rich1MapmtListInModuleDE( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                                   int aModuleCopyNum ) const;

  DEList Rich2PDModuleListDE( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, int aSide,
                              int aMType ) const;

  DEList Rich2MapmtListInModuleDE( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                                   int aModuleCopyNum, int aMType ) const;

  RichPmtGeoAux();

private:
  int m_RichPmtMaxNumGeomTransforms{};

  std::vector<std::string>                   m_RichPmtCompTransformNames;
  std::map<std::string, dd4hep::Transform3D> m_RichPmtCompGeomTransforms;
  std::vector<std::string>                   m_RichPmtCompPhysVolNames;
  std::vector<int>                           m_RichPmtCompPhysVolCopyNumbers;

  bool   m_Rich1PmtStructureCreated{false};
  bool   m_Rich2PmtStructureCreated{false};
  size_t m_RichNumStdPmtCompUploaded{};
  size_t m_RichNumGrandPmtCompUploaded{};

  std::vector<std::string> m_Rich1PmtModuleType;
  std::vector<std::string> m_Rich1PmtModuleLogVolNames;
  std::vector<std::string> m_Rich1PmtModulePhysVolNames;
  std::vector<std::string> m_Rich1PmtModuleShapeNames;
  std::vector<int>         m_Rich1PmtModuleTypeIndex;
  std::vector<std::string> m_Rich1PmtModuleDetNames;
  std::string              m_Rich1PmtModuleCommonDetNamePref;

  std::vector<std::string>              m_RichECRType;
  std::vector<std::vector<std::string>> m_Rich1PmtECRLogVolNames;
  std::vector<std::vector<std::string>> m_Rich1PmtECRPhysVolNames;
  std::vector<std::vector<std::string>> m_Rich1PmtECRShapeNames;
  std::vector<std::vector<bool>>        m_Rich1PmtECRValid;
  std::vector<std::vector<std::string>> m_Rich1PmtECRDetNames;
  std::string                           m_Rich1PmtECRCommonDetNamePref;

  std::vector<std::string>              m_RichPmtMasterType;
  std::vector<std::vector<std::string>> m_Rich1PmtMasterLogVolNames;
  std::vector<std::vector<std::string>> m_Rich1PmtMasterPhysVolNames;
  std::vector<std::vector<std::string>> m_Rich1PmtMasterShapeNames;
  std::vector<std::vector<bool>>        m_Rich1PmtMasterValid;
  std::vector<std::vector<std::string>> m_Rich1PmtMasterDetNames;
  std::string                           m_Rich1PmtMasterCommonDetNamePref;

  int m_NumRich1PmtModuleTypes{};
  int m_Rh1NumPMTModulesInRow{};
  int m_Rh1NumPMTModulesInColumn{};
  int m_Rh1TotalNumPmtModulesInPanel{};
  int m_Rh1NumPanels{};
  int m_Rh1TotalNumPmtModulesInRich1{};

  int m_RhTotalNumPmtModulesInRich{};
  int m_RhTotalNumPmtDefLiveModulesInRich{};

  int m_RhMaxNumECRInModule{};

  int m_RhNumPMTInECRRow{};
  int m_RhNumPMTInECRCol{};
  int m_RhNumPMTInECR{};

  int m_RhNumPmtInStdModule{};

  int m_RhPmtAnodeDetIdShiftFactor{};
  int m_RhPmtPhCathodeDetIdShiftFactor{};
  // extra Rich1pmt user parameters
  int m_Rich1MaxNumberOfPMTInPanel{};
  int m_Rich1MaxNumberOfPMT{};
  int m_Rh1NumPmtStdModulesInPanel{};
  int m_Rh1NumPmtEdgeNonOuterCornerModulesInPanel{};
  int m_Rh1NumPmtOuterCornerModulesInPanel{};

  // end extra Rich1pmt user parameters

  std::vector<dd4hep::Transform3D> m_Rich1PmtModuleTransforms;
  std::vector<dd4hep::Transform3D> m_Rich1PmtECRTransforms;
  std::vector<dd4hep::Transform3D> m_Rich1PmtMasterTransforms;

  std::string m_RichMaPmtSubMasterLVName;
  std::string m_RichMaPmtStdAnodeLVName;
  std::string m_RichMaPmtQuartzLVName;
  std::string m_RichMaPmtPhCathodeLVName;
  std::string m_RichMaPmtSideEnvelopeLVName;
  std::string m_RichMaPmtBackEnvelopeLVName;
  std::string m_RichMaPmtFrontRingLVName;

  std::string m_Rich2MaPmtSubMasterLVName;
  std::string m_Rich2MaPmtStdAnodeLVName;
  std::string m_Rich2MaPmtQuartzLVName;
  std::string m_Rich2MaPmtPhCathodeLVName;
  std::string m_Rich2MaPmtSideEnvelopeLVName;
  std::string m_Rich2MaPmtBackEnvelopeLVName;
  std::string m_Rich2MaPmtFrontRingLVName;

  std::string m_RichGrandMaPmtSubMasterLVName;
  std::string m_RichGrandMaPmtAnodeLVName;
  std::string m_RichGrandMaPmtQuartzLVName;
  std::string m_RichGrandMaPmtPhCathodeLVName;
  std::string m_RichGrandMaPmtSideEnvelopeLVName;
  std::string m_RichGrandMaPmtBackEnvelopeLVName;
  std::string m_RichGrandMaPmtFrontRingLVName;

  std::string m_RichMaPmtPhCathodeDetName;
  std::string m_RichGrandMaPmtPhCathodeDetName;

  std::string m_RichMaPmtAnodeDetName;
  std::string m_RichGrandMaPmtAnodeDetName;

  int m_Rh2NumPmtModulesInRow{};
  int m_Rh2NumPmtModulesInColumn{};

  int m_Rh2DefLiveNumPmtModulesInRow{};
  int m_Rh2PmtModuleDefLiveStartCol{};
  int m_Rh2PmtModuleDefLiveEndCol{};

  int m_Rh2TotalNumPmtModulesInPanel{};
  int m_Rh2TotalNumPmtDefLiveModulesInPanel{};
  int m_Rh2NumPanels{};
  int m_Rh2TotalNumPmtModulesInRich2{};
  int m_Rh2TotalNumPmtDefLiveModulesInRich2{};

  int m_RhMaxNumECHInGrandModule{};
  int m_RhNumGrandPmtInECH{};
  int m_RhNumGrandPmtInGrandModule{};

  std::vector<std::string> m_RichECHType;

  std::vector<int>               m_Rh2StdModuleRowIndex;
  std::vector<int>               m_Rh2GrandModuleRowIndex;
  std::vector<int>               m_Rh2GrandModuleTopRowIndex;
  std::vector<int>               m_Rh2GrandModuleBotRowIndex;
  std::vector<int>               m_Rh2ModuleDefLiveColRangeIndex;
  std::vector<bool>              m_IsRich2ActiveModule;
  std::vector<std::vector<int>>  m_Rich2DefLiveModuleCopyNumRange;
  std::vector<std::vector<bool>> m_Rich2PmtECRHValid;
  std::vector<std::vector<bool>> m_Rich2PmtMasterValid;

  std::vector<std::string> m_Rich2PmtModuleType;
  std::vector<std::string> m_Rich2PmtModuleLogVolNames;
  std::vector<std::string> m_Rich2PmtModulePhysVolNames;
  std::vector<std::string> m_Rich2PmtModuleShapeNames;
  std::vector<int>         m_Rich2PmtModuleCopyNumbers;
  std::vector<bool>        m_IsRich2GrandPmtModule;
  std::vector<std::string> m_Rich2PmtModuleDetNames;
  std::string              m_Rich2PmtModuleCommonDetNamePref;

  std::vector<std::vector<std::string>> m_Rich2PmtECRHLogVolNames;
  std::vector<std::vector<std::string>> m_Rich2PmtECRHPhysVolNames;
  std::vector<std::vector<std::string>> m_Rich2PmtECRHShapeNames;
  std::vector<std::vector<std::string>> m_Rich2PmtECRHDetNames;
  std::string                           m_Rich2PmtECRHCommonDetNamePref;

  std::vector<std::vector<std::string>> m_Rich2PmtMasterLogVolNames;
  std::vector<std::vector<std::string>> m_Rich2PmtMasterPhysVolNames;
  std::vector<std::vector<std::string>> m_Rich2PmtMasterShapeNames;
  std::vector<std::vector<std::string>> m_Rich2PmtMasterDetNames;
  std::string                           m_Rich2PmtMasterCommonDetNamePref;

  std::vector<dd4hep::Transform3D> m_Rich2PmtModuleTransforms;
  std::vector<dd4hep::Transform3D> m_Rich2PmtECRTransforms;
  std::vector<dd4hep::Transform3D> m_Rich2PmtECHTransforms;
  std::vector<dd4hep::Transform3D> m_Rich2PmtMasterTransforms;
  dd4hep::Transform3D              m_Rich2GrandPmtMasterTransform;
  int                              m_NumRichPmtCompForSurf{};

  // Parameters used in simulation

  std::string m_RichPmtQuartzMatName;

  std::string m_RichPmtPhCathMatName;
  std::string m_RichPmtQWMatName;
  std::string m_RichPmtVacMatName;

  bool m_RichPmtGeoAuxDebug{true};
};
