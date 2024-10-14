//==========================================================================
//  LHCb Rich1 Detector Element Dump in DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2022-01-10
//
//==========================================================================
#include "Detector/Rich2/DetElemAccess/DeRich2.h"

#include "Detector/Rich2/Rich2GeoUtil.h"

#include "Core/Keys.h"

#include "Core/ConditionsRepository.h"
#include "Core/DetectorDataService.h"
#include "Core/MagneticFieldExtension.h"

#include "DD4hep/Conditions.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"

#include "DDCond/ConditionsDataLoader.h"
#include "DDCond/ConditionsManager.h"
#include "DDCond/ConditionsSlice.h"

#include "TTimeStamp.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

//=============================================================================//

static long test_dump_Rich2_DetElem( dd4hep::Detector& description, int argc, char** argv ) {
  // test print
  std::cout << " Printing from test_dump_Rich2_DetElem " << std::endl;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    std::cout << " Input argc argv " << i << "  " << argc << "  " << argv[i] << std::endl;
  }

  bool        help = false;
  std::string conditions, match;
  for ( int i = 0; i < argc && argv[i]; ++i ) {
    if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
      if ( 0 == ::strncmp( "-help", argv[i], 4 ) ) {
        std::cout << " argv[i] help true " << i << "  " << argv[i] << std::endl;
        help = true;
      } else if ( 0 == ::strncmp( "-conditions", argv[i], 4 ) )
        conditions = argv[++i];
      else if ( 0 == ::strncmp( "-match", argv[i], 4 ) )
        match = argv[++i];
      else {
        std::cout << " help true option " << i << "  " << argv[i] << std::endl;
        help = true;
      }
    }
  }

  std::cout << "Rich2_dump_detelem help conditions " << help << "  " << conditions << std::endl;

  if ( help || conditions.empty() ) {
    /// Help printout describing the basic command line interface
    std::cout << "Usage: -plugin <name> -arg [-arg]                                   \n"
                 "     name:   factory name     LHCb_TEST_cond_content                \n"
                 "     -detector   <name>       Name of the sub-detector to analyze.  \n"
                 "     -conditions <directory>  Top-directory with conditions files.  \n"
                 "                              Fully qualified: <protocol>://<path>  \n"
                 "     -help                    Show this help.                       \n"
                 "\tArguments given: "
              << dd4hep::arguments( argc, argv ) << std::endl;
    ::exit( EINVAL );
  }

  // Show VERBOSE dd4hep printout from DetElems themselves
  dd4hep::setPrintLevel( dd4hep::DEBUG );

  dd4hep::printout( dd4hep::INFO, "Conditions", "+++ Conditions directory: %s [%s] ", conditions.c_str(),
                    match.c_str() );

  //====================================================================

  // Loading the detector and retrieving the condition with the file names for the Magnetic field service
  LHCb::Magnet::setup_magnetic_field_extension( description, "/cvmfs/lhcb.cern.ch/lib/lhcb/DBASE/FieldMap/v5r7/cdf" );
  std::vector<std::string>            detector_list{"/world", "Magnet", "Rich1", "Rich2"};
  LHCb::Detector::DetectorDataService dds( description, detector_list );
  dds.initialize( nlohmann::json{{"repository", conditions}} );

  std::cout << "Now access the DB slice " << std::endl;

  int  slice1_id = 100;
  auto slice     = dds.get_slice( slice1_id );

  std::cout << "Now access the info froms detelems in Rich2 " << std::endl;

  dd4hep::DetElement rich2Det = description.detector( "Rich2" );

  std::cout << "Now access the Rich2 Gas radiator detelem object " << std::endl;

  const LHCb::Detector::DeRich2& r2De = slice->get( rich2Det, LHCb::Detector::Keys::deKey );
  //-------------------------------------------------------------------------------------------//
  // Few tests of parameters. These are commented out normally, just to limit the size of the printout
  /*
   // Nominal Rich2 Sph mirr refl
  const auto aR2sphMNominalRefl = r2De.nominalSphMirrorRefl();
  for ( const auto& elem : aR2sphMNominalRefl ) {
    std::cout << "Rich2 Sph Mirr Nominal Energy refl " << elem.first << "  " << elem.second << std::endl;
  }
 // Nominal Rich2 Sec mirr refl
  const auto aR2secMNominalRefl = r2De.nominalSecMirrorRefl();
  for ( const auto& elem : aR2sphMNominalRefl ) {
    std::cout << "Rich2 sec Mirr Nominal Energy refl " << elem.first << "  " << elem.second << std::endl;
  }
  // Rich2 QW abs len
  const auto aR2QWAbs = r2De.gasWinAbsLength() ;
  for ( const auto& elem : aR2QWAbs ) {
    std::cout << "Rich2 QW abslen Nominal Energy abslen " << elem.first << "  " << elem.second << std::endl;
  }
  */
  //--------------------------------------------------------------------------------------------//
  // testing Rich2 Beam Pipe
  // auto r2BeamPipeDe = r2De.BeamPipe();
  // Parameters for the beam pipe to be added
  // end of testing Rich2 Beam pipe
  //--------------------------------------------------------------------------------------------//
  // testing the access of Radiator Gas

  std::cout << "Nominal Rich2 gas pressure:    " << r2De.radiatorGas().Gas_Presure_Nominal() << std::endl;
  std::cout << "Nominal Rich2 gas temperature: " << r2De.radiatorGas().Gas_Temperature_Nominal() << std::endl;

  auto r2RadGasDe = r2De.radiatorGas();

  //   r2RadGasDe.PrintGasGeneralInfoFromDB();

  // The following lines are for illustration. They print the same info as that from the
  // line above and hence line above is commented out

  std::cout << " Now retrieving and then printing ref index of Rich2 Gas " << std::endl;
  const auto aR2gasRefInd = r2RadGasDe.GasRefIndex();
  for ( const auto& elem : aR2gasRefInd ) {
    std::cout << "R2 gas Energy RefIndex " << elem.first << "  " << elem.second << std::endl;
  }
  std::cout << " End of retrieving and then printing ref index of Rich2 Gas " << std::endl;

  /*
  // std::cout<<" Debug test "<<std::endl;
  //debug test
  // r2RadGasDe.RetrieveAndPrintDebugGasNominalRefIndexFromLocalInp();
  */
  // Now for the size and location of rich2 radiator gas
  const dd4hep::BooleanSolid& r2RadGasA     = r2RadGasDe.access()->detector().solid();
  const dd4hep::BooleanSolid& s1            = r2RadGasA.leftShape();
  const dd4hep::BooleanSolid& s2            = s1.leftShape();
  const dd4hep::Trd2&         r2RadGasShape = s2.leftShape();
  double                      aHdz          = r2RadGasShape.dZ();
  double                      aHdx1         = r2RadGasShape.dX1();
  double                      aHdx2         = r2RadGasShape.dX2();
  double                      aHdy1         = r2RadGasShape.dY1();
  double                      aHdy2         = r2RadGasShape.dY2();

  std::cout << " Rich2 Radiator Trd HalfSize DZ DX1 DX2 DY1 DY2 " << aHdz << "   " << aHdx1 << "   " << aHdx2 << "   "
            << aHdy1 << "   " << aHdy2 << std::endl;

  dd4hep::Position  aR2RadGasGlobalPos   = r2RadGasDe.access()->detectorAlignment().localToWorld( {0, 0, 0} );
  dd4hep::Position  aR2RadGasLocalPos    = r2RadGasDe.access()->detector().placement().position();
  const TGeoMatrix& aR2RadGasOrientation = r2RadGasDe.access()->detector().placement().matrix();
  std::cout << " Rich2 Radiator Position Glocal Local " << aR2RadGasGlobalPos << "  " << aR2RadGasLocalPos << std::endl;
  aR2RadGasOrientation.Print();

  //  std::cout << "Now accessed the Gas radiator detelem object  " << std::endl;

  //-----------------------------------------------------------------------------------//
  // Now testing the access of Rich2 Mirror1 (Sph mirror)

  for ( int iS = 0; iS < ( r2RadGasDe.NumSides() ); iS++ ) {
    std::cout << " Current Rich2 Mirror1 Master Side " << iS << std::endl;
    auto r2M1MasDe    = r2RadGasDe.SphMM( iS );
    int  aNumMSegSide = r2M1MasDe.NumSphMirrorSegSide();
    for ( int iSeg = 0; iSeg < aNumMSegSide; iSeg++ ) {
      const int iSegCopy = ( iS == 0 ) ? iSeg : ( iSeg + aNumMSegSide );
      std::cout << " Rich2 Sph Mirror Side and copy number " << iS << "   " << iSegCopy << std::endl;
      auto r2M1SegDe     = r2M1MasDe.SphMirrorSeg( iSeg );
      auto r2M1ROC       = r2M1SegDe.ROC();
      auto r2M1LocalCOC  = r2M1SegDe.LocalCOC();
      auto r2M1GlobalCOC = r2M1SegDe.GlobalCOC();
      std::cout << "R2M1 Seg ROC LocalCOC:XYZ  GlobalCOC:XYZ " << iSegCopy << "   " << r2M1ROC << "  "
                << r2M1LocalCOC.X() << "  " << r2M1LocalCOC.Y() << "  " << r2M1LocalCOC.Z() << "  " << r2M1GlobalCOC.X()
                << "   " << r2M1GlobalCOC.Y() << "   " << r2M1GlobalCOC.Z() << std::endl;

      auto R2M1Refl = r2M1SegDe.RetrieveReflectivity();

      // To limit the size of the printout the reflectivity is printed only for the first segment

      if ( ( iSeg == 0 ) && ( iS == 0 ) ) {
        std::cout << " Now printing the reflectivity of R2 mirror1 segment " << iSeg << std::endl;
        for ( const auto& refel : R2M1Refl ) {
          std::cout << "R2M1 ener Refl " << refel.first << "  " << refel.second << std::endl;
        }
        std::cout << " End of printing the reflectivity of mirror1 segment " << iSeg << std::endl;
      }
    }
    // another test of rich2mirror1
    auto aR2M1SegVect = r2M1MasDe.SphMirrorSegVect();
    for ( int ii = 0; ii < (int)aR2M1SegVect.size(); ii++ ) {
      std::cout << "R2M1Vect Side Seg ROC " << iS << " " << ii << "  " << aR2M1SegVect[ii].ROC() << std::endl;
    }
  }

  // End of testing the access of Rich2 Mirror1
  //------------------------------------------------------------------------------------//
  // Testing access to Rich2SecMirror
  for ( int iS = 0; iS < ( r2RadGasDe.NumSides() ); iS++ ) {
    std::cout << " Current Rich2 Sec Mirror Master Side " << iS << std::endl;
    auto r2M2MasDe    = r2RadGasDe.SecMM( iS );
    int  bNumMSegSide = r2M2MasDe.SecMirrorNumSegSide();
    for ( int iSeg = 0; iSeg < bNumMSegSide; iSeg++ ) {
      const int iSegCopy = ( iS == 0 ) ? iSeg : ( iSeg + bNumMSegSide );
      std::cout << " Rich2 Sec Mirror Side and copy number " << iS << "   " << iSegCopy << std::endl;
      auto r2M2SegDe     = r2M2MasDe.SecMirrorSeg( iSeg );
      auto r2M2ROC       = r2M2SegDe.ROC();
      auto r2M2LocalCOC  = r2M2SegDe.LocalCOC();
      auto r2M2GlobalCOC = r2M2SegDe.GlobalCOC();
      std::cout << "R2M2 Seg ROC LocalCOC:XYZ  GlobalCOC:XYZ " << iSegCopy << "   " << r2M2ROC << "  "
                << r2M2LocalCOC.X() << "  " << r2M2LocalCOC.Y() << "  " << r2M2LocalCOC.Z() << "  " << r2M2GlobalCOC.X()
                << "   " << r2M2GlobalCOC.Y() << "   " << r2M2GlobalCOC.Z() << std::endl;

      const auto& R2M2Refl = r2M2SegDe.RetrieveReflectivity();
      // To limit the size of the printout the reflectivity is printed only for the first segment
      if ( ( iSeg == 0 ) && ( iS == 0 ) ) {
        std::cout << " Now printing the reflectivity of R2 mirror2 segment " << iSeg << std::endl;
        for ( const auto& refel : R2M2Refl ) {
          std::cout << "R2M2 ener Refl " << refel.first << "  " << refel.second << std::endl;
        }
        std::cout << " End of printing the reflectivity of mirror2 segment " << iSeg << std::endl;
      }
    }
    // another test of rich2mirror1
    auto aR2M2SegVect = r2M2MasDe.SecMirrorSegVect();
    for ( int ii = 0; ii < (int)aR2M2SegVect.size(); ii++ ) {
      std::cout << "R2M2Vect Side Seg ROC " << iS << " " << ii << "  " << aR2M2SegVect[ii].ROC() << std::endl;
    }
  }
  // yet another test of rich2mirror1 with vector
  auto r2M1SegLeftVect  = r2De.primaryMirrors( Rich::left );
  auto r2M1SegRightVect = r2De.primaryMirrors( Rich::right );
  auto r2M2SegLeftVect  = r2De.secondaryMirrors( Rich::left );
  auto r2M2SegRightVect = r2De.secondaryMirrors( Rich::right );
  std::cout << "  R2 Mirror seg sizes R2M1L R2M1R R2M2L R2M2R " << r2M1SegLeftVect.size() << "  "
            << r2M1SegRightVect.size() << "   " << r2M2SegLeftVect.size() << "  " << r2M2SegRightVect.size()
            << std::endl;

  for ( int ii = 0; ii < (int)r2M1SegLeftVect.size(); ii++ ) {
    std::cout << " R2M1LeftVect Seg ROC " << ii << "  " << r2M1SegLeftVect[ii].ROC() << std::endl;
  }
  for ( int ii = 0; ii < (int)r2M1SegRightVect.size(); ii++ ) {
    std::cout << " R2M1RightVect Seg ROC " << ii << "  " << r2M1SegRightVect[ii].ROC() << std::endl;
  }
  for ( int ii = 0; ii < (int)r2M2SegLeftVect.size(); ii++ ) {
    std::cout << " R2M2LeftVect Seg ROC " << ii << "  " << r2M2SegLeftVect[ii].ROC() << std::endl;
  }
  for ( int ii = 0; ii < (int)r2M2SegRightVect.size(); ii++ ) {
    std::cout << " R2M2RightVect Seg ROC " << ii << "  " << r2M2SegRightVect[ii].ROC() << std::endl;
  }

  //------------------------------------------------------------------------------------------------//
  // End of testing the access of Rich2 Mirror2
  // Now testing the access of Rich2 PhDet Panel and Rich2 Mapmts   --------------------------------//
  // Loop through the two panels
  for ( const auto iPanel : Rich::sides() ) {

    auto r2PhDetP = r2De.PhDetPanel( iPanel );
    // Now for the size of the panel
    const dd4hep::Box& aPanelShape = r2PhDetP.access()->detector().solid();
    std::cout << " R2 Panel Num  Half size XYZ  " << iPanel << "  " << aPanelShape.x() << "    " << aPanelShape.y()
              << "   " << aPanelShape.z() << std::endl;
    // Now for Panel position
    const dd4hep::Position aPanelGlobalPos   = r2PhDetP.access()->detectorAlignment().localToWorld( {0, 0, 0} );
    const dd4hep::Position aPanelLocalPos    = r2PhDetP.access()->detector().placement().position();
    const TGeoMatrix&      aPanelOrientation = r2PhDetP.access()->detector().placement().matrix();
    std::cout << "R2 Panel num GlobalPos LocalPos " << iPanel << "  " << aPanelGlobalPos << "  " << aPanelLocalPos
              << "  " << std::endl;
    aPanelOrientation.Print();

    // Loop through the Std modules in a panel
    // For now limiting the printout to a few Modules
    for ( int iMs = 0; iMs < ( r2PhDetP.NumMapmtStdModulesInAPanel() ); iMs++ ) {
      if ( iMs < 2 ) { // This is to limit the size of the printout
        auto r2StdPD = r2PhDetP.StdPDModule( iMs );
        std::cout << " Rich2 Panel StdModule StdModuleCopyNum " << iPanel << "   " << iMs << "   "
                  << r2StdPD.PDModuleCopyNum() << std::endl;

        for ( int iPs = 0; iPs < ( r2StdPD.NumMapmtsInModule() ); iPs++ ) {
          auto r2StdPmt = r2StdPD.Mapmt( iPs );
          std::cout << " Rich2 StdModule Pmt Pmtcopynum " << iMs << "   " << iPs << "   " << r2StdPmt.copyNum()
                    << std::endl;
        }
      }
    }
    // Loop through the Grand modules in a panel
    for ( int iMg = 0; iMg < ( r2PhDetP.NumMapmtGrandModulesInAPanel() ); iMg++ ) {
      if ( iMg < 2 ) { // This is to limit the size of the printout

        auto r2GrandPD = r2PhDetP.GrandPDModule( iMg );
        std::cout << " Rich2 Panel GrandModule GrandModuleCopyNum " << iPanel << "   " << iMg << "   "
                  << r2GrandPD.PDModuleCopyNum() << std::endl;

        for ( int iPs = 0; iPs < ( r2GrandPD.NumMapmtsInModule() ); iPs++ ) {
          auto r2GrandPmt = r2GrandPD.Mapmt( iPs );
          std::cout << " Rich2 GrandModule Pmt Pmtcopynum " << iMg << "   " << iPs << "   " << r2GrandPmt.copyNum()
                    << std::endl;
        }
      }
    }
  }
  //------------------------------------------------------------------------------//
  // End of testing Rich2 PhDet Panel and Ric2 Mapmts

  return 1;
}
DECLARE_APPLY( LHCb_TEST_dump_Rich2, test_dump_Rich2_DetElem )
