//==========================================================================
//  LHCb Rich1 Detector description implementation using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-07-03
//
//==========================================================================

#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DD4hep/PropertyTable.h"
#include "Detector/Rich1/RichGeoTransAux.h"
#include "Detector/Rich1/RichGeoUtil.h"
#include "Detector/Rich1/RichGeomVersion.h"
#include "Detector/Rich1/RichMatOPD.h"
#include "Detector/Rich1/RichMatPropData.h"
#include "Detector/Rich1/RichPmtGeoAux.h"
#include "Detector/Rich1/RichSurfaceUtil.h"
#include "XML/Utilities.h"
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace {

  // Global parameters for activating various parts

  bool m_activate_MagneticShield_build       = true;
  bool m_activate_PhotonDetector_build       = true;
  bool m_activate_QuartzWindow_build         = true;
  bool m_activate_ExitWall_build             = true;
  bool m_activate_ExitWallDiaphram_build     = true;
  bool m_activate_Rich1Mirror1_build         = true;
  bool m_activate_Rich1Mirror1_Segment_build = true;
  bool m_activate_Rich1Mirror1_CaFiCyl_build = true;
  bool m_activate_Rich1Mirror2_build         = true;
  bool m_activate_Rich1SubMaster_build       = true;
  bool m_activate_Rich1BeamPipe_build        = true;

  bool m_activate_RichMatPropTable_build = true;

  bool m_activate_RichStd_MatPropTable_Attach      = true;
  bool m_activate_RichSpecific_MatPropTable_Attach = true;

  bool m_activate_RichMatGeneralPropTable_build = true;
  bool m_activate_RichMatPmtPT_ExtraSet_build   = false;

  bool m_activate_Rich1Surface_build    = true;
  bool m_activate_Rich1PmtSurface_build = true;
  bool m_activate_Rich1QWSurface_build  = true;

  bool m_activate_RetrieveAndPrintForDebug_MatTable     = false;
  bool m_activate_RetrieveAndPrintForDebug_GeneralTable = false;

  bool m_debugActivate                          = true;
  bool m_VolumeBuilderDebugActivate             = true;
  bool m_debugLvListActivate                    = true;
  bool m_generalutilDebugActivate               = true;
  bool m_pmtutilDebugActivate                   = true;
  bool m_R1MirrorDebugActivate                  = true;
  bool m_R1magShieldDebugActivate               = true;
  bool m_RichSurfaceDebugActivate               = true;
  bool m_RichPmtSurfaceDebugActivate            = true;
  bool m_activate_Rich1_DetElem_For_CurrentAppl = true;
  bool m_activate_Rich1_DetElem_For_PmtEcr      = true;

  bool        m_activateVolumeDebug = false;
  std::string m_attachVolumeForDebug;

  /// Helper class to build the Rich1 detector of LHCb
  struct Rich1Build final : public dd4hep::xml::tools::VolumeBuilder {

    inline static std::string select_Rich1_Volume{"lvRich1Master"};
    RichGeomVersion           m_Rich1GeomVersion;

    /// Initializing constructor
    Rich1Build( dd4hep::Detector& dsc, const xml_elt_t& e, const dd4hep::SensitiveDetector& sens, RichGeomLabel aRG )
        : dd4hep::xml::tools::VolumeBuilder( dsc, e, sens ), m_Rich1GeomVersion( RichGeomVersion() ) {
      m_Rich1GeomVersion.InitRichGeomVersion( aRG );

      xml_comp_t x_dbg = x_det.child( _U( debug ), false );
      if ( x_dbg ) {
        for ( xml_coll_t i( x_dbg, _U( item ) ); i; ++i ) {
          xml_comp_t  c( i );
          std::string aS = c.nameStr();

          if ( aS == "activate_MagneticShield_build" ) {
            m_activate_MagneticShield_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_PhotonDetector_build" ) {
            m_activate_PhotonDetector_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_QuartzWindow_build" ) {
            m_activate_QuartzWindow_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_ExitWall_build" ) {
            m_activate_ExitWall_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_ExitWallDiaphram_build" ) {
            m_activate_ExitWallDiaphram_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "debugActivate" ) {
            m_debugActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "VolumeBuilderDebugActivate" ) {
            m_VolumeBuilderDebugActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "debugLvListActivate" ) {
            m_debugLvListActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "generalutilDebugActivate" ) {
            m_generalutilDebugActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "pmtutilDebugActivate" ) {
            m_pmtutilDebugActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "R1MirrorDebugActivate" ) {
            m_R1MirrorDebugActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "R1magShieldDebugActivate" ) {
            m_R1magShieldDebugActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "RichSurfaceDebugActivate" ) {
            m_RichSurfaceDebugActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "RichPmtSurfaceDebugActivate" ) {
            m_RichPmtSurfaceDebugActivate = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1Mirror1_build" ) {
            m_activate_Rich1Mirror1_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1Mirror2_build" ) {
            m_activate_Rich1Mirror2_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1Mirror1_Segment_build" ) {
            m_activate_Rich1Mirror1_Segment_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1Mirror1_CaFiCyl_build" ) {
            m_activate_Rich1Mirror1_CaFiCyl_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1SubMaster_build" ) {
            m_activate_Rich1SubMaster_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1BeamPipe_build" ) {
            m_activate_Rich1BeamPipe_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activateVolumeDebug" ) {
            m_activateVolumeDebug = c.attr<bool>( _U( value ) );
          } else if ( aS == "attachVolumeForDebug" ) {
            m_attachVolumeForDebug = c.attr<std::string>( _U( value ) );
          } else if ( aS == "activate_RichMatPropTable_build" ) {
            m_activate_RichMatPropTable_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_RichStd_MatPropTable_Attach" ) {
            m_activate_RichStd_MatPropTable_Attach = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_RichSpecific_MatPropTable_Attach" ) {
            m_activate_RichSpecific_MatPropTable_Attach = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_RichMatGeneralPropTable_build" ) {
            m_activate_RichMatGeneralPropTable_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_RichMatPmtPT_ExtraSet_build" ) {
            m_activate_RichMatPmtPT_ExtraSet_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1Surface_build" ) {
            m_activate_Rich1Surface_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1PmtSurface_build" ) {
            m_activate_Rich1PmtSurface_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1QWSurface_build" ) {
            m_activate_Rich1QWSurface_build = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_RetrieveAndPrintForDebug_MatTable" ) {
            m_activate_RetrieveAndPrintForDebug_MatTable = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_RetrieveAndPrintForDebug_GeneralTable" ) {
            m_activate_RetrieveAndPrintForDebug_GeneralTable = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1_DetElem_For_CurrentAppl" ) {
            m_activate_Rich1_DetElem_For_CurrentAppl = c.attr<bool>( _U( value ) );
          } else if ( aS == "activate_Rich1_DetElem_For_PmtEcr" ) {
            m_activate_Rich1_DetElem_For_PmtEcr = c.attr<bool>( _U( value ) );
          }
        }
      }
      debug = m_VolumeBuilderDebugActivate;
    }

    /// Rich1 build methods

    void build_Rich1Mirror1();
    void build_Rich1Mirror1CaFiCyl();
    void build_Rich1Mirror2();

    void build_Rich1Mirror2_Version_Rich_RUN3_v0();

    void build_Rich1PhDetSupFrames();
    void build_Rich1GasQuartzWindows();
    void build_Rich1PmtFullSystem();
    void build_Rich1PmtModuleVols();
    void build_Rich1PmtECRVols();
    void build_Rich1PmtMasterVols();
    void build_Rich1MagSh();
    void build_Rich1MagneticShieldComponents();
    void build_Rich1ExitWall();
    void build_Rich1SubMaster();
    void build_Rich1Master();

    void build_Rich_OpticalProperties();
    void build_Rich_General_Properties();

    void build_RichSingle_QE_Table( RichMatNameWOP aMatNameW, RichMatPropType aPropType, RichMatQE aQEType );
    void build_Rich_SingleMaterial_StdOpticalProperty( RichMatNameWOP aMatNameW, RichMatPropType aPropType );
    void Attach_RichMaterial_OpticalPropertiesTable( RichMatNameWOP aMatNameW, RichMatPropType aPropType );
    void Attach_RichMaterial_ScintPropertiesTable( RichMatNameWOP aMatNameW, RichMatPropType aPropType,
                                                   RichMatScint aScType );

    void build_RichMaterial_OpticalProperties();
    void RetrieveRichMatProperty( RichMatNameWOP aMatNameW, RichMatPropType aPropType );
    void RetriveAndPrintRichPmtQE( RichMatPropType aPropType, RichMatQE aQEType );
    void RetriveAndPrintRichPmtHV( RichMatPropType aPropType, RichMatHV aHvType );
    void RetriveAndPrintRichGasQWARCoatingRefl( RichMatPropType aPropType, RichSurfCoat aARType );

    void build_Rich_PmtHV( RichMatNameWOP aMatNameW, RichMatPropType aPropType, RichMatHV ahvType );
    void build_Rich_ScProp( RichMatNameWOP aMatNameW, RichMatPropType aPropType, RichMatScint aScType );
    void build_Rich_QWARCoatRefl( RichMatNameWOP aMatNameW, RichMatPropType aPropType, RichSurfCoat aARType );

    void build_Rich1_Main();

    void BuildARich1PhysVol( const std::string& aMotherLVName, const std::string& aChildLVName,
                             const RichGeomTransformLabel aTransformLabel );

    void BuildARichPmtPhysVol( const std::string& aMotherLVName, const std::string& aChildLVName,
                               const RichPmtGeomTransformLabel aTransformLabel );

    dd4hep::PlacedVolume BuildARich1PhysVolWithPVRet( const std::string& aMotherLVName, const std::string& aChildLVName,
                                                      const RichGeomTransformLabel aTransformLabel );
    dd4hep::PlacedVolume BuildARichPmtPhysVolWithPVRet( const std::string&              aMotherLVName,
                                                        const std::string&              aChildLVName,
                                                        const RichPmtGeomTransformLabel aTransformLabel );
    std::vector<dd4hep::PlacedVolume> BuildRich1StdMapmtCompPhysVols();

    dd4hep::PlacedVolume BuildARich1PmtModulePhysVol( int iModule );

    dd4hep::PlacedVolume BuildARich1PmtEcrPhysVol( int aModule, int aEcr );
    dd4hep::PlacedVolume BuildARich1PmtMasterPhysVol( int iModule, int aEcr, int aPmt );
    dd4hep::PlacedVolume BuildARich1PmtSMasterPhysVol( const std::string& aPmtMasterName );
    void                 BuildARich1PmtShieldingPhysVol( const std::string& aEcrName );
    void                 build_Rich1PmtAllPhysVols();
  };

  //=====================================================================================//
  void Rich1Build::BuildARich1PhysVol( const std::string& aMotherLVName, const std::string& aChildLVName,
                                       const RichGeomTransformLabel aTransformLabel ) {

    auto aPvol = BuildARich1PhysVolWithPVRet( aMotherLVName, aChildLVName, aTransformLabel );
    if ( m_debugActivate ) {
      const auto aPvolName = aPvol->GetName();

      printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                "Rich1_Geo : ", "Created Rich1 PhysVol with the name  %s", aPvolName );
    }
  }
  //=====================================================================================//
  dd4hep::PlacedVolume Rich1Build::BuildARich1PhysVolWithPVRet( const std::string&           aMotherLVName,
                                                                const std::string&           aChildLVName,
                                                                const RichGeomTransformLabel aTransformLabel ) {
    auto       lvMother     = volume( aMotherLVName );
    auto       lvChild      = volume( aChildLVName );
    auto       aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();
    const auto TransformForChild =
        aRichGeoUtil->getRichGeomTransform( aRichGeoUtil->getRichTransformName( aTransformLabel ) );

    const auto aChildCopyNumber = aRichGeoUtil->getPhysVolCopyNumber( aTransformLabel );

    auto pvChild = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
    // dd4hep::PlacedVolume pvChild =  lvMother.placeVolume(lvChild,TransformForChild);

    pvChild->SetName( ( aRichGeoUtil->getRichPhysVolName( aTransformLabel ) ).c_str() );

    // Now setup debug printout options

    if ( m_debugActivate ) {
      bool aCdebug = m_debugActivate;

      if ( aChildLVName.find( "lvRich1Mgs" ) != std::string::npos ) aCdebug = m_R1magShieldDebugActivate;
      printout( aCdebug ? dd4hep::ALWAYS : dd4hep::DEBUG,
                "Rich1_Geo : ", "Created PhysVol with name %s , copynumber %d , TransformName  %s ",
                ( aRichGeoUtil->getRichPhysVolName( aTransformLabel ) ).c_str(), aChildCopyNumber,
                ( aRichGeoUtil->getRichTransformName( aTransformLabel ) ).c_str() );
    }
    return pvChild;
  }
  //=====================================================================================//
  void Rich1Build::BuildARichPmtPhysVol( const std::string& aMotherLVName, const std::string& aChildLVName,
                                         const RichPmtGeomTransformLabel aTransformLabel ) {

    auto aPvol = BuildARichPmtPhysVolWithPVRet( aMotherLVName, aChildLVName, aTransformLabel );
    if ( m_debugActivate ) {
      const auto aPvolName = aPvol->GetName();
      printout( m_pmtutilDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                "Rich1_Geo : ", "Created Rich1 Pmt PhysVol with the name  %s", aPvolName );
    }
  }

  //=====================================================================================//
  dd4hep::PlacedVolume Rich1Build::BuildARichPmtPhysVolWithPVRet( const std::string&              aMotherLVName,
                                                                  const std::string&              aChildLVName,
                                                                  const RichPmtGeomTransformLabel aTransformLabel ) {
    auto lvMother       = volume( aMotherLVName );
    auto lvChild        = volume( aChildLVName );
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto TransformForChild =
        aRichPmtGeoAux->getRichPmtCompGeomTransform( aRichPmtGeoAux->getRichPmtCompTransformName( aTransformLabel ) );

    const auto aChildCopyNumber = aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( aTransformLabel );

    auto pvChild = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
    // dd4hep::PlacedVolume pvChild =  lvMother.placeVolume(lvChild,TransformForChild);

    pvChild->SetName( ( aRichPmtGeoAux->getRichPmtCompPhysVolName( aTransformLabel ) ).c_str() );

    // Now setup debug printout options
    if ( m_debugActivate ) {
      printout( m_pmtutilDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                "Rich1_Geo : ", "Created Pmt PhysVol with name %s , copynumber %d , TransformName  %s ",
                ( aRichPmtGeoAux->getRichPmtCompPhysVolName( aTransformLabel ) ).c_str(), aChildCopyNumber,
                ( aRichPmtGeoAux->getRichPmtCompTransformName( aTransformLabel ) ).c_str() );
    }

    return pvChild;
  }

  //=====================================================================================//
  dd4hep::PlacedVolume Rich1Build::BuildARich1PmtModulePhysVol( int iModule ) {
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();

    auto lvMother = ( iModule < aRichPmtGeoAux->Rh1TotalNumPmtModulesInPanel() ) ? volume( "lvRich1PhDetSupFrameH0" )
                                                                                 : volume( "lvRich1PhDetSupFrameH1" );

    auto lvChild = volume( aRichPmtGeoAux->Rich1PmtModuleLogVolName( iModule ) );
    // dd4hep::Volume  lvChild  = aPmtModuleLV;
    auto TransformForChild = aRichPmtGeoAux->getRich1PmtModuleTransform( iModule );
    auto aChildCopyNumber  = iModule;
    auto pvChild           = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
    pvChild->SetName( ( aRichPmtGeoAux->Rich1PmtModulePhysVolName( iModule ) ).c_str() );
    // Now for the detector element structure
    // if(m_activate_Rich1_DetElem_For_CurrentAppl ) {
    const auto aR1PmtModuleDetName = aRichPmtGeoAux->Rich1PmtModuleDetName( iModule );
    pvChild.addPhysVolID( aR1PmtModuleDetName + "Det", aChildCopyNumber );

    // }

    printout( m_pmtutilDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich1_Geo : ", "Created Rich1 PhysVol for MaPMT Module %d", iModule );

    return pvChild;
  }

  //=====================================================================================//
  dd4hep::PlacedVolume Rich1Build::BuildARich1PmtEcrPhysVol( int aModule, int aEcr ) {
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto aNe            = aRichPmtGeoAux->RhMaxNumECRInModule();

    auto lvMother = volume( aRichPmtGeoAux->Rich1PmtModuleLogVolName( aModule ) );
    auto lvChild  = volume( aRichPmtGeoAux->Rich1PmtECRLogVolName( aModule, aEcr ) );

    auto TransformForChild = aRichPmtGeoAux->getRich1PmtECRTransform( aEcr );

    const auto aChildCopyNumber = ( aModule * aNe ) + aEcr;

    auto pvChild = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
    pvChild->SetName( ( aRichPmtGeoAux->Rich1PmtECRPhysVolName( aModule, aEcr ) ).c_str() );
    const auto aR1PmtEcrDetName = aRichPmtGeoAux->Rich1PmtECRDetName( aModule, aEcr );
    pvChild.addPhysVolID( aR1PmtEcrDetName + "Det", aChildCopyNumber );

    printout( m_pmtutilDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich1_Geo : ", "Created Rich1 PhysVol for MaPMT Ecr %d in Module %d", aEcr, aModule );
    return pvChild;
  }

  //=====================================================================================//

  dd4hep::PlacedVolume Rich1Build::BuildARich1PmtMasterPhysVol( int aModule, int aEca, int aPmt ) {

    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    // int            aMe            = aRichPmtGeoAux->RhNumPmtInStdModule();
    // int            aPe            = aRichPmtGeoAux->RhNumPMTInECR();
    auto lvMother = volume( aRichPmtGeoAux->Rich1PmtECRLogVolName( aModule, aEca ) );
    auto lvChild  = volume( aRichPmtGeoAux->Rich1PmtMasterLogVolName( aModule, aEca, aPmt ) );

    auto TransformForChild = aRichPmtGeoAux->getRich1PmtMasterTransform( aPmt );

    // int aChildCopyNumber = ( aModule * aMe ) + ( aEca * aPe ) + aPmt;
    const auto aChildCopyNumber = aRichPmtGeoAux->getRich1PmtMasterCopyNumber( aModule, aEca, aPmt );

    auto pvChild = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );

    pvChild->SetName( ( aRichPmtGeoAux->Rich1PmtMasterPhysVolName( aModule, aEca, aPmt ) ).c_str() );
    const auto aR1PmtMasterDetName = aRichPmtGeoAux->Rich1PmtMasterDetName( aModule, aEca, aPmt );
    pvChild.addPhysVolID( aR1PmtMasterDetName + "Det", aChildCopyNumber );

    printout( m_pmtutilDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich1_Geo : ", "Created Rich1 PhysVol for MaPMT Master %d in Ecr %d  in Module %d", aPmt, aEca,
              aModule );
    return pvChild;
  }

  //=====================================================================================//

  dd4hep::PlacedVolume Rich1Build::BuildARich1PmtSMasterPhysVol( const std::string& aPmtMasterName ) {

    //  The following line commented out and replaced by the lines below until it is determined
    // if each pv for pmtSMaster need to have a different name.
    //   BuildARichPmtPhysVol( aPmtMasterName,"lvRichPMTSMaster",
    //			RichPmtGeomTransformLabel::pvRichPMTSMasterTN);

    auto lvMother         = volume( aPmtMasterName );
    auto lvChild          = volume( "lvRichPMTSMaster" );
    auto aRichPmtGeoAux   = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto aPmtSMasterLabel = RichPmtGeomTransformLabel::pvRichPMTSMasterTN;
    auto TransformForChild =
        aRichPmtGeoAux->getRichPmtCompGeomTransform( aRichPmtGeoAux->getRichPmtCompTransformName( aPmtSMasterLabel ) );
    const auto aChildCopyNumber = aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( aPmtSMasterLabel );

    auto pvSMaster = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );

    // Possibility here to give different physvol names to different PmtSMaster in different pmt master.
    // For that one may need extra arguements for the module ecr numbers etc to this method.
    // for now same name is assigned.
    pvSMaster->SetName( ( aRichPmtGeoAux->getRichPmtCompPhysVolName( aPmtSMasterLabel ) ).c_str() );

    printout( m_pmtutilDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich1_Geo : ", "Created Rich1 Pmt PhysVol for MaPMT SMaster in PmtMaster %s", aPmtMasterName.c_str() );

    return pvSMaster;
  }

  //=====================================================================================//

  void Rich1Build::BuildARich1PmtShieldingPhysVol( const std::string& aEcrVolName ) {

    BuildARichPmtPhysVol( aEcrVolName, "lvRichPmtShieldingLongPlate",
                          RichPmtGeomTransformLabel::pvRichPmtSingleShieldingLongPlateTN );
    BuildARichPmtPhysVol( aEcrVolName, "lvRichPmtShieldingSemiPlate",
                          RichPmtGeomTransformLabel::pvRichPmtSingleShieldingSemiPlateTopTN );
    BuildARichPmtPhysVol( aEcrVolName, "lvRichPmtShieldingSemiPlate",
                          RichPmtGeomTransformLabel::pvRichPmtSingleShieldingSemiPlateBottomTN );

    printout( m_pmtutilDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
              "Rich1_Geo : ", "Created Rich1 PhysVol for MaPMT Shielding Vols in PmtEcr %s", aEcrVolName.c_str() );
  }

  //=====================================================================================//

  void Rich1Build::build_Rich1Mirror1() {

    auto aRichGeoUtil     = RichGeoUtil::getRichGeoUtilInstance();
    auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto surfMgr          = description.surfaceManager();

    dd4hep::Volume lvMother = volume( aRichGeoUtil->Rich1Mirror1MasterLVName() );

    for ( int iq = 0; iq < ( aRichGeoUtil->Rich1NumMirror1Seg() ); iq++ ) {
      auto       lvD           = volume( aRichGeoUtil->Rich1Mirror1LogVolName( 0, iq ) );
      auto       TransformForD = aRichGeoUtil->getRichGeomTransform( aRichGeoUtil->Rich1Mirror1TransformName( 0, iq ) );
      const auto aDCopyNumber  = aRichGeoUtil->Rich1Mirror1CopyNumber( 0, iq );
      auto       pvD           = lvMother.placeVolume( lvD, aDCopyNumber, TransformForD );
      pvD->SetName( ( aRichGeoUtil->Rich1Mirror1PhysVolName( 0, iq ) ).c_str() );
      // Now for DetElem structure for this quadrant
      std::optional<dd4hep::DetElement> aRich1M1QDet;
      if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
        const auto aR1M1QDetName = aRichGeoUtil->Rich1Mirror1DetName( 0, iq );
        pvD.addPhysVolID( aR1M1QDetName + "Det", aDCopyNumber );
        aRich1M1QDet.emplace( detector, aR1M1QDetName, aDCopyNumber );
        aRich1M1QDet->setPlacement( pvD );
      }

      if ( m_activate_Rich1Mirror1_Segment_build ) {
        for ( int itp = 1; itp < ( aRichGeoUtil->Rich1NumMirror1VolTypes() ); itp++ ) {
          auto lvChild = volume( aRichGeoUtil->Rich1Mirror1LogVolName( itp, iq ) );
          auto TransformForChild =
              aRichGeoUtil->getRichGeomTransform( aRichGeoUtil->Rich1Mirror1TransformName( itp, iq ) );
          const auto aChildCopyNumber = aRichGeoUtil->Rich1Mirror1CopyNumber( itp, iq );
          auto       pvChild          = lvD.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
          pvChild->SetName( ( aRichGeoUtil->Rich1Mirror1PhysVolName( itp, iq ) ).c_str() );
          printout( m_R1MirrorDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                    "Rich1_Geo : ", "Created Rich1 PhysVol for Rich1 Mirror1 Segment Vol %s in %d ",
                    ( aRichGeoUtil->Rich1Mirror1PhysVolName( itp, iq ) ).c_str(), iq );

          // Now for DetElem structure for the inner layer of this segment
          if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
            if ( itp == 1 ) {
              const auto aR1M1CfDetName = aRichGeoUtil->Rich1Mirror1DetName( itp, iq );
              pvChild.addPhysVolID( aR1M1CfDetName + "Det", aChildCopyNumber );
              assert( aRich1M1QDet.has_value() );
              auto aRich1M1CfDet = dd4hep::DetElement( aRich1M1QDet.value(), aR1M1CfDetName, aChildCopyNumber );
              aRich1M1CfDet.setPlacement( pvChild );
            }
          }
          // Create the Rich1 Mirror1 surfaces
          if ( m_activate_Rich1Surface_build ) {
            if ( itp == 1 ) {
              auto       aCurM1Surface = surfMgr.opticalSurface( aRichSurfaceUtil->Rich1Mirror1SurfaceName( iq ) );
              const auto aCurBorderSurfaceName = aRichSurfaceUtil->Rich1Mirror1BorderSurfaceName( iq );
              auto       aR1M1QuadBorderSurface =
                  dd4hep::BorderSurface( description, detector, aCurBorderSurfaceName, aCurM1Surface, pvD, pvChild );
              aR1M1QuadBorderSurface.isValid();

              printout( m_RichSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                        "Rich1_Geo : ", "Created Rich1Mirror1 Surface in quadrant %d with Name %s using %s", iq,
                        aCurBorderSurfaceName.c_str(), ( aRichSurfaceUtil->Rich1Mirror1SurfaceName( iq ) ).c_str() );
              if ( m_RichSurfaceDebugActivate && m_R1MirrorDebugActivate ) {
                aRichSurfaceUtil->PrintRichSurfProperty( aCurM1Surface, RichSurfPropType::Refl );
                aRichSurfaceUtil->PrintRichSurfProperty( aCurM1Surface, RichSurfPropType::Effic );
              }
            }
          }
        }
      }
    }

    if ( m_activate_Rich1Mirror1_CaFiCyl_build ) build_Rich1Mirror1CaFiCyl();

    BuildARich1PhysVol( ( aRichGeoUtil->Rich1SubMasterLVName() ), ( aRichGeoUtil->Rich1Mirror1MasterLVName() ),
                        RichGeomTransformLabel::pvRich1Mirror1MasterTN );
  }
  //=====================================================================================//
  void Rich1Build::build_Rich1Mirror1CaFiCyl() {

    auto aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();

    auto lvCylChild = volume( aRichGeoUtil->Rich1Mirror1CaFiLVName() );

    for ( int iQ = 0; iQ < ( aRichGeoUtil->Rich1NumMirror1Seg() ); iQ++ ) {
      auto lvMQ = volume( aRichGeoUtil->Rich1Mirror1LogVolName( 0, iQ ) );
      for ( int ic = 0; ic < ( aRichGeoUtil->Rich1Mirror1NumCaFiCylInASeg() ); ++ic ) {
        auto       TransformForCylChild = aRichGeoUtil->Rich1Mirror1CaFiCylinderTransform( iQ, ic );
        const auto aCylChildCopyNumber  = aRichGeoUtil->Rich1Mirror1CaFiCylinderCopyNumber( iQ, ic );
        auto       pvCylChild           = lvMQ.placeVolume( lvCylChild, aCylChildCopyNumber, TransformForCylChild );
        pvCylChild->SetName( ( aRichGeoUtil->Rich1Mirror1CaFiCylinderPhysVolName( iQ, ic ) ).c_str() );
        printout( m_R1MirrorDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich1_Geo : ", "Created Rich1 PhysVol for Rich1 Mirror1 CaFi Vol %d in quadrant %d ", ic, iQ );
      }
    }
  }
  //=====================================================================================//

  void Rich1Build::build_Rich1Mirror2() {

    auto aRichGeoUtil     = RichGeoUtil::getRichGeoUtilInstance();
    auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto surfMgr          = description.surfaceManager();

    const auto aNumM2Systems         = aRichGeoUtil->Rich1NumMirror2Systems();
    const auto aNumSegInHalf         = aRichGeoUtil->Rich1NumMirror2SegInAHalf();
    const auto aR1SubMLVName         = aRichGeoUtil->Rich1SubMasterLVName();
    const auto aNumM2QuadrantsInHalf = aRichGeoUtil->Rich1NumMirror2QuadrantsInHalf();

    for ( int iMM = 0; iMM < aNumM2Systems; iMM++ ) {
      const auto aRich1Mirror2MasterLVName = aRichGeoUtil->Rich1Mirror2MasterLVName( iMM );
      // auto       lvMother                  = volume( aRich1Mirror2MasterLVName );

      auto aR1M2MasterTN = ( iMM == 0 ) ? ( RichGeomTransformLabel::pvRich1Mirror2MasterTopTN )
                                        : ( RichGeomTransformLabel::pvRich1Mirror2MasterBotTN );
      auto pvRich1Mirror2CurMaster =
          BuildARich1PhysVolWithPVRet( aR1SubMLVName, aRich1Mirror2MasterLVName, aR1M2MasterTN );

      auto aR1M2SupTN = ( iMM == 0 ) ? ( RichGeomTransformLabel::pvRich1Mirror2SupportTopTN )
                                     : ( RichGeomTransformLabel::pvRich1Mirror2SupportBotTN );

      BuildARich1PhysVol( aR1SubMLVName, ( aRichGeoUtil->Rich1Mirror2SupportLVName( iMM ) ), aR1M2SupTN );
      // Now for the detector element structure for the current Rich1 Mirror2 master
      std::optional<dd4hep::DetElement> aRich1M2MasterDet;
      if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
        const auto aR1M2MasterDetName = aRichGeoUtil->Rich1Mirror2MasterDetName( iMM );
        const auto aR1M2MasterDetId   = aRichGeoUtil->getPhysVolCopyNumber( aR1M2MasterTN );
        pvRich1Mirror2CurMaster.addPhysVolID( aR1M2MasterDetName + "Det", aR1M2MasterDetId );
        aRich1M2MasterDet.emplace( detector, aR1M2MasterDetName, aR1M2MasterDetId );
        aRich1M2MasterDet->setPlacement( pvRich1Mirror2CurMaster );
      }
      // Now for the Mirror2 quadrant sub master
      for ( int iQh = 0; iQh < aNumM2QuadrantsInHalf; iQh++ ) {
        int iQ = ( iMM == 0 ) ? iQh : ( iQh + aNumM2QuadrantsInHalf );
        // const auto aM2Sys = aRichGeoUtil->Rich2Mirror2QuadInM2Sys(iQ);
        const auto aRich1Mirror2QuadrantMasterLVName = aRichGeoUtil->Rich1Mirror2QuadrantMasterLVName( iQ );
        auto       aRich1Mirror2QuadrantMasterLV     = volume( aRich1Mirror2QuadrantMasterLVName );
        auto       aR1M2QuadrantMasterTN             = aRichGeoUtil->getR1M2QuadrantMasterTN( iQ );
        auto       pvRich1Mirror2CurQuadrantMaster   = BuildARich1PhysVolWithPVRet(
            aRich1Mirror2MasterLVName, aRich1Mirror2QuadrantMasterLVName, aR1M2QuadrantMasterTN );
        printout( m_R1MirrorDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich1_Geo : ", "Created Rich1 PhysVol for Rich1 Mirror2 quadrantmaster %d in Half %d ", iQ, iMM );

        if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
          const auto aR1M2QuadrantMasterDetName = aRichGeoUtil->Rich1Mirror2QuadrantMasterDetName( iQ );
          const auto aR1M2QuadrantMasterDetId   = aRichGeoUtil->getPhysVolCopyNumber( aR1M2QuadrantMasterTN );
          pvRich1Mirror2CurQuadrantMaster.addPhysVolID( aR1M2QuadrantMasterDetName + "Det", aR1M2QuadrantMasterDetId );
          assert( aRich1M2MasterDet.has_value() );
          auto aRich1M2QuadDet =
              dd4hep::DetElement( aRich1M2MasterDet.value(), aR1M2QuadrantMasterDetName, aR1M2QuadrantMasterDetId );
          aRich1M2QuadDet.setPlacement( pvRich1Mirror2CurQuadrantMaster );
        }

        for ( int isegQ = 0; isegQ < ( aNumSegInHalf / 2 ); isegQ++ ) {
          auto iseg = ( iQh == 0 ) ? isegQ : ( isegQ + ( aNumSegInHalf / 2 ) );
          if ( iMM == 1 ) {
            iseg =
                ( iseg < ( aNumSegInHalf / 2 ) ) ? ( iseg + ( aNumSegInHalf / 2 ) ) : ( iseg - ( aNumSegInHalf / 2 ) );
          }
          const auto isegF   = ( iMM * aNumSegInHalf ) + iseg;
          auto       lvChild = volume( aRichGeoUtil->Rich1Mirror2LogVolName( isegF ) );
          const auto TransformForChild =
              aRichGeoUtil->getRichGeomTransform( aRichGeoUtil->Rich1Mirror2TransformName( isegF ) );
          const auto aChildCopyNumber = aRichGeoUtil->Rich1Mirror2CopyNumber( isegF );
          auto pvChild = aRich1Mirror2QuadrantMasterLV.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
          pvChild->SetName( ( aRichGeoUtil->Rich1Mirror2PhysVolName( isegF ) ).c_str() );

          printout( m_R1MirrorDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich1_Geo : ",
                    "Created Rich1 PhysVol for Rich1 Mirror2 Segment %d  iseg %d in quadrant %d in Half %d ", iseg,
                    isegF, iQ, iMM );

          // Now for the detector element structure for the current Rich1 Mirror2 segment
          if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
            const auto aR1M2SegDetName = aRichGeoUtil->Rich1Mirror2DetName( isegF );
            pvChild.addPhysVolID( aR1M2SegDetName + "Det", aChildCopyNumber );
            assert( aRich1M2MasterDet.has_value() );
            auto aRich1M2SegDet = dd4hep::DetElement( aRich1M2MasterDet.value(), aR1M2SegDetName, aChildCopyNumber );
            aRich1M2SegDet.setPlacement( pvChild );
          }
          // Now create Rich1Mirror2 surface
          if ( m_activate_Rich1Surface_build ) {

            auto       aCurM2Surface = surfMgr.opticalSurface( aRichSurfaceUtil->Rich1Mirror2SurfaceName( isegF ) );
            const auto aCurM2BorderSurfaceName = aRichSurfaceUtil->Rich1Mirror2BorderSurfaceName( isegF );
            auto       aR1M2SegBorderSurface =
                dd4hep::BorderSurface( description, detector, aCurM2BorderSurfaceName, aCurM2Surface,
                                       pvRich1Mirror2CurQuadrantMaster, pvChild );
            aR1M2SegBorderSurface.isValid();
            printout( m_RichSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                      "Rich1_Geo : ", "Created Rich1Mirror2 Surface in segment %d with Name %s using %s", isegF,
                      aCurM2BorderSurfaceName.c_str(), ( aRichSurfaceUtil->Rich1Mirror2SurfaceName( isegF ) ).c_str() );

            if ( m_RichSurfaceDebugActivate && m_R1MirrorDebugActivate ) {

              aRichSurfaceUtil->PrintRichSurfProperty( aCurM2Surface, RichSurfPropType::Refl );
              aRichSurfaceUtil->PrintRichSurfProperty( aCurM2Surface, RichSurfPropType::Effic );
            }
          }
        }
      }
    }
  }
  //=====================================================================================//

  void Rich1Build::build_Rich1Mirror2_Version_Rich_RUN3_v0() {

    auto aRichGeoUtil     = RichGeoUtil::getRichGeoUtilInstance();
    auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto surfMgr          = description.surfaceManager();

    const auto aNumM2Systems = aRichGeoUtil->Rich1NumMirror2Systems();
    const auto aNumSegInHalf = aRichGeoUtil->Rich1NumMirror2SegInAHalf();
    const auto aR1SubMLVName = aRichGeoUtil->Rich1SubMasterLVName();

    for ( int iMM = 0; iMM < aNumM2Systems; iMM++ ) {
      const auto aRich1Mirror2MasterLVName = aRichGeoUtil->Rich1Mirror2MasterLVName( iMM );
      auto       lvMother                  = volume( aRich1Mirror2MasterLVName );

      auto aR1M2MasterTN = ( iMM == 0 ) ? ( RichGeomTransformLabel::pvRich1Mirror2MasterTopTN )
                                        : ( RichGeomTransformLabel::pvRich1Mirror2MasterBotTN );
      auto pvRich1Mirror2CurMaster =
          BuildARich1PhysVolWithPVRet( aR1SubMLVName, aRich1Mirror2MasterLVName, aR1M2MasterTN );

      auto aR1M2SupTN = ( iMM == 0 ) ? ( RichGeomTransformLabel::pvRich1Mirror2SupportTopTN )
                                     : ( RichGeomTransformLabel::pvRich1Mirror2SupportBotTN );

      BuildARich1PhysVol( aR1SubMLVName, ( aRichGeoUtil->Rich1Mirror2SupportLVName( iMM ) ), aR1M2SupTN );
      // Now for the detector element structure for the current Rich1 Mirror2 master
      std::optional<dd4hep::DetElement> aRich1M2MasterDet;
      if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
        const auto aR1M2MasterDetName = aRichGeoUtil->Rich1Mirror2MasterDetName( iMM );
        const auto aR1M2MasterDetId   = aRichGeoUtil->getPhysVolCopyNumber( aR1M2MasterTN );
        pvRich1Mirror2CurMaster.addPhysVolID( aR1M2MasterDetName + "Det", aR1M2MasterDetId );
        aRich1M2MasterDet.emplace( detector, aR1M2MasterDetName, aR1M2MasterDetId );
        aRich1M2MasterDet->setPlacement( pvRich1Mirror2CurMaster );
      }

      for ( int iseg = 0; iseg < aNumSegInHalf; iseg++ ) {
        const auto isegF   = ( iMM * aNumSegInHalf ) + iseg;
        auto       lvChild = volume( aRichGeoUtil->Rich1Mirror2LogVolName( isegF ) );
        const auto TransformForChild =
            aRichGeoUtil->getRichGeomTransform( aRichGeoUtil->Rich1Mirror2TransformName( isegF ) );
        const auto aChildCopyNumber = aRichGeoUtil->Rich1Mirror2CopyNumber( isegF );
        auto       pvChild          = lvMother.placeVolume( lvChild, aChildCopyNumber, TransformForChild );
        pvChild->SetName( ( aRichGeoUtil->Rich1Mirror2PhysVolName( isegF ) ).c_str() );

        printout( m_R1MirrorDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                  "Rich1_Geo : ", "Created Rich1 PhysVol for Rich1 Mirror2 Segment %d in Half %d ", iseg, iMM );

        // Now for the detector element structure for the current Rich1 Mirror2 segment
        if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
          const auto aR1M2SegDetName = aRichGeoUtil->Rich1Mirror2DetName( isegF );
          pvChild.addPhysVolID( aR1M2SegDetName + "Det", aChildCopyNumber );
          assert( aRich1M2MasterDet.has_value() );
          auto aRich1M2SegDet = dd4hep::DetElement( aRich1M2MasterDet.value(), aR1M2SegDetName, aChildCopyNumber );
          aRich1M2SegDet.setPlacement( pvChild );
        }
        // Now create Rich1Mirror2 surface
        if ( m_activate_Rich1Surface_build ) {

          auto       aCurM2Surface = surfMgr.opticalSurface( aRichSurfaceUtil->Rich1Mirror2SurfaceName( isegF ) );
          const auto aCurM2BorderSurfaceName = aRichSurfaceUtil->Rich1Mirror2BorderSurfaceName( isegF );
          auto       aR1M2SegBorderSurface   = dd4hep::BorderSurface( description, detector, aCurM2BorderSurfaceName,
                                                              aCurM2Surface, pvRich1Mirror2CurMaster, pvChild );
          aR1M2SegBorderSurface.isValid();
          printout( m_RichSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                    "Rich1_Geo : ", "Created Rich1Mirror2 Surface in segment %d with Name %s using %s", isegF,
                    aCurM2BorderSurfaceName.c_str(), ( aRichSurfaceUtil->Rich1Mirror2SurfaceName( isegF ) ).c_str() );

          if ( m_RichSurfaceDebugActivate && m_R1MirrorDebugActivate ) {

            aRichSurfaceUtil->PrintRichSurfProperty( aCurM2Surface, RichSurfPropType::Refl );
            aRichSurfaceUtil->PrintRichSurfProperty( aCurM2Surface, RichSurfPropType::Effic );
          }
        }
      }
    }
  }

  //======================================================================================//
  void Rich1Build::build_Rich1PhDetSupFrames() {
    auto aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();

    for ( int iSide = 0; iSide < ( aRichGeoUtil->Rich1NumSides() ); iSide++ ) {

      const auto aMagShLvName = aRichGeoUtil->Rich1MagShLVName( iSide );

      auto aRich1PhDetSupFrameTN = ( iSide == 0 ) ? RichGeomTransformLabel::pvRich1PhDetSupFrameH0TN
                                                  : RichGeomTransformLabel::pvRich1PhDetSupFrameH1TN;

      const auto aRich1PhDetSupFrameLvName = aRichGeoUtil->Rich1PhDetSupFrameLVName( iSide );

      // BuildARich1PhysVol( aMagShLvName, aRich1PhDetSupFrameLvName, aRich1PhDetSupFrameTN );

      auto aRich1PhDetSupFramePv =
          BuildARich1PhysVolWithPVRet( aMagShLvName, aRich1PhDetSupFrameLvName, aRich1PhDetSupFrameTN );
      // Now for the detector element structure
      if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
        const auto aR1PhDetSupDetName = aRichGeoUtil->Rich1PhDetSupFrameDetName( iSide );
        const auto aR1PhDetSupId      = aRichGeoUtil->getPhysVolCopyNumber( aRich1PhDetSupFrameTN );
        aRich1PhDetSupFramePv.addPhysVolID( aR1PhDetSupDetName + "Det", aR1PhDetSupId );
        auto aR1PhDetSupDet = dd4hep::DetElement( detector, aR1PhDetSupDetName, aR1PhDetSupId );
        aR1PhDetSupDet.setPlacement( aRich1PhDetSupFramePv );
      }
    }

    if ( m_activate_PhotonDetector_build ) { build_Rich1PmtFullSystem(); }
  }

  //=====================================================================================//

  void Rich1Build::build_Rich1GasQuartzWindows() {

    auto aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();

    auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto surfMgr          = description.surfaceManager();
    auto lvRich1SubM      = volume( aRichGeoUtil->Rich1SubMasterLVName() );
    auto aR1GasQWSurface  = surfMgr.opticalSurface( aRichSurfaceUtil->Rich1GasQuartzWindowSurfaceName() );

    for ( int iSide = 0; iSide < ( aRichGeoUtil->Rich1NumSides() ); iSide++ ) {

      const auto aMagShLvName = aRichGeoUtil->Rich1MagShLVName( iSide );
      auto       aRich1GQuartzWTN =
          ( iSide == 0 ) ? RichGeomTransformLabel::pvRich1GQuartzWH0TN : RichGeomTransformLabel::pvRich1GQuartzWH1TN;
      const auto aRich1GQuartzWLvName = aRichGeoUtil->Rich1GasQuartzWLVName();

      auto aPvRich1GQuartzWCur = BuildARich1PhysVolWithPVRet( aMagShLvName, aRich1GQuartzWLvName, aRich1GQuartzWTN );

      if ( m_activate_Rich1QWSurface_build ) {

        auto aRich1MagShTN =
            ( iSide == 0 ) ? RichGeomTransformLabel::pvRich1MagShH0TN : RichGeomTransformLabel::pvRich1MagShH1TN;
        const auto aPvMagShName = aRichGeoUtil->getRichPhysVolName( aRich1MagShTN );

        auto       pvMagSHCur     = dd4hep::PlacedVolume( lvRich1SubM->GetNode( aPvMagShName.c_str() ) );
        const auto aGasQWBorderSN = std::array{aRichSurfaceUtil->Rich1GasQuartzWindowBorderSurfaceName( iSide ),
                                               aRichSurfaceUtil->Rich1GasQuartzWindowBorderBackSurfaceName( iSide )};
        for ( int iss = 0; iss < (int)aGasQWBorderSN.size(); iss++ ) {

          auto aR1GasQWBorderSurface = ( iss == 0 )
                                           ? dd4hep::BorderSurface( description, detector, aGasQWBorderSN[iss],
                                                                    aR1GasQWSurface, pvMagSHCur, aPvRich1GQuartzWCur )
                                           : dd4hep::BorderSurface( description, detector, aGasQWBorderSN[iss],
                                                                    aR1GasQWSurface, aPvRich1GQuartzWCur, pvMagSHCur );
          aR1GasQWBorderSurface.isValid();

          printout( m_RichSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                    "Rich1_Geo : ", "Created Rich1GasQW Surface with Name %s using %s", ( aGasQWBorderSN[iss] ).c_str(),
                    ( aRichSurfaceUtil->Rich1GasQuartzWindowSurfaceName() ).c_str() );
        }
      }
    }

    if ( m_activate_Rich1QWSurface_build && m_RichSurfaceDebugActivate ) {

      aRichSurfaceUtil->PrintRichSurfProperty( aR1GasQWSurface, RichSurfPropType::Refl );
      aRichSurfaceUtil->PrintRichSurfProperty( aR1GasQWSurface, RichSurfPropType::Effic );
      aRichSurfaceUtil->PrintRichSurfProperty( aR1GasQWSurface, RichSurfPropType::Trans );
    }
  }
  //=====================================================================================//
  void Rich1Build::build_Rich1PmtFullSystem() {
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();

    aRichPmtGeoAux->initRich1PmtStructures();
    if ( aRichPmtGeoAux->Rich1PmtStructureCreated() ) {

      build_Rich1PmtModuleVols();
      build_Rich1PmtECRVols();
      build_Rich1PmtMasterVols();
      build_Rich1PmtAllPhysVols();
    }
  }
  //=====================================================================================//
  void Rich1Build::build_Rich1PmtModuleVols() {
    if ( m_debugActivate ) { dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", "Begin to build PMT modules " ); }

    // get a single pmtModule
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();

    auto       lvSinglePmtModule = volume( "lvRich1PmtSingleStdModule" );
    const auto aMaterial         = lvSinglePmtModule.material();
    const auto aShape            = lvSinglePmtModule.solid();
    const auto aVis              = lvSinglePmtModule.visAttributes();

    for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich1TotalNumPmtModules() ); iM++ ) {

      const auto aVolName   = aRichPmtGeoAux->Rich1PmtModuleLogVolName( iM );
      const auto aShapeName = aRichPmtGeoAux->Rich1PmtModuleShapeName( iM );
      if ( m_pmtutilDebugActivate ) {
        dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", " Rich1Pmt LogVolName ShapeName %s %s", aVolName.c_str(),
                          aShapeName.c_str() );
      }

      dd4hep::Solid_type aShapeM( aShape );
      aShapeM.setName( aShapeName.c_str() );

      dd4hep::Volume aLV( aVolName.c_str(), aShapeM, aMaterial );
      aLV.setVisAttributes( aVis );

      registerShape( aShapeName, aShapeM );
      registerVolume( aVolName, aLV );

    } // end loop over modules

    if ( m_debugActivate ) { dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", "End build PMT modules " ); }
  }
  //=====================================================================================//

  void Rich1Build::build_Rich1PmtECRVols() {
    if ( m_debugActivate ) { dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", "Begin to build PMT ECR " ); }
    // get a single pmtECR
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();

    auto       lvSinglePmtEcr = volume( "lvRich1PmtSingleECR" );
    const auto aMaterial      = lvSinglePmtEcr.material();
    const auto aShape         = lvSinglePmtEcr.solid();
    const auto aVis           = lvSinglePmtEcr.visAttributes();

    // Loop over modules

    for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich1TotalNumPmtModules() ); iM++ ) {

      const auto curRange = aRichPmtGeoAux->get_Rich1EcrRange( iM );
      const auto ibeg     = curRange[0];
      const auto iend     = curRange[1];

      // first loop over ecr
      for ( int iec = ibeg; iec < iend; iec++ ) {
        const auto aEcVolName   = aRichPmtGeoAux->Rich1PmtECRLogVolName( iM, iec );
        const auto aEcShapeName = aRichPmtGeoAux->Rich1PmtECRShapeName( iM, iec );
        if ( m_pmtutilDebugActivate ) {
          dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", " Rich1Pmt ECR LogVolName ShapeName %s %s", aEcVolName.c_str(),
                            aEcShapeName.c_str() );
        }

        dd4hep::Solid_type aShapeE( aShape );
        aShapeE.setName( aEcShapeName.c_str() );

        dd4hep::Volume aLVE( aEcVolName.c_str(), aShapeE, aMaterial );
        aLVE.setVisAttributes( aVis );
        registerShape( aEcShapeName, aShapeE );
        registerVolume( aEcVolName, aLVE );

      } // end first loop over ECR

    } // end loop over Modules
  }
  //=====================================================================================//

  void Rich1Build::build_Rich1PmtMasterVols() {
    if ( m_debugActivate ) { dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", "Begin to build PMT Master " ); }
    // get a single pmt Master
    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();

    auto       lvSinglePmtMaster = volume( "lvRich1PmtSingleStdMaster" );
    const auto aMaterial         = lvSinglePmtMaster.material();
    const auto aShape            = lvSinglePmtMaster.solid();
    const auto aVis              = lvSinglePmtMaster.visAttributes();

    // Loop over modules

    for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich1TotalNumPmtModules() ); iM++ ) {

      const auto curRange = aRichPmtGeoAux->get_Rich1EcrRange( iM );
      auto       ibeg     = curRange[0];
      auto       iend     = curRange[1];

      // first loop over ecr
      for ( int iec = ibeg; iec < iend; iec++ ) {

        // Loop over Pmts
        for ( int ip = 0; ip < ( aRichPmtGeoAux->RhNumPMTInECR() ); ip++ ) {

          const auto aPmtVolName   = aRichPmtGeoAux->Rich1PmtMasterLogVolName( iM, iec, ip );
          const auto aPmtShapeName = aRichPmtGeoAux->Rich1PmtMasterShapeName( iM, iec, ip );
          if ( m_pmtutilDebugActivate ) {
            dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", " Rich1Pmt Master LogVolName ShapeName %d %d %d %s %s", iM,
                              iec, ip, aPmtVolName.c_str(), aPmtShapeName.c_str() );
          }

          dd4hep::Solid_type aShapeP( aShape );
          aShapeP.setName( aPmtShapeName.c_str() );

          dd4hep::Volume aLVP( aPmtVolName.c_str(), aShapeP, aMaterial );
          aLVP.setVisAttributes( aVis );
          registerShape( aPmtShapeName, aShapeP );
          registerVolume( aPmtVolName, aLVP );
        }
      }
    }
  }
  //=====================================================================================//

  void Rich1Build::build_Rich1PmtAllPhysVols() {
    if ( m_debugActivate ) {
      dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", "Begin to build PMT array structure Phys Vols " );
    }
    auto aRichPmtGeoAux   = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();

    // Acquire the info needed to create pmt surfaces
    auto surfMgr       = description.surfaceManager();
    auto aRMapmtQWSurf = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtQWSurfaceName() );

    auto aRMapmtMTSurf = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtMTSurfaceName() );
    auto aRMapmtANSurf = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtAnodeSurfaceName() );

    const int  aNumSurfPairInPmt  = 2; // For the pair of surfaces in the vectors listed below.
    const auto aR1MapmtQWBorderSN = std::array{"RichMaPMTQWBorderSurfaceIn", "RichMaPMTQWBorderBackSurfaceIn"};

    const auto aR1MapmtMTBorderSN =
        std::array{"RichMaPMTSideEnvMTBorderSurfaceIn", "RichMapmtBackEnvMTBorderSurfaceIn"};
    const auto aR1MapmtANBorderSN = "RichMaPMTAnodeBorderSurfaceIn";

    const auto aPvRichMapmtComp       = BuildRich1StdMapmtCompPhysVols();
    const auto aR1PmtAnodeDetName     = aRichPmtGeoAux->RichMaPmtAnodeDetName();
    const auto aR1PmtPhCathodeDetName = aRichPmtGeoAux->RichMaPmtPhCathodeDetName();
    const auto aR1PmtAnodeIdSuffix =
        aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel::pvRichPMTAnode0000TN );
    const auto aR1PmtPhCathodeIdSuffix =
        aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel::pvRichPMTPhCathodeTN );

    for ( int iM = 0; iM < ( aRichPmtGeoAux->Rich1TotalNumPmtModules() ); iM++ ) {
      std::optional<dd4hep::DetElement> aR1PmtModuleDet;
      if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
        const auto aR1PmtModuleDetName = aRichPmtGeoAux->Rich1PmtModuleDetName( iM );
        aR1PmtModuleDet                = dd4hep::DetElement(
            detector.child( aRichPmtGeoAux->getRich1PhDetSupDetNameFromModuleNum( iM ) ), aR1PmtModuleDetName, iM );
      }
      const auto curRange = aRichPmtGeoAux->get_Rich1EcrRange( iM );
      const auto ibeg     = curRange[0];
      const auto iend     = curRange[1];

      // first loop over ecr
      for ( int iec = ibeg; iec < iend; iec++ ) {

        // Loop over Pmts
        for ( int ip = 0; ip < ( aRichPmtGeoAux->RhNumPMTInECR() ); ip++ ) {

          const auto curPmtMasterVolName     = aRichPmtGeoAux->Rich1PmtMasterLogVolName( iM, iec, ip );
          auto       aRichMapmtSMPhysVol     = BuildARich1PmtSMasterPhysVol( curPmtMasterVolName );
          auto       aRichMapmtMasterPhysVol = BuildARich1PmtMasterPhysVol( iM, iec, ip );

          if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
            const auto aR1PmtMasterDetName = aRichPmtGeoAux->Rich1PmtMasterDetName( iM, iec, ip );
            const auto aPmtMasterId        = aRichMapmtMasterPhysVol.copyNumber();
            assert( aR1PmtModuleDet.has_value() );
            auto aR1PmtMasterDet = dd4hep::DetElement( aR1PmtModuleDet.value(), aR1PmtMasterDetName, aPmtMasterId );
            aR1PmtMasterDet.setPlacement( aRichMapmtMasterPhysVol );
            // defining Anode Id to avoid duplication
            const auto CurPmtMasterCopyNum = aRichPmtGeoAux->getRich1PmtMasterCopyNumber( iM, iec, ip );
            const auto aR1PmtAnodeId =
                ( ( aRichPmtGeoAux->RhPmtAnodeDetIdShiftFactor() ) * ( 1 + aR1PmtAnodeIdSuffix ) ) +
                CurPmtMasterCopyNum;
            const auto aR1PmtPhCathodeId =
                ( ( aRichPmtGeoAux->RhPmtPhCathodeDetIdShiftFactor() ) * ( 1 + aR1PmtPhCathodeIdSuffix ) ) +
                CurPmtMasterCopyNum;

            auto aR1PmtAnodeDet     = dd4hep::DetElement( aR1PmtMasterDet, aR1PmtAnodeDetName, aR1PmtAnodeId );
            auto aR1PmtPhCathodeDet = dd4hep::DetElement( aR1PmtMasterDet, aR1PmtPhCathodeDetName, aR1PmtPhCathodeId );

            aR1PmtAnodeDet.setPlacement( aPvRichMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtAnode )] );
            aR1PmtPhCathodeDet.setPlacement(
                aPvRichMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtPhc )] );
          }
          aRichPmtGeoAux->setRich1PmtMasterValid( iM, iec, ip );

          if ( m_activate_Rich1PmtSurface_build ) {
            for ( int isPair = 0; isPair < aNumSurfPairInPmt; isPair++ ) {
              auto aR1MapmtQWBorderSurface =
                  ( isPair == 0 )
                      ? dd4hep::BorderSurface(
                            description, detector, ( aR1MapmtQWBorderSN[isPair] + curPmtMasterVolName ), aRMapmtQWSurf,
                            aRichMapmtSMPhysVol,
                            aPvRichMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtQW )] )
                      : dd4hep::BorderSurface(
                            description, detector, ( aR1MapmtQWBorderSN[isPair] + curPmtMasterVolName ), aRMapmtQWSurf,
                            aPvRichMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtQW )],
                            aRichMapmtSMPhysVol );

              auto aR1MapmtMTBorderSurface =
                  ( isPair == 0 )
                      ? dd4hep::BorderSurface(
                            description, detector, ( aR1MapmtMTBorderSN[isPair] + curPmtMasterVolName ), aRMapmtMTSurf,
                            aRichMapmtSMPhysVol,
                            aPvRichMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtSideEnv )] )
                      : dd4hep::BorderSurface(
                            description, detector, ( aR1MapmtMTBorderSN[isPair] + curPmtMasterVolName ), aRMapmtMTSurf,
                            aRichMapmtSMPhysVol,
                            aPvRichMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtBackEnv )] );

              aR1MapmtQWBorderSurface.isValid();
              aR1MapmtMTBorderSurface.isValid();
              if ( m_RichPmtSurfaceDebugActivate ) {

                printout( m_RichPmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                          "Rich1_Geo : ", "Created Rich1Pmt QW Surface with Name %s using %s",
                          ( ( aR1MapmtQWBorderSN[isPair] + curPmtMasterVolName ).c_str() ),
                          ( ( aRichSurfaceUtil->RichMapmtQWSurfaceName() ).c_str() ) );

                printout( m_RichPmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                          "Rich1_Geo : ", "Created Rich1Pmt MT Surface with Name %s using %s",
                          ( ( aR1MapmtMTBorderSN[isPair] + curPmtMasterVolName ).c_str() ),
                          ( ( aRichSurfaceUtil->RichMapmtMTSurfaceName() ).c_str() ) );
              }
            }
            auto aR1MapmtAnodeBorderSurface = dd4hep::BorderSurface(
                description, detector, ( aR1MapmtANBorderSN + curPmtMasterVolName ), aRMapmtANSurf, aRichMapmtSMPhysVol,
                aPvRichMapmtComp[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtAnode )] );
            aR1MapmtAnodeBorderSurface.isValid();
            if ( m_RichPmtSurfaceDebugActivate ) {
              printout( m_RichPmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                        "Rich1_Geo : ", "Created Rich1Pmt Anode Surface with Name %s using %s",
                        ( ( aR1MapmtANBorderSN + curPmtMasterVolName ).c_str() ),
                        ( ( aRichSurfaceUtil->RichMapmtAnodeSurfaceName() ).c_str() ) );
            }
          }
        }

        const auto curEcrVolname = aRichPmtGeoAux->Rich1PmtECRLogVolName( iM, iec );

        BuildARich1PmtShieldingPhysVol( curEcrVolname );
        auto pvPmtEcr = BuildARich1PmtEcrPhysVol( iM, iec );
        if ( m_activate_Rich1_DetElem_For_CurrentAppl && m_activate_Rich1_DetElem_For_PmtEcr ) {
          const auto aR1PmtEcrDetName = aRichPmtGeoAux->Rich1PmtECRDetName( iM, iec );
          const auto aEcrId           = pvPmtEcr.copyNumber();
          assert( aR1PmtModuleDet.has_value() );
          auto aR1PmtEcrDet = dd4hep::DetElement( aR1PmtModuleDet.value(), aR1PmtEcrDetName, aEcrId );
          aR1PmtEcrDet.setPlacement( pvPmtEcr );
        }

        aRichPmtGeoAux->setRich1PmtECRValid( iM, iec );
      }
      auto pvPmtModule = BuildARich1PmtModulePhysVol( iM );
      if ( m_activate_Rich1_DetElem_For_CurrentAppl ) { aR1PmtModuleDet->setPlacement( pvPmtModule ); }
    }

    if ( m_activate_Rich1PmtSurface_build && m_RichPmtSurfaceDebugActivate ) {
      aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtQWSurf, RichSurfPropType::Refl );
      aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtQWSurf, RichSurfPropType::Effic );
      aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtQWSurf, RichSurfPropType::Trans );

      aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtMTSurf, RichSurfPropType::Refl );
      aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtMTSurf, RichSurfPropType::Effic );
      aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtANSurf, RichSurfPropType::Refl );
      aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtANSurf, RichSurfPropType::Effic );
    }
  }
  //=====================================================================================//

  std::vector<dd4hep::PlacedVolume> Rich1Build::BuildRich1StdMapmtCompPhysVols() {

    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();

    std::vector<dd4hep::PlacedVolume> aPmtCompPvVect( aRichPmtGeoAux->NumRichPmtCompForSurf() );
    const int                         aNumSurfPairInPmt = 2; // For the pair of surfaces in the vectors listed below.
    auto                              aRichSurfaceUtil  = RichSurfaceUtil::getRichSurfaceUtilInstance();
    auto                              surfMgr           = description.surfaceManager();
    auto       aRMapmtPCSurf      = surfMgr.opticalSurface( aRichSurfaceUtil->RichMapmtPCSurfaceName() );
    const auto aR1MapmtPCBorderSN = std::array{"RichMaPMTPCBorderSurfaceIn", "RichMaPMTPCBorderBackSurfaceIn"};

    const auto aPmtSMLvName = aRichPmtGeoAux->RichMaPmtSubMasterLVName();

    auto       aPmtSMLv          = volume( aPmtSMLvName );
    const auto NumVolInPmtSM     = (int)( aPmtSMLv->GetNdaughters() );
    const bool PmtCompPvNotBuilt = ( NumVolInPmtSM == 0 ) ? true : false;

    // First the pmt components
    if ( PmtCompPvNotBuilt ) {
      auto aPvRichMapmtAnode =
          BuildARichPmtPhysVolWithPVRet( aPmtSMLvName, ( aRichPmtGeoAux->RichMaPmtStdAnodeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTAnode0000TN );
      const auto aR1PmtAnodeDetName = aRichPmtGeoAux->RichMaPmtAnodeDetName();
      const auto aR1PmtAnodeId =
          aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel::pvRichPMTAnode0000TN );

      aPvRichMapmtAnode.addPhysVolID( aR1PmtAnodeDetName + "Det", aR1PmtAnodeId );

      auto aPvRichMapmtQuartz = BuildARichPmtPhysVolWithPVRet(
          aPmtSMLvName, ( aRichPmtGeoAux->RichMaPmtQuartzLVName() ), RichPmtGeomTransformLabel::pvRichPMTQuartzTN );
      auto aPvRichMapmtPhCathode =
          BuildARichPmtPhysVolWithPVRet( aPmtSMLvName, ( aRichPmtGeoAux->RichMaPmtPhCathodeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTPhCathodeTN );
      const auto aR1PmtPhCathodeDetName = aRichPmtGeoAux->RichMaPmtPhCathodeDetName();
      const auto aR1PmtPhCathodeId =
          aRichPmtGeoAux->getRichPmtCompPhysVolCopyNumber( RichPmtGeomTransformLabel::pvRichPMTPhCathodeTN );

      aPvRichMapmtAnode.addPhysVolID( aR1PmtAnodeDetName + "Det", aR1PmtAnodeId );
      aPvRichMapmtPhCathode.addPhysVolID( aR1PmtPhCathodeDetName + "Det", aR1PmtPhCathodeId );

      auto aPvRichMapmtSideEnv =
          BuildARichPmtPhysVolWithPVRet( aPmtSMLvName, ( aRichPmtGeoAux->RichMaPmtSideEnvelopeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTSideEnvelopeTN );
      auto aPvRichMapmtBackEnv =
          BuildARichPmtPhysVolWithPVRet( aPmtSMLvName, ( aRichPmtGeoAux->RichMaPmtBackEnvelopeLVName() ),
                                         RichPmtGeomTransformLabel::pvRichPMTBackEnvelopeTN );
      BuildARichPmtPhysVol( aPmtSMLvName, ( aRichPmtGeoAux->RichMaPmtFrontRingLVName() ),
                            RichPmtGeomTransformLabel::pvRichPMTFrontRingTN );
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtAnode )] = aPvRichMapmtAnode;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtQW )]    = aPvRichMapmtQuartz;

      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtSideEnv )] = aPvRichMapmtSideEnv;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtBackEnv )] = aPvRichMapmtBackEnv;
      aPmtCompPvVect[aRichPmtGeoAux->toInTg( RichStdPmtCompLabel::stdPmtPhc )]     = aPvRichMapmtPhCathode;

      if ( m_activate_Rich1PmtSurface_build ) {
        for ( int isPair = 0; isPair < aNumSurfPairInPmt; isPair++ ) {

          auto aR1MapmtPCBorderSurface =
              ( isPair == 0 ) ? dd4hep::BorderSurface( description, detector, ( aR1MapmtPCBorderSN[isPair] ),
                                                       aRMapmtPCSurf, aPvRichMapmtQuartz, aPvRichMapmtPhCathode )
                              : dd4hep::BorderSurface( description, detector, ( aR1MapmtPCBorderSN[isPair] ),
                                                       aRMapmtPCSurf, aPvRichMapmtPhCathode, aPvRichMapmtQuartz );

          aR1MapmtPCBorderSurface.isValid();
          if ( m_RichPmtSurfaceDebugActivate ) {
            printout( m_RichPmtSurfaceDebugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG,
                      "Rich1_Geo : ", "Created Rich1Pmt PC Surface with Name %s using %s", //
                      aR1MapmtPCBorderSN[isPair], aRichSurfaceUtil->RichMapmtPCSurfaceName().c_str() );
            aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtPCSurf, RichSurfPropType::Refl );
            aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtPCSurf, RichSurfPropType::Effic );
            aRichSurfaceUtil->PrintRichSurfProperty( aRMapmtPCSurf, RichSurfPropType::Trans );
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

  //=====================================================================================//

  void Rich1Build::build_Rich1MagSh() {
    auto aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();
    auto aR1SMLName   = aRichGeoUtil->Rich1SubMasterLVName();

    build_Rich1PhDetSupFrames();
    for ( int iSide = 0; iSide < ( aRichGeoUtil->Rich1NumSides() ); iSide++ ) {

      const auto aMagShLvName = aRichGeoUtil->Rich1MagShLVName( iSide );
      auto       aRich1MagShTN =
          ( iSide == 0 ) ? RichGeomTransformLabel::pvRich1MagShH0TN : RichGeomTransformLabel::pvRich1MagShH1TN;

      BuildARich1PhysVol( aR1SMLName, aMagShLvName, aRich1MagShTN );
    }
    if ( m_activate_QuartzWindow_build ) { build_Rich1GasQuartzWindows(); }
  }
  //=====================================================================================//

  void Rich1Build::build_Rich1MagneticShieldComponents() {
    auto       aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();
    const auto aR1SMLName   = aRichGeoUtil->Rich1SubMasterLVName();
    const auto aR1MLName    = aRichGeoUtil->Rich1MasterLVName();

    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsOuterH0", RichGeomTransformLabel::pvRich1MgsOuterH0TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsOuterH1", RichGeomTransformLabel::pvRich1MgsOuterH1TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsUpstrH0", RichGeomTransformLabel::pvRich1MgsUpstrH0TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsUpstrH1", RichGeomTransformLabel::pvRich1MgsUpstrH1TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsSide", RichGeomTransformLabel::pvRich1MgsSideQ0TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsSide", RichGeomTransformLabel::pvRich1MgsSideQ1TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsSide", RichGeomTransformLabel::pvRich1MgsSideQ2TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsSide", RichGeomTransformLabel::pvRich1MgsSideQ3TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsUpstrCorner", RichGeomTransformLabel::pvRich1MgsUpstrCornerH0TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsUpstrCorner", RichGeomTransformLabel::pvRich1MgsUpstrCornerH1TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsTeeth", RichGeomTransformLabel::pvRich1MgsTeethQ0TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsTeeth", RichGeomTransformLabel::pvRich1MgsTeethQ1TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsTeeth", RichGeomTransformLabel::pvRich1MgsTeethQ2TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsTeeth", RichGeomTransformLabel::pvRich1MgsTeethQ3TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsMidH0", RichGeomTransformLabel::pvRich1MgsMidH0TN );
    BuildARich1PhysVol( aR1SMLName, "lvRich1MgsMidH1", RichGeomTransformLabel::pvRich1MgsMidH1TN );

    BuildARich1PhysVol( aR1MLName, "lvRich1MgsDnstrUTH0", RichGeomTransformLabel::pvRich1MgsDnstrUTH0TN );
    BuildARich1PhysVol( aR1MLName, "lvRich1MgsDnstrUTH1", RichGeomTransformLabel::pvRich1MgsDnstrUTH1TN );
  }

  //=====================================================================================//
  void Rich1Build::build_Rich1ExitWall() {
    auto       aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();
    const auto aR1SMLName   = aRichGeoUtil->Rich1SubMasterLVName();

    if ( m_activate_ExitWall_build ) {
      BuildARich1PhysVol( "lvRich1ExitWallMaster", "lvRich1ExitG10Upstr",
                          RichGeomTransformLabel::pvRich1ExitG10UpstrTN );
      BuildARich1PhysVol( "lvRich1ExitWallMaster", "lvRich1ExitG10Dnstr",
                          RichGeomTransformLabel::pvRich1ExitG10DnstrTN );
      BuildARich1PhysVol( "lvRich1ExitWallMaster", "lvRich1ExitPMI", RichGeomTransformLabel::pvRich1ExitPMITN );

      BuildARich1PhysVol( aR1SMLName, "lvRich1ExitWallMaster", RichGeomTransformLabel::pvRich1ExitWallMasterTN );
    }
    if ( m_activate_ExitWallDiaphram_build ) {
      BuildARich1PhysVol( aR1SMLName, "lvRich1ExitDiaphramCentral",
                          RichGeomTransformLabel::pvRich1ExitDiaphramCentralUpsTN );
      BuildARich1PhysVol( aR1SMLName, "lvRich1ExitDiaphramCentral",
                          RichGeomTransformLabel::pvRich1ExitDiaphramCentralDnsTN );
      BuildARich1PhysVol( aR1SMLName, "lvRich1ExitDiaphramMiddle",
                          RichGeomTransformLabel::pvRich1ExitDiaphramMiddleTN );
      BuildARich1PhysVol( aR1SMLName, "lvRich1ExitDiaphramPeriphery",
                          RichGeomTransformLabel::pvRich1ExitDiaphramPeripheryTN );
      BuildARich1PhysVol( aR1SMLName, "lvRich1ExitDiaphramCentralPlugUps",
                          RichGeomTransformLabel::pvRich1ExitDiaphramCentralPlugUpsTN );
      BuildARich1PhysVol( aR1SMLName, "lvRich1ExitDiaphramCentralPlugDns",
                          RichGeomTransformLabel::pvRich1ExitDiaphramCentralPlugDnsTN );
    }
  }

  //=====================================================================================//

  void Rich1Build::build_Rich1SubMaster() {
    auto       aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();
    const auto aR1SMLName   = aRichGeoUtil->Rich1SubMasterLVName();
    const auto aR1MLName    = aRichGeoUtil->Rich1MasterLVName();

    build_Rich1MagSh();

    if ( m_activate_Rich1Mirror1_build ) { build_Rich1Mirror1(); }
    if ( m_activate_Rich1Mirror2_build ) {

      // printout( dd4hep::ALWAYS, "Rich1SubMaster ",
      //           " Version %d   ", m_Rich1GeomVersion.Current_RichGeomVersion()  );

      if ( ( m_Rich1GeomVersion.Current_RichGeomVersion() ) != ( RichGeomLabel::Rich_RUN3_v0 ) ) {

        build_Rich1Mirror2();

      } else {

        // create the old version of Rich1 Mirror2
        build_Rich1Mirror2_Version_Rich_RUN3_v0();
      }
    }

    if ( m_activate_MagneticShield_build ) { build_Rich1MagneticShieldComponents(); }

    if ( m_activate_ExitWall_build || m_activate_ExitWallDiaphram_build ) { build_Rich1ExitWall(); }

    const auto aRich1BeamPipeInSubMId =
        aRichGeoUtil->getPhysVolCopyNumber( RichGeomTransformLabel::pvUX851InRich1SubMasterTN );
    const auto                          aRich1BeamPipeInSubMDetName = aRichGeoUtil->Rich1BeamPipeInSubMDetName();
    std::optional<dd4hep::PlacedVolume> aRich1BeamPipeInSubMPhys;
    if ( m_activate_Rich1BeamPipe_build ) {
      // The beampipe segment in Rich1SubMaster
      aRich1BeamPipeInSubMPhys = BuildARich1PhysVolWithPVRet( aR1SMLName, aRichGeoUtil->Rich1BeamPipeSegmentLVName( 0 ),
                                                              RichGeomTransformLabel::pvUX851InRich1SubMasterTN );
      aRich1BeamPipeInSubMPhys->addPhysVolID( aRich1BeamPipeInSubMDetName + "Det", aRich1BeamPipeInSubMId );

      // Now the beampipe sections in Rich1Master
      const auto aBPLabelInMaster = std::array{RichGeomTransformLabel::pvUX851InRich1BeforeSubMTN,
                                               RichGeomTransformLabel::pvUX851InRich1AfterSubMTN};

      for ( int ib = 1; ib < aRichGeoUtil->Rich1NumBeamPipeSeg(); ib++ ) {
        BuildARich1PhysVol( aR1MLName, ( aRichGeoUtil->Rich1BeamPipeSegmentLVName( ib ) ), aBPLabelInMaster[ib - 1] );
      }
    }

    if ( m_activate_Rich1SubMaster_build ) {

      auto aRich1SubMasterPhys =
          BuildARich1PhysVolWithPVRet( aR1MLName, aR1SMLName, RichGeomTransformLabel::pvRich1SubMasterTN );

      // Now for DetElem structure
      if ( m_activate_Rich1_DetElem_For_CurrentAppl ) {
        const auto aR1SubMDetName = aRichGeoUtil->Rich1SubMasterDetName();
        const auto aR1SubMDetId   = aRichGeoUtil->getPhysVolCopyNumber( RichGeomTransformLabel::pvRich1SubMasterTN );
        aRich1SubMasterPhys.addPhysVolID( aR1SubMDetName + "Det", aR1SubMDetId );
        auto aRich1SubMDet = dd4hep::DetElement( detector, aR1SubMDetName, aR1SubMDetId );
        aRich1SubMDet.setPlacement( aRich1SubMasterPhys );

        if ( m_activate_Rich1BeamPipe_build ) {
          auto aRich1BeamPipeInSubMDet =
              dd4hep::DetElement( aRich1SubMDet, aRich1BeamPipeInSubMDetName, aRich1BeamPipeInSubMId );
          assert( aRich1BeamPipeInSubMPhys.has_value() );
          aRich1BeamPipeInSubMDet.setPlacement( aRich1BeamPipeInSubMPhys.value() );
        }
      }
    }
  }
  //=====================================================================================//

  void Rich1Build::build_Rich1Master() {

    build_Rich1SubMaster();

    // test volumes names
    const auto len = volumes.size();
    printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich1_geo", " Number of Volumes  %d ", ( (int)len ) );

    if ( m_debugLvListActivate ) {
      for ( auto iv = volumes.begin(); iv != volumes.end(); ++iv ) {
        auto vol     = ( ( *iv ).second.second );
        auto VolName = vol.name();

        printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich1_geo", " Volume name %s ", VolName );
        const auto aNumDaughters = (int)( ( static_cast<TGeoVolume*>( vol ) )->GetNdaughters() );
        const auto aNumNodes     = (int)( ( static_cast<TGeoVolume*>( vol ) )->CountNodes() );
        printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich1_geo", "Number of daughters  nodes %d   %d ",
                  aNumDaughters, aNumNodes );

        //(static_cast<TGeoVolume*> (vol))->PrintNodes();
      }
    }
    // end test volumes names
  }
  //=====================================================================================//

  void Rich1Build::build_Rich_OpticalProperties() {
    if ( m_activate_RichMatPropTable_build ) { build_RichMaterial_OpticalProperties(); }
    if ( m_activate_RichMatGeneralPropTable_build ) { build_Rich_General_Properties(); }
  }

  //=====================================================================================//

  void Rich1Build::build_RichSingle_QE_Table( RichMatNameWOP aMatNameW, RichMatPropType aPropType, RichMatQE aQEType ) {

    auto        aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
    std::string aTableName  = "DummyNonMatQETableName";

    const auto aNumTabCol  = aRichMatOpd->NumTabCol(); // column size of property tables
    int        aNumTabRows = 0;

    if ( aMatNameW == RichMatNameWOP::RhNonMat ) {
      if ( aPropType == RichMatPropType::pmtQE ) {
        aTableName  = aRichMatOpd->GetQEGeneralTableName( aPropType, aQEType );
        aNumTabRows = aRichMatOpd->GetQENumBinsInGeneralTable( aPropType, aQEType );

        dd4hep::PropertyTable aPropertyTable( description, aTableName, "", aNumTabRows, aNumTabCol );

        auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();
        aRichMatPropData->SelectAndFillQETable( aPropertyTable, aQEType );
      }
    }
  }
  //=====================================================================================//

  void Rich1Build::build_Rich_PmtHV( RichMatNameWOP aMatNameW, RichMatPropType aPropType, RichMatHV ahvType ) {

    auto        aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
    std::string aTableName  = "DummyNonMatHVTableName";
    const auto  aNumTabCol  = aRichMatOpd->NumTabCol(); // column size of property tables
    int         aNumTabRows = 0;

    if ( aMatNameW == RichMatNameWOP::RhNonMat ) {
      if ( aPropType == RichMatPropType::pmtHV ) {
        aTableName  = aRichMatOpd->getPmtHVTableName( ahvType );
        aNumTabRows = aRichMatOpd->GetHVNumBinsInGeneralTable( aPropType, ahvType );
      }
    }

    dd4hep::PropertyTable aPropertyTable( description, aTableName, "", aNumTabRows, aNumTabCol );

    auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();
    aRichMatPropData->SelectAndFillHVTable( aPropertyTable, ahvType );
  }

  //=============================================================================================================//
  void Rich1Build::build_Rich_QWARCoatRefl( RichMatNameWOP aMatNameW, RichMatPropType aPropType,
                                            RichSurfCoat aARType ) {

    auto        aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
    std::string aTableName  = "DummyNonMatARReflTableName";
    const auto  aNumTabCol  = aRichMatOpd->NumTabCol(); // column size of property tables
    int         aNumTabRows = 0;

    if ( aMatNameW == RichMatNameWOP::RhNonMat ) {
      if ( aPropType == RichMatPropType::FresAR ) {
        aTableName  = aRichMatOpd->getFresARTableName( aARType );
        aNumTabRows = aRichMatOpd->GetQWARNumBinsInGeneralTable( aPropType, aARType );
      }
    }

    dd4hep::PropertyTable aPropertyTable( description, aTableName, "", aNumTabRows, aNumTabCol );

    auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();
    aRichMatPropData->SelectAndFillQWARTable( aPropertyTable, aARType );
  }

  //=============================================================================================================//

  void Rich1Build::build_Rich_ScProp( RichMatNameWOP aMatNameW, RichMatPropType aPropType, RichMatScint aScType ) {

    auto        aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
    std::string aTableName  = "DummyCf4ScintTableName";

    const auto aNumTabCol  = aRichMatOpd->NumTabCol(); // column size of property tables
    int        aNumTabRows = 0;

    if ( ( aMatNameW == RichMatNameWOP::R2cf4 ) || ( aMatNameW == RichMatNameWOP::R2RGas ) ) {
      if ( aPropType == RichMatPropType::scintGeneral ) {
        aTableName  = aRichMatOpd->getCF4ScintTableName( aScType );
        aNumTabRows = aRichMatOpd->GetSCNumBinsInGeneralTable( aPropType, aScType );
      }
    }

    dd4hep::PropertyTable aPropertyTable( description, aTableName, "", aNumTabRows, aNumTabCol );
    auto                  aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();
    aRichMatPropData->SelectAndFillScTable( aPropertyTable, aScType );
  }
  //=====================================================================================//
  void Rich1Build::RetriveAndPrintRichPmtQE( RichMatPropType aPropType, RichMatQE aQEType ) {

    auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();

    const std::string aTableName =
        ( aPropType == RichMatPropType::pmtQE ? aRichMatOpd->GetQEGeneralTableName( aPropType, aQEType )
                                              : "ADummyQETableName" );

    const auto aTableQE = description.manager().GetGDMLMatrix( aTableName.c_str() );

    dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", " Current Table Name Type and QE Type  %s %d %d", aTableName.c_str(),
                      aRichMatOpd->toInTy( aPropType ), aRichMatOpd->toInTy( aQEType ) );

    aRichMatOpd->PrintAGDMLMatrix( aTableQE );
  }
  //=====================================================================================//
  void Rich1Build::RetriveAndPrintRichPmtHV( RichMatPropType aPropType, RichMatHV ahvType ) {

    auto              aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
    const std::string aTableName =
        ( aPropType == RichMatPropType::pmtHV ? aRichMatOpd->getPmtHVTableName( ahvType ) : "ADummyHVTableName" );

    auto aTableHV = description.manager().GetGDMLMatrix( aTableName.c_str() );

    dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", " Current Table Name Type and HV Type  %s %d %d", aTableName.c_str(),
                      aRichMatOpd->toInTy( aPropType ), aRichMatOpd->toInTy( ahvType ) );

    aRichMatOpd->PrintAGDMLMatrix( aTableHV );
  }
  //=====================================================================================//
  void Rich1Build::RetriveAndPrintRichGasQWARCoatingRefl( RichMatPropType aPropType, RichSurfCoat aARType ) {

    auto              aRichMatOpd = RichMatOPD::getRichMatOPDInstance();
    const std::string aTableName  = ( aPropType == RichMatPropType::FresAR ? aRichMatOpd->getFresARTableName( aARType )
                                                                          : "ADummyQWARCoatingReflTableName" );
    auto              aTableAR    = description.manager().GetGDMLMatrix( aTableName.c_str() );

    dd4hep::printout( dd4hep::DEBUG, "Rich1_geo", " Current Table Name Type and AR Coating Refl Type  %s %d %d",
                      aTableName.c_str(), aRichMatOpd->toInTy( aPropType ), aRichMatOpd->toInTy( aARType ) );

    aRichMatOpd->PrintAGDMLMatrix( aTableAR );
  }

  //=====================================================================================//
  void Rich1Build::build_Rich_General_Properties() {
    // These property tables are not attached to a specific material.

    build_RichSingle_QE_Table( RichMatNameWOP::RhNonMat, RichMatPropType::pmtQE, RichMatQE::qePmtNominal );
    build_Rich_PmtHV( RichMatNameWOP::RhNonMat, RichMatPropType::pmtHV, RichMatHV::hvPmtNominal );
    build_Rich_QWARCoatRefl( RichMatNameWOP::RhNonMat, RichMatPropType::FresAR, RichSurfCoat::arR1GasQW );

    if ( m_activate_RichMatPmtPT_ExtraSet_build ) {
      build_RichSingle_QE_Table( RichMatNameWOP::RhNonMat, RichMatPropType::pmtQE, RichMatQE::qePmtCBAUV );
      build_RichSingle_QE_Table( RichMatNameWOP::RhNonMat, RichMatPropType::pmtQE, RichMatQE::qePmtMeanMeas );
      build_RichSingle_QE_Table( RichMatNameWOP::RhNonMat, RichMatPropType::pmtQE, RichMatQE::qePmtHamamatsuNominal );
      build_RichSingle_QE_Table( RichMatNameWOP::RhNonMat, RichMatPropType::pmtQE, RichMatQE::qePmtPre2020 );

      build_Rich_PmtHV( RichMatNameWOP::RhNonMat, RichMatPropType::pmtHV, RichMatHV::hvPmtClassic );
    }

    if ( m_activate_RetrieveAndPrintForDebug_GeneralTable ) {
      RetriveAndPrintRichPmtQE( RichMatPropType::pmtQE, RichMatQE::qePmtNominal );
      RetriveAndPrintRichPmtHV( RichMatPropType::pmtHV, RichMatHV::hvPmtNominal );
      RetriveAndPrintRichGasQWARCoatingRefl( RichMatPropType::FresAR, RichSurfCoat::arR1GasQW );
      if ( m_activate_RichMatPmtPT_ExtraSet_build ) {

        RetriveAndPrintRichPmtQE( RichMatPropType::pmtQE, RichMatQE::qePmtCBAUV );
        RetriveAndPrintRichPmtQE( RichMatPropType::pmtQE, RichMatQE::qePmtMeanMeas );
        RetriveAndPrintRichPmtQE( RichMatPropType::pmtQE, RichMatQE::qePmtHamamatsuNominal );
        RetriveAndPrintRichPmtQE( RichMatPropType::pmtQE, RichMatQE::qePmtPre2020 );

        RetriveAndPrintRichPmtHV( RichMatPropType::pmtHV, RichMatHV::hvPmtClassic );
      }
    }
  }

  //=====================================================================================//
  void Rich1Build::build_Rich_SingleMaterial_StdOpticalProperty( RichMatNameWOP aMatNameW, RichMatPropType aPropType ) {

    auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();

    // std::cout<< "material and property index "<<  aRichMatOpd -> toInTy(aMatNameW)  <<"  "
    //     <<  aRichMatOpd  -> toInTy(aPropType) <<std::endl;

    const auto aNumTabCol = aRichMatOpd->NumTabCol(); // column size of property tables

    const auto aTableNumRows = aRichMatOpd->GetNumBinsInTable( aMatNameW, aPropType );

    const auto aTableName = aRichMatOpd->GetTableName( aMatNameW, aPropType );

    // Create,register and populate a Table

    dd4hep::PropertyTable aPropertyTable( description, aTableName, "", aTableNumRows, aNumTabCol );
    aRichMatOpd->FillGeneralTable( aPropertyTable, aMatNameW, aPropType );
  }
  //=====================================================================================//
  void Rich1Build::Attach_RichMaterial_OpticalPropertiesTable( RichMatNameWOP aMatNameW, RichMatPropType aPropType ) {

    auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();

    const auto cMatName    = aRichMatOpd->RichMatNameWithOptProp( aMatNameW );
    auto       aRichCurMat = description.material( cMatName.c_str() );
    const auto aTableName  = aRichMatOpd->GetTableName( aMatNameW, aPropType );

    // Retrieve the newly created Table,test its existence and attach it to the material

    auto aTableT = description.manager().GetGDMLMatrix( aTableName.c_str() );
    aRichMatOpd->AttachTableToMaterial( aTableT, aPropType, aTableName, aRichCurMat );
  }
  //=====================================================================================//

  void Rich1Build::Attach_RichMaterial_ScintPropertiesTable( RichMatNameWOP aMatNameW, RichMatPropType aPropType,
                                                             RichMatScint aScType ) {

    auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();

    const auto cMatName    = aRichMatOpd->RichMatNameWithOptProp( aMatNameW );
    auto       aRichCurMat = description.material( cMatName.c_str() );
    const auto aTableName  = aRichMatOpd->getCF4ScintTableName( aScType );

    // Retrieve the newly created Table,test its existence and attach it to the material

    auto aTableT = description.manager().GetGDMLMatrix( aTableName.c_str() );
    aRichMatOpd->AttachTableToMaterial( aTableT, aPropType, aTableName, aRichCurMat );
  }
  //=====================================================================================//
  void Rich1Build::build_RichMaterial_OpticalProperties() {

    for ( const auto RhMABL :
          {RichMatNameWOP::Rhair, RichMatNameWOP::R1GasQw, RichMatNameWOP::PmtVac, RichMatNameWOP::R1M2Glass,
           RichMatNameWOP::PmtQw, RichMatNameWOP::PmtPhc, RichMatNameWOP::R1c4f10, RichMatNameWOP::R2cf4,
           RichMatNameWOP::R1Nitrogen, RichMatNameWOP::Rhco2} ) {
      build_Rich_SingleMaterial_StdOpticalProperty( RhMABL, RichMatPropType::AbLe );
    }

    for ( const auto RhMRDX :
          {RichMatNameWOP::Rhair, RichMatNameWOP::R1GasQw, RichMatNameWOP::PmtVac, RichMatNameWOP::R1M2Glass,
           RichMatNameWOP::PmtQw, RichMatNameWOP::PmtPhc, RichMatNameWOP::R1c4f10, RichMatNameWOP::R2cf4,
           RichMatNameWOP::R1Nitrogen, RichMatNameWOP::Rhco2} ) {
      build_Rich_SingleMaterial_StdOpticalProperty( RhMRDX, RichMatPropType::Rindex );
    }

    for ( const auto RhMCKR : {RichMatNameWOP::R1GasQw, RichMatNameWOP::PmtQw, RichMatNameWOP::R1c4f10,
                               RichMatNameWOP::R2cf4, RichMatNameWOP::R1Nitrogen, RichMatNameWOP::Rhco2} ) {
      build_Rich_SingleMaterial_StdOpticalProperty( RhMCKR, RichMatPropType::CkvRndx );
    }

    const auto RhScT =
        std::array{RichMatScint::scFast, RichMatScint::scFastTime, RichMatScint::scYield, RichMatScint::scRes};
    for ( int ik = 0; ik < (int)RhScT.size(); ik++ ) {
      build_Rich_ScProp( RichMatNameWOP::R2cf4, RichMatPropType::scintGeneral, RhScT[ik] );
    }

    const auto RhMABLAth = std::array{
        RichMatNameWOP::Rhair,      RichMatNameWOP::R1GasQw,    RichMatNameWOP::R2GasQw, RichMatNameWOP::PmtVac,
        RichMatNameWOP::R1M2Glass,  RichMatNameWOP::R2MGlass,   RichMatNameWOP::PmtQw,   RichMatNameWOP::PmtPhc,
        RichMatNameWOP::R1c4f10,    RichMatNameWOP::R1RGas,     RichMatNameWOP::R2cf4,   RichMatNameWOP::R2RGas,
        RichMatNameWOP::R2Nitrogen, RichMatNameWOP::R1Nitrogen, RichMatNameWOP::Rhco2,
    };

    const auto RhMRDXAth = std::array{RichMatNameWOP::Rhair,      RichMatNameWOP::R1GasQw,    RichMatNameWOP::R2GasQw,
                                      RichMatNameWOP::PmtVac,     RichMatNameWOP::R1M2Glass,  RichMatNameWOP::R2MGlass,
                                      RichMatNameWOP::PmtQw,      RichMatNameWOP::PmtPhc,     RichMatNameWOP::R1c4f10,
                                      RichMatNameWOP::R1RGas,     RichMatNameWOP::R2cf4,      RichMatNameWOP::R2RGas,
                                      RichMatNameWOP::R1Nitrogen, RichMatNameWOP::R2Nitrogen, RichMatNameWOP::Rhco2};

    const auto RhMCKRAth = std::array{RichMatNameWOP::R1GasQw, RichMatNameWOP::R2GasQw,    RichMatNameWOP::PmtQw,
                                      RichMatNameWOP::R1c4f10, RichMatNameWOP::R1RGas,     RichMatNameWOP::R2cf4,
                                      RichMatNameWOP::R2RGas,  RichMatNameWOP::R1Nitrogen, RichMatNameWOP::R2Nitrogen,
                                      RichMatNameWOP::Rhco2};

    const auto RhPSC = std::array{RichMatPropType::scFast, RichMatPropType::scFastTime, RichMatPropType::scYield,
                                  RichMatPropType::scRes};

    if ( m_activate_RichStd_MatPropTable_Attach ) {

      for ( int ib = 0; ib < (int)RhMABLAth.size(); ib++ ) {
        Attach_RichMaterial_OpticalPropertiesTable( RhMABLAth[ib], RichMatPropType::AbLe );
      }
      for ( int ix = 0; ix < (int)RhMRDXAth.size(); ix++ ) {
        Attach_RichMaterial_OpticalPropertiesTable( RhMRDXAth[ix], RichMatPropType::Rindex );
      }

      if ( m_activate_RichSpecific_MatPropTable_Attach ) {

        for ( int iy = 0; iy < (int)RhMCKRAth.size(); iy++ ) {
          Attach_RichMaterial_OpticalPropertiesTable( RhMCKRAth[iy], RichMatPropType::CkvRndx );
        }

        for ( int isc = 0; isc < (int)RhScT.size(); isc++ ) {
          Attach_RichMaterial_ScintPropertiesTable( RichMatNameWOP::R2cf4, RhPSC[isc], RhScT[isc] );
          Attach_RichMaterial_ScintPropertiesTable( RichMatNameWOP::R2RGas, RhPSC[isc], RhScT[isc] );
        }
      }
    }
    // Retrieve the tables and print the contents for testing

    if ( m_activate_RetrieveAndPrintForDebug_MatTable ) {
      for ( int ija = 0; ija < (int)RhMABLAth.size(); ija++ ) {
        RetrieveRichMatProperty( RhMABLAth[ija], RichMatPropType::AbLe );
      }
      for ( int ijr = 0; ijr < (int)RhMRDXAth.size(); ijr++ ) {
        RetrieveRichMatProperty( RhMRDXAth[ijr], RichMatPropType::Rindex );
      }
      if ( m_activate_RichSpecific_MatPropTable_Attach ) {
        for ( int ijc = 0; ijc < (int)RhMCKRAth.size(); ijc++ ) {
          RetrieveRichMatProperty( RhMCKRAth[ijc], RichMatPropType::CkvRndx );
        }

        for ( int itc = 0; itc < (int)RhPSC.size(); itc++ ) {
          RetrieveRichMatProperty( RichMatNameWOP::R2cf4, RhPSC[itc] );
          RetrieveRichMatProperty( RichMatNameWOP::R2RGas, RhPSC[itc] );
        }
      }
    }
  }

  //=====================================================================================//
  void Rich1Build::RetrieveRichMatProperty( RichMatNameWOP aMatNameW, RichMatPropType aPropType ) {
    // Retrieve the property from the material for test
    auto aRichMatOpd = RichMatOPD::getRichMatOPDInstance();

    const auto aMatName = aRichMatOpd->RichMatNameWithOptProp( aMatNameW );

    auto aRichMat = description.material( aMatName.c_str() );
    aRichMatOpd->PrintRichMatProperty( aRichMat, aPropType );
  }
  //=====================================================================================//

  void Rich1Build::build_Rich1_Main() {

    // sensitive.setType( "Rich" );
    dd4hep::PlacedVolume pv;
    // dd4hep::DetElement   deSubM;

    load( x_det, "include" );

    auto aRichGeoUtil = RichGeoUtil::getRichGeoUtilInstance();
    aRichGeoUtil->setDebugLevel( m_generalutilDebugActivate );
    aRichGeoUtil->setRich1GeometryVersion( m_Rich1GeomVersion.Current_RichGeomVersion() );
    aRichGeoUtil->InitRichTransformsAndNames();

    printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, "Rich1 Builder : Rich1 Geometry", " Version %d   ",
              m_Rich1GeomVersion.Current_RichGeomVersion() );

    printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich1_geo ", " Initialized RichGeoUtil " );

    auto aRichPmtGeoAux = RichPmtGeoAux::getRichPmtGeoAuxInstance();
    aRichPmtGeoAux->setPmtDebugLevel( m_pmtutilDebugActivate );

    // Create Optical properties for Rich Materials
    build_Rich_OpticalProperties();

    auto aRichGeoTransAux = RichGeoTransAux::getRichGeoTransAuxInstance();
    aRichGeoTransAux->setRichGeoTransDebug( m_generalutilDebugActivate );
    const auto aNumR1F = aRichGeoTransAux->loadRichTransforms( x_det, "include", "Rich1" );
    printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich1_geo ",
              " Uploaded RichTransforms for Rich1 in RichGeoUtil from %zu Rich1 Transform files ", aNumR1F );

    if ( !( aRichPmtGeoAux->IsRichStdPmtCompUploaded() ) ) {
      const auto aNumRPmF = aRichGeoTransAux->loadRichTransforms( x_det, "include", "RichPMT" );
      aRichPmtGeoAux->setRichNumStdPmtCompUploaded( aNumRPmF );
      printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich1_geo ",
                " Uploaded RichTransforms for Rich StdPmt in RichPmtGeoAux from  %zu  "
                "RichPmt Transform Files ",
                aNumRPmF );

    } else {
      printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich1_geo ",
                "  Rich StdPmt Transforms are already uploaded  from elsewhere for  %zu  "
                "RichPmt Transform Files ",
                ( aRichPmtGeoAux->RichNumStdPmtCompUploaded() ) );
    }

    // aRichGeoUtil->TestRichGeomParameters();

    // Initiate Rich1 Surface utility
    if ( m_activate_Rich1Surface_build || m_activate_Rich1PmtSurface_build || m_activate_Rich1QWSurface_build ) {
      auto aRichSurfaceUtil = RichSurfaceUtil::getRichSurfaceUtilInstance();
      aRichSurfaceUtil->setRichSurfaceUtilDebug( m_RichSurfaceDebugActivate );
      aRichSurfaceUtil->InitRich1SurfNames();
    }

    buildVolumes( x_det );
    placeDaughters( detector, dd4hep::Volume(), x_det );
    build_Rich1Master();

    if ( m_activateVolumeDebug ) {
      if ( !( m_attachVolumeForDebug.empty() ) ) {
        select_Rich1_Volume = m_attachVolumeForDebug;
        printout( m_debugActivate ? dd4hep::ALWAYS : dd4hep::DEBUG, " Rich1_geo ",
                  " Now selecting the volume %s for display and debug . ", select_Rich1_Volume.c_str() );
      }
    }
    auto vol = volume( select_Rich1_Volume );
    pv       = placeDetector( vol );
    if ( x_det.hasAttr( _U( id ) ) ) { pv.addPhysVolID( "system", x_det.id() ); }
  }
} // end namespace

//========================================================================================//

static dd4hep::Ref_t create_element_Rich_RUN3_v1( dd4hep::Detector& description, xml_h e,
                                                  dd4hep::SensitiveDetector sens_det ) {
  Rich1Build builder( description, e, sens_det, RichGeomLabel::Rich_RUN3_v1 );

  // printout( dd4hep::ALWAYS, " Rich1_geo V1",
  //               " Version %d   ", builder.m_Rich1GeomVersion.Current_RichGeomVersion()  );

  builder.build_Rich1_Main();
  return builder.detector;
}
//===========================================================================================//

static dd4hep::Ref_t create_element_Rich_RUN3_v0( dd4hep::Detector& description, xml_h e,
                                                  dd4hep::SensitiveDetector sens_det ) {
  Rich1Build builder( description, e, sens_det, RichGeomLabel::Rich_RUN3_v0 );

  // printout( dd4hep::ALWAYS, " Rich1_geo V0",
  //                " Version %d   ", builder.m_Rich1GeomVersion.Current_RichGeomVersion()  );

  builder.build_Rich1_Main();
  return builder.detector;
}
//===========================================================================================//

// The old versions are kept for backward compatibility.

DECLARE_DETELEMENT( LHCb_Rich1_Geometry_RUN3_v1, create_element_Rich_RUN3_v1 )

DECLARE_DETELEMENT( LHCb_Rich1_Geometry, create_element_Rich_RUN3_v0 )

//===========================================================================================//
