//==========================================================================
//  LHCb Rich2 Detector description implementation using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-12-01
//
//==========================================================================

#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DD4hep/PropertyTable.h"
#include "Detector/Rich1/RichGeoTransAux.h"
#include "Detector/Rich1/RichGeomVersion.h"
#include "Detector/Rich1/RichMatOPD.h"
#include "Detector/Rich1/RichMatPropData.h"
#include "Detector/Rich1/RichPmtGeoAux.h"
#include "Detector/Rich1/RichSurfaceUtil.h"
#include "Detector/Rich2/Rich2GeoUtil.h"
#include "Detector/Rich2/Rich2MirrorGeoAux.h"
#include "XML/Utilities.h"
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace {
  bool m_activate_Rich2GasEnclosure                  = true;
  bool m_activate_Rich2EntryExitWall                 = true;
  bool m_activate_Rich2_MagneticShield_build         = true;
  bool m_activate_Rich2_GasQW_build                  = true;
  bool m_activate_Rich2_PhotonDetector_build         = true;
  bool m_activate_Rich2_RichSystem_build             = true;
  bool m_activate_Rich2Mirror1_build                 = true;
  bool m_activate_Rich2Mirror2_build                 = true;
  bool m_activate_Rich2Mirror_System_build           = true;
  bool m_activate_Rich2Surface_build                 = true;
  bool m_activate_Rich2PmtSurface_build              = true;
  bool m_activate_Rich2QWSurface_build               = true;
  bool m_activate_Rich2SuperStructure_build          = true;
  bool m_activate_Rich2SuperStructureComponent_build = true;
  bool m_activate_Rich2_DetElem_For_CurrentAppl      = true;
  bool m_activate_Rich2_DetElem_For_PmtEcrh          = true;

  bool m_debugRich2Activate                 = true;
  bool m_volumeBuilderForRich2DebugActivate = true;

  bool m_activateRich2VolumeDebug         = false;
  bool m_debugRich2LvListActivate         = false;
  bool m_generalRich2UtilDebugActivate    = false;
  bool m_pmtutilRich2DebugActivate        = false;
  bool m_debugRich2Mirror1Activate        = false;
  bool m_debugRich2Mirror2Activate        = false;
  bool m_Rich2SurfaceDebugActivate        = false;
  bool m_Rich2PmtSurfaceDebugActivate     = false;
  bool m_Rich2SuperStructureDebugActivate = false;

  std::string m_attachRich2VolumeForDebug;

  //=====================================================================//

  /// Helper class to build the Rich2 detector of LHCb
  struct Rich2Build final : public dd4hep::xml::tools::VolumeBuilder {

    inline static std::string select_Rich2_Volume{"lvRich2Master"};
    RichGeomVersion           m_Rich2GeomVersion;

    /// Initializing constructor
    Rich2Build( dd4hep::Detector& description, const xml_elt_t& e, const dd4hep::SensitiveDetector& sens,
                RichGeomLabel aRG );

    /// Rich2 build methods
    void BuildARich2PhysVol( const std::string& aMotherLVName, const std::string& aChildLVName,
                             const Rich2GeomTransformLabel aTransformLabel );

    dd4hep::PlacedVolume BuildARich2PhysVolWithPVRet( const std::string& aMotherLVName, const std::string& aChildLVName,
                                                      const Rich2GeomTransformLabel aTransformLabel );
    void                 BuildARich2SuperStrPhysVol( const std::string& aSuperStrMasterLVName, const int iLS );

    void build_Rich2_Main();
    void build_Rich2Master();
    void build_Rich2MasterMainComponents();
    void build_Rich2GasEnclosure();
    void build_Rich2EntryExitWalls();
    void build_Rich2MagShield();
    void build_Rich2QW( const dd4hep::PlacedVolume& aPv );
    void build_Rich2_PhDetSupFrame();
    void build_Rich2_RichSystem();
    void build_Rich2_MirrorSystem();
    void build_Rich2_PrimarySph_MirrorSegments( const std::vector<dd4hep::PlacedVolume>& aSphSegMasterPv,
                                                const std::vector<dd4hep::DetElement>&   aSphSegMasterDet );
    void build_Rich2_Secondary_MirrorSegments( const std::vector<dd4hep::PlacedVolume>& aSecSegMasterPv,
                                               const std::vector<dd4hep::DetElement>&   aSecSegMasterDet );
    void build_Rich2PmtFullSystem();
    void build_Rich2PmtModuleVols();
    void build_Rich2PmtECRHVols();
    void build_Rich2PmtMasterVols();
    void build_Rich2PmtAllPhysVols();
    void build_Rich2SuperStructureVols();

    dd4hep::PlacedVolume BuildARich2PmtSMasterPhysVol( const std::string& aPmtMasterName, const int iM );
    void                 BuildARichPmtPhysVol( const std::string& aMotherLVName, const std::string& aChildLVName,
                                               const RichPmtGeomTransformLabel aTransformLabel );

    dd4hep::PlacedVolume BuildARichPmtPhysVolWithPVRet( const std::string&              aMotherLVName,
                                                        const std::string&              aChildLVName,
                                                        const RichPmtGeomTransformLabel aTransformLabel );

    std::vector<dd4hep::PlacedVolume> BuildRich2StdMapmtCompPhysVols();
    std::vector<dd4hep::PlacedVolume> BuildRich2GrandMapmtCompPhysVols();

    dd4hep::PlacedVolume BuildARich2PmtMasterPhysVol( int iM, int ec, int ip );
    dd4hep::PlacedVolume BuildARich2PmtEcrhPhysVol( int iM, int ec );
    dd4hep::PlacedVolume BuildARich2PmtModulePhysVol( int iM );
  };

  //=====================================================================//

  /// Initializing constructor
  Rich2Build::Rich2Build( dd4hep::Detector& dsc, const xml_elt_t& e, const dd4hep::SensitiveDetector& sens,
                          RichGeomLabel aRG )
      : dd4hep::xml::tools::VolumeBuilder( dsc, e, sens ), m_Rich2GeomVersion( RichGeomVersion() ) {
    m_Rich2GeomVersion.InitRichGeomVersion( aRG );

    xml_comp_t x_dbg = x_det.child( _U( debug ), false );
    if ( x_dbg ) {
      for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
        xml_comp_t  c( i );
        std::string aS = c.nameStr();
        if ( aS == "activate_Rich2GasEnclosure" ) {
          m_activate_Rich2GasEnclosure = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2EntryExitWall" ) {
          m_activate_Rich2EntryExitWall = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2_MagneticShield_build" ) {
          m_activate_Rich2_MagneticShield_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2_GasQW_build" ) {
          m_activate_Rich2_GasQW_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2_PhotonDetector_build" ) {
          m_activate_Rich2_PhotonDetector_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2_RichSystem_build" ) {
          m_activate_Rich2_RichSystem_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2Mirror1_build" ) {
          m_activate_Rich2Mirror1_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2Mirror2_build" ) {
          m_activate_Rich2Mirror2_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2Mirror_System_build" ) {
          m_activate_Rich2Mirror_System_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2Surface_build" ) {
          m_activate_Rich2Surface_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2PmtSurface_build" ) {
          m_activate_Rich2PmtSurface_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2QWSurface_build" ) {
          m_activate_Rich2QWSurface_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2SuperStructure_build" ) {
          m_activate_Rich2SuperStructure_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2SuperStructureComponent_build" ) {
          m_activate_Rich2SuperStructureComponent_build = c.attr<bool>( _U( value ) );
        } else if ( aS == "debugRich2Activate" ) {
          m_debugRich2Activate = c.attr<bool>( _U( value ) );
        } else if ( aS == "volumeBuilderForRich2DebugActivate" ) {
          m_volumeBuilderForRich2DebugActivate = c.attr<bool>( _U( value ) );
        } else if ( aS == "activateRich2VolumeDebug" ) {
          m_activateRich2VolumeDebug = c.attr<bool>( _U( value ) );
        } else if ( aS == "debugRich2LvListActivate" ) {
          m_debugRich2LvListActivate = c.attr<bool>( _U( value ) );
        } else if ( aS == "generalRich2UtilDebugActivate" ) {
          m_generalRich2UtilDebugActivate = c.attr<bool>( _U( value ) );
        } else if ( aS == "pmtutilRich2DebugActivate" ) {
          m_pmtutilRich2DebugActivate = c.attr<bool>( _U( value ) );
        } else if ( aS == "debugRich2Mirror1Activate" ) {
          m_debugRich2Mirror1Activate = c.attr<bool>( _U( value ) );
        } else if ( aS == "debugRich2Mirror2Activate" ) {
          m_debugRich2Mirror2Activate = c.attr<bool>( _U( value ) );
        } else if ( aS == "Rich2SurfaceDebugActivate" ) {
          m_Rich2SurfaceDebugActivate = c.attr<bool>( _U( value ) );
        } else if ( aS == "Rich2PmtSurfaceDebugActivate" ) {
          m_Rich2PmtSurfaceDebugActivate = c.attr<bool>( _U( value ) );
        } else if ( aS == "Rich2SuperStructureDebugActivate" ) {
          m_Rich2SuperStructureDebugActivate = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2_DetElem_For_CurrentAppl" ) {
          m_activate_Rich2_DetElem_For_CurrentAppl = c.attr<bool>( _U( value ) );
        } else if ( aS == "activate_Rich2_DetElem_For_PmtEcrh" ) {
          m_activate_Rich2_DetElem_For_PmtEcrh = c.attr<bool>( _U( value ) );
        } else if ( aS == "attachRich2VolumeForDebug" ) {
          m_attachRich2VolumeForDebug = c.attr<std::string>( _U( value ) );
        }
      }
    }
    debug = m_volumeBuilderForRich2DebugActivate;
  }

  //=====================================================================================//

  void Rich2Build::BuildARich2PhysVol( const std::string& aMotherLVName, const std::string& aChildLVName,
                                       const Rich2GeomTransformLabel aTransformLabel ) {
    auto aPvol = BuildARich2PhysVolWithPVRet( aMotherLVName, aChildLVName, aTransformLabel );
    if ( m_debugRich2Activate ) {
      const auto aPvolName = aPvol->GetName();
      printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                "Rich2_Geo : ", "Created Rich2 PhysVol with the name  %s", aPvolName );
    }
  }

  //=====================================================================================//

  dd4hep::PlacedVolume Rich2Build::BuildARich2PhysVolWithPVRet( const std::string&            aMotherLVName,
                                                                const std::string&            aChildLVName,
                                                                const Rich2GeomTransformLabel aTransformLabel ) {
    auto       lvMother      = volume( aMotherLVName );
    auto       lvChild       = volume( aChildLVName );
    auto       aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto TransformForChild =
        aRich2GeoUtil->getRich2GeomTransform( aRich2GeoUtil->getRich2TransformName( aTransformLabel ) );

    const auto aChildCopyNumber = aRich2GeoUtil->getRich2PhysVolCopyNumber( aTransformLabel );

    auto pvChild = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );

    pvChild->SetName( ( aRich2GeoUtil->getRich2PhysVolName( aTransformLabel ) ).c_str() );

    // Now setup debug printout options

    if ( m_debugRich2Activate ) {
      auto aCdebug = m_debugRich2Activate;
      if ( ( aChildLVName.find( "lvRichPmt" ) != std::string::npos ) ||
           ( aChildLVName.find( "lvRichPMT" ) != std::string::npos ) ||
           ( aChildLVName.find( "lvRich2Pmt" ) != std::string::npos ) ||
           ( aChildLVName.find( "lvRich2PMT" ) != std::string::npos ) )
        aCdebug = m_pmtutilRich2DebugActivate;

      printout( aCdebug ? dd4hep::ALWAYS : dd4hep::DEBUG,
                "Rich2_Geo : ", "Created PhysVol with name %s , copynumber %d , TransformName  %s ",
                ( aRich2GeoUtil->getRich2PhysVolName( aTransformLabel ) ).c_str(), aChildCopyNumber,
                ( aRich2GeoUtil->getRich2TransformName( aTransformLabel ) ).c_str() );
    }
    return pvChild;
  }

  //=====================================================================================//

  void Rich2Build::BuildARichPmtPhysVol( const std::string& aMotherLVName, const std::string& aChildLVName,
                                         const RichPmtGeomTransformLabel aTransformLabel ) {
    auto aPvol = BuildARichPmtPhysVolWithPVRet( aMotherLVName, aChildLVName, aTransformLabel );
    if ( m_debugRich2Activate ) {
      const auto aPvolName = aPvol->GetName();
      printout( m_pmtutilRich2DebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                "Rich2_Geo : ", "Created Rich2 Pmt PhysVol with the name  %s", aPvolName );
    }
  }

  //=====================================================================================//

  void Rich2Build::BuildARich2SuperStrPhysVol( const std::string& aSuperStrMasterLVName, const int iLS ) {

    auto lvMother = volume( aSuperStrMasterLVName );

    auto aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    for ( int iCh = 0; iCh < (int)( aRich2GeoUtil->getNumRich2SuperStrPhysVols( iLS ) ); iCh++ ) {
      const auto aChLogVolName     = aRich2GeoUtil->Rich2SuperStrLogVolName( iLS, iCh );
      const auto aChPhysVolName    = aRich2GeoUtil->Rich2SuperStrPhysVolName( iLS, iCh );
      auto       lvChild           = volume( aChLogVolName );
      const auto TransformForChild = aRich2GeoUtil->getRich2SuperStrTransform( iLS, iCh );
      auto       pvChild           = lvMother.placeVolume( lvChild, TransformForChild );
      pvChild->SetName( aChPhysVolName.c_str() );
      if ( m_Rich2SuperStructureDebugActivate ) {
        printout( m_Rich2SuperStructureDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich2_Geo : ",
                  "Created Rich2 SuperStructre PhysVol in LateralSide %d and index %d with the name  %s using Logvol "
                  "with Name %s ",
                  iLS, iCh, aChPhysVolName.c_str(), aChLogVolName.c_str() );
      }
    }
  }

  //=====================================================================================//
  dd4hep::PlacedVolume Rich2Build::BuildARichPmtPhysVolWithPVRet( const std::string&              aMotherLVName,
                                                                  const std::string&              aChildLVName,
                                                                  const RichPmtGeomTransformLabel aTransformLabel ) {
    auto       lvMother       = volume( aMotherLVName );
    auto       lvChild        = volume( aChildLVName );
    auto       aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    const auto TransformForChild =
        aRichPmtGeoAux->getRichPmtCompGeomTransform( aRichPmtGeoAux->getRichPmtCompTransformName( aTransformLabel ) );

    const auto aChildCopyNumber = aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( aTransformLabel );

    auto pvChild = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
    // dd4hep::PlacedVolume pvChild =  lvMother.placeVolume(lvChild,TransformForChild);

    pvChild->SetName( ( aRichPmtGeoAux->getRichPmtCompPhysVolName( aTransformLabel ) ).c_str() );

    // Now setup debug printout options
    if ( m_debugRich2Activate ) {
      printout( m_pmtutilRich2DebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                "Rich2_Geo : ", "Created Pmt PhysVol with name %s , copynumber %d , TransformName  %s ",
                ( aRichPmtGeoAux->getRichPmtCompPhysVolName( aTransformLabel ) ).c_str(), aChildCopyNumber,
                ( aRichPmtGeoAux->getRichPmtCompTransformName( aTransformLabel ) ).c_str() );
    }

    return pvChild;
  }
  //=====================================================================//
  void Rich2Build::build_Rich2GasEnclosure() {
    auto       aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto aR2MLName     = aRich2GeoUtil->Rich2MasterLVName();
    const auto aR2GasEnLName = aRich2GeoUtil->Rich2GasEnclosureLVName();

    auto aGasEnclPv =
        BuildARich2PhysVolWithPVRet( aR2MLName, aR2GasEnLName, Rich2GeomTransformLabel::pvRich2GasEnclosureTN );

    std::optional<dd4hep::DetElement> aGasEnclDet;
    if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
      const auto aGasEnclDetName = aRich2GeoUtil->Rich2GasEnclosureDetName();
      const auto aGasEnclDetId =
          aRich2GeoUtil->getRich2PhysVolCopyNumber( Rich2GeomTransformLabel::pvRich2GasEnclosureTN );
      aGasEnclPv.addPhysVolID( aGasEnclDetName + "Det", aGasEnclDetId );
      aGasEnclDet.emplace( detector, aGasEnclDetName, aGasEnclDetId );
      aGasEnclDet->setPlacement( aGasEnclPv );
    }

    const auto aRich2TubeLName = aRich2GeoUtil->Rich2_BeamTubeLVName();
    auto       aRich2TubePv =
        BuildARich2PhysVolWithPVRet( aR2MLName, aRich2TubeLName, Rich2GeomTransformLabel::pvRich2TubeTN );
    if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
      const auto aRich2TubeDetName = aRich2GeoUtil->Rich2_BeamTubeDetName();
      const auto aRich2TubeId      = aRich2GeoUtil->getRich2PhysVolCopyNumber( Rich2GeomTransformLabel::pvRich2TubeTN );
      aRich2TubePv.addPhysVolID( aRich2TubeDetName + "Det", aRich2TubeId );
      auto aRich2TubeDet = dd4hep::DetElement( detector, aRich2TubeDetName, aRich2TubeId );
      aRich2TubeDet.setPlacement( aRich2TubePv );
    }

    BuildARich2PhysVol( aR2MLName, "lvRich2GasContWallTopAndBot", Rich2GeomTransformLabel::pvRich2GasContWallTopTN );
    BuildARich2PhysVol( aR2MLName, "lvRich2GasContWallTopAndBot", Rich2GeomTransformLabel::pvRich2GasContWallBotTN );
    BuildARich2PhysVol( aR2MLName, "lvRich2GasContWallSide0", Rich2GeomTransformLabel::pvRich2GasContWallSide0TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2GasContWallSide1", Rich2GeomTransformLabel::pvRich2GasContWallSide1TN );

    if ( m_activate_Rich2Mirror_System_build ) build_Rich2_MirrorSystem();
    if ( m_activate_Rich2_GasQW_build ) build_Rich2QW( aGasEnclPv );
  }

  //=====================================================================//

  void Rich2Build::build_Rich2EntryExitWalls() {
    auto       aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto aR2MLName     = aRich2GeoUtil->Rich2MasterLVName();

    BuildARich2PhysVol( aR2MLName, "lvRich2EntryWindowSkin", Rich2GeomTransformLabel::pvRich2EntryWindowSkinDnsTN );
    BuildARich2PhysVol( aR2MLName, "lvRich2EntryWindowSkin", Rich2GeomTransformLabel::pvRich2EntryWindowSkinUpsTN );
    BuildARich2PhysVol( aR2MLName, "lvRich2EntryWindowPMI", Rich2GeomTransformLabel::pvRich2EntryWindowPMITN );
    BuildARich2PhysVol( aR2MLName, "lvRich2EntryWinTubeLock", Rich2GeomTransformLabel::pvRich2EntryWinTubeLockTN );
    BuildARich2PhysVol( aR2MLName, "lvRich2ExitWindowSkin", Rich2GeomTransformLabel::pvRich2ExitWindowSkinDnsTN );
    BuildARich2PhysVol( aR2MLName, "lvRich2ExitWindowSkin", Rich2GeomTransformLabel::pvRich2ExitWindowSkinUpsTN );
    BuildARich2PhysVol( aR2MLName, "lvRich2ExitWindowPMI", Rich2GeomTransformLabel::pvRich2ExitWindowPMITN );
    BuildARich2PhysVol( aR2MLName, "lvRich2ExitWinTubeLock", Rich2GeomTransformLabel::pvRich2ExitWinTubeLockTN );
  }

  //=====================================================================//

  void Rich2Build::build_Rich2MagShield() {
    auto       aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto aR2MLName     = aRich2GeoUtil->Rich2MasterLVName();

    BuildARich2PhysVol( aR2MLName, "lvRich2MagShFront0", Rich2GeomTransformLabel::pvRich2MagShFront0TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2MagShTopBot", Rich2GeomTransformLabel::pvRich2MagShTop0TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2MagShTopBot", Rich2GeomTransformLabel::pvRich2MagShBottom0TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2MagShSideBack", Rich2GeomTransformLabel::pvRich2MagShSideBack0TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2MagShSideFront", Rich2GeomTransformLabel::pvRich2MagShSideFront0TN );

    BuildARich2PhysVol( aR2MLName, "lvRich2MagShFront1", Rich2GeomTransformLabel::pvRich2MagShFront1TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2MagShTopBot", Rich2GeomTransformLabel::pvRich2MagShTop1TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2MagShTopBot", Rich2GeomTransformLabel::pvRich2MagShBottom1TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2MagShSideBack", Rich2GeomTransformLabel::pvRich2MagShSideBack1TN );
    BuildARich2PhysVol( aR2MLName, "lvRich2MagShSideFront", Rich2GeomTransformLabel::pvRich2MagShSideFront1TN );
  }

  //=====================================================================//

  void Rich2Build::build_Rich2QW( const dd4hep::PlacedVolume& aAdjacentPv ) {
    auto       aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto aR2MLName     = aRich2GeoUtil->Rich2MasterLVName();

    auto aQw0Pv = BuildARich2PhysVolWithPVRet( aR2MLName, "lvRich2QuartzWindow",
                                               Rich2GeomTransformLabel::pvRich2QuartzWindow0TN );
    auto aQw1Pv = BuildARich2PhysVolWithPVRet( aR2MLName, "lvRich2QuartzWindow",
                                               Rich2GeomTransformLabel::pvRich2QuartzWindow1TN );
    if ( m_activate_Rich2QWSurface_build ) {
      auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
      auto surfMgr          = description.surfaceManager();
      auto aR2GasQWSurface  = surfMgr.opticalSurface( aRichSurfaceUtil->Rich2GasQuartzWindowSurfaceName() );
      auto aGasQWBorderLSN  = std::array{aRichSurfaceUtil->Rich2GasQuartzWindowBorderSurfaceName( 0 ),
                                        aRichSurfaceUtil->Rich2GasQuartzWindowBorderBackSurfaceName( 0 )};
      auto aGasQWBorderRSN  = std::array{aRichSurfaceUtil->Rich2GasQuartzWindowBorderSurfaceName( 1 ),
                                        aRichSurfaceUtil->Rich2GasQuartzWindowBorderBackSurfaceName( 1 )};

      for ( int iss = 0; iss < (int)aGasQWBorderLSN.size(); iss++ ) {
        auto aR2GasQW0BorderSurface = ( iss == 0 ) ? dd4hep::BorderSurface( description, detector, aGasQWBorderLSN[0],
                                                                            aR2GasQWSurface, aAdjacentPv, aQw0Pv )
                                                   : dd4hep::BorderSurface( description, detector, aGasQWBorderLSN[1],
                                                                            aR2GasQWSurface, aQw0Pv, aAdjacentPv );
        auto aR2GasQW1BorderSurface = ( iss == 0 ) ? dd4hep::BorderSurface( description, detector, aGasQWBorderRSN[0],
                                                                            aR2GasQWSurface, aAdjacentPv, aQw1Pv )
                                                   : dd4hep::BorderSurface( description, detector, aGasQWBorderRSN[1],
                                                                            aR2GasQWSurface, aQw1Pv, aAdjacentPv );

        aR2GasQW0BorderSurface.isValid();
        aR2GasQW1BorderSurface.isValid();
        printout( m_Rich2SurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich2_Geo : ", "Created Rich2GasQW Surface with Name %s and %s using %s",
                  ( aGasQWBorderLSN[iss] ).c_str(), ( aGasQWBorderRSN[iss] ).c_str(),
                  ( aRichSurfaceUtil->Rich2GasQuartzWindowSurfaceName() ).c_str() );
      }
    }
  }
  //=====================================================================//
  void Rich2Build::build_Rich2_PhDetSupFrame() {
    auto       aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto aR2MLName     = aRich2GeoUtil->Rich2MasterLVName();
    auto       agTN =
        std::array{Rich2GeomTransformLabel::pvRich2PhDetGasEncl0TN, Rich2GeomTransformLabel::pvRich2PhDetGasEncl1TN};
    auto apTN =
        std::array{Rich2GeomTransformLabel::pvRich2PhDetPanel0TN, Rich2GeomTransformLabel::pvRich2PhDetPanel1TN};
    // for debug test building only one side
    //    for ( int iS = 0; iS < ( aRich2GeoUtil->Rich2NumSides() )/2; iS++ ) {
    // end of debug test

    for ( int iS = 0; iS < ( aRich2GeoUtil->Rich2NumSides() ); iS++ ) {
      const auto aR2PhDetGELName = aRich2GeoUtil->Rich2PhDetGasEnclLVName( iS );
      const auto aR2PhDetPALName = aRich2GeoUtil->Rich2PhDetPanelLVName( iS );
      BuildARich2PhysVol( aR2MLName, aR2PhDetGELName, agTN[iS] );

      auto aPhDetPanelPv = BuildARich2PhysVolWithPVRet( aR2PhDetGELName, aR2PhDetPALName, apTN[iS] );
      if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
        const auto aR2PhDetPanelDetName = aRich2GeoUtil->Rich2PhDetPanelDetName( iS );
        const auto aR2PhDetPanelId      = aRich2GeoUtil->getRich2PhysVolCopyNumber( apTN[iS] );
        aPhDetPanelPv.addPhysVolID( aR2PhDetPanelDetName + "Det", aR2PhDetPanelId );
        dd4hep::DetElement aR2PhDetPanelDet( detector, aR2PhDetPanelDetName, aR2PhDetPanelId );
        aR2PhDetPanelDet.setPlacement( aPhDetPanelPv );
      }
    }

    build_Rich2PmtFullSystem();
  }
  //=====================================================================//
  void Rich2Build::build_Rich2PmtFullSystem() {
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    aRichPmtGeoAux->initRich2PmtStructures();
    if ( aRichPmtGeoAux->Rich2PmtStructureCreated() ) {
      build_Rich2PmtModuleVols();
      build_Rich2PmtECRHVols();
      build_Rich2PmtMasterVols();
      build_Rich2PmtAllPhysVols();
    }
  }
  //=====================================================================//
  void Rich2Build::build_Rich2PmtModuleVols() {
    if ( m_debugRich2Activate ) { dd4hep::printout( dd4hep::DEBUG, "Rich2_geo", "Begin to build Rich2 PMT modules " ); }
    // get a single std  pmtModule
    auto aRichPmtGeoAux           = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto lvSingleR2StdPmtModule   = volume( "lvRich2PmtSingleStdModule" );
    auto lvSingleR2GrandPmtModule = volume( "lvRich2GrandPmtSingleModule" );

    const auto aMaterialStd   = lvSingleR2StdPmtModule.material();
    const auto aShapeStd      = lvSingleR2StdPmtModule.solid();
    const auto aVisStd        = lvSingleR2StdPmtModule.visAttributes();
    const auto aMaterialGrand = lvSingleR2GrandPmtModule.material();
    const auto aShapeGrand    = lvSingleR2GrandPmtModule.solid();
    const auto aVisGrand      = lvSingleR2GrandPmtModule.visAttributes();

    for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich2TotalNumPmtModules() ); iM++ ) {
      if ( aRichPmtGeoAux->IsRich2ActiveModule( iM ) ) {
        const auto aVolName   = aRichPmtGeoAux->Rich2PmtModuleLogVolName( iM );
        const auto aShapeName = aRichPmtGeoAux->Rich2PmtModuleShapeName( iM );
        if ( m_pmtutilRich2DebugActivate ) {
          dd4hep::printout( dd4hep::DEBUG, "Rich2_geo", "Rich2Pmt LogVolName ShapeName %d %s %s", iM, aVolName.c_str(),
                            aShapeName.c_str() );
        }
        const auto aN          = ( aRichPmtGeoAux->IsRich2GrandPmtModule( iM ) );
        const auto curMaterial = aN ? aMaterialGrand : aMaterialStd;
        const auto curSolid    = aN ? aShapeGrand : aShapeStd;
        const auto curVis      = aN ? aVisGrand : aVisStd;

        dd4hep::Solid_type aShapeM( curSolid );
        aShapeM.setName( aShapeName.c_str() );

        dd4hep::Volume aLV( aVolName.c_str(), aShapeM, curMaterial );
        aLV.setVisAttributes( curVis );

        registerShape( aShapeName, aShapeM );
        registerVolume( aVolName, aLV );
      }
    }

    if ( m_debugRich2Activate ) { dd4hep::printout( dd4hep::DEBUG, "Rich2_geo", "End build Rich2 PMT modules " ); }
  }
  //=====================================================================//
  void Rich2Build::build_Rich2PmtECRHVols() {
    if ( m_debugRich2Activate ) { dd4hep::printout( dd4hep::DEBUG, "Rich2_geo", "Begin to build PMT ECR " ); }
    // get a single pmtECR
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto lvSinglePmtEcr = volume( "lvRich2PmtSingleECR" );
    auto lvSinglePmtEch = volume( "lvRich2GrandPmtSingleECH" );

    const auto aMaterialEcr = lvSinglePmtEcr.material();
    const auto aShapeEcr    = lvSinglePmtEcr.solid();
    const auto aVisEcr      = lvSinglePmtEcr.visAttributes();
    const auto aMaterialEch = lvSinglePmtEch.material();
    const auto aShapeEch    = lvSinglePmtEch.solid();
    const auto aVisEch      = lvSinglePmtEch.visAttributes();

    // Loop over Rich2 modules
    for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich2TotalNumPmtModules() ); iM++ ) {
      if ( aRichPmtGeoAux->IsRich2ActiveModule( iM ) ) {

        // loop over EC in a module.
        const auto aMaxNumEcInMod = aRichPmtGeoAux->Rh2NumECInCurrentModule( iM );
        const auto aE             = ( aRichPmtGeoAux->IsRich2GrandPmtModule( iM ) );
        const auto curMatEc       = aE ? aMaterialEch : aMaterialEcr;
        const auto curShapeEc     = aE ? aShapeEch : aShapeEcr;
        const auto curVisEc       = aE ? aVisEch : aVisEcr;

        for ( int ec = 0; ec < aMaxNumEcInMod; ec++ ) {
          const auto aEcVolName   = aRichPmtGeoAux->Rich2PmtECRHLogVolName( iM, ec );
          const auto aEcShapeName = aRichPmtGeoAux->Rich2PmtECRHShapeName( iM, ec );
          if ( m_pmtutilRich2DebugActivate ) {
            dd4hep::printout( dd4hep::DEBUG, "Rich2_geo", "Rich2 Ec vol and shape Names %d %d %s %s", iM, ec,
                              aEcVolName.c_str(), aEcShapeName.c_str() );
          }

          dd4hep::Solid_type aShapeE( curShapeEc );
          aShapeE.setName( aEcShapeName.c_str() );

          dd4hep::Volume aLVE( aEcVolName.c_str(), aShapeE, curMatEc );
          aLVE.setVisAttributes( curVisEc );
          registerShape( aEcShapeName, aShapeE );
          registerVolume( aEcVolName, aLVE );
        }
      }
    }
  }
  //=====================================================================//
  void Rich2Build::build_Rich2PmtMasterVols() {
    if ( m_debugRich2Activate ) { dd4hep::printout( dd4hep::DEBUG, "Rich2_geo", "Begin to build Rich2 PMT Master " ); }
    // get a single pmt Master
    auto aRichPmtGeoAux         = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto lvSingleStdPmtMaster   = volume( "lvRich2PmtSingleStdMaster" );
    auto lvSingleGrandPmtMaster = volume( "lvRich2GrandPmtSingleMaster" );

    const auto aMaterialStd   = lvSingleStdPmtMaster.material();
    const auto aShapeStd      = lvSingleStdPmtMaster.solid();
    const auto aVisStd        = lvSingleStdPmtMaster.visAttributes();
    const auto aMaterialGrand = lvSingleGrandPmtMaster.material();
    const auto aShapeGrand    = lvSingleGrandPmtMaster.solid();
    const auto aVisGrand      = lvSingleGrandPmtMaster.visAttributes();

    // Loop over Rich2  modules
    for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich2TotalNumPmtModules() ); iM++ ) {
      if ( aRichPmtGeoAux->IsRich2ActiveModule( iM ) ) {

        const auto aMaxNumEcInMod = aRichPmtGeoAux->Rh2NumECInCurrentModule( iM );
        const auto aNumPmtinEC    = aRichPmtGeoAux->Rh2NumPmtInCurrentEC( iM );

        const auto aE         = ( aRichPmtGeoAux->IsRich2GrandPmtModule( iM ) );
        const auto curMatPm   = aE ? aMaterialGrand : aMaterialStd;
        const auto curShapePm = aE ? aShapeGrand : aShapeStd;
        const auto curVisPm   = aE ? aVisGrand : aVisStd;
        // loop over EC in a module.
        for ( int ec = 0; ec < aMaxNumEcInMod; ec++ ) {
          // Loop over Pmts
          for ( int ip = 0; ip < aNumPmtinEC; ip++ ) {
            const auto aPmtVolName   = aRichPmtGeoAux->Rich2PmtMasterLogVolName( iM, ec, ip );
            const auto aPmtShapeName = aRichPmtGeoAux->Rich2PmtMasterShapeName( iM, ec, ip );
            if ( m_pmtutilRich2DebugActivate ) {
              dd4hep::printout( dd4hep::DEBUG, "Rich2_geo", " Rich2 PmtMaster vol and shape Names %d %d %d %s %s", iM,
                                ec, ip, aPmtVolName.c_str(), aPmtShapeName.c_str() );
            }
            //        if(aE ) {
            dd4hep::Solid_type aShapeP( curShapePm );
            aShapeP.setName( aPmtShapeName.c_str() );

            dd4hep::Volume aLVP( aPmtVolName.c_str(), aShapeP, curMatPm );
            aLVP.setVisAttributes( curVisPm );
            registerShape( aPmtShapeName, aShapeP );
            registerVolume( aPmtVolName, aLVP );
          }
        }
      }
    }
  }
  //=====================================================================//
  void Rich2Build::build_Rich2PmtAllPhysVols() {
    if ( m_debugRich2Activate ) {
      dd4hep::printout( dd4hep::DEBUG, "Rich2_geo", "Begin to build Rich2 PMT array structure Phys Vols " );
    }
    auto aRichPmtGeoAux   = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();

    // Acquire the info needed to create pmt surfaces
    auto       surfMgr            = description.surfaceManager();
    auto       aRMapmtQWSurf      = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtQWSurfaceName() );
    auto       aRMapmtMTSurf      = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtMTSurfaceName() );
    auto       aRMapmtANSurf      = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtAnodeSurfaceName() );
    const auto aNumSurfPairInPmt  = 2; // For the pair of surfaces in the vectors listed below.
    auto       aR2MapmtQWBorderSN = std::array{"RichMaPMTQWBorderSurfaceIn", "RichMaPMTQWBorderBackSurfaceIn"};
    auto aR2MapmtMTBorderSN = std::array{"RichMaPMTSideEnvMTBorderSurfaceIn", "RichMapmtBackEnvMTBorderSurfaceIn"};
    const auto aR2MapmtANBorderSN = "RichMaPMTAnodeBorderSurfaceIn";

    const auto aPvRich2StdMapmtComp   = BuildRich2StdMapmtCompPhysVols();
    const auto aPvRich2GrandMapmtComp = BuildRich2GrandMapmtCompPhysVols();

    // debug test to build only one side
    // for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich2TotalNumPmtModules() )/2; iM++ ) {
    // end debug test.

    for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich2TotalNumPmtModules() ); iM++ ) {

      if ( aRichPmtGeoAux->IsRich2ActiveModule( iM ) ) {

        const auto aMaxNumEcInMod = aRichPmtGeoAux->Rh2NumECInCurrentModule( iM );
        const auto aNumPmtinEC    = aRichPmtGeoAux->Rh2NumPmtInCurrentEC( iM );
        const auto aG             = ( aRichPmtGeoAux->IsRich2GrandPmtModule( iM ) );
        auto       aPvRich2PmtQW = aG ? aPvRich2GrandMapmtComp[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtQW )]
                                : aPvRich2StdMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtQW )];
        auto aPvRich2PmtSideEnv =
            aG ? aPvRich2GrandMapmtComp[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtSideEnv )]
               : aPvRich2StdMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtSideEnv )];
        auto aPvRich2PmtBackEnv =
            aG ? aPvRich2GrandMapmtComp[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtBackEnv )]
               : aPvRich2StdMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtBackEnv )];
        auto aPvRich2PmtAnode = aG ? aPvRich2GrandMapmtComp[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtAnode )]
                                   : aPvRich2StdMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtAnode )];
        auto aPvRich2PmtPhCathode =
            aG ? aPvRich2GrandMapmtComp[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtPhc )]
               : aPvRich2StdMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtPhc )];

        const auto aR2PmtAnodeDetName =
            aG ? aRichPmtGeoAux->RichGrandMaPmtAnodeDetName() : aRichPmtGeoAux->RichMaPmtAnodeDetName();
        const auto aR2PmtAnodeIdSuffix =
            aG ? aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel::pvRichPMTAnode0001TN )
               : aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel::pvRichPMTAnode0000TN );
        const auto aR2PmtPhCathodeDetName =
            aG ? aRichPmtGeoAux->RichGrandMaPmtPhCathodeDetName() : aRichPmtGeoAux->RichMaPmtPhCathodeDetName();
        const auto aR2PmtPhCathodeIdSuffix =
            aG ? aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel::pvRichGrandPMTPhCathodeTN )
               : aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel::pvRichPMTPhCathodeTN );

        std::optional<dd4hep::DetElement> aR2PmtModuleDet;
        if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
          const auto aR2PmtModuleDetName = aRichPmtGeoAux->Rich2PmtModuleDetName( iM );
          aR2PmtModuleDet.emplace( detector.child( aRichPmtGeoAux->getRich2PhDetPanelDetNameFromModuleNum( iM ) ),
                                   aR2PmtModuleDetName, iM );
        }

        // loop over EC in a module.
        for ( int ec = 0; ec < aMaxNumEcInMod; ec++ ) {
          // Loop over Pmts
          for ( int ip = 0; ip < aNumPmtinEC; ip++ ) {
            const auto aPmtMasterVolName       = aRichPmtGeoAux->Rich2PmtMasterLogVolName( iM, ec, ip );
            auto       aRichMapmtSMPhysVol     = BuildARich2PmtSMasterPhysVol( aPmtMasterVolName, iM );
            auto       aRichMapmtMasterPhysVol = BuildARich2PmtMasterPhysVol( iM, ec, ip );
            if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
              const auto aR2PmtMasterDetName = aRichPmtGeoAux->Rich2PmtMasterDetName( iM, ec, ip );
              const auto aPmtMasterId        = aRichMapmtMasterPhysVol.copyNumber();
              assert( aR2PmtModuleDet.has_value() );
              auto aR2PmtMasterDet = dd4hep::DetElement( aR2PmtModuleDet.value(), aR2PmtMasterDetName, aPmtMasterId );
              aR2PmtMasterDet.setPlacement( aRichMapmtMasterPhysVol );
              // defining Anode Id to avoid duplication
              const auto CurPmtMasterCopyNum = aRichPmtGeoAux->getRich2PmtMasterCopyNumber( iM, ec, ip );
              const auto aR2PmtAnodeId =
                  ( ( aRichPmtGeoAux->RhPmtAnodeDetIdShiftFactor() ) * ( 1 + aR2PmtAnodeIdSuffix ) ) +
                  CurPmtMasterCopyNum;

              dd4hep::DetElement aR2PmtAnodeDet( aR2PmtMasterDet, aR2PmtAnodeDetName, aR2PmtAnodeId );
              aR2PmtAnodeDet.setPlacement( aPvRich2PmtAnode );

              const auto aR2PmtPhCathodeId =
                  ( ( aRichPmtGeoAux->RhPmtPhCathodeDetIdShiftFactor() ) * ( 1 + aR2PmtPhCathodeIdSuffix ) ) +
                  CurPmtMasterCopyNum;
              dd4hep::DetElement aR2PmtPhCathodeDet( aR2PmtMasterDet, aR2PmtPhCathodeDetName, aR2PmtPhCathodeId );
              aR2PmtPhCathodeDet.setPlacement( aPvRich2PmtPhCathode );
            }

            aRichPmtGeoAux->setRich2PmtMasterValid( iM, ec, ip );

            if ( m_activate_Rich2PmtSurface_build ) {

              for ( int isPair = 0; isPair < aNumSurfPairInPmt; isPair++ ) {
                auto aR2MapmtQWBorderSurface =
                    ( isPair == 0 ) ? dd4hep::BorderSurface( description, detector,
                                                             ( aR2MapmtQWBorderSN[isPair] + aPmtMasterVolName ),
                                                             aRMapmtQWSurf, aRichMapmtSMPhysVol, aPvRich2PmtQW )
                                    : dd4hep::BorderSurface( description, detector,
                                                             ( aR2MapmtQWBorderSN[isPair] + aPmtMasterVolName ),
                                                             aRMapmtQWSurf, aPvRich2PmtQW, aRichMapmtSMPhysVol );

                auto aR2MapmtMTBorderSurface =
                    ( isPair == 0 ) ? dd4hep::BorderSurface( description, detector,
                                                             ( aR2MapmtMTBorderSN[isPair] + aPmtMasterVolName ),
                                                             aRMapmtMTSurf, aRichMapmtSMPhysVol, aPvRich2PmtSideEnv )
                                    : dd4hep::BorderSurface( description, detector,
                                                             ( aR2MapmtMTBorderSN[isPair] + aPmtMasterVolName ),
                                                             aRMapmtMTSurf, aRichMapmtSMPhysVol, aPvRich2PmtBackEnv );

                aR2MapmtQWBorderSurface.isValid();
                aR2MapmtMTBorderSurface.isValid();
                if ( m_Rich2PmtSurfaceDebugActivate ) {

                  printout( m_Rich2PmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                            "Rich2_Geo : ", "Created Rich2Pmt QW Surface with Name %s using %s",
                            ( ( aR2MapmtQWBorderSN[isPair] + aPmtMasterVolName ).c_str() ),
                            ( ( aRichSurfaceUtil->RichMapmtQWSurfaceName() ).c_str() ) );

                  printout( m_Rich2PmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                            "Rich2_Geo : ", "Created Rich2Pmt MT Surface with Name %s using %s",
                            ( ( aR2MapmtMTBorderSN[isPair] + aPmtMasterVolName ).c_str() ),
                            ( ( aRichSurfaceUtil->RichMapmtMTSurfaceName() ).c_str() ) );
                }
              }
              dd4hep::BorderSurface aR2MapmtAnodeBorderSurface( description, detector,
                                                                ( aR2MapmtANBorderSN + aPmtMasterVolName ),
                                                                aRMapmtANSurf, aRichMapmtSMPhysVol, aPvRich2PmtAnode );
              aR2MapmtAnodeBorderSurface.isValid();
              if ( m_Rich2PmtSurfaceDebugActivate ) {
                printout( m_Rich2PmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                          "Rich2_Geo : ", "Created Rich2Pmt Anode Surface with Name %s using %s",
                          ( ( aR2MapmtANBorderSN + aPmtMasterVolName ).c_str() ),
                          ( ( aRichSurfaceUtil->RichMapmtAnodeSurfaceName() ).c_str() ) );
              }
            }
          }
          auto pvPmtEcrh = BuildARich2PmtEcrhPhysVol( iM, ec );
          aRichPmtGeoAux->setRich2PmtECRHValid( iM, ec );
          if ( m_activate_Rich2_DetElem_For_CurrentAppl && m_activate_Rich2_DetElem_For_PmtEcrh ) {
            const auto         aR2PmtEcrhDetName = aRichPmtGeoAux->Rich2PmtECRHDetName( iM, ec );
            const auto         aEcrhId           = pvPmtEcrh.copyNumber();
            dd4hep::DetElement aR2PmtEcrhDet( aR2PmtModuleDet.value(), aR2PmtEcrhDetName, aEcrhId );
            aR2PmtEcrhDet.setPlacement( pvPmtEcrh );
          }
        }
        auto pvPmtModule = BuildARich2PmtModulePhysVol( iM );
        if ( m_activate_Rich2_DetElem_For_CurrentAppl ) { aR2PmtModuleDet->setPlacement( pvPmtModule ); }
      }
    }
  }
  //=====================================================================//
  std::vector<dd4hep::PlacedVolume> Rich2Build::BuildRich2StdMapmtCompPhysVols() {

    auto                              aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    std::vector<dd4hep::PlacedVolume> aPmtCompPvVect( aRichPmtGeoAux->NumRichPmtCompForSurf() );
    const int                         aNumSurfPairInPmt = 2; // For the pair of surfaces in the vectors listed below.
    auto                              aRichSurfaceUtil  = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto                              surfMgr           = description.surfaceManager();
    auto aRMapmtPCSurf      = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtPCSurfaceName() );
    auto aR2MapmtPCBorderSN = std::array{"RichStdMaPMTPCBorderSurfaceIn", "RichStdMaPMTPCBorderBackSurfaceIn"};

    const auto aPmtSMLvName = aRichPmtGeoAux->Rich2MaPmtSubMasterLVName();

    const auto aPmtSMLv          = volume( aPmtSMLvName );
    const auto NumVolInPmtSM     = (int)( aPmtSMLv->GetNdaughters() );
    const bool PmtCompPvNotBuilt = ( NumVolInPmtSM == 0 );

    // First the pmt components
    if ( PmtCompPvNotBuilt ) {
      auto aPvRichMapmtAnode =
          BuildARichPmtPhysVolWithPVRet( aPmtSMLvName, ( aRichPmtGeoAux->Rich2MaPmtStdAnodeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTAnode0000TN );
      auto aPvRichMapmtQuartz = BuildARichPmtPhysVolWithPVRet(
          aPmtSMLvName, ( aRichPmtGeoAux->Rich2MaPmtQuartzLVName() ), RichPmtGeomTransformLabel::pvRichPMTQuartzTN );
      auto aPvRichMapmtPhCathode =
          BuildARichPmtPhysVolWithPVRet( aPmtSMLvName, ( aRichPmtGeoAux->Rich2MaPmtPhCathodeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTPhCathodeTN );
      auto aPvRichMapmtSideEnv =
          BuildARichPmtPhysVolWithPVRet( aPmtSMLvName, ( aRichPmtGeoAux->Rich2MaPmtSideEnvelopeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTSideEnvelopeTN );
      auto aPvRichMapmtBackEnv =
          BuildARichPmtPhysVolWithPVRet( aPmtSMLvName, ( aRichPmtGeoAux->Rich2MaPmtBackEnvelopeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTBackEnvelopeTN );
      BuildARichPmtPhysVol( aPmtSMLvName, ( aRichPmtGeoAux->Rich2MaPmtFrontRingLVName() ),
                            RichPmtGeomTransformLabel::pvRichPMTFrontRingTN );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtAnode )]   = aPvRichMapmtAnode;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtQW )]      = aPvRichMapmtQuartz;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtSideEnv )] = aPvRichMapmtSideEnv;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtBackEnv )] = aPvRichMapmtBackEnv;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtPhc )]     = aPvRichMapmtPhCathode;

      if ( m_activate_Rich2PmtSurface_build ) {
        for ( int isPair = 0; isPair < aNumSurfPairInPmt; isPair++ ) {

          auto aR2MapmtPCBorderSurface =
              ( isPair == 0 ) ? dd4hep::BorderSurface( description, detector, ( aR2MapmtPCBorderSN[isPair] ),
                                                       aRMapmtPCSurf, aPvRichMapmtQuartz, aPvRichMapmtPhCathode )
                              : dd4hep::BorderSurface( description, detector, ( aR2MapmtPCBorderSN[isPair] ),
                                                       aRMapmtPCSurf, aPvRichMapmtPhCathode, aPvRichMapmtQuartz );

          aR2MapmtPCBorderSurface.isValid();
          if ( m_Rich2PmtSurfaceDebugActivate ) {
            printout( m_Rich2PmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                      "Rich2_Geo : ", "Created Rich2Pmt PC Surface with Name %s using %s",
                      ( aR2MapmtPCBorderSN[isPair] ), ( ( aRichSurfaceUtil->RichMapmtPCSurfaceName() ).c_str() ) );

            aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtPCSurf, RichSurfPropType::Refl );
            aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtPCSurf, RichSurfPropType::Effic );
          }
        }
      }

    } else {
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtAnode )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichPMTAnode0000TN ) ).c_str() );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtQW )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichPMTQuartzTN ) ).c_str() );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtSideEnv )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichPMTSideEnvelopeTN ) ).c_str() );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtBackEnv )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichPMTBackEnvelopeTN ) ).c_str() );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtPhc )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichPMTPhCathodeTN ) ).c_str() );
    }
    return aPmtCompPvVect;
  }

  //=====================================================================//

  std::vector<dd4hep::PlacedVolume> Rich2Build::BuildRich2GrandMapmtCompPhysVols() {

    auto                              aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    std::vector<dd4hep::PlacedVolume> aPmtCompPvVect( aRichPmtGeoAux->NumRichPmtCompForSurf() );

    const int aNumSurfPairInPmt  = 2; // For the pair of surfaces in the vectors listed below.
    auto      aRichSurfaceUtil   = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto      surfMgr            = description.surfaceManager();
    auto      aRMapmtPCSurf      = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtPCSurfaceName() );
    auto      aR2MapmtPCBorderSN = std::array{"RichGrandMaPMTPCBorderSurfaceIn", "RichGrandMaPMTPCBorderBackSurfaceIn"};

    const auto aGrandPmtSMLvName = aRichPmtGeoAux->RichGrandMaPmtSubMasterLVName();

    auto       aPmtSMLv          = volume( aGrandPmtSMLvName );
    const auto NumVolInPmtSM     = (int)( aPmtSMLv->GetNdaughters() );
    const auto PmtCompPvNotBuilt = ( NumVolInPmtSM == 0 );

    // First the pmt components
    if ( PmtCompPvNotBuilt ) {
      auto aPvRichMapmtAnode =
          BuildARichPmtPhysVolWithPVRet( aGrandPmtSMLvName, ( aRichPmtGeoAux->RichGrandMaPmtAnodeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTAnode0001TN );
      auto aPvRichMapmtQuartz =
          BuildARichPmtPhysVolWithPVRet( aGrandPmtSMLvName, ( aRichPmtGeoAux->RichGrandMaPmtQuartzLVName() ),
                                         RichPmtGeomTransformLabel::pvRichGrandPMTQuartzTN );
      auto aPvRichMapmtPhCathode =
          BuildARichPmtPhysVolWithPVRet( aGrandPmtSMLvName, ( aRichPmtGeoAux->RichGrandMaPmtPhCathodeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichGrandPMTPhCathodeTN );
      auto aPvRichMapmtSideEnv =
          BuildARichPmtPhysVolWithPVRet( aGrandPmtSMLvName, ( aRichPmtGeoAux->RichGrandMaPmtSideEnvelopeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichGrandPMTSideEnvelopeTN );
      auto aPvRichMapmtBackEnv =
          BuildARichPmtPhysVolWithPVRet( aGrandPmtSMLvName, ( aRichPmtGeoAux->RichGrandMaPmtBackEnvelopeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichGrandPMTBackEnvelopeTN );
      BuildARichPmtPhysVol( aGrandPmtSMLvName, ( aRichPmtGeoAux->RichGrandMaPmtFrontRingLVName() ),
                            RichPmtGeomTransformLabel::pvRichGrandPMTFrontRingTN );

      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtAnode )]   = aPvRichMapmtAnode;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtQW )]      = aPvRichMapmtQuartz;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtSideEnv )] = aPvRichMapmtSideEnv;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtBackEnv )] = aPvRichMapmtBackEnv;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtPhc )]     = aPvRichMapmtPhCathode;

      if ( m_activate_Rich2PmtSurface_build ) {
        for ( int isPair = 0; isPair < aNumSurfPairInPmt; isPair++ ) {

          auto aR2MapmtPCBorderSurface =
              ( isPair == 0 ) ? dd4hep::BorderSurface( description, detector, ( aR2MapmtPCBorderSN[isPair] ),
                                                       aRMapmtPCSurf, aPvRichMapmtQuartz, aPvRichMapmtPhCathode )
                              : dd4hep::BorderSurface( description, detector, ( aR2MapmtPCBorderSN[isPair] ),
                                                       aRMapmtPCSurf, aPvRichMapmtPhCathode, aPvRichMapmtQuartz );

          aR2MapmtPCBorderSurface.isValid();
          if ( m_Rich2PmtSurfaceDebugActivate ) {
            printout( m_Rich2PmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                      "Rich2_Geo : ", "Created Rich2Pmt PC Surface with Name %s using %s",
                      ( aR2MapmtPCBorderSN[isPair] ), ( ( aRichSurfaceUtil->RichMapmtPCSurfaceName() ).c_str() ) );

            aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtPCSurf, RichSurfPropType::Refl );
            aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtPCSurf, RichSurfPropType::Effic );
          }
        }
      }

    } else {
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtAnode )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichPMTAnode0001TN ) ).c_str() );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtQW )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichGrandPMTQuartzTN ) ).c_str() );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtSideEnv )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichGrandPMTSideEnvelopeTN ) )
              .c_str() );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtBackEnv )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichGrandPMTBackEnvelopeTN ) )
              .c_str() );

      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichGrandPmtCompLabel::grPmtPhc )] = aPmtSMLv->GetNode(
          ( aRichPmtGeoAux->getRichPmtCompPhysVolName( RichPmtGeomTransformLabel::pvRichGrandPMTPhCathodeTN ) )
              .c_str() );
    }
    return aPmtCompPvVect;
  }

  //=====================================================================//

  dd4hep::PlacedVolume Rich2Build::BuildARich2PmtSMasterPhysVol( const std::string& aPmtMasterName, const int iM ) {
    // for now using all PMTSMaster is set to have the same name.
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto apVol          = ( aRichPmtGeoAux->IsRich2GrandPmtModule( iM ) )
                     ? BuildARichPmtPhysVolWithPVRet( aPmtMasterName, aRichPmtGeoAux->RichGrandMaPmtSubMasterLVName(),
                                                      RichPmtGeomTransformLabel::pvRichGrandPMTSMasterTN )
                     : BuildARichPmtPhysVolWithPVRet( aPmtMasterName, aRichPmtGeoAux->Rich2MaPmtSubMasterLVName(),
                                                      RichPmtGeomTransformLabel::pvRichPMTSMasterTN );
    return apVol;
  }
  //=====================================================================//
  dd4hep::PlacedVolume Rich2Build::BuildARich2PmtMasterPhysVol( int iM, int ec, int ip ) {
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();

    // int            aMe      = aRichPmtGeoAux->RhNumPmtInStdModule();
    // int            aPe      = aRichPmtGeoAux->RhNumPMTInECR();
    auto lvMother = volume( aRichPmtGeoAux->Rich2PmtECRHLogVolName( iM, ec ) );
    auto lvChild  = volume( aRichPmtGeoAux->Rich2PmtMasterLogVolName( iM, ec, ip ) );

    const auto TransformForChild = ( aRichPmtGeoAux->IsRich2GrandPmtModule( iM ) )
                                       ? ( aRichPmtGeoAux->Rich2GrandPmtMasterTransform() )
                                       : ( aRichPmtGeoAux->getRich2PmtMasterTransform( ip ) );

    //  int aChildCopyNumber = ( ( aRichPmtGeoAux->Rich2PmtModuleCopyNumber( iM ) ) * aMe ) + ( ec * aPe ) + ip;

    const auto aChildCopyNumber = aRichPmtGeoAux->getRich2PmtMasterCopyNumber( iM, ec, ip );

    auto pvChild = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
    pvChild->SetName( ( aRichPmtGeoAux->Rich2PmtMasterPhysVolName( iM, ec, ip ) ).c_str() );

    printout( m_pmtutilRich2DebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich2_Geo : ", "Created Rich2 PhysVol for MaPMT Master %d in Ec %d  in Module %d with CopyNumber %d", ip,
              ec, iM, aChildCopyNumber );

    const auto aR2PmtMasterDetName = aRichPmtGeoAux->Rich2PmtMasterDetName( iM, ec, ip );
    pvChild.addPhysVolID( aR2PmtMasterDetName + "Det", aChildCopyNumber );

    return pvChild;
  }
  //=====================================================================//
  dd4hep::PlacedVolume Rich2Build::BuildARich2PmtEcrhPhysVol( int iM, int ec ) {
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    int  aNe            = aRichPmtGeoAux->RhMaxNumECRInModule();
    auto lvMother       = volume( aRichPmtGeoAux->Rich2PmtModuleLogVolName( iM ) );
    auto lvChild        = volume( aRichPmtGeoAux->Rich2PmtECRHLogVolName( iM, ec ) );

    const auto TransformForChild = ( aRichPmtGeoAux->IsRich2GrandPmtModule( iM ) )
                                       ? ( aRichPmtGeoAux->getRich2PmtECHTransform( ec ) )
                                       : ( aRichPmtGeoAux->getRich2PmtECRTransform( ec ) );

    const auto aChildCopyNumber = ( ( aRichPmtGeoAux->Rich2PmtModuleCopyNumber( iM ) ) * aNe ) + ec;

    auto pvChild = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
    pvChild->SetName( ( aRichPmtGeoAux->Rich2PmtECRHPhysVolName( iM, ec ) ).c_str() );
    printout( m_pmtutilRich2DebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich2_Geo : ", "Created Rich2 PhysVol for MaPMT Ec %d in Module %d with copyNumber %d", ec, iM,
              aChildCopyNumber );
    const auto aR2PmtEcrhDetName = aRichPmtGeoAux->Rich2PmtECRHDetName( iM, ec );
    pvChild.addPhysVolID( aR2PmtEcrhDetName + "Det", aChildCopyNumber );

    return pvChild;
  }
  //=====================================================================//
  dd4hep::PlacedVolume Rich2Build::BuildARich2PmtModulePhysVol( int iM ) {
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto lvMother       = ( iM < aRichPmtGeoAux->Rh2TotalNumPmtModulesInPanel() ) ? volume( "lvRich2PhDetPanel0" )
                                                                            : volume( "lvRich2PhDetPanel1" );
    auto       lvChild           = volume( aRichPmtGeoAux->Rich2PmtModuleLogVolName( iM ) );
    const auto TransformForChild = aRichPmtGeoAux->getRich2PmtModuleTransform( iM );
    const auto aChildCopyNumber  = aRichPmtGeoAux->Rich2PmtModuleCopyNumber( iM );
    auto       pvChild           = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
    pvChild->SetName( ( aRichPmtGeoAux->Rich2PmtModulePhysVolName( iM ) ).c_str() );
    printout( m_pmtutilRich2DebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich2_Geo : ", "Created Rich2 PhysVol for MaPMT Module %d with CopyNumber %d", iM, aChildCopyNumber );
    // Now for the detector element structure
    // if(m_activate_Rich1_DetElem_For_CurrentAppl ) {
    const auto aR2PmtModuleDetName = aRichPmtGeoAux->Rich2PmtModuleDetName( iM );
    pvChild.addPhysVolID( aR2PmtModuleDetName + "Det", aChildCopyNumber );

    // }

    return pvChild;
  }
  //=====================================================================//
  void Rich2Build::build_Rich2_RichSystem() {
    auto       aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto aR2MLName     = aRich2GeoUtil->Rich2MasterLVName();
    const auto aR2SLName     = aRich2GeoUtil->Rich2RichSystemLVName();

    dd4hep::PlacedVolume aR2SyPv =
        BuildARich2PhysVolWithPVRet( aR2MLName, aR2SLName, Rich2GeomTransformLabel::pvRichSystemTN );
    if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
      const auto aR2SyDetName = aRich2GeoUtil->Rich2RichSystemDetName();
      const auto aR2SyId      = aRich2GeoUtil->getRich2PhysVolCopyNumber( Rich2GeomTransformLabel::pvRichSystemTN );
      aR2SyPv.addPhysVolID( aR2SyDetName + "Det", aR2SyId );
      dd4hep::DetElement aR2SyDet( detector, aR2SyDetName, aR2SyId );
      aR2SyDet.setPlacement( aR2SyPv );
    }
  }
  //=====================================================================//
  void Rich2Build::build_Rich2_MirrorSystem() {

    auto       aRich2GeoUtil   = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto aR2GasEnLName   = aRich2GeoUtil->Rich2GasEnclosureLVName();
    const auto aR2GasEnDetName = aRich2GeoUtil->Rich2GasEnclosureDetName();

    auto aSphMTN = std::array{Rich2GeomTransformLabel::pvRich2SphMirrorMaster0TN,
                              Rich2GeomTransformLabel::pvRich2SphMirrorMaster1TN};
    auto aSphSTN =
        std::array{Rich2GeomTransformLabel::pvRich2SphMSupport0TN, Rich2GeomTransformLabel::pvRich2SphMSupport1TN};
    auto aSecMTN = std::array{Rich2GeomTransformLabel::pvRich2SecMirrorMaster0TN,
                              Rich2GeomTransformLabel::pvRich2SecMirrorMaster1TN};
    auto aSecSTN =
        std::array{Rich2GeomTransformLabel::pvRich2SecMSupport0TN, Rich2GeomTransformLabel::pvRich2SecMSupport1TN};
    const auto                        aRich2NumSides = aRich2GeoUtil->Rich2NumSides();
    std::vector<dd4hep::PlacedVolume> aSphSegMasterPv( aRich2NumSides );
    std::vector<dd4hep::PlacedVolume> aSecSegMasterPv( aRich2NumSides );
    std::vector<dd4hep::DetElement>   aSphSegMasterDet( aRich2NumSides );
    std::vector<dd4hep::DetElement>   aSecSegMasterDet( aRich2NumSides );

    const auto aR2SphMSuLName = aRich2GeoUtil->Rich2SphMirrorSupportLVName();
    const auto aR2SecMSuLName = aRich2GeoUtil->Rich2SecMirrorSupportLVName();

    for ( int iSide = 0; iSide < aRich2NumSides; iSide++ ) {
      const auto aR2SphMMaLName = aRich2GeoUtil->Rich2SphMirrorMasterLVName( iSide );
      const auto aR2SecMMaLName = aRich2GeoUtil->Rich2SecMirrorMasterLVName( iSide );

      BuildARich2PhysVol( aR2SphMMaLName, aR2SphMSuLName, aSphSTN[iSide] );
      aSphSegMasterPv[iSide] = BuildARich2PhysVolWithPVRet( aR2GasEnLName, aR2SphMMaLName, aSphMTN[iSide] );
      aSecSegMasterPv[iSide] = BuildARich2PhysVolWithPVRet( aR2GasEnLName, aR2SecMMaLName, aSecMTN[iSide] );
      BuildARich2PhysVol( aR2SecMMaLName, aR2SecMSuLName, aSecSTN[iSide] );

      if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
        const auto aR2SphMMDetName = aRich2GeoUtil->Rich2SphMirrorMasterDetName( iSide );
        const auto aR2SphMMId      = aRich2GeoUtil->getRich2PhysVolCopyNumber( aSphMTN[iSide] );
        aSphSegMasterPv[iSide].addPhysVolID( aR2SphMMDetName + "Det", aR2SphMMId );
        aSphSegMasterDet[iSide] = dd4hep::DetElement( detector.child( aR2GasEnDetName ), aR2SphMMDetName, aR2SphMMId );
        aSphSegMasterDet[iSide].setPlacement( aSphSegMasterPv[iSide] );

        const auto aR2SecMMDetName = aRich2GeoUtil->Rich2SecMirrorMasterDetName( iSide );
        const auto aR2SecMMId      = aRich2GeoUtil->getRich2PhysVolCopyNumber( aSecMTN[iSide] );
        aSecSegMasterPv[iSide].addPhysVolID( aR2SecMMDetName + "Det", aR2SecMMId );
        aSecSegMasterDet[iSide] = dd4hep::DetElement( detector.child( aR2GasEnDetName ), aR2SecMMDetName, aR2SecMMId );
        aSecSegMasterDet[iSide].setPlacement( aSecSegMasterPv[iSide] );
      }
    }

    if ( m_activate_Rich2Mirror1_build ) build_Rich2_PrimarySph_MirrorSegments( aSphSegMasterPv, aSphSegMasterDet );

    if ( m_activate_Rich2Mirror2_build ) build_Rich2_Secondary_MirrorSegments( aSecSegMasterPv, aSecSegMasterDet );
  }
  //=====================================================================//
  void Rich2Build::build_Rich2_PrimarySph_MirrorSegments( const std::vector<dd4hep::PlacedVolume>& aSphSegMasterPv,
                                                          const std::vector<dd4hep::DetElement>&   aSphSegMasterDet ) {

    auto aRich2MirrorGeoAux = Rich2MirrorGeoAux::getRich2MirrorGeoAuxInstance();
    auto aRichSurfaceUtil   = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto surfMgr            = description.surfaceManager();

    aRich2MirrorGeoAux->BuildRich2SphMirrorShapes();
    aRich2MirrorGeoAux->BuildRich2SphMirrorTransforms();
    const auto aNumSphMirrSegments = aRich2MirrorGeoAux->NumRich2SphMirrSeg();
    const auto aSphMirrorMaterial  = description.material( "Rich:Rich2MirrorGlass" );

    for ( int iM = 0; iM < aNumSphMirrSegments; iM++ ) {

      auto       aSphMirrorShape     = aRich2MirrorGeoAux->Rich2SphMirrShape( iM );
      const auto aSphMirrorShapeName = aSphMirrorShape.name();

      const auto aSphMirrorLVName = aRich2MirrorGeoAux->Rich2SphMirrLVName( iM );

      dd4hep::Volume aSphLV( aSphMirrorLVName.c_str(), aSphMirrorShape, aSphMirrorMaterial );
      aSphLV.setVisAttributes( description, "Rich:VisRich2SphMirrSegment" );

      registerShape( std::string( aSphMirrorShapeName ), aSphMirrorShape );
      registerVolume( aSphMirrorLVName, aSphLV );
      auto curPvM = aSphSegMasterPv[( aRich2MirrorGeoAux->Rich2SphMirrorSide( iM ) )];

      const auto aMotherLVName = aRich2MirrorGeoAux->Rich2SphMirrSegMasterLVName( iM );

      auto alvMother = volume( aMotherLVName );

      const auto aCopyNum         = aRich2MirrorGeoAux->Rich2SphMirrCopyNumber( iM );
      const auto aSphSegTransform = aRich2MirrorGeoAux->Rich2SphMirrTransfom( iM );
      auto       pvSphSeg         = alvMother.placeVolume( aSphLV, aCopyNum, aSphSegTransform );
      pvSphSeg->SetName( ( aRich2MirrorGeoAux->Rich2SphMirrPVName( iM ) ).c_str() );
      if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
        const auto aR2SphSegDetName = aRich2MirrorGeoAux->Rich2SphMirrDetName( iM );
        const auto aR2SphSegId      = aCopyNum;
        pvSphSeg.addPhysVolID( aR2SphSegDetName + "Det", aR2SphSegId );
        dd4hep::DetElement aR2SphSegDet( aSphSegMasterDet[( aRich2MirrorGeoAux->Rich2SphMirrorSide( iM ) )],
                                         aR2SphSegDetName, aR2SphSegId );
        aR2SphSegDet.setPlacement( pvSphSeg );
      }

      // debug printout
      if ( m_debugRich2Activate ) {

        printout( m_debugRich2Mirror1Activate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich2_Geo : ", "Created SphMirrorSeg PhysVol with name %s , copynumber %d , from LogVol  %s ",
                  ( ( aRich2MirrorGeoAux->Rich2SphMirrPVName( iM ) ).c_str() ), aCopyNum,
                  ( aSphMirrorLVName.c_str() ) );
      }

      if ( m_activate_Rich2Surface_build ) {
        auto       aCurR2M1Surface = surfMgr.opticalSurface( aRichSurfaceUtil->Rich2Mirror1SurfaceName( iM ) );
        const auto aCurR2M1BorderSurfaceName = aRichSurfaceUtil->Rich2Mirror1BorderSurfaceName( iM );
        dd4hep::BorderSurface aR2M1SegBorderSurface( description, detector, aCurR2M1BorderSurfaceName, aCurR2M1Surface,
                                                     curPvM, pvSphSeg );
        aR2M1SegBorderSurface.isValid();

        // debug printouts
        printout( m_Rich2SurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich2_Geo : ", "Created Rich2Mirror1 Surface in segment %d with Name %s using %s", iM,
                  aCurR2M1BorderSurfaceName.c_str(), ( aRichSurfaceUtil->Rich2Mirror1SurfaceName( iM ) ).c_str() );
        if ( m_Rich2SurfaceDebugActivate && m_debugRich2Mirror1Activate ) {
          aRichSurfaceUtil->PrintRichSurfProperty( aCurR2M1Surface, RichSurfPropType::Refl );
          aRichSurfaceUtil->PrintRichSurfProperty( aCurR2M1Surface, RichSurfPropType::Effic );
        }
      }
    }
  }
  //=====================================================================//
  void Rich2Build::build_Rich2_Secondary_MirrorSegments( const std::vector<dd4hep::PlacedVolume>& aSecSegMasterPv,
                                                         const std::vector<dd4hep::DetElement>&   aSecSegMasterDet ) {

    auto aRich2MirrorGeoAux = Rich2MirrorGeoAux::getRich2MirrorGeoAuxInstance();
    auto aRichSurfaceUtil   = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto surfMgr            = description.surfaceManager();
    aRich2MirrorGeoAux->BuildRich2SecMirrorShapes();
    aRich2MirrorGeoAux->BuildRich2SecMirrorTransforms();
    const auto aNumSecMirrSegments = aRich2MirrorGeoAux->NumRich2SecMirrSeg();
    const auto aSecMirrorMaterial  = description.material( "Rich:Rich2MirrorGlass" );

    for ( int iM = 0; iM < aNumSecMirrSegments; iM++ ) {

      auto       aSecMirrorShape     = aRich2MirrorGeoAux->Rich2SecMirrShape( iM );
      const auto aSecMirrorShapeName = aSecMirrorShape.name();
      auto       curSecPvM           = aSecSegMasterPv[( aRich2MirrorGeoAux->Rich2SecMirrorSide( iM ) )];

      const auto aSecMirrorLVName = aRich2MirrorGeoAux->Rich2SecMirrLVName( iM );

      dd4hep::Volume aSecLV( aSecMirrorLVName.c_str(), aSecMirrorShape, aSecMirrorMaterial );
      aSecLV.setVisAttributes( description, "Rich:VisRich2SecMirrSegment" );

      registerShape( std::string( aSecMirrorShapeName ), aSecMirrorShape );
      registerVolume( aSecMirrorLVName, aSecLV );

      const auto aMotherLVName = aRich2MirrorGeoAux->Rich2SecMirrSegMasterLVName( iM );
      auto       alvMother     = volume( aMotherLVName );

      const auto aCopyNum         = aRich2MirrorGeoAux->Rich2SecMirrCopyNumber( iM );
      auto       aSecSegTransform = aRich2MirrorGeoAux->Rich2SecMirrTransfom( iM );
      auto       pvSecSeg         = alvMother.placeVolume( aSecLV, aCopyNum, aSecSegTransform );
      pvSecSeg->SetName( ( aRich2MirrorGeoAux->Rich2SecMirrPVName( iM ) ).c_str() );

      if ( m_activate_Rich2_DetElem_For_CurrentAppl ) {
        const auto aR2SecSegDetName = aRich2MirrorGeoAux->Rich2SecMirrDetName( iM );
        const auto aR2SecSegId      = aCopyNum;
        pvSecSeg.addPhysVolID( aR2SecSegDetName + "Det", aR2SecSegId );
        dd4hep::DetElement aR2SecSegDet( aSecSegMasterDet[( aRich2MirrorGeoAux->Rich2SecMirrorSide( iM ) )],
                                         aR2SecSegDetName, aR2SecSegId );
        aR2SecSegDet.setPlacement( pvSecSeg );
      }

      // debug printout
      if ( m_debugRich2Activate ) {
        printout( m_debugRich2Mirror2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich2_Geo : ", "Created SecMirrorSeg PhysVol with name %s , copynumber %d , from LogVol  %s ",
                  ( ( aRich2MirrorGeoAux->Rich2SecMirrPVName( iM ) ).c_str() ), aCopyNum,
                  ( aSecMirrorLVName.c_str() ) );
      }

      if ( m_activate_Rich2Surface_build ) {
        auto       aCurR2M2Surface = surfMgr.opticalSurface( aRichSurfaceUtil->Rich2Mirror2SurfaceName( iM ) );
        const auto aCurR2M2BorderSurfaceName = aRichSurfaceUtil->Rich2Mirror2BorderSurfaceName( iM );
        dd4hep::BorderSurface aR2M2SegBorderSurface( description, detector, aCurR2M2BorderSurfaceName, aCurR2M2Surface,
                                                     curSecPvM, pvSecSeg );
        aR2M2SegBorderSurface.isValid();

        // debug printouts
        printout( m_Rich2SurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich2_Geo : ", "Created Rich2Mirror2 Surface in segment %d with Name %s using %s", iM,
                  aCurR2M2BorderSurfaceName.c_str(), ( aRichSurfaceUtil->Rich2Mirror2SurfaceName( iM ) ).c_str() );
        if ( m_Rich2SurfaceDebugActivate && m_debugRich2Mirror2Activate ) {
          aRichSurfaceUtil->PrintRichSurfProperty( aCurR2M2Surface, RichSurfPropType::Refl );
          aRichSurfaceUtil->PrintRichSurfProperty( aCurR2M2Surface, RichSurfPropType::Effic );
        }
      }
    }
  }

  //=====================================================================//

  void Rich2Build::build_Rich2SuperStructureVols() {
    auto       aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    const auto aR2MLName     = aRich2GeoUtil->Rich2MasterLVName();

    for ( int iLS = 0; iLS < ( aRich2GeoUtil->Rich2NumSuperStrLateralSides() ); iLS++ ) {
      const auto aSuperStrMasterLVName = aRich2GeoUtil->Rich2SuperStrMasterLVName( iLS );
      BuildARich2PhysVol( aR2MLName, aSuperStrMasterLVName, ( aRich2GeoUtil->Rich2SuperStrMasterTN( iLS ) ) );

      if ( m_activate_Rich2SuperStructureComponent_build ) { BuildARich2SuperStrPhysVol( aSuperStrMasterLVName, iLS ); }
    }
  }

  //=====================================================================//

  void Rich2Build::build_Rich2MasterMainComponents() {
    if ( m_activate_Rich2GasEnclosure ) build_Rich2GasEnclosure();
    if ( m_activate_Rich2EntryExitWall ) build_Rich2EntryExitWalls();
    if ( m_activate_Rich2_MagneticShield_build ) build_Rich2MagShield();
    if ( m_activate_Rich2_PhotonDetector_build ) build_Rich2_PhDetSupFrame();
    if ( m_activate_Rich2_RichSystem_build ) build_Rich2_RichSystem();
    if ( m_activate_Rich2SuperStructure_build ) build_Rich2SuperStructureVols();
  }

  //=====================================================================//

  void Rich2Build::build_Rich2Master() {
    build_Rich2MasterMainComponents();

    // test volumes names
    const auto len = volumes.size();
    printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2_geo", " Number of Volumes  %d ",
              ( (int)len ) );
    if ( m_debugRich2LvListActivate ) {

      for ( auto iv = volumes.begin(); iv != volumes.end(); ++iv ) {
        auto vol     = ( ( *iv ).second.second );
        auto VolName = vol.name();

        printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich2_geo", " Volume name %s ", VolName );
        const auto aNumDaughters = (int)( ( static_cast<TGeoVolume*>( vol ) )->GetNdaughters() );
        const auto aNumNodes     = (int)( ( static_cast<TGeoVolume*>( vol ) )->CountNodes() );
        printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich2_geo",
                  "Number of daughters  nodes %d   %d ", aNumDaughters, aNumNodes );
        //(static_cast<TGeoVolume*> (vol))->PrintNodes();
      }
    }
  }
  //=====================================================================//
  void Rich2Build::build_Rich2_Main() {

    // sensitive.setType( "Rich" );
    dd4hep::PlacedVolume pv;
    // dd4hep::DetElement   deSubM;

    load( x_det, "include" );

    auto aRich2GeoUtil = Rich2GeoUtil::getRich2GeoUtilInstance();
    aRich2GeoUtil->setRh2DebugLevel( m_generalRich2UtilDebugActivate );
    aRich2GeoUtil->setRich2GeometryVersion( m_Rich2GeomVersion.Current_RichGeomVersion() );
    aRich2GeoUtil->InitRich2TransformsAndNames();

    printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2 Builder : Rich2 Geometry ",
              " Version %d   ", m_Rich2GeomVersion.Current_RichGeomVersion() );

    printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2_geo ", " Initialized Rich2GeoUtil " );

    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    aRichPmtGeoAux->setPmtDebugLevel( m_pmtutilRich2DebugActivate );

    auto aRichGeoTransAux = RichGeoTransAux::getRichGeoTransAuxInstance();
    aRichGeoTransAux->setRichGeoTransDebug( m_generalRich2UtilDebugActivate );
    const auto aNumR2F = aRichGeoTransAux->loadRichTransforms( x_det, "include", "Rich2" );
    printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2_geo ",
              " Uploaded Rich2Transforms for Rich2 in Rich2GeoUtil from %zu Rich2 Transform files ", aNumR2F );

    if ( !( aRichPmtGeoAux->IsRichStdPmtCompUploaded() ) ) {
      const auto aNumRPmF = aRichGeoTransAux->loadRichTransforms( x_det, "include", "RichPMT" );
      aRichPmtGeoAux->setRichNumStdPmtCompUploaded( aNumRPmF );
      printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2_geo ",
                " Uploaded RichTransforms for Rich StdPmt in RichPmtGeoAux from  %zu  "
                "RichPmt Transform Files ",
                aNumRPmF );

    } else {
      printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2_geo ",
                "  Rich StdPmt Transforms are already uploaded  from elsewhere for  %zu  "
                "RichPmt Transform Files ",
                ( aRichPmtGeoAux->RichNumStdPmtCompUploaded() ) );
    }

    if ( !( aRichPmtGeoAux->IsRichGrandPmtCompUploaded() ) ) {
      const auto aNumRGrPmF = aRichGeoTransAux->loadRichTransforms( x_det, "include", "RichGrandPMT" );
      aRichPmtGeoAux->setRichNumGrandPmtCompUploaded( aNumRGrPmF );
      printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2_geo ",
                " Uploaded RichTransforms for Rich GrandPmt in RichPmtGeoAux from  %zu  "
                "RichGrandPmt Transform Files ",
                aNumRGrPmF );

    } else {
      printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2_geo ",
                "  Rich GrandPmt Transforms are already uploaded  from elsewhere for  %zu  "
                "RichGrandPmt Transform Files ",
                ( aRichPmtGeoAux->RichNumGrandPmtCompUploaded() ) );
    }

    if ( m_activate_Rich2Surface_build || m_activate_Rich2PmtSurface_build ) {
      auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
      aRichSurfaceUtil->setRichSurfaceUtilDebug( m_Rich2SurfaceDebugActivate );
      aRichSurfaceUtil->InitRich2SurfNames();
    }

    buildVolumes( x_det );
    placeDaughters( detector, dd4hep::Volume(), x_det );
    build_Rich2Master();

    if ( m_activateRich2VolumeDebug ) {
      if ( !( m_attachRich2VolumeForDebug.empty() ) ) {
        select_Rich2_Volume = m_attachRich2VolumeForDebug;
        printout( m_debugRich2Activate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich2_geo ",
                  " Now selecting the volume %s for display and debug . ", select_Rich2_Volume.c_str() );
        if ( m_debugRich2Activate ) {
          std::cout << "Now selecting the volume for display and debug " << select_Rich2_Volume << std::endl;
        }
      }
    }
    auto vol = volume( select_Rich2_Volume );
    pv       = placeDetector( vol );
    if ( x_det.hasAttr( _U( id ) ) ) { pv.addPhysVolID( "system", x_det.id() ); }
  }

  //=====================================================================//

} // end namespace

//==========================================================================//
static dd4hep::Ref_t create_element_Rich_RUN3_v1( dd4hep::Detector& description, xml_h e,
                                                  dd4hep::SensitiveDetector sens_det ) {
  Rich2Build builder( description, e, sens_det, RichGeomLabel::Rich_RUN3_v1 );

  // printout( dd4hep::ALWAYS, " Rich2_geo V1",
  //               " Version %d   ", builder.m_Rich2GeomVersion.Current_RichGeomVersion()  );

  builder.build_Rich2_Main();
  return builder.detector;
}

//==========================================================================//
static dd4hep::Ref_t create_element_Rich_RUN3_v0( dd4hep::Detector& description, xml_h e,
                                                  dd4hep::SensitiveDetector sens_det ) {
  Rich2Build builder( description, e, sens_det, RichGeomLabel::Rich_RUN3_v0 );

  // printout( dd4hep::ALWAYS, " Rich2_geo V0",
  //                " Version %d   ", builder.m_Rich2GeomVersion.Current_RichGeomVersion()  );

  builder.build_Rich2_Main();
  return builder.detector;
}

//==========================================================================//

// The old versions are kept for backward compatibility.

DECLARE_DETELEMENT( LHCb_Rich2_Geometry_RUN3_v1, create_element_Rich_RUN3_v1 )

DECLARE_DETELEMENT( LHCb_Rich2_Geometry, create_element_Rich_RUN3_v0 )

//============================================================================
