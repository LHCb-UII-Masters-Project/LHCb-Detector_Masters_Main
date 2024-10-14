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

#include "Core/DeIOV.h"
#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "Detector/Rich/Utilities.h"
#include "XML/Utilities.h"
#include <type_traits>
#include <vector>

class Rich2MirrorGeoAux {
public:
  void BuildRich2SphMirrorShapes();
  void BuildRich2SphMirrorTransforms();

  void BuildRich2SecMirrorShapes();
  void BuildRich2SecMirrorTransforms();

  void InitRich2MirrorGeneralParam();
  void InitRich2SphMirrorParam();
  void InitRich2SecMirrorParam();

  auto IsRich2SphSegWithCuts( int iSM ) const noexcept {
    auto it = std::find( m_Rich2SphMHalfHexIndex.begin(), m_Rich2SphMHalfHexIndex.end(), iSM );
    return ( it != m_Rich2SphMHalfHexIndex.end() );
  }
  auto IsRich2SphLeftHalfSeg( int iLM ) const noexcept {
    auto it = std::find( m_Rich2SphMLeftHalfHexIndex.begin(), m_Rich2SphMLeftHalfHexIndex.end(), iLM );
    return ( it != m_Rich2SphMLeftHalfHexIndex.end() );
  }

  auto IsRich2SphRightHalfSeg( int iLM ) const noexcept {
    auto it = std::find( m_Rich2SphMRightHalfHexIndex.begin(), m_Rich2SphMRightHalfHexIndex.end(), iLM );
    return ( it != m_Rich2SphMRightHalfHexIndex.end() );
  }

  auto IsRich2SphHalfSeg( int iLM ) const noexcept {
    auto it = std::find( m_Rich2SphMHalfHexIndex.begin(), m_Rich2SphMHalfHexIndex.end(), iLM );
    return ( it != m_Rich2SphMHalfHexIndex.end() );
  }

  auto IsRich2SphMirrSegInPositiveXSide( int iM ) const noexcept { return ( iM <= m_Rich2SphMirrPositiveMaxCopyNum ); }
  auto IsRich2SecMirrSegInPositiveXSide( int iM ) const noexcept { return ( iM <= m_Rich2SecMirrPositiveMaxCopyNum ); }

  auto Rich2SphMirrorSide( int iM ) const noexcept { return ( IsRich2SphMirrSegInPositiveXSide( iM ) ) ? 0 : 1; }
  auto Rich2SecMirrorSide( int iM ) const noexcept { return ( IsRich2SecMirrSegInPositiveXSide( iM ) ) ? 0 : 1; }

  const auto& Rich2SphMIndexStr( int iM ) const noexcept { return m_Rich2SphMIndexStr[iM]; }
  const auto& Rich2SecMIndexStr( int iM ) const noexcept { return m_Rich2SecMIndexStr[iM]; }

  const auto& Rich2SphMirrLVName( int aSphN ) const noexcept { return m_Rich2SphMirrLVNames[aSphN]; }
  const auto& Rich2SecMirrLVName( int aSecN ) const noexcept { return m_Rich2SecMirrLVNames[aSecN]; }
  const auto& Rich2SphMirrPVName( int aSphN ) const noexcept { return m_Rich2SphMirrPVNames[aSphN]; }
  auto        Rich2SphMirrCopyNumber( int aSphN ) const noexcept { return m_Rich2SphMirrCopyNumbers[aSphN]; }
  const auto& Rich2SphMirrDetName( int aSphN ) const noexcept { return m_Rich2SphMirrDetNames[aSphN]; }

  const auto& Rich2SecMirrPVName( int aSecN ) const noexcept { return m_Rich2SecMirrPVNames[aSecN]; }
  auto        Rich2SecMirrCopyNumber( int aSecN ) const noexcept { return m_Rich2SecMirrCopyNumbers[aSecN]; }
  const auto& Rich2SecMirrDetName( int aSecN ) const noexcept { return m_Rich2SecMirrDetNames[aSecN]; }

  const auto& Rich2SphMirrTransfom( int aSphN ) const noexcept { return m_Rich2SphMirrTransfoms[aSphN]; }
  const auto& Rich2SecMirrTransfom( int aSecN ) const noexcept { return m_Rich2SecMirrTransfoms[aSecN]; }

  const auto& Rich2SphMirrShape( int aSeg ) const noexcept { return m_Rich2SphMirrShapes[aSeg]; }
  const auto& Rich2SecMirrShape( int aSeg ) const noexcept { return m_Rich2SecMirrShapes[aSeg]; }

  auto NumRich2SphMirrSeg() const noexcept { return m_NumRich2SphMirrSeg; }
  auto NumRich2SecMirrSeg() const noexcept { return m_NumRich2SecMirrSeg; }

  const auto& Rich2SphMirrorMasterLVName( int iSide ) const noexcept { return m_Rich2SphMirrorMasterLVNames[iSide]; }
  const auto& Rich2SecMirrorMasterLVName( int iSide ) const noexcept { return m_Rich2SecMirrorMasterLVNames[iSide]; }

  const auto& Rich2SphMirrSegMasterLVName( int iM ) const noexcept {
    const auto iSide = ( IsRich2SphMirrSegInPositiveXSide( iM ) ) ? 0 : 1;
    return ( Rich2SphMirrorMasterLVName( iSide ) );
  }

  const auto& Rich2SecMirrSegMasterLVName( int iM ) const noexcept {
    const auto iSide = ( IsRich2SecMirrSegInPositiveXSide( iM ) ) ? 0 : 1;
    return ( Rich2SecMirrorMasterLVName( iSide ) );
  }

  using DEList = std::vector<std::pair<dd4hep::DetElement, dd4hep::cond::ConditionUpdateContext&>>;

  DEList Rich2SphMirrorListDE( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                               int aSide ) const;

  DEList Rich2SecMirrorListDE( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                               int aSide ) const;

  static Rich2MirrorGeoAux* getRich2MirrorGeoAuxInstance();

  Rich2MirrorGeoAux();

private:
  int    m_NumRich2SphMirrSeg{};
  int    m_NumRich2SecMirrSeg{};
  double m_Rich2PiOver2Ang{};
  double m_Rich2SphMirrNominalInnerROC{};
  double m_Rich2SecMirrNominalInnerROC{};
  double m_Rich2SphMirrorThickness{};
  double m_Rich2SecMirrorThickness{};

  double m_Rich2SphMirrorHexSegHorizSize{};
  double m_Rich2SphMirrorHexSegVertSize{};
  double m_Rich2HalfSize{};
  double m_Rich2SphMirrorHexSegSubBoxLargeXSize{};
  double m_Rich2SphMirrorHexSegSubBoxLargeYSize{};
  double m_Rich2SphMirrorHexSegSubBoxLargeZSize{};
  double m_Rich2SphMirrHexSubBoxRotXQ02{};
  double m_Rich2SphMirrHexSubBoxRotXQ13{};
  double m_Rich2SphMirrHexSubBoxShiftX{};
  double m_Rich2SphMirrHexSubBoxShiftYQ0{};
  double m_Rich2SphMirrHexSubBoxShiftYQ1{};
  double m_Rich2SphMirrHexSubBoxShiftYQ2{};
  double m_Rich2SphMirrHexSubBoxShiftYQ3{};
  double m_Rich2SphMirrHexSubBoxShiftZQ0{};
  double m_Rich2SphMirrHexSubBoxShiftZQ1{};
  double m_Rich2SphMirrHexSubBoxShiftZQ2{};
  double m_Rich2SphMirrHexSubBoxShiftZQ3{};

  double m_Rich2SphMirrLeftHalfHexSubBoxShiftX{};
  double m_Rich2SphMirrLeftHalfHexSubBoxShiftZ{};
  double m_Rich2SphMirrLeftHalfHexSubBoxRotXQ1{};
  double m_Rich2SphMirrLeftHalfHexSubBoxRotXQ2{};
  double m_Rich2SphMirrLeftHalfHexSubBoxShiftYQ1{};
  double m_Rich2SphMirrLeftHalfHexSubBoxShiftYQ2{};
  double m_Rich2SphMirrLeftHalfHexSubBoxShiftZQ1{};
  double m_Rich2SphMirrLeftHalfHexSubBoxShiftZQ2{};
  double m_Rich2SphMirrRightHalfHexSubBoxShiftX{};
  double m_Rich2SphMirrRightHalfHexSubBoxShiftZ{};
  double m_Rich2SphMirrRightHalfHexSubBoxRotXQ0{};
  double m_Rich2SphMirrRightHalfHexSubBoxRotXQ3{};
  double m_Rich2SphMirrRightHalfHexSubBoxShiftYQ0{};
  double m_Rich2SphMirrRightHalfHexSubBoxShiftYQ3{};
  double m_Rich2SphMirrRightHalfHexSubBoxShiftZQ0{};
  double m_Rich2SphMirrRightHalfHexSubBoxShiftZQ3{};

  double m_Rich2SphMirCutOutBPipeCylinderHalfSizeZ{};
  double m_Rich2SphMirCutOutBPipeCylinderRadius{};

  double m_Rich2SphMirCutOutBPipeCylinderRotY{};

  double m_Rich2SphMir12CutOutBPipeCylinderPosZ{};
  double m_Rich2SphMir43CutOutBPipeCylinderPosZ{};

  double m_Rich2SphMirCutOutBPipeCornerBoxSizeX{};
  double m_Rich2SphMirCutOutBPipeCornerBoxSizeY{};
  double m_Rich2SphMirCutOutBPipeCornerBoxSizeZ{};
  double m_Rich2SphMirCutOutBPipeCornerBoxBotPosY{};
  double m_Rich2SphMirCutOutBPipeCornerBoxTopPosY{};
  double m_Rich2SphMirCutOutBPipeCornerBoxPosZ{};
  double m_Rich2SphMirCutOutBPipeCornerBoxPosRotX{};
  double m_Rich2SphMirCutOutBPipeCornerBoxNegRotX{};

  int    m_Rich2SphMirrPositiveMaxCopyNum{};
  double m_Rich2SphMirrorMaster0PosX{};
  double m_Rich2SphMirrorMaster1PosX{};
  double m_Rich2SphMirrorMaster0PosY{};
  double m_Rich2SphMirrorMaster1PosY{};
  double m_Rich2SphMirrorMaster0PosZ{};
  double m_Rich2SphMirrorMaster1PosZ{};

  std::vector<std::string> m_Rich2SphMirrorMasterLVNames;

  std::vector<double> m_Rich2SphMirrInnerROC;
  std::vector<double> m_Rich2SecMirrInnerROC;
  std::vector<double> m_Rich2SphMirrOuterROC;
  std::vector<double> m_Rich2SecMirrOuterROC;

  std::vector<std::string> m_Rich2SphMIndexStr;
  std::vector<std::string> m_Rich2SecMIndexStr;

  std::vector<int>         m_Rich2SphMHalfHexIndex;
  std::vector<int>         m_Rich2SphMLeftHalfHexIndex;
  std::vector<int>         m_Rich2SphMRightHalfHexIndex;
  std::vector<double>      m_Rich2SphMirrHexSegEndTheta;
  std::vector<double>      m_Rich2SphMirrHexSegEndPhi;
  std::vector<double>      m_Rich2SphMirrHexSegDeltaTheta;
  std::vector<double>      m_Rich2SphMirrHexSegDeltaPhi;
  std::vector<double>      m_Rich2SphMirrHexSegStartTheta;
  std::vector<double>      m_Rich2SphMirrHexSegStartPhi;
  std::vector<std::string> m_Rich2SphMirrLVNames;
  std::vector<std::string> m_Rich2SphMirrPVNames;
  std::vector<int>         m_Rich2SphMirrCopyNumbers;
  std::vector<std::string> m_Rich2SphMirrDetNames;

  std::vector<dd4hep::SubtractionSolid> m_Rich2SphMirrShapes;
  std::vector<dd4hep::Transform3D>      m_Rich2SphMirrTransfoms;

  std::vector<double> m_Rich2SphMirrorSegCentreInRich2PosX;
  std::vector<double> m_Rich2SphMirrorSegCentreInRich2PosY;
  std::vector<double> m_Rich2SphMirrorSegRotX;

  double m_Rich2SphMirrorNominalCoCInGasEnclPositivePosX{};
  double m_Rich2SphMirrorNominalCoCInGasEnclNegativePosX{};
  double m_Rich2SphMirrorNominalCoCInGasEnclPosY{};
  double m_Rich2SphMirrorNominalCoCInGasEnclPosZ{};

  double m_Rich2MainGasEnclInRich2PosX{};
  double m_Rich2MainGasEnclInRich2PosY{};

  double m_Rich2SecMirrorSegSizeX{};
  double m_Rich2SecMirrorSegSizeY{};

  int m_Rich2NumSecMirrorSegRows{};
  int m_Rich2NumSecMirrorSegCols{};
  int m_Rich2NumSecMirrorMasters{};
  int m_Rich2SecMirrPositiveMaxCopyNum{};

  double m_Rich2SecMirrorArrayStartX{};
  double m_Rich2SecMirrorArrayStartY{};
  double m_Rich2SecMirrorArrayStepX{};
  double m_Rich2SecMirrorArrayStepY{};
  double m_Rich2SecMirrorSegRot{};

  std::vector<double> m_Rich2SecMirrSegDeltaTheta;
  std::vector<double> m_Rich2SecMirrSegDeltaPhi;
  std::vector<double> m_Rich2SecMirrSegStartTheta;
  std::vector<double> m_Rich2SecMirrSegStartPhi;
  std::vector<double> m_Rich2SecMirrSegEndTheta;
  std::vector<double> m_Rich2SecMirrSegEndPhi;

  std::vector<std::string> m_Rich2SecMirrLVNames;
  std::vector<std::string> m_Rich2SecMirrPVNames;
  std::vector<int>         m_Rich2SecMirrCopyNumbers;
  std::vector<std::string> m_Rich2SecMirrDetNames;

  std::vector<std::string> m_Rich2SecMirrorMasterLVNames;

  std::vector<dd4hep::Sphere>      m_Rich2SecMirrShapes;
  std::vector<dd4hep::Transform3D> m_Rich2SecMirrTransfoms;
};
