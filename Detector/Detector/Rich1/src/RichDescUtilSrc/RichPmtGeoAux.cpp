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
//==========================================================================//

#include "Detector/Rich1/RichPmtGeoAux.h"
#include "Detector/Rich1/RichGeoUtil.h"
#include "Detector/Rich2/Rich2GeoUtil.h"
#include <array>
#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

using namespace LHCb::Detector;

//==========================================================================//
void RichPmtGeoAux::build_RichPmtTransforms( xml_h tr_handle ) {

  printout( m_RichPmtGeoAuxDebug ? dd4hep::ALWAYS : dd4hep::DEBUG, "RichPmtGeoAux", "Now building RichPmtTransforms" );

  for ( xml_coll_t i( tr_handle, _U( item ) ); i; ++i ) {

    xml_comp_t c( i );
    const auto itemLabel = c.attr<std::string>( _U( name ) );
    if ( itemLabel.find( "TransformName" ) != std::string::npos ) {
      const auto trName = c.attr<std::string>( _U( value ) );
      xml_elt_t  trn    = c.child( _U( transformation ) );
      printout( m_RichPmtGeoAuxDebug ? dd4hep::ALWAYS : dd4hep::DEBUG, "RichPmtGeoAux", "build RichTransform for %s",
                trName.c_str() );

      const auto tr = dd4hep::xml::createTransformation( trn );

      // here there is only a cross-check that m_RichPmtTransformNames is filled.
      // In the future one may try to store info in the m_RichPmtTransformNames once
      // a set of specialized unicode tags are setup for storing string constants.

      // std::vector<std::string>::iterator itTr =
      //    std::find( m_RichPmtTransformNames.begin(), m_RichTransformNames.end(), trName );
      // index = std::distance(m_RichTransformNames.begin(),itTr);

      if ( IsInRichPmtTransformList( trName ) ) {

        auto retRTrans = m_RichPmtCompGeomTransforms.insert( std::make_pair( trName, tr ) );

        if ( !retRTrans.second ) {
          printout( m_RichPmtGeoAuxDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "RichPmtGeoAux",
                    " RichPmtCompGeom Transform for %s  not stored since it already existed with a value of %d ",
                    trName.c_str(), retRTrans.first->second );
        }

        // end of test on m_RichTransformNames
      } else {

        printout( m_RichPmtGeoAuxDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "RichPmtGeoAux",
                  "Transform name not available for %s and hence Transform not stored ", trName.c_str() );
      }
      // end test on transformName in label
    } else if ( itemLabel.find( "PhysVolName" ) != std::string::npos ) {

      const auto physVolName = c.attr<std::string>( _U( value ) );

      // here there is only a cross-check that m_RichPhysVolNames is filled.
      // In the future one may try to store info in m_RichPmtCompPhysVolNames once
      // a set of specialized unicode tags are setup for storing string constants.
      // Similar thing also may be done for copy numbers, just so that these xml files need to
      // be loaded only once.

      if ( std::find( m_RichPmtCompPhysVolNames.begin(), m_RichPmtCompPhysVolNames.end(), physVolName ) !=
           m_RichPmtCompPhysVolNames.end() ) {

        printout( m_RichPmtGeoAuxDebug ? dd4hep::ALWAYS : dd4hep::DEBUG, "RichPmtGeoAux", "PhysVol name OK for  %s ",
                  physVolName.c_str() );

      } else {

        printout( m_RichPmtGeoAuxDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "RichPmtGeoAux",
                  "PhysVol name not available for %s ", physVolName.c_str() );
      }

    } // end test on physvolname

  } // end loop over transforms with trhandle
}
//==========================================================================//
void RichPmtGeoAux::initRich1PmtModuleTypes() {

  const std::string lvPref            = "lv";
  const std::string pvPref            = "pv";
  const std::string aModuleShapeLabel = "Rich1PmtModuleBox";

  for ( int ip = 0; ip < m_Rh1NumPanels; ip++ ) { // loop over the two panels

    for ( int irow = 0; irow < m_Rh1NumPMTModulesInColumn; irow++ ) {
      for ( int icol = 0; icol < m_Rh1NumPMTModulesInRow; icol++ ) {
        const auto curModuleNum  = icol + irow * m_Rh1NumPMTModulesInRow + ip * m_Rh1TotalNumPmtModulesInPanel;
        auto       aModuleNumSuf = std::to_string( curModuleNum );
        if ( curModuleNum > 9 && curModuleNum <= 99 ) {
          aModuleNumSuf = "0" + aModuleNumSuf;
        } else if ( curModuleNum <= 9 ) {
          aModuleNumSuf = "00" + aModuleNumSuf;
        }
        int aModuleType = 0;
        if ( icol == 0 ) {
          aModuleType = ( irow == 0 ) ? 3 : 1;
        } else if ( icol == ( m_Rh1NumPMTModulesInRow - 1 ) ) {
          aModuleType = ( irow == 0 ) ? 4 : 2;
        }

        const auto& aModuleLabel = m_Rich1PmtModuleType[aModuleType];

        m_Rich1PmtModuleLogVolNames[curModuleNum]  = lvPref + aModuleLabel + aModuleNumSuf;
        m_Rich1PmtModulePhysVolNames[curModuleNum] = pvPref + aModuleLabel + aModuleNumSuf;
        m_Rich1PmtModuleShapeNames[curModuleNum]   = aModuleShapeLabel + aModuleNumSuf;
        m_Rich1PmtModuleTypeIndex[curModuleNum]    = aModuleType;
        m_Rich1PmtModuleDetNames[curModuleNum]     = m_Rich1PmtModuleCommonDetNamePref + aModuleNumSuf;

        initRich1PmtECTypes( curModuleNum, aModuleNumSuf );
        initRich1PmtMasterTypes( curModuleNum, aModuleNumSuf );

      } // end loop over column
    }   // end loop over row
  }     // end loop over panels
}
//==========================================================================//
void RichPmtGeoAux::initRich1PmtECTypes( int bModuleNum, const std::string& bModuleLabel ) {

  const std::string aEStringA       = "RichPmtStd";
  const std::string aEStringB       = "InModule";
  const std::string aECRShapeLabelA = "Rich1Pmt";
  const std::string aECRShapeLabelB = "Box";

  for ( int ic = 0; ic < m_RhMaxNumECRInModule; ic++ ) {

    m_Rich1PmtECRLogVolNames[bModuleNum][ic]  = "lv" + aEStringA + m_RichECRType[ic] + aEStringB + bModuleLabel;
    m_Rich1PmtECRPhysVolNames[bModuleNum][ic] = "pv" + aEStringA + m_RichECRType[ic] + aEStringB + bModuleLabel;
    m_Rich1PmtECRShapeNames[bModuleNum][ic] =
        aECRShapeLabelA + m_RichECRType[ic] + aECRShapeLabelB + aEStringB + bModuleLabel;
    m_Rich1PmtECRDetNames[bModuleNum][ic] =
        m_Rich1PmtECRCommonDetNamePref + m_RichECRType[ic] + aEStringB + bModuleLabel;

    if ( m_RichPmtGeoAuxDebug ) {
      printout( dd4hep::DEBUG, "RichPmtGeoAux",
                "Rich1 Module ECR : Creating ECR Log Phys Shape and Det names %i %i %s %s %s %s", //
                bModuleNum, ic, m_Rich1PmtECRLogVolNames[bModuleNum][ic].c_str(),
                m_Rich1PmtECRPhysVolNames[bModuleNum][ic].c_str(), m_Rich1PmtECRShapeNames[bModuleNum][ic].c_str(),
                m_Rich1PmtECRDetNames[bModuleNum][ic].c_str() );
    }
  }
}

//==========================================================================//
void RichPmtGeoAux::initRich1PmtMasterTypes( int aModuleNum, const std::string& aModuleLabel ) {

  const std::string aPStringA      = "RichPmtMaster";
  const std::string aPStringB      = "In";
  const std::string aPStringC      = "InModule";
  const std::string aPmtShapeLabel = "Box";

  for ( int ic = 0; ic < m_RhMaxNumECRInModule; ic++ ) {
    const auto aS = aPStringB + m_RichECRType[ic] + aPStringC + aModuleLabel;

    for ( int ip = 0; ip < m_RhNumPMTInECR; ip++ ) {
      const auto ipc                                = ( ic * m_RhNumPMTInECR ) + ip;
      m_Rich1PmtMasterLogVolNames[aModuleNum][ipc]  = "lv" + aPStringA + m_RichPmtMasterType[ip] + aS;
      m_Rich1PmtMasterPhysVolNames[aModuleNum][ipc] = "pv" + aPStringA + m_RichPmtMasterType[ip] + aS;
      m_Rich1PmtMasterShapeNames[aModuleNum][ipc]   = aPStringA + aPmtShapeLabel + m_RichPmtMasterType[ip] + aS;
      m_Rich1PmtMasterDetNames[aModuleNum][ipc]     = m_Rich1PmtMasterCommonDetNamePref + m_RichPmtMasterType[ip] + aS;

      if ( m_RichPmtGeoAuxDebug ) {
        printout( dd4hep::DEBUG, "RichPmtGeoAux",
                  "Rich1 Module ECR PmtMaster: Creating Pmt Log Phys and Shape names %i %i %i %s %s %s", //
                  aModuleNum, ic, ip, m_Rich1PmtMasterLogVolNames[aModuleNum][ipc].c_str(),
                  m_Rich1PmtMasterPhysVolNames[aModuleNum][ipc].c_str(),
                  m_Rich1PmtMasterShapeNames[aModuleNum][ipc].c_str() );
      }
    }
  }
}
//==========================================================================//
void RichPmtGeoAux::CreateRich1PmtModuleTransforms() {

  const auto a_Rh1PMTModuleXLocationStartTop = detail::dd4hep_param<double>( "Rh1PMTModuleXLocationStartTop" );
  const auto a_Rh1PMTModuleYLocationStartTop = detail::dd4hep_param<double>( "Rh1PMTModuleYLocationStartTop" );

  const auto a_Rh1PMTModuleTopShiftX = detail::dd4hep_param<double>( "Rh1PMTModuleTopShiftX" );
  const auto a_Rh1PMTModuleTopShiftY = detail::dd4hep_param<double>( "Rh1PMTModuleTopShiftY" );

  const auto a_Rh1PMTModuleMasterZ = detail::dd4hep_param<double>( "Rh1PMTModuleMasterZ" );
  const auto a_RhPMTModuleRotZ     = detail::dd4hep_param<double>( "RhPMTModuleRotZ" );

  const auto a_Rh1PMTModuleXLocationStartBot = detail::dd4hep_param<double>( "Rh1PMTModuleXLocationStartBot" );
  const auto a_Rh1PMTModuleYLocationStartBot = detail::dd4hep_param<double>( "Rh1PMTModuleYLocationStartBot" );

  const auto a_Rh1PMTModuleBotShiftX = detail::dd4hep_param<double>( "Rh1PMTModuleBotShiftX" );
  const auto a_Rh1PMTModuleBotShiftY = detail::dd4hep_param<double>( "Rh1PMTModuleBotShiftY" );

  // Hack to test if we have the old or new PMT arrangement, as per
  // https://gitlab.cern.ch/lhcb/Detector/-/merge_requests/205
  // maybe better to have some dedicated parameter in the DB for this, or ... ?
  // can anyway be removed once need to support old DetDesc MC is gone.
  const bool isNewArrangement = detail::dd4hep_param<int>( "Rh1NumECRInOuterCornerModuleRow" ) == 0;

  for ( int ip = 0; ip < m_Rh1NumPanels; ip++ ) { // Loop over the two panels.

    for ( int irow = 0; irow < m_Rh1NumPMTModulesInColumn; irow++ ) {
      for ( int icol = 0; icol < m_Rh1NumPMTModulesInRow; icol++ ) {

        int curModuleNum = icol + irow * m_Rh1NumPMTModulesInRow + ip * m_Rh1TotalNumPmtModulesInPanel;

        if ( ip == 0 ) { // top panel

          const auto             xPosMT  = a_Rh1PMTModuleXLocationStartTop + ( icol * a_Rh1PMTModuleTopShiftX );
          const auto             yPosMT  = a_Rh1PMTModuleYLocationStartTop + ( irow * a_Rh1PMTModuleTopShiftY );
          const dd4hep::Position topPosM = dd4hep::Position( xPosMT, yPosMT, a_Rh1PMTModuleMasterZ );

          if ( isNewArrangement ) {
            m_Rich1PmtModuleTransforms[curModuleNum] = dd4hep::Transform3D( topPosM );
          } else {
            const dd4hep::RotationZYX topRotM( a_RhPMTModuleRotZ, 0, 0 );
            m_Rich1PmtModuleTransforms[curModuleNum] = dd4hep::Transform3D( topRotM, topPosM );
          }

        } else if ( ip == 1 ) { // bottom panel

          const auto             xPosMB = a_Rh1PMTModuleXLocationStartBot + ( icol * a_Rh1PMTModuleBotShiftX );
          const auto             yPosMB = a_Rh1PMTModuleYLocationStartBot + ( irow * a_Rh1PMTModuleBotShiftY );
          const dd4hep::Position botPosM( xPosMB, yPosMB, a_Rh1PMTModuleMasterZ );

          if ( isNewArrangement ) {
            const dd4hep::RotationZYX botRotM( a_RhPMTModuleRotZ, 0, 0 );
            m_Rich1PmtModuleTransforms[curModuleNum] = dd4hep::Transform3D( botRotM, botPosM );
          } else {
            m_Rich1PmtModuleTransforms[curModuleNum] = dd4hep::Transform3D( botPosM );
          }
        }
      }
    }
  }

  CreateRich1PmtECRTransforms();
  CreateRich1PmtMasterTransforms();
}
//==========================================================================//
void RichPmtGeoAux::CreateRich1PmtECRTransforms() {

  const auto a_RhStdECRXLocationInModuleBegin = detail::dd4hep_param<double>( "RhStdECRXLocationInModuleBegin" );
  const auto a_RhStdECRXDelta                 = detail::dd4hep_param<double>( "RhStdECRXDelta" );
  const auto a_RhStdECRYLocationInModule      = detail::dd4hep_param<double>( "RhStdECRYLocationInModule" );
  const auto a_RhStdECRZLocationInModule      = detail::dd4hep_param<double>( "RhStdECRZLocationInModule" );
  const auto a_RhStdECRRotZ                   = detail::dd4hep_param<double>( "RhStdECRRotZ" );

  for ( int ic = 0; ic < m_RhMaxNumECRInModule; ic++ ) {
    const auto                aXPos = a_RhStdECRXLocationInModuleBegin + ( ic * a_RhStdECRXDelta );
    const dd4hep::Position    aEcrPos( aXPos, a_RhStdECRYLocationInModule, a_RhStdECRZLocationInModule );
    const dd4hep::RotationZYX aEcrRot( a_RhStdECRRotZ, 0, 0 );
    m_Rich1PmtECRTransforms[ic] = dd4hep::Transform3D( aEcrRot, aEcrPos );
  }
}
//==========================================================================//
void RichPmtGeoAux::CreateRich1PmtMasterTransforms() {
  double     a_RhPMTRotY                 = detail::dd4hep_param<double>( "RhPMTRotY" );
  const auto a_RhPMTMasterXLocationInECR = std::array{detail::dd4hep_param<double>( "RhPMTMasterXLocationInECRP0" ),
                                                      detail::dd4hep_param<double>( "RhPMTMasterXLocationInECRP1" ),
                                                      detail::dd4hep_param<double>( "RhPMTMasterXLocationInECRP2" ),
                                                      detail::dd4hep_param<double>( "RhPMTMasterXLocationInECRP3" )};
  const auto a_RhPMTMasterYLocationInECR = std::array{detail::dd4hep_param<double>( "RhPMTMasterYLocationInECRP0" ),
                                                      detail::dd4hep_param<double>( "RhPMTMasterYLocationInECRP1" ),
                                                      detail::dd4hep_param<double>( "RhPMTMasterYLocationInECRP2" ),
                                                      detail::dd4hep_param<double>( "RhPMTMasterYLocationInECRP3" )};
  const auto a_RhPMTMasterZLocationInECR = detail::dd4hep_param<double>( "RhPMTMasterZLocationInECR" );
  const auto a_RhPMTRotationZInECR       = std::array{
      detail::dd4hep_param<double>( "RhPMTRotationZInECRP0" ), detail::dd4hep_param<double>( "RhPMTRotationZInECRP1" ),
      detail::dd4hep_param<double>( "RhPMTRotationZInECRP2" ), detail::dd4hep_param<double>( "RhPMTRotationZInECRP3" )};

  for ( int ip = 0; ip < m_RhNumPMTInECR; ip++ ) {

    const dd4hep::Position    aPmtPosA( 0, 0, 0 );
    const dd4hep::RotationZYX aPmtRotA( 0, a_RhPMTRotY, 0 );

    const dd4hep::Position aPmtPosB( a_RhPMTMasterXLocationInECR[ip], a_RhPMTMasterYLocationInECR[ip],
                                     a_RhPMTMasterZLocationInECR );

    const dd4hep::RotationZYX aPmtRotB( a_RhPMTRotationZInECR[ip], 0, 0 );

    m_Rich1PmtMasterTransforms[ip] =
        ( dd4hep::Transform3D( aPmtRotB, aPmtPosB ) ) * ( dd4hep::Transform3D( aPmtRotA, aPmtPosA ) );
  }
}
//=====================================================================================//

std::array<int, 2> RichPmtGeoAux::get_Rich1EcrRange( int iModule ) const {
  int       ibeg    = 0;
  int       iend    = RhMaxNumECRInModule();
  const int aMIndex = Rich1PmtModuleTypeIndex( iModule );

  // Hack to test if we have the old or new PMT arrangement, as per
  // https://gitlab.cern.ch/lhcb/Detector/-/merge_requests/205
  // maybe better to have some dedicated parameter in the DB for this, or ... ?
  // can anyway be removed once need to support old DetDesc MC is gone.
  const bool isNewArrangement = detail::dd4hep_param<int>( "Rh1NumECRInOuterCornerModuleRow" ) == 0;

  if ( aMIndex == 1 ) {
    ibeg = 1;
  } else if ( aMIndex == 2 ) {
    iend = 3;
  } else if ( aMIndex == 3 ) {
    if ( isNewArrangement ) {
      iend = 0;
    } else {
      ibeg = 2;
    }
  } else if ( aMIndex == 4 ) {
    if ( isNewArrangement ) {
      iend = 0;
    } else {
      iend = 2;
    }
  }

  return {ibeg, iend};
}

//==========================================================================//
void RichPmtGeoAux::loadRich1PmtArraySpecParam() {

  detail::init_param( m_Rh1NumPMTModulesInRow, "Rh1NumPMTModulesInRow" );
  detail::init_param( m_Rh1NumPMTModulesInColumn, "Rh1NumPMTModulesInColumn" );
  detail::init_param( m_Rh1TotalNumPmtModulesInPanel, "Rh1TotalNumPMTModulesInPanel" );
  detail::init_param( m_Rh1NumPanels, "Rh1NumPanels" );
  m_Rh1TotalNumPmtModulesInRich1 = m_Rh1TotalNumPmtModulesInPanel * m_Rh1NumPanels;

  detail::init_param( m_RhMaxNumECRInModule, "Rh1NumECRInStdModule" );

  detail::init_param( m_RhNumPMTInECRRow, "RhNumPMTInECRRow" );
  detail::init_param( m_RhNumPMTInECRCol, "RhNumPMTInECRCol" );
  detail::init_param( m_RhNumPMTInECR, "RhNumPMTInECR" );
  detail::init_param( m_RhNumPmtInStdModule, "Rh1NumPMTInStdModule" );

  // The following factor is 10000 which is accessed from xml.
  detail::init_param( m_RhPmtAnodeDetIdShiftFactor, "pvRichPMTAnodeDetIdShiftFactor" );
  // The following factor is 20000 which is accessed from xml.
  detail::init_param( m_RhPmtPhCathodeDetIdShiftFactor, "pvRichPMTPhCathodeDetIdShiftFactor" );
}
//========================================================================//
void RichPmtGeoAux::loadRich1PmtArrayExtraUserParam() {
  // These are not needed for creating geometry. But useful for other purposes.
  detail::init_param( m_Rich1MaxNumberOfPMTInPanel, "Rh1MaxNumberOfPMTInPanel" );
  detail::init_param( m_Rich1MaxNumberOfPMT, "Rh1MaxNumberOfPMT" );

  detail::init_param( m_Rh1NumPmtStdModulesInPanel, "Rh1TotalNumPMTStdModulesInPanel" );
  detail::init_param( m_Rh1NumPmtEdgeNonOuterCornerModulesInPanel, "Rh1NumPMTEdgeNonOuterCornerModulesInPanel" );
  detail::init_param( m_Rh1NumPmtOuterCornerModulesInPanel, "Rh1NumPMTOuterCornerModulesInPanel" );
}
//==========================================================================//
void RichPmtGeoAux::loadRich2PmtArraySpecParam() {

  detail::init_param( m_Rh2NumPmtModulesInRow, "Rh2MixedPMTNumModulesInRow" );
  detail::init_param( m_Rh2NumPmtModulesInColumn, "Rh2MixedPMTNumModulesInCol" );
  detail::init_param( m_Rh2DefLiveNumPmtModulesInRow, "Rh2MixedDefLivePMTNumModulesInRow" );
  detail::init_param( m_Rh2PmtModuleDefLiveStartCol, "Rh2MixedDefLivePMTModuleStartColNum" );
  detail::init_param( m_Rh2PmtModuleDefLiveEndCol, "Rh2MixedDefLivePMTModuleEndColNum" );

  m_Rh2TotalNumPmtModulesInPanel        = m_Rh2NumPmtModulesInRow * m_Rh2NumPmtModulesInColumn;
  m_Rh2TotalNumPmtDefLiveModulesInPanel = m_Rh2DefLiveNumPmtModulesInRow * m_Rh2NumPmtModulesInColumn;

  detail::init_param( m_Rh2NumPanels, "Rh2MixedPMTNumPanels" );
  m_Rh2TotalNumPmtModulesInRich2        = m_Rh2NumPanels * m_Rh2TotalNumPmtModulesInPanel;
  m_Rh2TotalNumPmtDefLiveModulesInRich2 = m_Rh2NumPanels * m_Rh2TotalNumPmtDefLiveModulesInPanel;

  m_RhTotalNumPmtModulesInRich        = m_Rh1TotalNumPmtModulesInRich1 + m_Rh2TotalNumPmtModulesInRich2;
  m_RhTotalNumPmtDefLiveModulesInRich = m_Rh1TotalNumPmtModulesInRich1 + m_Rh2TotalNumPmtDefLiveModulesInRich2;

  detail::init_param( m_RhMaxNumECHInGrandModule, "Rh2NumECHInGrandModule" );
  detail::init_param( m_RhNumGrandPmtInECH, "RhNumGrandPMTInECH" );

  detail::init_param( m_RhNumGrandPmtInGrandModule, "RhNumGrandPMTInModule" );

  m_Rh2StdModuleRowIndex.clear();
  m_Rh2GrandModuleRowIndex.clear();
  m_Rh2GrandModuleTopRowIndex.clear();
  m_Rh2GrandModuleBotRowIndex.clear();
  m_Rh2StdModuleRowIndex          = {2, 3};
  m_Rh2GrandModuleRowIndex        = {0, 1, 4, 5};
  m_Rh2GrandModuleTopRowIndex     = {0, 1};
  m_Rh2GrandModuleBotRowIndex     = {4, 5};
  m_Rh2ModuleDefLiveColRangeIndex = {m_Rh2PmtModuleDefLiveStartCol, m_Rh2PmtModuleDefLiveEndCol};
}
//==========================================================================//
void RichPmtGeoAux::initRichPmtGeneralStructureLabels() {

  m_RichPmtMaxNumGeomTransforms = 30;

  m_RichPmtCompTransformNames.clear();
  m_RichPmtCompPhysVolNames.clear();
  m_RichPmtCompPhysVolCopyNumbers.clear();

  m_RichPmtCompTransformNames.assign( m_RichPmtMaxNumGeomTransforms, "DummyRichPmtTransform" );
  m_RichPmtCompPhysVolNames.assign( m_RichPmtMaxNumGeomTransforms, "DummyRichPmtPhysVolName" );
  m_RichPmtCompPhysVolCopyNumbers.assign( m_RichPmtMaxNumGeomTransforms, 0 );

  m_RichMaPmtSubMasterLVName    = "lvRichPMTSMaster";
  m_RichMaPmtStdAnodeLVName     = "lvRichPMTAnode0000";
  m_RichMaPmtQuartzLVName       = "lvRichPMTQuartz";
  m_RichMaPmtPhCathodeLVName    = "lvRichPMTPhCathode";
  m_RichMaPmtSideEnvelopeLVName = "lvRichPMTSideEnvelope";
  m_RichMaPmtBackEnvelopeLVName = "lvRichPMTBackEnvelope";
  m_RichMaPmtFrontRingLVName    = "lvRichPMTFrontRing";

  m_Rich2MaPmtSubMasterLVName    = "lvRich2PMTSMaster";
  m_Rich2MaPmtStdAnodeLVName     = "lvRich2PMTAnode0000";
  m_Rich2MaPmtQuartzLVName       = "lvRich2PMTQuartz";
  m_Rich2MaPmtPhCathodeLVName    = "lvRich2PMTPhCathode";
  m_Rich2MaPmtSideEnvelopeLVName = "lvRich2PMTSideEnvelope";
  m_Rich2MaPmtBackEnvelopeLVName = "lvRich2PMTBackEnvelope";
  m_Rich2MaPmtFrontRingLVName    = "lvRich2PMTFrontRing";

  m_RichGrandMaPmtSubMasterLVName    = "lvRichGrandPMTSMaster";
  m_RichGrandMaPmtAnodeLVName        = "lvRichPMTAnode0001";
  m_RichGrandMaPmtQuartzLVName       = "lvRichGrandPMTQuartz";
  m_RichGrandMaPmtPhCathodeLVName    = "lvRichGrandPMTPhCathode";
  m_RichGrandMaPmtSideEnvelopeLVName = "lvRichGrandPMTSideEnvelope";
  m_RichGrandMaPmtBackEnvelopeLVName = "lvRichGrandPMTBackEnvelope";
  m_RichGrandMaPmtFrontRingLVName    = "lvRichGrandPMTFrontRing";

  m_RichMaPmtAnodeDetName      = "MAPMTAnode";
  m_RichGrandMaPmtAnodeDetName = "GrandMAPMTAnode";

  m_RichMaPmtPhCathodeDetName      = "MAPMTPhCathode";
  m_RichGrandMaPmtPhCathodeDetName = "GrandMAPMTPhCathode";

  // Parameters used in Simulation

  m_RichPmtQuartzMatName = "Rich:RichPMTQuartzMaterial";

  m_RichPmtPhCathMatName = "Rich:RichPMTPhCathodeMaterial";
  m_RichPmtQWMatName     = "Rich:RichPMTQuartzMaterial";
  m_RichPmtVacMatName    = "Rich:RichPMTVacuum";

  m_RichNumStdPmtCompUploaded   = 0;
  m_RichNumGrandPmtCompUploaded = 0;

  initRichPmtComponentLabels();

  // m_NumRichPmtCompForSurf = 4;
  m_NumRichPmtCompForSurf = 5;
}

//==========================================================================//
void RichPmtGeoAux::initRichPmtComponentLabels() {

  const Rich::Detector::NamedVector<int> RichPmtPvI{
      {"pvRichPMTSMaster", toInTg( RichPmtGeomTransformLabel::pvRichPMTSMasterTN )},
      {"pvRichPMTAnode0000", toInTg( RichPmtGeomTransformLabel::pvRichPMTAnode0000TN )},
      {"pvRichPMTQuartz", toInTg( RichPmtGeomTransformLabel::pvRichPMTQuartzTN )},
      {"pvRichPMTPhCathode", toInTg( RichPmtGeomTransformLabel::pvRichPMTPhCathodeTN )},
      {"pvRichPMTSideEnvelope", toInTg( RichPmtGeomTransformLabel::pvRichPMTSideEnvelopeTN )},
      {"pvRichPMTBackEnvelope", toInTg( RichPmtGeomTransformLabel::pvRichPMTBackEnvelopeTN )},
      {"pvRichPMTFrontRing", toInTg( RichPmtGeomTransformLabel::pvRichPMTFrontRingTN )},
      {"pvRichPmtSingleShieldingLongPlate", toInTg( RichPmtGeomTransformLabel::pvRichPmtSingleShieldingLongPlateTN )},
      {"pvRichPmtSingleShieldingSemiPlateTop",
       toInTg( RichPmtGeomTransformLabel::pvRichPmtSingleShieldingSemiPlateTopTN )},
      {"pvRichPmtSingleShieldingSemiPlateBottom",
       toInTg( RichPmtGeomTransformLabel::pvRichPmtSingleShieldingSemiPlateBottomTN )},
      {"pvRichGrandPMTSMaster", toInTg( RichPmtGeomTransformLabel::pvRichGrandPMTSMasterTN )},
      {"pvRichPMTAnode0001", toInTg( RichPmtGeomTransformLabel::pvRichPMTAnode0001TN )},
      {"pvRichGrandPMTQuartz", toInTg( RichPmtGeomTransformLabel::pvRichGrandPMTQuartzTN )},
      {"pvRichGrandPMTPhCathode", toInTg( RichPmtGeomTransformLabel::pvRichGrandPMTPhCathodeTN )},
      {"pvRichGrandPMTSideEnvelope", toInTg( RichPmtGeomTransformLabel::pvRichGrandPMTSideEnvelopeTN )},
      {"pvRichGrandPMTBackEnvelope", toInTg( RichPmtGeomTransformLabel::pvRichGrandPMTBackEnvelopeTN )},
      {"pvRichGrandPMTFrontRing", toInTg( RichPmtGeomTransformLabel::pvRichGrandPMTFrontRingTN )}};
  InitRichPmtFillTN( RichPmtPvI );
}
//==========================================================================//
void RichPmtGeoAux::InitRichPmtFillTN( const Rich::Detector::NamedVector<int>& aRichPmtCompPvI ) {
  const std::string TNSuf = "Transform";
  const std::string CNSuf = "CopyNumber";
  for ( const auto& G : aRichPmtCompPvI ) {
    const auto& gIndex                  = G.second;
    const auto& gLabel                  = G.first;
    m_RichPmtCompTransformNames[gIndex] = gLabel + TNSuf;
    m_RichPmtCompPhysVolNames[gIndex]   = gLabel;
    detail::init_param( m_RichPmtCompPhysVolCopyNumbers[gIndex], gLabel + CNSuf );
  }
}

//==========================================================================//
void RichPmtGeoAux::initRich1PmtStructureLabels() {

  m_Rich1PmtModuleType.clear();
  m_Rich1PmtModuleType     = {"Rich1PmtStdModule", "Rich1PmtNoEC0TypeModule", "Rich1PmtNoEC3TypeModule",
                          "Rich1PmtNoEC01TypeModule", "Rich1PmtNoEC23TypeModule"};
  m_NumRich1PmtModuleTypes = (int)m_Rich1PmtModuleType.size();

  m_Rich1PmtModuleCommonDetNamePref = "MAPMT_MODULE";
  m_Rich1PmtECRCommonDetNamePref    = "MAPMT_";
  m_Rich1PmtMasterCommonDetNamePref = "MAPMT";

  m_Rich1PmtModuleLogVolNames.clear();
  m_Rich1PmtModuleLogVolNames.assign( m_Rh1TotalNumPmtModulesInRich1, "Rich1PmtModuleDummyLogName" );

  m_Rich1PmtModulePhysVolNames.clear();
  m_Rich1PmtModulePhysVolNames.assign( m_Rh1TotalNumPmtModulesInRich1, "Rich1PmtModuleDummyPhysName" );

  m_Rich1PmtModuleShapeNames.clear();
  m_Rich1PmtModuleShapeNames.assign( m_Rh1TotalNumPmtModulesInRich1, "Rich1PmtModuleDummyShapeName" );

  m_Rich1PmtModuleDetNames.clear();
  m_Rich1PmtModuleDetNames.assign( m_Rh1TotalNumPmtModulesInRich1, "Rich1PmtModuleDummyDetName" );

  m_Rich1PmtModuleTransforms.clear();
  dd4hep::Transform3D aDummyTransform;
  m_Rich1PmtModuleTransforms.assign( m_Rh1TotalNumPmtModulesInRich1, aDummyTransform );

  m_Rich1PmtModuleTypeIndex.clear();
  m_Rich1PmtModuleTypeIndex.assign( m_Rh1TotalNumPmtModulesInRich1, 0 );

  m_RichECRType.clear();
  m_RichECRType = {"ECR0", "ECR1", "ECR2", "ECR3"};

  m_Rich1PmtECRLogVolNames.clear();
  m_Rich1PmtECRLogVolNames.resize( m_Rh1TotalNumPmtModulesInRich1,
                                   std::vector<std::string>( m_RhMaxNumECRInModule, "Rich1PmtECRDummyLogName" ) );

  m_Rich1PmtECRPhysVolNames.clear();
  m_Rich1PmtECRPhysVolNames.resize( m_Rh1TotalNumPmtModulesInRich1,
                                    std::vector<std::string>( m_RhMaxNumECRInModule, "Rich1PmtECRDummyPhysName" ) );

  m_Rich1PmtECRShapeNames.clear();
  m_Rich1PmtECRShapeNames.resize( m_Rh1TotalNumPmtModulesInRich1,
                                  std::vector<std::string>( m_RhMaxNumECRInModule, "Rich1PmtECRDummyShapeName" ) );

  m_Rich1PmtECRDetNames.clear();
  m_Rich1PmtECRDetNames.resize( m_Rh1TotalNumPmtModulesInRich1,
                                std::vector<std::string>( m_RhMaxNumECRInModule, "Rich1PmtECRDummyDetName" ) );

  m_Rich1PmtECRTransforms.clear();
  m_Rich1PmtECRTransforms.assign( m_RhMaxNumECRInModule, aDummyTransform );

  m_Rich1PmtECRValid.clear();
  m_Rich1PmtECRValid.resize( m_Rh1TotalNumPmtModulesInRich1, std::vector<bool>( m_RhMaxNumECRInModule, false ) );

  m_RichPmtMasterType.clear();
  m_RichPmtMasterType = {"P0", "P1", "P2", "P3"};

  m_Rich1PmtMasterLogVolNames.clear();
  m_Rich1PmtMasterLogVolNames.resize( m_Rh1TotalNumPmtModulesInRich1,
                                      std::vector<std::string>( m_RhNumPmtInStdModule, "Rich1PmtMasterDummyLogName" ) );
  m_Rich1PmtMasterPhysVolNames.clear();
  m_Rich1PmtMasterPhysVolNames.resize(
      m_Rh1TotalNumPmtModulesInRich1,
      std::vector<std::string>( m_RhNumPmtInStdModule, "Rich1PmtMasterDummyPhysName" ) );
  m_Rich1PmtMasterShapeNames.clear();
  m_Rich1PmtMasterShapeNames.resize(
      m_Rh1TotalNumPmtModulesInRich1,
      std::vector<std::string>( m_RhNumPmtInStdModule, "Rich1PmtMasterDummyShapeName" ) );

  m_Rich1PmtMasterDetNames.clear();
  m_Rich1PmtMasterDetNames.resize( m_Rh1TotalNumPmtModulesInRich1,
                                   std::vector<std::string>( m_RhNumPmtInStdModule, "Rich1PmtMasterDummyDetName" ) );

  m_Rich1PmtMasterValid.clear();
  m_Rich1PmtMasterValid.resize( m_Rh1TotalNumPmtModulesInRich1, std::vector<bool>( m_RhNumPmtInStdModule, false ) );

  m_Rich1PmtMasterTransforms.clear();
  m_Rich1PmtMasterTransforms.assign( m_RhNumPMTInECR, aDummyTransform );
}
//==========================================================================//
void RichPmtGeoAux::initRich2PmtStructureLabels() {
  m_Rich2PmtModuleType.clear();
  m_Rich2PmtModuleType = {"Rich2PmtStdModule", "Rich2PmtGrandModule"};

  m_Rich2PmtModuleCommonDetNamePref = "MAPMT_MODULE";
  m_Rich2PmtECRHCommonDetNamePref   = "MAPMT_";
  m_Rich2PmtMasterCommonDetNamePref = "MAPMT";

  m_Rich2PmtModuleLogVolNames.clear();
  m_Rich2PmtModuleLogVolNames.assign( m_Rh2TotalNumPmtModulesInRich2, "Rich2PmtModuleDummyLogName" );

  m_Rich2PmtModulePhysVolNames.clear();
  m_Rich2PmtModulePhysVolNames.assign( m_Rh2TotalNumPmtModulesInRich2, "Rich2PmtModuleDummyPhysName" );

  m_Rich2PmtModuleShapeNames.clear();
  m_Rich2PmtModuleShapeNames.assign( m_Rh2TotalNumPmtModulesInRich2, "Rich2PmtModuleDummyShapeName" );

  m_Rich2PmtModuleDetNames.clear();
  m_Rich2PmtModuleDetNames.assign( m_Rh2TotalNumPmtModulesInRich2, "Rich2PmtModuleDummyDetName" );

  m_Rich2PmtModuleCopyNumbers.clear();
  m_Rich2PmtModuleCopyNumbers.assign( m_Rh2TotalNumPmtModulesInRich2, 0 );

  m_IsRich2GrandPmtModule.clear();
  m_IsRich2GrandPmtModule.assign( m_Rh2TotalNumPmtModulesInRich2, true );

  m_Rich2PmtECRHLogVolNames.clear();
  m_Rich2PmtECRHLogVolNames.resize( m_Rh2TotalNumPmtModulesInRich2,
                                    std::vector<std::string>( m_RhMaxNumECRInModule, "Rich2PmtECRHDummyLogName" ) );

  m_Rich2PmtECRHPhysVolNames.clear();
  m_Rich2PmtECRHPhysVolNames.resize( m_Rh2TotalNumPmtModulesInRich2,
                                     std::vector<std::string>( m_RhMaxNumECRInModule, "Rich2PmtECRHDummyPhysName" ) );

  m_Rich2PmtECRHShapeNames.clear();
  m_Rich2PmtECRHShapeNames.resize( m_Rh2TotalNumPmtModulesInRich2,
                                   std::vector<std::string>( m_RhMaxNumECRInModule, "Rich2PmtECRHDummyShapeName" ) );

  m_Rich2PmtECRHDetNames.clear();
  m_Rich2PmtECRHDetNames.resize( m_Rh2TotalNumPmtModulesInRich2,
                                 std::vector<std::string>( m_RhMaxNumECRInModule, "Rich2PmtECRHDummyDetName" ) );

  m_Rich2PmtMasterLogVolNames.clear();
  m_Rich2PmtMasterLogVolNames.resize( m_Rh2TotalNumPmtModulesInRich2,
                                      std::vector<std::string>( m_RhNumPmtInStdModule, "Rich2PmtDummyLogName" ) );
  m_Rich2PmtMasterPhysVolNames.clear();
  m_Rich2PmtMasterPhysVolNames.resize( m_Rh2TotalNumPmtModulesInRich2,
                                       std::vector<std::string>( m_RhNumPmtInStdModule, "Rich2PmtDummyPhysName" ) );

  m_Rich2PmtMasterShapeNames.clear();
  m_Rich2PmtMasterShapeNames.resize( m_Rh2TotalNumPmtModulesInRich2,
                                     std::vector<std::string>( m_RhNumPmtInStdModule, "Rich2PmtDummyShapeName" ) );
  m_Rich2PmtMasterDetNames.clear();
  m_Rich2PmtMasterDetNames.resize( m_Rh2TotalNumPmtModulesInRich2,
                                   std::vector<std::string>( m_RhNumPmtInStdModule, "Rich2PmtDummyDetName" ) );

  m_Rich2PmtModuleTransforms.clear();
  dd4hep::Transform3D aDummyTransform;
  m_Rich2PmtModuleTransforms.assign( m_Rh2TotalNumPmtModulesInRich2, aDummyTransform );

  m_Rich2PmtECRTransforms.clear();
  m_Rich2PmtECRTransforms.assign( m_RhMaxNumECRInModule, aDummyTransform );

  m_Rich2PmtECHTransforms.clear();
  m_Rich2PmtECHTransforms.assign( m_RhMaxNumECHInGrandModule, aDummyTransform );

  m_Rich2PmtMasterTransforms.clear();
  m_Rich2PmtMasterTransforms.assign( m_RhNumPMTInECR, aDummyTransform );

  m_Rich2GrandPmtMasterTransform = aDummyTransform;

  m_RichECHType.clear();
  m_RichECHType = {"ECH0", "ECH1", "ECH2", "ECH3"};

  m_IsRich2ActiveModule.clear();
  m_IsRich2ActiveModule.assign( m_Rh2TotalNumPmtModulesInRich2, false );
  m_Rich2DefLiveModuleCopyNumRange.clear();
  int              aBegCp          = m_Rh1TotalNumPmtModulesInRich1 + m_Rh2NumPmtModulesInColumn;
  std::vector<int> aLM             = {aBegCp, aBegCp + m_Rh2TotalNumPmtDefLiveModulesInPanel - 1};
  std::vector<int> aRM             = {m_Rh2TotalNumPmtModulesInPanel + aLM[0], m_Rh2TotalNumPmtModulesInPanel + aLM[1]};
  m_Rich2DefLiveModuleCopyNumRange = {aLM, aRM};

  m_Rich2PmtECRHValid.clear();
  m_Rich2PmtECRHValid.resize( m_Rh2TotalNumPmtModulesInRich2, std::vector<bool>( m_RhMaxNumECRInModule, false ) );
  m_Rich2PmtMasterValid.clear();
  m_Rich2PmtMasterValid.resize( m_Rh2TotalNumPmtModulesInRich2, std::vector<bool>( m_RhNumPmtInStdModule, false ) );
}
//==========================================================================//
dd4hep::Transform3D RichPmtGeoAux::getRichPmtCompGeomTransform( const std::string& aName ) const {
  const auto trans = m_RichPmtCompGeomTransforms.find( aName );
  if ( trans != m_RichPmtCompGeomTransforms.end() ) {
    return trans->second;
  } else {
    printout( m_RichPmtGeoAuxDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "RichPmtGeoAux",
              "Rich Pmt component Transform not found for %s ", aName.c_str() );
    return dd4hep::Transform3D{};
  }
}

//==========================================================================//
std::string RichPmtGeoAux::getRich1PhDetSupDetNameFromModuleNum( int iM ) const {
  auto aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();
  return ( iM < Rh1TotalNumPmtModulesInPanel() ) ? aRichGeoUtil->Rich1PhDetSupFrameDetName( 0 )
                                                 : aRichGeoUtil->Rich1PhDetSupFrameDetName( 1 );
}

//==========================================================================//
std::string RichPmtGeoAux::getRich2PhDetPanelDetNameFromModuleNum( int iM ) const {
  auto aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
  return ( iM < Rh2TotalNumPmtModulesInPanel() ) ? aRich2GeoUtil->Rich2PhDetPanelDetName( 0 )
                                                 : aRich2GeoUtil->Rich2PhDetPanelDetName( 1 );
}

//==========================================================================//
void RichPmtGeoAux::initRich2PmtModuleTypes() {
  const std::string lvPref = "lv";
  const std::string pvPref = "pv";

  const std::string aStdModuleShapeLabel   = "Rich2StdPmtModuleBox";
  const std::string aGrandModuleShapeLabel = "Rich2GrandPmtModuleBox";
  const std::string aStdModuleLabel        = "Rich2PmtStdModule";
  const std::string aGrandModuleLabel      = "Rich2PmtGrandModule";

  for ( int ip = 0; ip < m_Rh2NumPanels; ip++ ) { // loop over the two panels

    for ( int icol = 0; icol < m_Rh2NumPmtModulesInRow; icol++ ) {

      for ( int irow = 0; irow < m_Rh2NumPmtModulesInColumn; irow++ ) {

        const auto curR2ModuleNum    = irow + icol * m_Rh2NumPmtModulesInColumn + ip * m_Rh2TotalNumPmtModulesInPanel;
        const auto curModuleNum      = curR2ModuleNum + m_Rh1TotalNumPmtModulesInRich1;
        const auto aModuleNumSuf     = std::to_string( curModuleNum );
        auto       aModuleShapeLabel = aGrandModuleShapeLabel;
        auto       aModuleLabel      = aGrandModuleLabel;
        if ( IsRich2StdPmtModuleRow( irow ) ) {
          aModuleShapeLabel                       = aStdModuleShapeLabel;
          aModuleLabel                            = aStdModuleLabel;
          m_IsRich2GrandPmtModule[curR2ModuleNum] = false;
        }
        m_Rich2PmtModuleLogVolNames[curR2ModuleNum]  = lvPref + aModuleLabel + aModuleNumSuf;
        m_Rich2PmtModulePhysVolNames[curR2ModuleNum] = pvPref + aModuleLabel + aModuleNumSuf;
        m_Rich2PmtModuleShapeNames[curR2ModuleNum]   = aModuleShapeLabel + aModuleNumSuf;
        m_Rich2PmtModuleCopyNumbers[curR2ModuleNum]  = curModuleNum;
        m_Rich2PmtModuleDetNames[curR2ModuleNum]     = m_Rich2PmtModuleCommonDetNamePref + aModuleNumSuf;
        // Here using the new numbering scheme from Feb2021. The EC and PMT structures are created only for the
        // current active modules
        if ( ( icol >= m_Rh2PmtModuleDefLiveStartCol ) && ( icol <= m_Rh2PmtModuleDefLiveEndCol ) ) {
          m_IsRich2ActiveModule[curR2ModuleNum] = true;
          initRich2PmtECTypes( curR2ModuleNum, irow, aModuleNumSuf );
          initRich2PmtMasterTypes( curR2ModuleNum, irow, aModuleNumSuf );
        }
      }
    }
  }
}
//==========================================================================//
void RichPmtGeoAux::initRich2PmtECTypes( int aR2ModuleNum, int irow, const std::string& aModuleLabel ) {

  const auto        isRow           = IsRich2StdPmtModuleRow( irow );
  const std::string aEStringA       = ( !isRow ? "RichPmtGrand" : "RichPmtStd" );
  const std::string aEStringB       = "InModule";
  const std::string aEStringC       = ( !isRow ? "Rich2PmtGrand" : "Rich2PmtStd" );
  const std::string aECRShapeLabelB = "Box";

  for ( int ic = 0; ic < m_RhMaxNumECRInModule; ic++ ) {

    const auto& aECType = ( IsRich2StdPmtModuleRow( irow ) ) ? m_RichECRType[ic] : m_RichECHType[ic];

    m_Rich2PmtECRHLogVolNames[aR2ModuleNum][ic]  = "lv" + aEStringA + aECType + aEStringB + aModuleLabel;
    m_Rich2PmtECRHPhysVolNames[aR2ModuleNum][ic] = "pv" + aEStringA + aECType + aEStringB + aModuleLabel;
    m_Rich2PmtECRHShapeNames[aR2ModuleNum][ic]   = aEStringC + aECType + aECRShapeLabelB + aEStringB + aModuleLabel;
    m_Rich2PmtECRHDetNames[aR2ModuleNum][ic]     = m_Rich2PmtECRHCommonDetNamePref + aECType + aEStringB + aModuleLabel;

    if ( m_RichPmtGeoAuxDebug ) {
      printout( dd4hep::DEBUG, "RichPmtGeoAux",
                "Rich2 Module ECR : Creating ECR Log Phys Shape and Det names %i %i %i %s %s %s %s", //
                aR2ModuleNum, aModuleLabel.c_str(), ic, m_Rich2PmtECRHLogVolNames[aR2ModuleNum][ic].c_str(),
                m_Rich2PmtECRHPhysVolNames[aR2ModuleNum][ic].c_str(),
                m_Rich2PmtECRHShapeNames[aR2ModuleNum][ic].c_str(), m_Rich2PmtECRHDetNames[aR2ModuleNum][ic].c_str() );
    }
  }
}
//==========================================================================//
void RichPmtGeoAux::initRich2PmtMasterTypes( int aR2ModuleNum, int irow, const std::string& aModuleLabel ) {

  const auto        IsStdP            = IsRich2StdPmtModuleRow( irow );
  const std::string aPStringA         = ( IsStdP ? "RichPmtMaster" : "RichGrandPmtMaster" );
  const std::string aPStringB         = "In";
  const std::string aPStringC         = ( IsStdP ? "InStdModule" : "InGrandModule" );
  int               aNumPmtMasterInEc = m_RhNumGrandPmtInECH;
  if ( IsStdP ) {
    aNumPmtMasterInEc = m_RhNumPMTInECR;
  } else {
    // save some space
    m_Rich2PmtMasterLogVolNames[aR2ModuleNum].resize( RhNumGrandPmtInGrandModule() );
    m_Rich2PmtMasterPhysVolNames[aR2ModuleNum].resize( RhNumGrandPmtInGrandModule() );
    m_Rich2PmtMasterShapeNames[aR2ModuleNum].resize( RhNumGrandPmtInGrandModule() );
    m_Rich2PmtMasterDetNames[aR2ModuleNum].resize( RhNumGrandPmtInGrandModule() );
  }

  const std::string aPmtShapeLabel = "Box";
  for ( int ic = 0; ic < m_RhMaxNumECRInModule; ic++ ) {
    const auto& aECType = ( IsStdP ? m_RichECRType[ic] : m_RichECHType[ic] );
    const auto  aS      = aPStringB + aECType + aPStringC + aModuleLabel;

    for ( int ip = 0; ip < aNumPmtMasterInEc; ip++ ) {
      const auto ipc                                  = ic * aNumPmtMasterInEc + ip;
      m_Rich2PmtMasterLogVolNames[aR2ModuleNum][ipc]  = "lv" + aPStringA + m_RichPmtMasterType[ip] + aS;
      m_Rich2PmtMasterPhysVolNames[aR2ModuleNum][ipc] = "pv" + aPStringA + m_RichPmtMasterType[ip] + aS;
      m_Rich2PmtMasterShapeNames[aR2ModuleNum][ipc]   = aPStringA + aPmtShapeLabel + m_RichPmtMasterType[ip] + aS;
      m_Rich2PmtMasterDetNames[aR2ModuleNum][ipc] = m_Rich2PmtMasterCommonDetNamePref + m_RichPmtMasterType[ip] + aS;
    }
  }
}
//==========================================================================//
void RichPmtGeoAux::CreateRich2PmtModuleTransforms() {
  auto aRh2StdPmtModuleXLocationStartLeft = detail::dd4hep_param<double>( "Rh2MixedStdPMTModuleXLocationStartLeft" );
  auto aRh2StdPmtModuleLeftXShift         = detail::dd4hep_param<double>( "Rh2MixedStdPMTModuleLeftXShift" );
  auto aRh2StdPmtModuleYLocationStartLeft = detail::dd4hep_param<double>( "Rh2MixedStdPMTModuleYLocationStartLeft" );
  auto aRh2StdPmtModuleLeftYShift         = detail::dd4hep_param<double>( "Rh2MixedStdPMTModuleLeftYShift" );
  auto aRh2MixedPmtModuleMasterPosZ       = detail::dd4hep_param<double>( "Rh2MixedPMTModuleMasterZ" );
  auto aRh2PmtModuleRotZ                  = detail::dd4hep_param<double>( "Rh2PMTModuleRotZ" );
  auto aRh2PmtModuleRot                   = dd4hep::RotationZYX( aRh2PmtModuleRotZ, 0, 0 );

  auto aRh2GrandPmtModuleXLocationStartLeft =
      detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleXLocationStartLeft" );
  auto aRh2GrandPmtModuleYLocationStartLeftAtTop =
      detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleYLocationStartLeftAtTop" );
  auto aRh2GrandPmtModuleYLocationStartLeftAtBottom =
      detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleYLocationStartLeftAtBottom" );

  auto aRh2GrandPmtModuleLeftXShift = detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleLeftXShift" );
  auto aRh2GrandPmtModuleLeftYShift = detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleLeftYShift" );

  auto aRh2StdPmtModuleXLocationStartRight = detail::dd4hep_param<double>( "Rh2MixedStdPMTModuleXLocationStartRight" );
  auto aRh2StdPmtModuleRightXShift         = detail::dd4hep_param<double>( "Rh2MixedStdPMTModuleRightXShift" );
  auto aRh2StdPmtModuleYLocationStartRight = detail::dd4hep_param<double>( "Rh2MixedStdPMTModuleYLocationStartRight" );
  auto aRh2StdPmtModuleRightYShift         = detail::dd4hep_param<double>( "Rh2MixedStdPMTModuleRightYShift" );
  auto aRh2GrandPmtModuleXLocationStartRight =
      detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleXLocationStartRight" );
  auto aRh2GrandPmtModuleYLocationStartRightAtTop =
      detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleYLocationStartRightAtTop" );
  auto aRh2GrandPmtModuleYLocationStartRightAtBottom =
      detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleYLocationStartRightAtBottom" );
  auto aRh2GrandPmtModuleRightXShift = detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleRightXShift" );
  auto aRh2GrandPmtModuleRightYShift = detail::dd4hep_param<double>( "Rh2MixedGrandPMTModuleRightYShift" );

  for ( int ip = 0; ip < m_Rh2NumPanels; ip++ ) { // loop over the two panels

    for ( int icol = 0; icol < m_Rh2NumPmtModulesInRow; icol++ ) {

      for ( int irow = 0; irow < m_Rh2NumPmtModulesInColumn; irow++ ) {
        int curR2ModuleNum = irow + icol * m_Rh2NumPmtModulesInColumn + ip * m_Rh2TotalNumPmtModulesInPanel;
        if ( ip == 0 ) {                          // left panel
          if ( IsRich2StdPmtModuleRow( irow ) ) { // std modules
            auto aCurStdLPosX = aRh2StdPmtModuleXLocationStartLeft + ( icol * aRh2StdPmtModuleLeftXShift );
            auto aCurStdLPosY = aRh2StdPmtModuleYLocationStartLeft +
                                ( ( irow - Rich2StdPmtModuleRowBeg() ) * aRh2StdPmtModuleLeftYShift );
            auto aCurStdLeftPos = dd4hep::Position( aCurStdLPosX, aCurStdLPosY, aRh2MixedPmtModuleMasterPosZ );
            m_Rich2PmtModuleTransforms[curR2ModuleNum] = dd4hep::Transform3D( aRh2PmtModuleRot, aCurStdLeftPos );

          } else { // grand modules
            auto aCurGrandLPosX = aRh2GrandPmtModuleXLocationStartLeft + ( icol * aRh2GrandPmtModuleLeftXShift );
            auto aCurGrandLPosY = aRh2GrandPmtModuleYLocationStartLeftAtTop;
            if ( IsRich2GrandPmtTopRow( irow ) ) {
              aCurGrandLPosY = aRh2GrandPmtModuleYLocationStartLeftAtTop +
                               ( ( irow - Rich2GrandPmtModuleTopRowBeg() ) * aRh2GrandPmtModuleLeftYShift );
            } else if ( IsRich2GrandPmtBotRow( irow ) ) {
              aCurGrandLPosY = aRh2GrandPmtModuleYLocationStartLeftAtBottom +
                               ( ( irow - Rich2GrandPmtModuleBotRowBeg() ) * aRh2GrandPmtModuleLeftYShift );
            }
            auto aCurGrandLeftPos = dd4hep::Position( aCurGrandLPosX, aCurGrandLPosY, aRh2MixedPmtModuleMasterPosZ );
            m_Rich2PmtModuleTransforms[curR2ModuleNum] = dd4hep::Transform3D( aRh2PmtModuleRot, aCurGrandLeftPos );
          }

        } else if ( ip == 1 ) { // right panel

          if ( IsRich2StdPmtModuleRow( irow ) ) { // std modules
            auto aCurStdRPosX = aRh2StdPmtModuleXLocationStartRight + ( icol * aRh2StdPmtModuleRightXShift );
            auto aCurStdRPosY = aRh2StdPmtModuleYLocationStartRight +
                                ( ( irow - Rich2StdPmtModuleRowBeg() ) * aRh2StdPmtModuleRightYShift );
            auto aCurStdRightPos = dd4hep::Position( aCurStdRPosX, aCurStdRPosY, aRh2MixedPmtModuleMasterPosZ );
            m_Rich2PmtModuleTransforms[curR2ModuleNum] = dd4hep::Transform3D( aRh2PmtModuleRot, aCurStdRightPos );

          } else { // grand modules
            auto aCurGrandRPosX = aRh2GrandPmtModuleXLocationStartRight + ( icol * aRh2GrandPmtModuleRightXShift );
            auto aCurGrandRPosY = aRh2GrandPmtModuleYLocationStartRightAtTop;
            if ( IsRich2GrandPmtTopRow( irow ) ) {
              aCurGrandRPosY = aRh2GrandPmtModuleYLocationStartRightAtTop +
                               ( ( irow - Rich2GrandPmtModuleTopRowBeg() ) * aRh2GrandPmtModuleRightYShift );
            } else if ( IsRich2GrandPmtBotRow( irow ) ) {
              aCurGrandRPosY = aRh2GrandPmtModuleYLocationStartRightAtBottom +
                               ( ( irow - Rich2GrandPmtModuleBotRowBeg() ) * aRh2GrandPmtModuleRightYShift );
            }
            auto aCurGrandRightPos = dd4hep::Position( aCurGrandRPosX, aCurGrandRPosY, aRh2MixedPmtModuleMasterPosZ );
            m_Rich2PmtModuleTransforms[curR2ModuleNum] = dd4hep::Transform3D( aRh2PmtModuleRot, aCurGrandRightPos );
          }
        }
      }
    }
  }

  CreateRich2PmtECRHTransforms();
  CreateRich2PmtMasterTransforms();
}
//======================================================================================================//
void RichPmtGeoAux::CreateRich2PmtECRHTransforms() {

  /*
   These lines may be activated in case the EC in RICH2 are positioned differently than those in Rich1.
  double a_Rh2StdECRXLocationInModuleBegin = detail::dd4hep_param<double>( "RhStdECR2XLocationInR2ModuleBegin" );
  double a_Rh2StdECRXDelta                 = detail::dd4hep_param<double>( "RhStdECR2XDelta" );
  double a_Rh2StdECRYLocationInModule      = detail::dd4hep_param<double>( "RhStdECR2YLocationInR2Module" );
  double a_Rh2StdECRZLocationInModule      = detail::dd4hep_param<double>( "RhStdECR2ZLocationInR2Module" );
  double a_Rh2StdECRRotZ                   = detail::dd4hep_param<double>( "RhStdECR2RotZ" );

  for ( int ic = 0; ic < m_RhMaxNumECRInModule; ic++ ) {

    double aXPos = a_Rh2StdECRXLocationInModuleBegin + ( ic * a_Rh2StdECRXDelta );

    dd4hep::Position    aEcrPos = dd4hep::Position( aXPos, a_Rh2StdECRYLocationInModule, a_Rh2StdECRZLocationInModule );
    dd4hep::RotationZYX aEcrRot = dd4hep::RotationZYX( a_Rh2StdECRRotZ, 0, 0 );
    m_Rich2PmtECRTransforms[ic] = dd4hep::Transform3D( aEcrRot, aEcrPos );
  }
  */

  for ( int ic = 0; ic < m_RhMaxNumECRInModule; ic++ ) { m_Rich2PmtECRTransforms[ic] = m_Rich1PmtECRTransforms[ic]; }

  auto a_Rh2GrandECHXLocationInModuleBegin = detail::dd4hep_param<double>( "RhGrandECHXLocationInGrandR2ModuleBegin" );
  auto a_Rh2GrandECHXDelta                 = detail::dd4hep_param<double>( "RhGrandECHXDelta" );
  auto a_Rh2GrandECHYLocationInModule      = detail::dd4hep_param<double>( "RhGrandECHYLocationInR2Module" );
  auto a_Rh2GrandECHZLocationInModule      = detail::dd4hep_param<double>( "RhGrandECHZLocationInR2Module" );
  auto a_Rh2GrandECHRotZ                   = detail::dd4hep_param<double>( "RhGrandECHRotZ" );
  for ( int ic = 0; ic < m_RhMaxNumECHInGrandModule; ic++ ) {
    auto aXPos   = a_Rh2GrandECHXLocationInModuleBegin + ( ic * a_Rh2GrandECHXDelta );
    auto aEchPos = dd4hep::Position( aXPos, a_Rh2GrandECHYLocationInModule, a_Rh2GrandECHZLocationInModule );
    auto aEchRot = dd4hep::RotationZYX( a_Rh2GrandECHRotZ, 0, 0 );
    m_Rich2PmtECHTransforms[ic] = dd4hep::Transform3D( aEchRot, aEchPos );
  }
}
//==========================================================================//
void RichPmtGeoAux::CreateRich2PmtMasterTransforms() {
  for ( int ip = 0; ip < m_RhNumPMTInECR; ip++ ) { m_Rich2PmtMasterTransforms[ip] = m_Rich1PmtMasterTransforms[ip]; }
  auto aGrandPmtXInECH    = detail::dd4hep_param<double>( "RhGrandPMTMasterXLocationInECH" );
  auto aGrandPmtYInECH    = detail::dd4hep_param<double>( "RhGrandPMTMasterYLocationInECH" );
  auto aGrandPmtZInECH    = detail::dd4hep_param<double>( "RhGrandPMTMasterZLocationInECH" );
  auto aGrandPmtRotZInECH = detail::dd4hep_param<double>( "RhPMTRotationZInECH" );
  auto aGrandPmtRotYInECH = detail::dd4hep_param<double>( "RhPMTRotY" );

  auto aGPmtPosA = dd4hep::Position( 0, 0, 0 );
  auto aGPmtRotA = dd4hep::RotationZYX( 0, aGrandPmtRotYInECH, 0 );
  auto aGPmtPosB = dd4hep::Position( aGrandPmtXInECH, aGrandPmtYInECH, aGrandPmtZInECH );
  auto aGPmtRotB = dd4hep::RotationZYX( aGrandPmtRotZInECH, 0, 0 );

  m_Rich2GrandPmtMasterTransform =
      ( dd4hep::Transform3D( aGPmtRotB, aGPmtPosB ) ) * ( dd4hep::Transform3D( aGPmtRotA, aGPmtPosA ) );
}
//==========================================================================//
void RichPmtGeoAux::initRich1PmtStructures() {
  if ( !m_Rich1PmtStructureCreated ) {
    loadRich1PmtArraySpecParam();
    initRich1PmtStructureLabels();
    initRich1PmtModuleTypes();
    CreateRich1PmtModuleTransforms();
    m_Rich1PmtStructureCreated = true;
    loadRich1PmtArrayExtraUserParam(); // these are not needed for creating geometry,
                                       // but useful for other purposes
  }
}
//==========================================================================//
void RichPmtGeoAux::initRich2PmtStructures() {
  // The Rich1 Pmt structures are to be created before creating Rich2 Pmt structures.
  // In case the Rich1 Pmt structures are not created already, they are created here.
  if ( !m_Rich1PmtStructureCreated ) { initRich1PmtStructures(); }

  if ( !m_Rich2PmtStructureCreated ) {
    loadRich2PmtArraySpecParam();
    initRich2PmtStructureLabels();
    initRich2PmtModuleTypes();
    CreateRich2PmtModuleTransforms();
    m_Rich2PmtStructureCreated = true;
  }
}
//============================================================================//

RichPmtGeoAux::DEList                                                            //
RichPmtGeoAux::Rich1PDModuleListDE( const dd4hep::DetElement&             de,    //
                                    dd4hep::cond::ConditionUpdateContext& ctxt,  //
                                    int                                   aSide, //
                                    int                                   aMType ) const {
  DEList aDep;
  aDep.reserve( m_Rh1TotalNumPmtModulesInPanel );
  for ( int iM = 0; iM < m_Rh1TotalNumPmtModulesInPanel; ++iM ) {
    const auto iMCopyNum = ( aSide == 0 ) ? iM : ( iM + m_Rh1TotalNumPmtModulesInPanel );
    const auto aMIndex   = m_Rich1PmtModuleTypeIndex[iMCopyNum];
    if ( aMType == 0 ) {
      if ( aMIndex == 0 ) { aDep.emplace_back( de.child( Rich1PmtModuleDetName( iMCopyNum ) ), ctxt ); }
    } else if ( aMType == 1 ) {
      if ( ( aMIndex == 1 ) || ( aMIndex == 2 ) ) {
        aDep.emplace_back( de.child( Rich1PmtModuleDetName( iMCopyNum ) ), ctxt );
      }
    } else if ( aMType == 2 ) {
      if ( ( aMIndex == 3 ) || ( aMIndex == 4 ) ) {
        aDep.emplace_back( de.child( Rich1PmtModuleDetName( iMCopyNum ) ), ctxt );
      }
    }
  }
  return aDep;
}
//==============================================================================//

RichPmtGeoAux::DEList                                                                //
RichPmtGeoAux::Rich1MapmtListInModuleDE( const dd4hep::DetElement&             de,   //
                                         dd4hep::cond::ConditionUpdateContext& ctxt, //
                                         int                                   aModuleCopyNum ) const {
  const auto curRange = get_Rich1EcrRange( aModuleCopyNum );
  const auto ibeg     = ( curRange[0] ) * RhNumPMTInECR();
  const auto iend     = ( curRange[1] ) * RhNumPMTInECR();
  DEList     aDep;
  aDep.reserve( iend - ibeg );
  for ( int iP = ibeg; iP < iend; ++iP ) {
    aDep.emplace_back( de.child( Rich1PmtMPDetName( aModuleCopyNum, iP ) ), ctxt );
  }
  return aDep;
}
//====================================================================================================================//

RichPmtGeoAux::DEList                                                            //
RichPmtGeoAux::Rich2PDModuleListDE( const dd4hep::DetElement&             de,    //
                                    dd4hep::cond::ConditionUpdateContext& ctxt,  //
                                    int                                   aSide, //
                                    int                                   aMType ) const {
  DEList aDep;
  aDep.reserve( m_Rh2TotalNumPmtModulesInPanel );
  for ( int iM = 0; iM < m_Rh2TotalNumPmtModulesInPanel; iM++ ) {
    const auto iMR2 = ( aSide == 0 ? iM : ( iM + m_Rh2TotalNumPmtModulesInPanel ) );
    if ( IsRich2ActiveModule( iMR2 ) ) {
      if ( aMType == 0 ) {
        if ( !m_IsRich2GrandPmtModule[iMR2] ) { aDep.emplace_back( de.child( Rich2PmtModuleDetName( iMR2 ) ), ctxt ); }
      } else if ( aMType == 1 ) {
        if ( m_IsRich2GrandPmtModule[iMR2] ) { aDep.emplace_back( de.child( Rich2PmtModuleDetName( iMR2 ) ), ctxt ); }
      }
    }
  }
  return aDep;
}
//======================================================================================================//

RichPmtGeoAux::DEList                                                                          //
RichPmtGeoAux::Rich2MapmtListInModuleDE( const dd4hep::DetElement&             de,             //
                                         dd4hep::cond::ConditionUpdateContext& ctxt,           //
                                         int                                   aModuleCopyNum, //
                                         int                                   aMType ) const {
  const auto iMR2        = Rich2ModuleNumInRich2FromCopyNum( aModuleCopyNum );
  const auto aNumEcInM   = Rh2NumECInCurrentModule( iMR2 );
  const auto aNumPmtInEC = Rh2NumPmtInCurrentEC( iMR2 );
  DEList     aDep;
  aDep.reserve( aNumEcInM * aNumPmtInEC );
  for ( int iec = 0; iec < aNumEcInM; iec++ ) {
    for ( int iP = 0; iP < aNumPmtInEC; iP++ ) {
      if ( aMType == 0 ) {
        if ( !m_IsRich2GrandPmtModule[iMR2] ) {
          aDep.emplace_back( de.child( Rich2PmtMasterDetName( iMR2, iec, iP ) ), ctxt );
        }
      } else if ( aMType == 1 ) {
        if ( m_IsRich2GrandPmtModule[iMR2] ) {
          aDep.emplace_back( de.child( Rich2PmtMasterDetName( iMR2, iec, iP ) ), ctxt );
        }
      }
    }
  }
  return aDep;
}
//=====================================================================================================//

RichPmtGeoAux::RichPmtGeoAux() { initRichPmtGeneralStructureLabels(); }
//==========================================================================//

RichPmtGeoAux* RichPmtGeoAux::getRichPmtGeoAuxInstance() {
  static std::once_flag                 alloc_instance_once;
  static std::unique_ptr<RichPmtGeoAux> instance;
  std::call_once( alloc_instance_once, []() { instance = std::make_unique<RichPmtGeoAux>(); } );
  assert( instance.get() );
  return instance.get();
}
