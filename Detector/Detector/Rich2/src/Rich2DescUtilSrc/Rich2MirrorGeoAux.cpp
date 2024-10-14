//==========================================================================
//  LHCb Rich2 Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2021-01-14
//
//==========================================================================

#include "Detector/Rich2/Rich2MirrorGeoAux.h"
#include <cassert>
#include <map>
#include <math.h>
#include <memory>
#include <mutex>

using namespace LHCb::Detector;

//==========================================================================//
void Rich2MirrorGeoAux::BuildRich2SphMirrorShapes() {

  m_Rich2SphMirrShapes.clear();
  m_Rich2SphMirrShapes.resize( m_NumRich2SphMirrSeg );

  std::string aSphSegShapePref   = "Rich2SphMirrorSphereSeg";
  std::string aSphHexSegSubPref  = "Rich2SphMirrorHexSegSub";
  std::string aBoxSubP           = "Rich2SphMirrorSubBoxQ";
  std::string aBoxSubBoolean     = "Rich2SphMirrorBooleanSubQ";
  std::string aSpSegSuf          = "InHexSeg";
  std::string aBoxSubBooleanSp8  = "Rich2SphMirrorLHSubBeamPipeBoxBotBH";
  std::string aBoxSubBooleanSp12 = "Rich2SphMirrorSubBeamPipeTubeLeftBH";
  std::string aBoxSubBooleanSp16 = "Rich2SphMirrorLHSubBeamPipeBoxTopBH";
  std::string aBoxSubBooleanSp39 = "Rich2SphMirrorRHSubBeamPipeBoxBotBH";
  std::string aBoxSubBooleanSp43 = "Rich2SphMirrorSubBeamPipeTubeRightBH";
  std::string aBoxSubBooleanSp47 = "Rich2SphMirrorRHSubBeamPipeBoxTopBH";

  int nQSubtract   = 4;
  int nQSubtractHH = 3;

  std::vector<std::string> aSphBoxSubInd = {"0", "1", "2", "3"};
  dd4hep::Box              aGenMirrSubBox( m_Rich2SphMirrorHexSegSubBoxLargeXSize * m_Rich2HalfSize,
                              m_Rich2SphMirrorHexSegSubBoxLargeYSize * m_Rich2HalfSize,
                              m_Rich2SphMirrorHexSegSubBoxLargeZSize * m_Rich2HalfSize );
  aGenMirrSubBox.setName( "aCMirrSegSubBox" );
  dd4hep::Box aBeamPipeCutBox( m_Rich2SphMirCutOutBPipeCornerBoxSizeX * m_Rich2HalfSize,
                               m_Rich2SphMirCutOutBPipeCornerBoxSizeY * m_Rich2HalfSize,
                               m_Rich2SphMirCutOutBPipeCornerBoxSizeZ * m_Rich2HalfSize );
  aBeamPipeCutBox.setName( "aRich2BeamPipeCutBox" );
  dd4hep::Tube aBeamPipeCutCylinder( 0.0 * ( dd4hep::mm ), m_Rich2SphMirCutOutBPipeCylinderRadius,
                                     m_Rich2SphMirCutOutBPipeCylinderHalfSizeZ );
  aBeamPipeCutCylinder.setName( "aRich2BeamPipeCylinder" );
  std::vector<double>      aBoxSubPosY   = {m_Rich2SphMirrHexSubBoxShiftYQ0, m_Rich2SphMirrHexSubBoxShiftYQ1,
                                     m_Rich2SphMirrHexSubBoxShiftYQ2, m_Rich2SphMirrHexSubBoxShiftYQ3};
  std::vector<double>      aBoxSubPosZ   = {m_Rich2SphMirrHexSubBoxShiftZQ0, m_Rich2SphMirrHexSubBoxShiftZQ1,
                                     m_Rich2SphMirrHexSubBoxShiftZQ2, m_Rich2SphMirrHexSubBoxShiftZQ3};
  std::vector<double>      aBoxRotX      = {m_Rich2SphMirrHexSubBoxRotXQ02, m_Rich2SphMirrHexSubBoxRotXQ13,
                                  m_Rich2SphMirrHexSubBoxRotXQ02, m_Rich2SphMirrHexSubBoxRotXQ13};
  std::vector<double>      aBoxLHSubPosY = {m_Rich2SphMirrLeftHalfHexSubBoxShiftYQ1,
                                       m_Rich2SphMirrLeftHalfHexSubBoxShiftYQ2};
  std::vector<double>      aBoxLHSubPosZ = {m_Rich2SphMirrLeftHalfHexSubBoxShiftZQ1,
                                       m_Rich2SphMirrLeftHalfHexSubBoxShiftZQ2};
  std::vector<std::string> aSphBoxLHInd  = {"1", "2", "3"};

  std::vector<double>      aBoxRHSubPosY = {m_Rich2SphMirrRightHalfHexSubBoxShiftYQ0,
                                       m_Rich2SphMirrRightHalfHexSubBoxShiftYQ3};
  std::vector<double>      aBoxRHSubPosZ = {m_Rich2SphMirrRightHalfHexSubBoxShiftZQ0,
                                       m_Rich2SphMirrRightHalfHexSubBoxShiftZQ3};
  std::vector<std::string> aSphBoxRHInd  = {"0", "3", "12"};
  //++++++++++++++++++++++++++++++++++++++++//

  // Create transforms
  std::vector<dd4hep::Transform3D> aBoxSubTransform( nQSubtract );
  for ( int iQ = 0; iQ < nQSubtract; iQ++ ) {
    auto aBoxSubPos      = dd4hep::Position( m_Rich2SphMirrHexSubBoxShiftX, aBoxSubPosY[iQ], aBoxSubPosZ[iQ] );
    auto aBoxSubRot      = dd4hep::RotationZYX( 0, 0, aBoxRotX[iQ] );
    aBoxSubTransform[iQ] = dd4hep::Transform3D( aBoxSubRot, aBoxSubPos );
  }
  //++++++++++++++++++++++++++++++++++++++++//
  std::vector<dd4hep::Transform3D> aBoxSubTransformLH( nQSubtractHH );
  for ( int iLQ = 0; iLQ < nQSubtractHH; iLQ++ ) {

    auto aBoxSubPosLH = dd4hep::Position( 0, 0, 0 );
    auto aBoxSubRotLH = dd4hep::RotationZYX( 0, 0, 0 );

    if ( iLQ == 0 || iLQ == 1 ) {
      aBoxSubPosLH = dd4hep::Position( m_Rich2SphMirrLeftHalfHexSubBoxShiftX, aBoxLHSubPosY[iLQ], aBoxLHSubPosZ[iLQ] );
      if ( iLQ == 0 ) aBoxSubRotLH = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirrLeftHalfHexSubBoxRotXQ1 );
      if ( iLQ == 1 ) aBoxSubRotLH = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirrLeftHalfHexSubBoxRotXQ2 );
      aBoxSubTransformLH[iLQ] = dd4hep::Transform3D( aBoxSubRotLH, aBoxSubPosLH );
    } else if ( iLQ == 2 ) {
      aBoxSubPosLH =
          dd4hep::Position( m_Rich2SphMirrLeftHalfHexSubBoxShiftX, 0, m_Rich2SphMirrLeftHalfHexSubBoxShiftZ );
      aBoxSubRotLH            = dd4hep::RotationZYX( 0, 0, 0 );
      aBoxSubTransformLH[iLQ] = dd4hep::Transform3D( aBoxSubRotLH, aBoxSubPosLH );
    }
  }
  //++++++++++++++++++++++++++++++++++++++++//
  std::vector<dd4hep::Transform3D> aBoxSubTransformRH( nQSubtractHH );
  for ( int iRQ = 0; iRQ < nQSubtractHH; iRQ++ ) {

    auto aBoxSubPosRH = dd4hep::Position( 0, 0, 0 );
    auto aBoxSubRotRH = dd4hep::RotationZYX( 0, 0, 0 );
    if ( iRQ == 0 || iRQ == 1 ) {
      aBoxSubPosRH = dd4hep::Position( m_Rich2SphMirrRightHalfHexSubBoxShiftX, aBoxRHSubPosY[iRQ], aBoxRHSubPosZ[iRQ] );
      if ( iRQ == 0 ) aBoxSubRotRH = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirrRightHalfHexSubBoxRotXQ0 );
      if ( iRQ == 1 ) aBoxSubRotRH = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirrRightHalfHexSubBoxRotXQ3 );
      aBoxSubTransformRH[iRQ] = dd4hep::Transform3D( aBoxSubRotRH, aBoxSubPosRH );
    } else if ( iRQ == 2 ) {
      aBoxSubPosRH =
          dd4hep::Position( m_Rich2SphMirrRightHalfHexSubBoxShiftX, 0, m_Rich2SphMirrRightHalfHexSubBoxShiftZ );
      aBoxSubRotRH            = dd4hep::RotationZYX( 0, 0, 0 );
      aBoxSubTransformRH[iRQ] = dd4hep::Transform3D( aBoxSubRotRH, aBoxSubPosRH );
    }
  }
  //++++++++++++++++++++++++++++++++++++++++//

  auto aSubPos8       = dd4hep::Position( m_Rich2SphMirrInnerROC[8], m_Rich2SphMirCutOutBPipeCornerBoxTopPosY,
                                    m_Rich2SphMirCutOutBPipeCornerBoxPosZ );
  auto aSubRot8       = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirCutOutBPipeCornerBoxPosRotX );
  auto aSubTransform8 = dd4hep::Transform3D( aSubRot8, aSubPos8 );

  //++++++++++++++++++++++++++++++++++++++++//
  auto aSubPos12       = dd4hep::Position( m_Rich2SphMirrInnerROC[12], 0.0, m_Rich2SphMir12CutOutBPipeCylinderPosZ );
  auto aSubRot12       = dd4hep::RotationZYX( 0, m_Rich2SphMirCutOutBPipeCylinderRotY, 0 );
  auto aSubTransform12 = dd4hep::Transform3D( aSubRot12, aSubPos12 );

  //++++++++++++++++++++++++++++++++++++++++//
  auto aSubPos16       = dd4hep::Position( m_Rich2SphMirrInnerROC[16], m_Rich2SphMirCutOutBPipeCornerBoxBotPosY,
                                     m_Rich2SphMirCutOutBPipeCornerBoxPosZ );
  auto aSubRot16       = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirCutOutBPipeCornerBoxNegRotX );
  auto aSubTransform16 = dd4hep::Transform3D( aSubRot16, aSubPos16 );

  //++++++++++++++++++++++++++++++++++++++++//
  auto aSubPos39       = dd4hep::Position( m_Rich2SphMirrInnerROC[39], m_Rich2SphMirCutOutBPipeCornerBoxTopPosY,
                                     m_Rich2SphMirCutOutBPipeCornerBoxPosZ );
  auto aSubRot39       = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirCutOutBPipeCornerBoxNegRotX );
  auto aSubTransform39 = dd4hep::Transform3D( aSubRot39, aSubPos39 );

  //++++++++++++++++++++++++++++++++++++++++//

  auto aSubPos43       = dd4hep::Position( m_Rich2SphMirrInnerROC[43], 0.0, m_Rich2SphMir43CutOutBPipeCylinderPosZ );
  auto aSubRot43       = dd4hep::RotationZYX( 0, m_Rich2SphMirCutOutBPipeCylinderRotY, 0 );
  auto aSubTransform43 = dd4hep::Transform3D( aSubRot43, aSubPos43 );
  //++++++++++++++++++++++++++++++++++++++++//
  auto aSubPos47       = dd4hep::Position( m_Rich2SphMirrInnerROC[47], m_Rich2SphMirCutOutBPipeCornerBoxBotPosY,
                                     m_Rich2SphMirCutOutBPipeCornerBoxPosZ );
  auto aSubRot47       = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirCutOutBPipeCornerBoxPosRotX );
  auto aSubTransform47 = dd4hep::Transform3D( aSubRot47, aSubPos47 );
  //++++++++++++++++++++++++++++++++++++++++//
  // Create shapes
  for ( int iM = 0; iM < m_NumRich2SphMirrSeg; iM++ ) {

    auto cSeg     = m_Rich2SphMIndexStr[iM];
    auto aMirrSph = dd4hep::Sphere( aSphSegShapePref + cSeg, m_Rich2SphMirrInnerROC[iM], m_Rich2SphMirrOuterROC[iM],
                                    m_Rich2SphMirrHexSegStartTheta[iM], m_Rich2SphMirrHexSegEndTheta[iM],
                                    m_Rich2SphMirrHexSegStartPhi[iM], m_Rich2SphMirrHexSegEndPhi[iM] );

    if ( !( IsRich2SphSegWithCuts( iM ) ) || ( iM == 12 ) || ( iM == 43 ) ) {

      std::vector<dd4hep::SubtractionSolid> aCurSphSub( nQSubtract );
      for ( int iQ = 0; iQ < nQSubtract; iQ++ ) {
        std::string aMirrSubBoxName     = aBoxSubP + aSphBoxSubInd[iQ] + aSpSegSuf + cSeg;
        std::string aMirrSubBooleanName = aBoxSubBoolean + aSphBoxSubInd[iQ] + aSpSegSuf + cSeg;
        dd4hep::Box curBoxSub           = aGenMirrSubBox;
        curBoxSub.setName( aMirrSubBoxName.c_str() );

        aCurSphSub[iQ] =
            ( iQ == 0 )
                ? dd4hep::SubtractionSolid( aMirrSubBooleanName, aMirrSph, curBoxSub, aBoxSubTransform[iQ] )
                : dd4hep::SubtractionSolid( aMirrSubBooleanName, aCurSphSub[iQ - 1], curBoxSub, aBoxSubTransform[iQ] );
      }

      if ( iM == 12 ) { // special case with beampipe hole
        auto aCurSphSub12 =
            dd4hep::SubtractionSolid( ( aBoxSubBooleanSp12 + aSpSegSuf + cSeg ), aCurSphSub[nQSubtract - 1],
                                      aBeamPipeCutCylinder, aSubTransform12 );

        m_Rich2SphMirrShapes[iM] = aCurSphSub12;

      } else if ( iM == 43 ) { // special case with beampipe hole
        auto aCurSphSub43 =
            dd4hep::SubtractionSolid( ( aBoxSubBooleanSp43 + aSpSegSuf + cSeg ), aCurSphSub[nQSubtract - 1],
                                      aBeamPipeCutCylinder, aSubTransform43 );
        m_Rich2SphMirrShapes[iM] = aCurSphSub43;

      } else {
        m_Rich2SphMirrShapes[iM] = aCurSphSub[nQSubtract - 1];
      }

    } else { // special cases with half Hexagons

      if ( ( IsRich2SphLeftHalfSeg( iM ) ) || ( iM == 8 ) || ( iM == 16 ) ) {
        std::vector<dd4hep::SubtractionSolid> aCurSphSubLH( nQSubtractHH );

        for ( int iQ = 0; iQ < nQSubtractHH; iQ++ ) {
          std::string aMirrSubBoxNameLH     = aBoxSubP + aSphBoxLHInd[iQ] + aSpSegSuf + cSeg;
          std::string aMirrSubBooleanNameLH = aBoxSubBoolean + aSphBoxLHInd[iQ] + aSpSegSuf + cSeg;
          dd4hep::Box curBoxSubLH           = aGenMirrSubBox;
          curBoxSubLH.setName( aMirrSubBoxNameLH.c_str() );

          aCurSphSubLH[iQ] = ( iQ == 0 ) ? dd4hep::SubtractionSolid( aMirrSubBooleanNameLH, aMirrSph, curBoxSubLH,
                                                                     aBoxSubTransformLH[iQ] )
                                         : dd4hep::SubtractionSolid( aMirrSubBooleanNameLH, aCurSphSubLH[iQ - 1],
                                                                     curBoxSubLH, aBoxSubTransformLH[iQ] );

        } // end loop over the three LH Sub
        // aBeamPipeCutBox
        if ( iM == 8 ) { // special case with beampipe hole
          auto aCurSphSubLH8 =
              dd4hep::SubtractionSolid( ( aBoxSubBooleanSp8 + aSpSegSuf + cSeg ), aCurSphSubLH[nQSubtractHH - 1],
                                        aBeamPipeCutBox, aSubTransform8 );
          m_Rich2SphMirrShapes[iM] = aCurSphSubLH8;

        } else if ( iM == 16 ) { // special case with beampipe hole
          auto aCurSphSubLH16 =
              dd4hep::SubtractionSolid( ( aBoxSubBooleanSp16 + aSpSegSuf + cSeg ), aCurSphSubLH[nQSubtractHH - 1],
                                        aBeamPipeCutBox, aSubTransform16 );
          m_Rich2SphMirrShapes[iM] = aCurSphSubLH16;

        } else {
          m_Rich2SphMirrShapes[iM] = aCurSphSubLH[nQSubtractHH - 1];
        }
      } else if ( ( IsRich2SphRightHalfSeg( iM ) ) || ( iM == 39 ) || ( iM == 47 ) ) { // end test on LH, start on RH
        std::vector<dd4hep::SubtractionSolid> aCurSphSubRH( nQSubtractHH );
        for ( int iQ = 0; iQ < nQSubtractHH; iQ++ ) {

          std::string aMirrSubBoxNameRH     = aBoxSubP + aSphBoxRHInd[iQ] + aSpSegSuf + cSeg;
          std::string aMirrSubBooleanNameRH = aBoxSubBoolean + aSphBoxRHInd[iQ] + aSpSegSuf + cSeg;
          dd4hep::Box curBoxSubRH           = aGenMirrSubBox;
          curBoxSubRH.setName( aMirrSubBoxNameRH.c_str() );

          aCurSphSubRH[iQ] = ( iQ == 0 ) ? dd4hep::SubtractionSolid( aMirrSubBooleanNameRH, aMirrSph, curBoxSubRH,
                                                                     aBoxSubTransformRH[iQ] )
                                         : dd4hep::SubtractionSolid( aMirrSubBooleanNameRH, aCurSphSubRH[iQ - 1],
                                                                     curBoxSubRH, aBoxSubTransformRH[iQ] );
        } // end loop over the RH Sub

        if ( iM == 39 ) { // special case with beampipe hole
          auto aCurSphSubRH39 =
              dd4hep::SubtractionSolid( ( aBoxSubBooleanSp39 + aSpSegSuf + cSeg ), aCurSphSubRH[nQSubtractHH - 1],
                                        aBeamPipeCutBox, aSubTransform39 );
          m_Rich2SphMirrShapes[iM] = aCurSphSubRH39;
        } else if ( iM == 47 ) { // special case with beampipe hole
          auto aCurSphSubRH47 =
              dd4hep::SubtractionSolid( ( aBoxSubBooleanSp47 + aSpSegSuf + cSeg ), aCurSphSubRH[nQSubtractHH - 1],
                                        aBeamPipeCutBox, aSubTransform47 );
          m_Rich2SphMirrShapes[iM] = aCurSphSubRH47;

        } else {
          m_Rich2SphMirrShapes[iM] = aCurSphSubRH[nQSubtractHH - 1];
        }

      } // end test on RH

    } // end test on special cases

  } // end loop over Mirror Seg
}

//==========================================================================//
void Rich2MirrorGeoAux::BuildRich2SecMirrorShapes() {

  m_Rich2SecMirrShapes.clear();
  m_Rich2SecMirrShapes.resize( m_NumRich2SecMirrSeg );
  std::string aSecSegShapePref = "Rich2SecMirrorSphereSeg";

  // Create LV
  for ( int iM = 0; iM < m_NumRich2SecMirrSeg; iM++ ) {

    std::string    cSeg = m_Rich2SecMIndexStr[iM];
    dd4hep::Sphere aMirrSec =
        dd4hep::Sphere( aSecSegShapePref + cSeg, m_Rich2SecMirrInnerROC[iM], m_Rich2SecMirrOuterROC[iM],
                        m_Rich2SecMirrSegStartTheta[iM], m_Rich2SecMirrSegEndTheta[iM], m_Rich2SecMirrSegStartPhi[iM],
                        m_Rich2SecMirrSegEndPhi[iM] );
    m_Rich2SecMirrShapes[iM] = aMirrSec;
  }
}
//==========================================================================//
void Rich2MirrorGeoAux::BuildRich2SphMirrorTransforms() {
  m_Rich2SphMirrTransfoms.clear();
  dd4hep::Transform3D aDummyTransform;
  m_Rich2SphMirrTransfoms.assign( m_NumRich2SphMirrSeg, aDummyTransform );
  // Nominal COC in GasEnclosure system
  // Here all calculations done in the Gas enclosure coordinate system.
  // Afterwards the COC of each segment is transformed into the Rich2SphMaster systems.

  double           aRich2SphCoCPosY = m_Rich2SphMirrorNominalCoCInGasEnclPosY;
  double           aRich2SphCoCPosZ = m_Rich2SphMirrorNominalCoCInGasEnclPosZ;
  dd4hep::Position aZeroPos         = dd4hep::Position( 0, 0, 0 );

  for ( int iM = 0; iM < m_NumRich2SphMirrSeg; iM++ ) {

    double aRich2SphCoCPosX          = m_Rich2SphMirrorNominalCoCInGasEnclPositivePosX;
    double aRich2SphMirrorMasterPosX = m_Rich2SphMirrorMaster0PosX;
    double aRich2SphMirrorMasterPosY = m_Rich2SphMirrorMaster0PosY;
    double aRich2SphMirrorMasterPosZ = m_Rich2SphMirrorMaster0PosZ;
    if ( !( IsRich2SphMirrSegInPositiveXSide( iM ) ) ) {
      aRich2SphCoCPosX          = m_Rich2SphMirrorNominalCoCInGasEnclNegativePosX;
      aRich2SphMirrorMasterPosX = m_Rich2SphMirrorMaster1PosX;
      aRich2SphMirrorMasterPosY = m_Rich2SphMirrorMaster1PosY;
      aRich2SphMirrorMasterPosZ = m_Rich2SphMirrorMaster1PosZ;
    }

    double aSphSegCePosX = m_Rich2SphMirrorSegCentreInRich2PosX[iM] - m_Rich2MainGasEnclInRich2PosX;
    double aSphSegCePosY = m_Rich2SphMirrorSegCentreInRich2PosY[iM] - m_Rich2MainGasEnclInRich2PosY;
    double aInR          = m_Rich2SphMirrInnerROC[iM];

    double aSphVecCCX = aRich2SphCoCPosX - aSphSegCePosX;
    double aSphVecCCY = aRich2SphCoCPosY - aSphSegCePosY;
    double aSphSegCePosZ =
        std::sqrt( ( aInR * aInR ) - ( aSphVecCCX * aSphVecCCX ) - ( aSphVecCCY * aSphVecCCY ) ) + aRich2SphCoCPosZ;

    double aSphVecCCZ = aRich2SphCoCPosZ - aSphSegCePosZ;

    double aRRatio           = aInR / m_Rich2SphMirrNominalInnerROC;
    double aSphSegModCOCPosX = aSphSegCePosX + ( aRRatio * aSphVecCCX );
    double aSphSegModCOCPosY = aSphSegCePosY + ( aRRatio * aSphVecCCY );
    double aSphSegModCOCPosZ = aSphSegCePosZ + ( aRRatio * aSphVecCCZ );

    double           aSphSegModCOCPosXInSphMirrMaster = aSphSegModCOCPosX - aRich2SphMirrorMasterPosX;
    double           aSphSegModCOCPosYInSphMirrMaster = aSphSegModCOCPosY - aRich2SphMirrorMasterPosY;
    double           aSphSegModCOCPosZInSphMirrMaster = aSphSegModCOCPosZ - aRich2SphMirrorMasterPosZ;
    dd4hep::Position aSphSegPos = dd4hep::Position( aSphSegModCOCPosXInSphMirrMaster, aSphSegModCOCPosYInSphMirrMaster,
                                                    aSphSegModCOCPosZInSphMirrMaster );

    double aSphRotZ     = asin( ( aSphSegCePosY - aSphSegModCOCPosY ) / aInR ) * ( dd4hep::rad );
    double aSphRCosRotZ = aInR * cos( aSphRotZ );
    double aSphCeLocalX = aSphSegModCOCPosX - aSphSegCePosX;
    double aSphRotY     = ( ( -1.0 ) * m_Rich2PiOver2Ang ) - ( asin( aSphCeLocalX / aSphRCosRotZ ) * ( dd4hep::rad ) );

    dd4hep::RotationZYX aSphRotA       = dd4hep::RotationZYX( 0, 0, m_Rich2SphMirrorSegRotX[iM] );
    dd4hep::Transform3D aSphTransformA = dd4hep::Transform3D( aSphRotA, aZeroPos );
    dd4hep::RotationZYX aSphRotB       = dd4hep::RotationZYX( aSphRotZ, aSphRotY, 0 );
    dd4hep::Transform3D aSphTransformB = dd4hep::Transform3D( aSphRotB, aSphSegPos );

    m_Rich2SphMirrTransfoms[iM] = aSphTransformB * aSphTransformA;
  }
}
//==========================================================================//
void Rich2MirrorGeoAux::BuildRich2SecMirrorTransforms() {
  m_Rich2SecMirrTransfoms.clear();
  dd4hep::Transform3D aDummyTransform;
  m_Rich2SecMirrTransfoms.assign( m_NumRich2SecMirrSeg, aDummyTransform );
  int                 aNumSegInAPanel = m_Rich2NumSecMirrorSegRows * m_Rich2NumSecMirrorSegCols;
  dd4hep::RotationZYX aSecSegRot      = dd4hep::RotationZYX( 0, m_Rich2SecMirrorSegRot, 0 );

  for ( int iP = 0; iP < m_Rich2NumSecMirrorMasters; iP++ ) {
    for ( int iY = 0; iY < m_Rich2NumSecMirrorSegRows; iY++ ) {

      double aPosY = m_Rich2SecMirrorArrayStartY + ( iY * m_Rich2SecMirrorArrayStepY );

      for ( int iX = 0; iX < m_Rich2NumSecMirrorSegCols; iX++ ) {

        int              iM         = ( iP * aNumSegInAPanel ) + ( iY * m_Rich2NumSecMirrorSegCols ) + iX;
        double           aPosX      = m_Rich2SecMirrorArrayStartX + ( iX * m_Rich2SecMirrorArrayStepX );
        double           aPosZ      = m_Rich2SecMirrInnerROC[iM];
        dd4hep::Position aSecSegPos = dd4hep::Position( aPosX, aPosY, aPosZ );
        m_Rich2SecMirrTransfoms[iM] = dd4hep::Transform3D( aSecSegRot, aSecSegPos );
      }
    }
  }
}

//==========================================================================//
void Rich2MirrorGeoAux::InitRich2MirrorGeneralParam() {
  m_Rich2SphMIndexStr.clear();
  m_Rich2SecMIndexStr.clear();
  m_Rich2SphMHalfHexIndex.clear();
  m_Rich2SphMirrorMasterLVNames.clear();
  m_Rich2SecMirrorMasterLVNames.clear();

  detail::init_param( m_NumRich2SphMirrSeg, "Rh2NumSphMirrorSegments" );
  detail::init_param( m_NumRich2SecMirrSeg, "Rh2NumSecMirrorSegments" );
  detail::init_param( m_Rich2PiOver2Ang, "Rh2PiOver2Ang" );
  detail::init_param( m_Rich2HalfSize, "RhHalfSize" );

  detail::init_param( m_Rich2SphMirrorHexSegHorizSize, "Rh2SphMirrHexSegHorizSize" );
  detail::init_param( m_Rich2SphMirrorHexSegVertSize, "Rh2SphMirrHexSegVertSize" );

  detail::init_param( m_Rich2SphMirrorHexSegSubBoxLargeXSize, "Rh2SPhMirrHexSubBoxLargeXSize" );
  detail::init_param( m_Rich2SphMirrorHexSegSubBoxLargeYSize, "Rh2SPhMirrHexSubBoxLargeYSize" );
  detail::init_param( m_Rich2SphMirrorHexSegSubBoxLargeZSize, "Rh2SPhMirrHexSubBoxLargeZSize" );
  detail::init_param( m_Rich2SphMirrHexSubBoxRotXQ02, "Rh2SphMirrHexSubBoxRotXQ02" );
  detail::init_param( m_Rich2SphMirrHexSubBoxRotXQ13, "Rh2SphMirrHexSubBoxRotXQ13" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftX, "Rh2SphMirrHexSubBoxShiftX" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftYQ0, "Rh2SphMirrHexSubBoxShiftYQ0" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftYQ1, "Rh2SphMirrHexSubBoxShiftYQ1" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftYQ2, "Rh2SphMirrHexSubBoxShiftYQ2" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftYQ3, "Rh2SphMirrHexSubBoxShiftYQ3" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftZQ0, "Rh2SphMirrHexSubBoxShiftZQ0" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftZQ1, "Rh2SphMirrHexSubBoxShiftZQ1" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftZQ2, "Rh2SphMirrHexSubBoxShiftZQ2" );
  detail::init_param( m_Rich2SphMirrHexSubBoxShiftZQ3, "Rh2SphMirrHexSubBoxShiftZQ3" );

  detail::init_param( m_Rich2SphMirrLeftHalfHexSubBoxShiftX, "Rh2SphMirrLeftHalfHexSubBoxShiftX" );
  detail::init_param( m_Rich2SphMirrLeftHalfHexSubBoxShiftZ, "Rh2SphMirrLeftHalfHexSubBoxShiftZ" );
  detail::init_param( m_Rich2SphMirrLeftHalfHexSubBoxRotXQ1, "Rh2SphMirrLeftHalfHexSubBoxRotXQ1" );
  detail::init_param( m_Rich2SphMirrLeftHalfHexSubBoxRotXQ2, "Rh2SphMirrLeftHalfHexSubBoxRotXQ2" );
  detail::init_param( m_Rich2SphMirrLeftHalfHexSubBoxShiftYQ1, "Rh2SphMirrLeftHalfHexSubBoxShiftYQ1" );
  detail::init_param( m_Rich2SphMirrLeftHalfHexSubBoxShiftYQ2, "Rh2SphMirrLeftHalfHexSubBoxShiftYQ2" );
  detail::init_param( m_Rich2SphMirrLeftHalfHexSubBoxShiftZQ1, "Rh2SphMirrLeftHalfHexSubBoxShiftZQ1" );
  detail::init_param( m_Rich2SphMirrLeftHalfHexSubBoxShiftZQ2, "Rh2SphMirrLeftHalfHexSubBoxShiftZQ2" );
  detail::init_param( m_Rich2SphMirrRightHalfHexSubBoxShiftX, "Rh2SphMirrRightHalfHexSubBoxShiftX" );
  detail::init_param( m_Rich2SphMirrRightHalfHexSubBoxShiftZ, "Rh2SphMirrRightHalfHexSubBoxShiftZ" );
  detail::init_param( m_Rich2SphMirrRightHalfHexSubBoxRotXQ0, "Rh2SphMirrRightHalfHexSubBoxRotXQ0" );
  detail::init_param( m_Rich2SphMirrRightHalfHexSubBoxRotXQ3, "Rh2SphMirrRightHalfHexSubBoxRotXQ3" );
  detail::init_param( m_Rich2SphMirrRightHalfHexSubBoxShiftYQ0, "Rh2SphMirrRightHalfHexSubBoxShiftYQ0" );
  detail::init_param( m_Rich2SphMirrRightHalfHexSubBoxShiftYQ3, "Rh2SphMirrRightHalfHexSubBoxShiftYQ3" );
  detail::init_param( m_Rich2SphMirrRightHalfHexSubBoxShiftZQ0, "Rh2SphMirrRightHalfHexSubBoxShiftZQ0" );
  detail::init_param( m_Rich2SphMirrRightHalfHexSubBoxShiftZQ3, "Rh2SphMirrRightHalfHexSubBoxShiftZQ3" );

  detail::init_param( m_Rich2SphMirCutOutBPipeCylinderHalfSizeZ, "Rh2SphMirCutOutBPipeCylinderHalfSizeZ" );
  detail::init_param( m_Rich2SphMirCutOutBPipeCylinderRadius, "Rh2SphMirCutOutBPipeCylinderRadius" );

  detail::init_param( m_Rich2SphMirCutOutBPipeCylinderRotY, "Rh2SphMirCutOutBPipeCylinderRotY" );

  detail::init_param( m_Rich2SphMir12CutOutBPipeCylinderPosZ, "Rh2SphMir12CutOutBPipeCylinderPosZ" );
  detail::init_param( m_Rich2SphMir43CutOutBPipeCylinderPosZ, "Rh2SphMir43CutOutBPipeCylinderPosZ" );

  detail::init_param( m_Rich2SphMirCutOutBPipeCornerBoxSizeX, "Rh2SphMirCutOutBPipeCornerBoxSizeX" );
  detail::init_param( m_Rich2SphMirCutOutBPipeCornerBoxSizeY, "Rh2SphMirCutOutBPipeCornerBoxSizeY" );
  detail::init_param( m_Rich2SphMirCutOutBPipeCornerBoxSizeZ, "Rh2SphMirCutOutBPipeCornerBoxSizeZ" );
  detail::init_param( m_Rich2SphMirCutOutBPipeCornerBoxBotPosY, "Rh2SphMirCutOutBPipeCornerBoxBotPosY" );
  detail::init_param( m_Rich2SphMirCutOutBPipeCornerBoxTopPosY, "Rh2SphMirCutOutBPipeCornerBoxTopPosY" );
  detail::init_param( m_Rich2SphMirCutOutBPipeCornerBoxPosZ, "Rh2SphMirCutOutBPipeCornerBoxPosZ" );
  detail::init_param( m_Rich2SphMirCutOutBPipeCornerBoxPosRotX, "Rh2SphMirCutOutBPipeCornerBoxPosRotX" );
  detail::init_param( m_Rich2SphMirCutOutBPipeCornerBoxNegRotX, "Rh2SphMirCutOutBPipeCornerBoxNegRotX" );

  m_Rich2SphMIndexStr = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13",
                         "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27",
                         "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41",
                         "42", "43", "44", "45", "46", "47", "48", "49", "50", "51", "52", "53", "54", "55"};

  m_Rich2SecMIndexStr = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13",
                         "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27",
                         "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39"};

  m_Rich2SphMHalfHexIndex = {0, 7, 8, 12, 15, 16, 23, 24, 31, 32, 39, 40, 43, 47, 48, 55};

  m_Rich2SphMLeftHalfHexIndex  = {0, 24, 32, 40, 48};
  m_Rich2SphMRightHalfHexIndex = {7, 15, 23, 31, 55};
  detail::init_param( m_Rich2SphMirrPositiveMaxCopyNum, "Rh2SphMirrPositiveMaxCopyNum" );
  detail::init_param( m_Rich2SphMirrorMaster0PosX, "Rh2SphMirrorMaster0PosX" );
  detail::init_param( m_Rich2SphMirrorMaster1PosX, "Rh2SphMirrorMaster1PosX" );
  detail::init_param( m_Rich2SphMirrorMaster0PosY, "Rh2SphMirrorMaster0PosY" );
  detail::init_param( m_Rich2SphMirrorMaster1PosY, "Rh2SphMirrorMaster1PosY" );
  detail::init_param( m_Rich2SphMirrorMaster0PosZ, "Rh2SphMirrorMaster0PosZ" );
  detail::init_param( m_Rich2SphMirrorMaster1PosZ, "Rh2SphMirrorMaster1PosZ" );
  detail::init_param( m_Rich2MainGasEnclInRich2PosX, "Rh2MainGasEnclosurePosX" );
  detail::init_param( m_Rich2MainGasEnclInRich2PosY, "Rh2MainGasEnclosurePosY" );

  m_Rich2SphMirrorMasterLVNames = {"lvRich2SphMirrorMaster0", "lvRich2SphMirrorMaster1"};
  m_Rich2SecMirrorMasterLVNames = {"lvRich2SecMirrorMaster0", "lvRich2SecMirrorMaster1"};

  detail::init_param( m_Rich2SecMirrPositiveMaxCopyNum, "Rh2SecMirrPositiveMaxCopyNum" );
}
//==========================================================================//
void Rich2MirrorGeoAux::InitRich2SphMirrorParam() {

  m_Rich2SphMirrInnerROC.clear();
  m_Rich2SphMirrOuterROC.clear();
  m_Rich2SphMirrHexSegEndTheta.clear();
  m_Rich2SphMirrHexSegEndPhi.clear();
  m_Rich2SphMirrHexSegDeltaTheta.clear();
  m_Rich2SphMirrHexSegDeltaPhi.clear();
  m_Rich2SphMirrHexSegStartTheta.clear();
  m_Rich2SphMirrHexSegStartPhi.clear();
  m_Rich2SphMirrLVNames.clear();
  m_Rich2SphMirrPVNames.clear();
  m_Rich2SphMirrCopyNumbers.clear();
  m_Rich2SphMirrorSegRotX.clear();
  m_Rich2SphMirrDetNames.clear();

  detail::init_param( m_Rich2SphMirrNominalInnerROC, "Rh2SphMirrorNomInnerROC" );
  detail::init_param( m_Rich2SphMirrorThickness, "Rh2SphMirrorThickness" );

  m_Rich2SphMirrInnerROC.assign( m_NumRich2SphMirrSeg, m_Rich2SphMirrNominalInnerROC );
  m_Rich2SphMirrOuterROC.assign( m_NumRich2SphMirrSeg, ( m_Rich2SphMirrNominalInnerROC + m_Rich2SphMirrorThickness ) );
  m_Rich2SphMirrHexSegEndTheta.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SphMirrHexSegEndPhi.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SphMirrHexSegDeltaTheta.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SphMirrHexSegDeltaPhi.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SphMirrHexSegStartTheta.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SphMirrHexSegStartPhi.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SphMirrLVNames.assign( m_NumRich2SphMirrSeg, "DummyRich2SphMirrLVName" );
  m_Rich2SphMirrPVNames.assign( m_NumRich2SphMirrSeg, "DummyRich2SphMirrPVName" );
  m_Rich2SphMirrCopyNumbers.assign( m_NumRich2SphMirrSeg, 0 );
  m_Rich2SphMirrDetNames.assign( m_NumRich2SphMirrSeg, "DummyRich2SphMirrDetName" );

  m_Rich2SphMirrorSegCentreInRich2PosX.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::mm ) );
  m_Rich2SphMirrorSegCentreInRich2PosY.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::mm ) );

  m_Rich2SphMirrorSegRotX.assign( m_NumRich2SphMirrSeg, 0.0 * ( dd4hep::rad ) );

  std::string aRPref        = "Rh2SphMirror";
  std::string aRSuf         = "InnerROC";
  std::string aStdLVPref    = "lv";
  std::string aStdPVPref    = "pv";
  std::string aStdLabelPref = "Rich2SphMirror";
  std::string aStdLVMid     = "HexSeg";
  std::string aLH           = "LeftHalf";
  std::string aRH           = "RightHalf";
  std::string aBPH          = "WithBotBPHole";
  std::string aTPH          = "WithTopBPHole";
  std::string aLPH          = "WithLeftBPHole";
  std::string aRPH          = "WithRightBPHole";

  std::string aCocPref  = "Rh2SphMirrorCentreSeg";
  std::string aCocXSuf  = "PosX";
  std::string aCocYSuf  = "PosY";
  std::string aRotXPref = "Rh2SphMirrorSeg";
  std::string aRotXSuf  = "RotX";

  for ( int iM = 0; iM < m_NumRich2SphMirrSeg; iM++ ) {
    auto aInR                  = detail::dd4hep_param<double>( aRPref + m_Rich2SphMIndexStr[iM] + aRSuf );
    m_Rich2SphMirrInnerROC[iM] = aInR;
    m_Rich2SphMirrOuterROC[iM] = aInR + m_Rich2SphMirrorThickness;
    m_Rich2SphMirrHexSegDeltaTheta[iM] =
        ( 2.0 * asin( ( 0.5 * m_Rich2SphMirrorHexSegHorizSize ) / aInR ) ) * ( dd4hep::rad );
    m_Rich2SphMirrHexSegDeltaPhi[iM] =
        ( 2.0 * asin( ( 0.5 * m_Rich2SphMirrorHexSegVertSize ) / aInR ) ) * ( dd4hep::rad );
    m_Rich2SphMirrHexSegStartTheta[iM] = m_Rich2PiOver2Ang - ( 0.5 * m_Rich2SphMirrHexSegDeltaTheta[iM] );
    m_Rich2SphMirrHexSegStartPhi[iM]   = ( -0.5 ) * m_Rich2SphMirrHexSegDeltaPhi[iM];
    m_Rich2SphMirrHexSegEndTheta[iM]   = m_Rich2SphMirrHexSegStartTheta[iM] + m_Rich2SphMirrHexSegDeltaTheta[iM];
    m_Rich2SphMirrHexSegEndPhi[iM]     = m_Rich2SphMirrHexSegStartPhi[iM] + m_Rich2SphMirrHexSegDeltaPhi[iM];

    double aCocXG = detail::dd4hep_param<double>( aCocPref + m_Rich2SphMIndexStr[iM] + aCocXSuf );
    double aCocYG = detail::dd4hep_param<double>( aCocPref + m_Rich2SphMIndexStr[iM] + aCocYSuf );
    m_Rich2SphMirrorSegCentreInRich2PosX[iM] = aCocXG;
    m_Rich2SphMirrorSegCentreInRich2PosY[iM] = aCocYG;
    double aRotXVal             = detail::dd4hep_param<double>( aRotXPref + m_Rich2SphMIndexStr[iM] + aRotXSuf );
    m_Rich2SphMirrorSegRotX[iM] = aRotXVal;

    std::string aCurLabel;
    if ( ( IsRich2SphSegWithCuts( iM ) ) ) {
      // special ones with half hex and beampipe hole
      if ( IsRich2SphLeftHalfSeg( iM ) ) {
        aCurLabel = aLH;
      } else if ( IsRich2SphRightHalfSeg( iM ) ) {
        aCurLabel = aRH;
      } else if ( iM == 8 ) {
        aCurLabel = aBPH + aLH;
      } else if ( iM == 12 ) {
        aCurLabel = aLPH;
      } else if ( iM == 16 ) {
        aCurLabel = aTPH + aLH;
      } else if ( iM == 39 ) {
        aCurLabel = aBPH + aRH;
      } else if ( iM == 43 ) {
        aCurLabel = aRPH;
      } else if ( iM == 47 ) {
        aCurLabel = aTPH + aRH;
      }
    } // end test on special cases

    m_Rich2SphMirrLVNames[iM]     = aStdLVPref + aStdLabelPref + aCurLabel + aStdLVMid + m_Rich2SphMIndexStr[iM];
    m_Rich2SphMirrPVNames[iM]     = aStdPVPref + aStdLabelPref + aCurLabel + aStdLVMid + m_Rich2SphMIndexStr[iM];
    m_Rich2SphMirrCopyNumbers[iM] = iM;
    m_Rich2SphMirrDetNames[iM]    = aStdLabelPref + aCurLabel + aStdLVMid + m_Rich2SphMIndexStr[iM];

  } // end loop over mirr seg

  detail::init_param( m_Rich2SphMirrorNominalCoCInGasEnclPositivePosX, "Rh2SphMirrorCoCXPositiveInGasEncl" );
  detail::init_param( m_Rich2SphMirrorNominalCoCInGasEnclNegativePosX, "Rh2SphMirrorCoCXNegativeInGasEncl" );
  detail::init_param( m_Rich2SphMirrorNominalCoCInGasEnclPosY, "Rh2SphMirrorCoCYinGasEncl" );
  detail::init_param( m_Rich2SphMirrorNominalCoCInGasEnclPosZ, "Rh2SphMirrorCoCZinGasEncl" );

  detail::init_param( m_Rich2MainGasEnclInRich2PosX, "Rh2MainGasEnclosurePosX" );
  detail::init_param( m_Rich2MainGasEnclInRich2PosY, "Rh2MainGasEnclosurePosY" );
}
//==========================================================================//

void Rich2MirrorGeoAux::InitRich2SecMirrorParam() {

  m_Rich2SecMirrInnerROC.clear();
  m_Rich2SecMirrOuterROC.clear();
  m_Rich2SecMirrSegDeltaTheta.clear();
  m_Rich2SecMirrSegDeltaPhi.clear();
  m_Rich2SecMirrSegStartTheta.clear();
  m_Rich2SecMirrSegStartPhi.clear();
  m_Rich2SecMirrSegEndTheta.clear();
  m_Rich2SecMirrSegEndPhi.clear();
  m_Rich2SecMirrLVNames.clear();
  m_Rich2SecMirrPVNames.clear();
  m_Rich2SecMirrCopyNumbers.clear();
  m_Rich2SecMirrDetNames.clear();

  detail::init_param( m_Rich2SecMirrNominalInnerROC, "Rh2SecMirrorNomInnerROC" );
  detail::init_param( m_Rich2SecMirrorThickness, "Rh2SecMirrorThickness" );
  detail::init_param( m_Rich2SecMirrorSegSizeX, "Rh2SecMirrorSegSizeX" );
  detail::init_param( m_Rich2SecMirrorSegSizeY, "Rh2SecMirrorSegSizeY" );

  detail::init_param( m_Rich2NumSecMirrorSegRows, "Rh2NumSecMirrorSegRows" );
  detail::init_param( m_Rich2NumSecMirrorSegCols, "Rh2NumSecMirrorSegCols" );
  detail::init_param( m_Rich2NumSecMirrorMasters, "Rh2NumSecMirrorMasters" );
  detail::init_param( m_Rich2SecMirrorArrayStartX, "Rh2SecMirrorArrayStartX" );
  detail::init_param( m_Rich2SecMirrorArrayStartY, "Rh2SecMirrorArrayStartY" );
  detail::init_param( m_Rich2SecMirrorArrayStepX, "Rh2SecMirrorSegStepX" );
  detail::init_param( m_Rich2SecMirrorArrayStepY, "Rh2SecMirrorSegStepY" );

  detail::init_param( m_Rich2SecMirrorSegRot, "Rh2SecMirrorSegRot" );

  m_Rich2SecMirrInnerROC.assign( m_NumRich2SecMirrSeg, m_Rich2SecMirrNominalInnerROC );
  m_Rich2SecMirrOuterROC.assign( m_NumRich2SecMirrSeg, ( m_Rich2SecMirrNominalInnerROC + m_Rich2SecMirrorThickness ) );
  m_Rich2SecMirrSegDeltaTheta.assign( m_NumRich2SecMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SecMirrSegDeltaPhi.assign( m_NumRich2SecMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SecMirrSegStartTheta.assign( m_NumRich2SecMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SecMirrSegStartPhi.assign( m_NumRich2SecMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SecMirrSegEndTheta.assign( m_NumRich2SecMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SecMirrSegEndPhi.assign( m_NumRich2SecMirrSeg, 0.0 * ( dd4hep::rad ) );
  m_Rich2SecMirrLVNames.assign( m_NumRich2SecMirrSeg, "aDummySecMirLVName" );
  m_Rich2SecMirrPVNames.assign( m_NumRich2SecMirrSeg, "DummyRich2SecMirrPVName" );
  m_Rich2SecMirrCopyNumbers.assign( m_NumRich2SecMirrSeg, 0 );
  m_Rich2SecMirrDetNames.assign( m_NumRich2SecMirrSeg, "DummyRich2SecMirrDetName" );

  std::string aRPref        = "Rh2SecMirror";
  std::string aRSuf         = "InnerROC";
  std::string aSecLVPref    = "lv";
  std::string aSecPVPref    = "pv";
  std::string aSecLabelPref = "Rich2SecMirrorSeg";

  for ( int iM = 0; iM < m_NumRich2SecMirrSeg; iM++ ) {
    const auto aInR = detail::dd4hep_param<double>( aRPref + m_Rich2SecMIndexStr[iM] + aRSuf );

    m_Rich2SecMirrInnerROC[iM]      = aInR;
    m_Rich2SecMirrOuterROC[iM]      = aInR + m_Rich2SecMirrorThickness;
    m_Rich2SecMirrSegDeltaTheta[iM] = 2.0 * ( asin( ( 0.5 * m_Rich2SecMirrorSegSizeX ) / aInR ) ) * ( dd4hep::rad );

    m_Rich2SecMirrSegDeltaPhi[iM] = 2.0 * ( asin( ( 0.5 * m_Rich2SecMirrorSegSizeY ) / aInR ) ) * ( dd4hep::rad );

    m_Rich2SecMirrSegStartTheta[iM] = m_Rich2PiOver2Ang - ( 0.5 * m_Rich2SecMirrSegDeltaTheta[iM] );
    m_Rich2SecMirrSegStartPhi[iM]   = ( -0.5 ) * m_Rich2SecMirrSegDeltaPhi[iM];

    m_Rich2SecMirrSegEndTheta[iM] = m_Rich2SecMirrSegStartTheta[iM] + m_Rich2SecMirrSegDeltaTheta[iM];
    m_Rich2SecMirrSegEndPhi[iM]   = m_Rich2SecMirrSegStartPhi[iM] + m_Rich2SecMirrSegDeltaPhi[iM];

    m_Rich2SecMirrLVNames[iM]     = aSecLVPref + aSecLabelPref + m_Rich2SecMIndexStr[iM];
    m_Rich2SecMirrPVNames[iM]     = aSecPVPref + aSecLabelPref + m_Rich2SecMIndexStr[iM];
    m_Rich2SecMirrCopyNumbers[iM] = iM;
    m_Rich2SecMirrDetNames[iM]    = aSecLabelPref + m_Rich2SecMIndexStr[iM];
  }
}
//============================================================================================//

Rich2MirrorGeoAux::DEList                                                            //
Rich2MirrorGeoAux::Rich2SphMirrorListDE( const dd4hep::DetElement&             de,   //
                                         dd4hep::cond::ConditionUpdateContext& ctxt, //
                                         int                                   aSide ) const {
  const auto aRich2NumSphMirrorSegTotal = detail::dd4hep_param<int>( "Rh2NumSphMirrorSegments" );
  const auto aRich2NumSphMirrorSegSide  = aRich2NumSphMirrorSegTotal / 2;
  DEList     aDep;
  aDep.reserve( aRich2NumSphMirrorSegSide );
  for ( int iSeg = 0; iSeg < aRich2NumSphMirrorSegSide; ++iSeg ) {
    const auto iSegCopyNum = ( aSide == 0 ? iSeg : ( iSeg + aRich2NumSphMirrorSegSide ) );
    aDep.emplace_back( de.child( Rich2SphMirrDetName( iSegCopyNum ) ), ctxt );
  }
  return aDep;
}
//=======================================================================================================//

Rich2MirrorGeoAux::DEList                                                            //
Rich2MirrorGeoAux::Rich2SecMirrorListDE( const dd4hep::DetElement&             de,   //
                                         dd4hep::cond::ConditionUpdateContext& ctxt, //
                                         int                                   aSide ) const {
  const auto aRich2NumSecMirrorSegTotal = detail::dd4hep_param<int>( "Rh2NumSecMirrorSegments" );
  const auto aRich2NumSecMirrorSegSide  = aRich2NumSecMirrorSegTotal / 2;
  DEList     aDep;
  aDep.reserve( aRich2NumSecMirrorSegSide );
  for ( int iSeg = 0; iSeg < aRich2NumSecMirrorSegSide; ++iSeg ) {
    const auto iSegCopyNum = ( aSide == 0 ? iSeg : ( iSeg + aRich2NumSecMirrorSegSide ) );
    aDep.emplace_back( de.child( Rich2SecMirrDetName( iSegCopyNum ) ), ctxt );
  }
  return aDep;
}
//==========================================================================//

/// Initializing constructor
Rich2MirrorGeoAux::Rich2MirrorGeoAux() {
  InitRich2MirrorGeneralParam();
  InitRich2SphMirrorParam();
  InitRich2SecMirrorParam();
}
//==========================================================================//

Rich2MirrorGeoAux* Rich2MirrorGeoAux::getRich2MirrorGeoAuxInstance() {
  static std::once_flag                     alloc_instance_once;
  static std::unique_ptr<Rich2MirrorGeoAux> RichGeoUtilInstance;
  std::call_once( alloc_instance_once, []() { RichGeoUtilInstance = std::make_unique<Rich2MirrorGeoAux>(); } );
  assert( RichGeoUtilInstance.get() );
  return RichGeoUtilInstance.get();
}
//==========================================================================//
