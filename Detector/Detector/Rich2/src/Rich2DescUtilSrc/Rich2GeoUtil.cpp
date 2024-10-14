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

#include "Detector/Rich2/Rich2GeoUtil.h"
#include <cassert>
#include <map>
#include <memory>
#include <mutex>

using namespace LHCb::Detector;

//==========================================================================//
void Rich2GeoUtil::build_Rich2Transforms( xml_h tr_handle ) {

  printout( m_Rich2GeomUtilDebug ? dd4hep::DEBUG : dd4hep::DEBUG, "Rich2GeoUtil : ", "Now building Rich2Transforms " );
  // std::cout<<"Rich2GeoUtil: Now building Rich2Transforms "<<std::endl;

  for ( xml_coll_t i( tr_handle, _U( item ) ); i; ++i ) {
    // std::cout<<" Now in the loop of transform in Rich2Transforms "<<std::endl;
    // printout(m_Rich2GeomUtilDebug  ? dd4hep::ALWAYS : dd4hep::DEBUG,
    //	    "Rich2GeoUtil: Now in the loop of transform in Rich2Transforms");

    xml_comp_t  c( i );
    std::string itemLabel = c.attr<std::string>( _U( name ) );
    if ( itemLabel.find( "TransformName" ) != std::string::npos ) {
      const auto trName = c.attr<std::string>( _U( value ) );
      // std::cout<<" test transform name " << trName <<std::endl;
      xml_elt_t trn = c.child( _U( transformation ) );
      printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich2GeoUtil: build Rich2Transform for %s",
                trName.c_str() );

      const auto tr = dd4hep::xml::createTransformation( trn );

      // here there is only a cross-check that m_Rich2TransformNames is filled.
      // In the future one may try to store info in the m_Rich2TransformNames once
      // a set of specialized unicode tags are setup for storing string constants.

      // std::vector<std::string>::iterator itTr =
      //    std::find( m_Rich2TransformNames.begin(), m_Rich2TransformNames.end(), trName );
      // index = std::distance(m_Rich2TransformNames.begin(),itTr);

      if ( IsInRich2TransformList( trName ) ) {

        // std::cout<<" Now store the transformation for  "<< trName << std::endl;

        auto retRTrans = m_Rich2GeomTransforms.insert( std::make_pair( trName, tr ) );

        if ( retRTrans.second == false ) {
          printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "Rich2GeoUtil: Error ",
                    " Rich2GeoUtil Transform for %s  not stored since it already existed with a value of %d ",
                    trName.c_str(), retRTrans.first->second );

          // std::cout<<" Rich2GeoUtil Transform for "<< trName
          //   << " not stored since it already existed with a value of  "
          //   <<retRTrans.first->second<<std::endl;
        }

        // end of test on m_Rich2TransformNames
      } else {

        printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "Rich2GeoUtilInfo: Error ",
                  "Transform name not available for %s and hence Transform not stored ", trName.c_str() );
      }
      // end test on transformName in label
    } else if ( itemLabel.find( "PhysVolName" ) != std::string::npos ) {

      const auto physVolName = c.attr<std::string>( _U( value ) );

      // here there is only a cross-check that m_Rich2PhysVolNames is filled.
      // In the future one may try to store info in m_Rich2PhysVolNames once
      // a set of specialized unicode tags are setup for storing string constants.
      // Similar thing also may be done for copy numbers, just so that these xml files need to
      // be loaded only once.

      if ( std::find( m_Rich2PhysVolNames.begin(), m_Rich2PhysVolNames.end(), physVolName ) !=
           m_Rich2PhysVolNames.end() ) {

        printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich2GeoUtilInfo: ", "PhysVol name OK for  %s ", physVolName.c_str() );

      } else {

        printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "Rich2GeoUtilInfo: Error ",
                  "PhysVol name not available for %s ", physVolName.c_str() );
      }

    } // end test on physvolname

  } // end loop over transforms with trhandle
}
//==========================================================================//
void Rich2GeoUtil::build_Rich2SuperStrPhysTransforms( xml_h tr_handle, const std::string aSupStrPhysFileName ) {
  printout( m_Rich2GeomUtilDebug ? dd4hep::DEBUG : dd4hep::DEBUG,
            "Rich2GeoUtil : ", "Now building Rich2SuperStrTransforms %s", aSupStrPhysFileName.c_str() );

  int aLS = -1;
  int aLI = 0;
  if ( aSupStrPhysFileName.find( "SuperStructure" ) != std::string::npos ) {
    while ( ( aLI < m_Rich2NumSuperStrLateralSides ) && ( aLS < 0 ) ) {
      if ( aSupStrPhysFileName.find( m_Rich2SuperStrSectionNames[aLI] ) != std::string::npos ) { aLS = aLI; }
      aLI++;
    }
  }
  if ( aLS > -1 ) {

    std::pair<std::map<std::string, dd4hep::Transform3D>::iterator, bool> retRTrans;

    //  std::vector<std::string> aSupStrPhysVoleNameVect;
    // aSupStrPhysVoleNameVect.clear();
    // aSupStrPhysVoleNameVect.reserve(m_Rich2MaxNumSuperStrTransforms);

    for ( xml_coll_t i( tr_handle, _U( item ) ); i; ++i ) {
      xml_comp_t  c( i );
      std::string itemLabel = c.attr<std::string>( _U( name ) );
      if ( itemLabel.find( "TransformName" ) != std::string::npos ) {
        std::string trName = c.attr<std::string>( _U( value ) );

        // std::cout<<" test super str transform name "<< trName<<std::endl;
        xml_elt_t trn = c.child( _U( transformation ) );
        printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich2GeoUtil: Info ",
                  "Rich2GeoUtil: build Rich2SuperStrTransform in LateralSide %d for %s", aLS, trName.c_str() );

        const auto tr = dd4hep::xml::createTransformation( trn );
        retRTrans     = m_Rich2SuperStrTransforms[aLS].insert( std::make_pair( trName, tr ) );
        if ( retRTrans.second == false ) {
          printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR, "Rich2GeoUtil: Error ",
                    " Rich2GeoUtil SuperStr Transform in LateralSide %d for %s  not stored since it already existed "
                    "with a value of %d ",
                    aLS, trName.c_str(), retRTrans.first->second );
        }
      } else if ( itemLabel.find( "PhysVolName" ) != std::string::npos ) {

        std::string physVolName = c.attr<std::string>( _U( value ) );

        m_Rich2SuperStrPhysVolNames[aLS].push_back( physVolName );
        printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich2GeoUtilInfo: ", "Rich2SuperStr PhysVol name Stored in LateralSide %d for  %s ", aLS,
                  physVolName.c_str() );

      } else if ( itemLabel.find( "LogVolName" ) != std::string::npos ) {
        std::string logVolName = c.attr<std::string>( _U( value ) );
        m_Rich2SuperStrLogVolNames[aLS].push_back( logVolName );
        printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich2GeoUtilInfo: ", "Rich2SuperStr LogVol name Stored in LateralSide %d for  %s ", aLS,
                  logVolName.c_str() );
      }
    }
  } else {
    printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich2GeoUtilInfo: ", " This file not used in build_Rich2SuperStrPhysTransforms. File name %s ",
              aSupStrPhysFileName.c_str() );
  }
}

//==========================================================================//
const dd4hep::Transform3D& Rich2GeoUtil::getRich2GeomTransform( const std::string& aName ) {

  const auto t = m_Rich2GeomTransforms.find( aName );
  if ( t != m_Rich2GeomTransforms.end() ) {
    return t->second;
  } else {
    printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR,
              "Rich2GeoUtil : ", "Rich2 Transform not found for %s ", aName.c_str() );
    if ( m_Rich2GeomUtilDebug ) {
      std::cout << "Rich2GeoUtil ERROR:  Rich2 Transform not found for " << aName << std::endl;
    }
    static dd4hep::Transform3D aDummyRich2Trans;
    return aDummyRich2Trans;
  }
}

//==========================================================================//
const dd4hep::Transform3D& Rich2GeoUtil::getRich2SuperStrTransform( int aLateralSide, int aIndex ) {

  const auto aNameTN = Rich2SuperStrPhysVolName( aLateralSide, aIndex ) + "Transform";

  const auto t = m_Rich2SuperStrTransforms[aLateralSide].find( aNameTN );
  if ( t != m_Rich2SuperStrTransforms[aLateralSide].end() ) {
    return t->second;
  } else {
    printout( m_Rich2GeomUtilDebug ? dd4hep::ALWAYS : dd4hep::ERROR,
              "Rich2GeoUtil : ", "Rich2 SuperStr Transform not found in %d for %s ", aLateralSide, aNameTN.c_str() );
    static dd4hep::Transform3D aDummyRich2SuperStrTrans;
    return aDummyRich2SuperStrTrans;
  }
}

//==========================================================================//
bool Rich2GeoUtil::IsRich2SuperStrComp( const std::string& aDetFileName ) {
  bool aFileF = false;
  if ( aDetFileName.find( "Rich2Run3SuperStructure" ) != std::string::npos ) {
    if ( aDetFileName.find( "Rich2Run3SuperStructureMasterPhys" ) == std::string::npos ) { aFileF = true; }
  }
  return aFileF;
}

//==========================================================================//
void Rich2GeoUtil::InitRich2TransformsAndNames() {
  m_Rich2MaxNumGeomTransforms     = 500;
  m_Rich2MaxNumSuperStrTransforms = 500;

  m_Rich2GeomTransforms.clear();

  m_Rich2TransformNames.clear();
  m_Rich2PhysVolNames.clear();
  m_Rich2PhysVolCopyNumbers.clear();
  m_Rich2PhDetGasEnclLVName.clear();
  m_Rich2PhDetPanelLVName.clear();
  m_Rich2PhDetPanelDetName.clear();
  m_Rich2SphMirrorMasterLVName.clear();
  m_Rich2SecMirrorMasterLVName.clear();
  m_Rich2SuperStrMasterLVNames.clear();
  m_Rich2SuperStrSectionNames.clear();
  m_Rich2SuperStrMasterTNs.clear();
  m_Rich2SuperStrPhysVolNames.clear();
  m_Rich2SuperStrLogVolNames.clear();

  detail::init_param( m_Rich2NumSides, "Rh2NumSides" );

  m_Rich2MasterLVName        = "lvRich2Master";
  m_Rich2GasEnclosureLVName  = "lvRich2GasEnclosure";
  m_Rich2GasEnclosureDetName = "Rich2GasEnclosure";

  m_Rich2_BeamTubeLVName  = "lvRich2Tube";
  m_Rich2_BeamTubeDetName = "Rich2BeamTube";

  m_Rich2PhDetGasEnclLVName = {"lvRich2PhDetGasEncl0", "lvRich2PhDetGasEncl1"};
  m_Rich2PhDetPanelLVName   = {"lvRich2PhDetPanel0", "lvRich2PhDetPanel1"};
  m_Rich2PhDetPanelDetName  = {"Rich2PhDetPanel0", "Rich2PhDetPanel1"};
  m_Rich2RichSystemLVName   = "lvRichSystem";
  m_Rich2RichSystemDetName  = "RichSystem";

  m_Rich2SphMirrorMasterLVName  = {"lvRich2SphMirrorMaster0", "lvRich2SphMirrorMaster1"};
  m_Rich2SecMirrorMasterLVName  = {"lvRich2SecMirrorMaster0", "lvRich2SecMirrorMaster1"};
  m_Rich2SphMirrorMasterDetName = {"Rich2SphMirrorMaster0", "Rich2SphMirrorMaster1"};
  m_Rich2SecMirrorMasterDetName = {"Rich2SecMirrorMaster0", "Rich2SecMirrorMaster1"};

  m_Rich2SphMirrorSupportLVName = "lvRich2SphMSupport";
  m_Rich2SecMirrorSupportLVName = "lvRich2SecMSupport";

  m_Rich2GasQuartzWMatName  = "Rich:Rich2GasWindowQuartz";
  m_Rich2NitrogenMatName    = "Rich:Rich2Nitrogen";
  m_Rich2CF4MatName         = "Rich:CF4";
  m_Rich2RadiatorGasMatName = "Rich:R2RadiatorGas";
  m_Rich2CO2GasMatName      = "Rich:CO2";

  m_Rich2SuperStrMasterLVNames = {"lvRich2SuperStrAirBoxTop", "lvRich2SuperStrAirBoxBot", "lvRich2SuperStrAirTrapRight",
                                  "lvRich2SuperStrAirTrapLeft"};
  m_Rich2SuperStrSectionNames  = {"Top", "Bot", "Right", "Left"};
  m_Rich2NumSuperStrLateralSides = (int)m_Rich2SuperStrSectionNames.size();
  m_Rich2SuperStrMasterTNs       = {
      Rich2GeomTransformLabel::pvRich2SuperStrAirBoxTopTN, Rich2GeomTransformLabel::pvRich2SuperStrAirBoxBotTN,
      Rich2GeomTransformLabel::pvRich2SuperStrAirTrapRightTN, Rich2GeomTransformLabel::pvRich2SuperStrAirTrapLeftTN};
  m_Rich2SuperStrPhysVolNames.resize( m_Rich2NumSuperStrLateralSides );
  m_Rich2SuperStrLogVolNames.resize( m_Rich2NumSuperStrLateralSides );
  m_Rich2SuperStrTransforms.resize( m_Rich2NumSuperStrLateralSides );

  for ( int i = 0; i < m_Rich2NumSuperStrLateralSides; i++ ) {
    m_Rich2SuperStrPhysVolNames[i].reserve( m_Rich2MaxNumSuperStrTransforms );
    m_Rich2SuperStrLogVolNames[i].reserve( m_Rich2MaxNumSuperStrTransforms );
    m_Rich2SuperStrTransforms[i] = std::map<std::string, dd4hep::Transform3D>();
  }
  // Initialize with dummy names

  m_Rich2TransformNames.assign( m_Rich2MaxNumGeomTransforms, "DummyRich2Transform" );
  m_Rich2PhysVolNames.assign( m_Rich2MaxNumGeomTransforms, "DummyRich2PhysvolName" );
  m_Rich2PhysVolCopyNumbers.assign( m_Rich2MaxNumGeomTransforms, 0 );

  // Now set the correct names
  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich2GeneralPvI{
      {"pvRich2GasEnclosure", toInHe( Rich2GeomTransformLabel::pvRich2GasEnclosureTN )},
      {"pvRich2Tube", toInHe( Rich2GeomTransformLabel::pvRich2TubeTN )},
      {"pvRich2GasContWallTop", toInHe( Rich2GeomTransformLabel::pvRich2GasContWallTopTN )},
      {"pvRich2GasContWallBot", toInHe( Rich2GeomTransformLabel::pvRich2GasContWallBotTN )},
      {"pvRich2GasContWallSide0", toInHe( Rich2GeomTransformLabel::pvRich2GasContWallSide0TN )},
      {"pvRich2GasContWallSide1", toInHe( Rich2GeomTransformLabel::pvRich2GasContWallSide1TN )},
      {"pvRichSystem", toInHe( Rich2GeomTransformLabel::pvRichSystemTN )}};
  InitRich2FillTN( Rich2GeneralPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich2EntryExitPvI{
      {"pvRich2EntryWindowSkinDns", toInHe( Rich2GeomTransformLabel::pvRich2EntryWindowSkinDnsTN )},
      {"pvRich2EntryWindowSkinUps", toInHe( Rich2GeomTransformLabel::pvRich2EntryWindowSkinUpsTN )},
      {"pvRich2EntryWindowPMI", toInHe( Rich2GeomTransformLabel::pvRich2EntryWindowPMITN )},
      {"pvRich2EntryWinTubeLock", toInHe( Rich2GeomTransformLabel::pvRich2EntryWinTubeLockTN )},
      {"pvRich2ExitWindowSkinDns", toInHe( Rich2GeomTransformLabel::pvRich2ExitWindowSkinDnsTN )},
      {"pvRich2ExitWindowSkinUps", toInHe( Rich2GeomTransformLabel::pvRich2ExitWindowSkinUpsTN )},
      {"pvRich2ExitWindowPMI", toInHe( Rich2GeomTransformLabel::pvRich2ExitWindowPMITN )},
      {"pvRich2ExitWinTubeLock", toInHe( Rich2GeomTransformLabel::pvRich2ExitWinTubeLockTN )}};
  InitRich2FillTN( Rich2EntryExitPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich2MirrorMPvI{
      {"pvRich2SphMirrorMaster0", toInHe( Rich2GeomTransformLabel::pvRich2SphMirrorMaster0TN )},
      {"pvRich2SphMirrorMaster1", toInHe( Rich2GeomTransformLabel::pvRich2SphMirrorMaster1TN )},
      {"pvRich2SphMSupport0", toInHe( Rich2GeomTransformLabel::pvRich2SphMSupport0TN )},
      {"pvRich2SphMSupport1", toInHe( Rich2GeomTransformLabel::pvRich2SphMSupport1TN )},
      {"pvRich2SecMirrorMaster0", toInHe( Rich2GeomTransformLabel::pvRich2SecMirrorMaster0TN )},
      {"pvRich2SecMirrorMaster1", toInHe( Rich2GeomTransformLabel::pvRich2SecMirrorMaster1TN )},
      {"pvRich2SecMSupport0", toInHe( Rich2GeomTransformLabel::pvRich2SecMSupport0TN )},
      {"pvRich2SecMSupport1", toInHe( Rich2GeomTransformLabel::pvRich2SecMSupport1TN )}};
  InitRich2FillTN( Rich2MirrorMPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich2QWPvI{
      {"pvRich2QuartzWindow0", toInHe( Rich2GeomTransformLabel::pvRich2QuartzWindow0TN )},
      {"pvRich2QuartzWindow1", toInHe( Rich2GeomTransformLabel::pvRich2QuartzWindow1TN )}};
  InitRich2FillTN( Rich2QWPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich2MagShPvI{
      {"pvRich2MagShFront0", toInHe( Rich2GeomTransformLabel::pvRich2MagShFront0TN )},
      {"pvRich2MagShTop0", toInHe( Rich2GeomTransformLabel::pvRich2MagShTop0TN )},
      {"pvRich2MagShBottom0", toInHe( Rich2GeomTransformLabel::pvRich2MagShBottom0TN )},
      {"pvRich2MagShSideBack0", toInHe( Rich2GeomTransformLabel::pvRich2MagShSideBack0TN )},
      {"pvRich2MagShSideFront0", toInHe( Rich2GeomTransformLabel::pvRich2MagShSideFront0TN )},
      {"pvRich2MagShFront1", toInHe( Rich2GeomTransformLabel::pvRich2MagShFront1TN )},
      {"pvRich2MagShTop1", toInHe( Rich2GeomTransformLabel::pvRich2MagShTop1TN )},
      {"pvRich2MagShBottom1", toInHe( Rich2GeomTransformLabel::pvRich2MagShBottom1TN )},
      {"pvRich2MagShSideBack1", toInHe( Rich2GeomTransformLabel::pvRich2MagShSideBack1TN )},
      {"pvRich2MagShSideFront1", toInHe( Rich2GeomTransformLabel::pvRich2MagShSideFront1TN )}};
  InitRich2FillTN( Rich2MagShPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich2PhDetSupPvI{
      {"pvRich2PhDetGasEncl0", toInHe( Rich2GeomTransformLabel::pvRich2PhDetGasEncl0TN )},
      {"pvRich2PhDetGasEncl1", toInHe( Rich2GeomTransformLabel::pvRich2PhDetGasEncl1TN )},
      {"pvRich2PhDetPanel0", toInHe( Rich2GeomTransformLabel::pvRich2PhDetPanel0TN )},
      {"pvRich2PhDetPanel1", toInHe( Rich2GeomTransformLabel::pvRich2PhDetPanel1TN )}};
  InitRich2FillTN( Rich2PhDetSupPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //

  const Rich::Detector::NamedVector<int> Rich2SupStrMasPvI{
      {"pvRich2SuperStrAirBoxTop", toInHe( Rich2GeomTransformLabel::pvRich2SuperStrAirBoxTopTN )},
      {"pvRich2SuperStrAirBoxBot", toInHe( Rich2GeomTransformLabel::pvRich2SuperStrAirBoxBotTN )},
      {"pvRich2SuperStrAirTrapRight", toInHe( Rich2GeomTransformLabel::pvRich2SuperStrAirTrapRightTN )},
      {"pvRich2SuperStrAirTrapLeft", toInHe( Rich2GeomTransformLabel::pvRich2SuperStrAirTrapLeftTN )}};
  InitRich2FillTN( Rich2SupStrMasPvI );

  //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    //
}

//========================================================================== //

void Rich2GeoUtil::InitRich2FillTN( const Rich::Detector::NamedVector<int>& aRich2CompPvI ) {
  const std::string TNSuf = "Transform";
  const std::string CNSuf = "CopyNumber";
  for ( const auto& G : aRich2CompPvI ) {
    const auto& gIndex            = G.second;
    const auto& gLabel            = G.first;
    m_Rich2TransformNames[gIndex] = gLabel + TNSuf;
    m_Rich2PhysVolNames[gIndex]   = gLabel;
    detail::init_param( m_Rich2PhysVolCopyNumbers[gIndex], gLabel + CNSuf );
  }
}

//========================================================================== //

bool Rich2GeoUtil::IsInRich2TransformList( const std::string& aTrName ) {
  return ( std::find( m_Rich2TransformNames.begin(), m_Rich2TransformNames.end(), aTrName ) !=
           m_Rich2TransformNames.end() );
}

//==================================================================================//

Rich2GeoUtil* Rich2GeoUtil::getRich2GeoUtilInstance() {
  static std::once_flag                alloc_instance_once;
  static std::unique_ptr<Rich2GeoUtil> instance;
  std::call_once( alloc_instance_once, []() { instance = std::make_unique<Rich2GeoUtil>(); } );
  assert( instance.get() );
  return instance.get();
}
//==================================================================================//
