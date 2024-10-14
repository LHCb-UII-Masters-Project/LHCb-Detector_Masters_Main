//==========================================================================
//  LHCb Rich Detector geometry utility class  using DD4HEP
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
//
// Author     : Sajan Easo
// Date       : 2020-09-29
//
//==========================================================================

#pragma once

#include "Core/UpgradeTags.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Objects.h"
#include "DD4hep/Printout.h"
#include "DD4hep/PropertyTable.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"
#include "XML/Utilities.h"
#include <type_traits>

enum class RichMatAbLe : int {
  alAir,
  alPmtVac,
  alMirrQw,
  alGasQw,
  alPmtQw,
  alNitrogen,
  alC4F10,
  alCF4,
  alCO2,
  alPmtPhCath
};
enum class RichMatRindex : int {
  riAir,
  riPmtVac,
  riMirrQw,
  riGasQw,
  riPmtQw,
  riPmtPhCath,
  riNitrogen,
  riC4F10,
  riCF4,
  riCO2
};
enum class RichMatCkvRndx : int { ciGasQw, ciPmtQw, ciNitrogen, ciC4F10, ciCF4, ciCO2 };
enum class RichMatScint : int { scFast, scFastTime, scYield, scRes };
enum class RichMatQE : int { qePmtNominal, qePmtCBAUV, qePmtMeanMeas, qePmtHamamatsuNominal, qePmtPre2020 };
enum class RichMatHV : int { hvPmtNominal, hvPmtClassic };
enum class RichSurfCoat : int { arR1GasQW };
enum class RichMatPropType : int {
  AbLe,
  Rindex,
  CkvRndx,
  scFast,
  scFastTime,
  scYield,
  scRes,
  scintGeneral,
  pmtQE,
  pmtHV,
  FresAR,
  TpName
};
enum class RichMatNameWOP : int {
  R1GasQw,
  R2GasQw,
  PmtQw,
  PmtVac,
  PmtPhc,
  R1Nitrogen,
  R2Nitrogen,
  R1RGas,
  R1c4f10,
  R2RGas,
  R2cf4,
  Rhco2,
  Rhair,
  R1M2Glass,
  R2MGlass,
  RhNonMat
};

/// Utility class for build the Rich Material Optical properties in LHCb

class RichMatOPD {

public:
  using TabData = LHCb::Detector::XYTable;

public:
  template <typename E>
  constexpr auto toInTy( const E e ) const noexcept {
    return static_cast<std::underlying_type_t<E>>( e );
  }

public:
  void InitRichMatOPD();
  void InitRichFillMatNames( const Rich::Detector::NamedVector<RichMatNameWOP>& aRichMatN );
  void InitRichFillMatPropNames( const Rich::Detector::NamedVector<int>& aRichMatC, RichMatPropType aType );
  void InitRichMPEnVal();

  const auto& RichMatNameWithOptProp( RichMatNameWOP aW ) const { return m_RichMatNameWithOptProp[toInTy( aW )]; }

  const auto& getAbLeTableName( RichMatAbLe aT ) const { return m_RichMatAbsLengthNames[toInTy( aT )]; }
  const auto& getRindexTableName( RichMatRindex aR ) const { return m_RichMatRindexNames[toInTy( aR )]; }
  const auto& getCkvRndxTableName( RichMatCkvRndx aC ) const { return m_RichMatCkvRndxNames[toInTy( aC )]; }
  const auto& getPmtQETableName( RichMatQE aQ ) const { return m_RichMatPmtQENames[toInTy( aQ )]; }
  const auto& getPmtHVTableName( RichMatHV aH ) const { return m_RichMatPmtHVNames[toInTy( aH )]; }
  const auto& getCF4ScintTableName( RichMatScint aI ) const { return m_RichMatScintillationNames[toInTy( aI )]; }
  const auto& getFresARTableName( RichSurfCoat aS ) const { return m_RichMatFresARNames[toInTy( aS )]; }
  const auto& RichMatPropTypeName( RichMatPropType aY ) const { return m_RichMatPropTypeName[toInTy( aY )]; }

  auto RichMatNumAbsLenPropNames() const { return m_RichMatNumAbsLenPropNames; }
  auto RichMatNumStdEnBins() const { return m_RichMatNumStdEnBins; }

  int RichMatNumGasQWAbleBins() const;

  int RichMatNumGasQWEnBins() const;

  int RichMatNumStdCkvEnBins() const { return m_RichMatNumStdCkvEnBins; }
  int RichMatNumGasQWCkvEnBins() const;
  int RichMatNumPmtQwRiEnBins() const;
  int RichMatNumPmtQwCkvRiEnBins() const;
  int RichMatNumPmtPhCathRiEnBins() const;

  int RichMatNumGasEnBins() const;
  int RichMatNumGasCkvEnBins() const;
  int RichMatNumCO2EnBins() const;
  int RichMatNumCO2CkvEnBins() const;

  void FillGeneralAbLeTable( dd4hep::PropertyTable& aTable, RichMatAbLe aAbLeType );
  void FillGeneralRindexTable( dd4hep::PropertyTable& aTable, RichMatRindex aRindexType );
  void FillStdSingleValueTable( dd4hep::PropertyTable& aTable, double aValue );
  void FillGeneralTable( dd4hep::PropertyTable& aTable, RichMatNameWOP aMatNameW, RichMatPropType aPropType );

  void AttachTableToMaterial( TGDMLMatrix* aTableT, RichMatPropType aPropType, std::string aTableName,
                              dd4hep::Material& aRichCurMat );

  void PrintRichMatProperty( const dd4hep::Material& aMat, RichMatPropType aTableType );
  void PrintAGDMLMatrix( TGDMLMatrix* aTableG );

  TabData GetRichMatProperty( const dd4hep::Material& aMat, RichMatPropType aTableType );
  TabData GetARichGeneralGDMLMatrix( TGDMLMatrix* aTableG );

  int NumTabCol() const { return m_NumTabCol; }

  std::string GetTableName( RichMatNameWOP aMatNameE, RichMatPropType aPropType );
  std::string GetQEGeneralTableName( RichMatPropType aPropType, RichMatQE aQEType );

  int GetNumBinsInTable( RichMatNameWOP aMatNameE, RichMatPropType aPropType );
  int GetQENumBinsInGeneralTable( RichMatPropType aPropType, RichMatQE aQEType );
  int GetHVNumBinsInGeneralTable( RichMatPropType aPropType, RichMatHV ahvType );
  int GetSCNumBinsInGeneralTable( RichMatPropType aPropType, RichMatScint aScintType );
  int GetQWARNumBinsInGeneralTable( RichMatPropType aPropType, RichSurfCoat aQWARType );

  static RichMatOPD* getRichMatOPDInstance();

  RichMatOPD() {
    InitRichMatOPD();
    InitRichMPEnVal();
  }

private:
  std::vector<std::string> m_RichMatNameWithOptProp;
  std::vector<std::string> m_RichMatPropTypeName;

  std::vector<std::string> m_RichMatAbsLengthNames;
  std::vector<std::string> m_RichMatRindexNames;
  std::vector<std::string> m_RichMatCkvRndxNames;
  std::vector<std::string> m_RichMatScintillationNames;
  std::vector<std::string> m_RichMatPmtQENames;
  std::vector<std::string> m_RichMatPmtHVNames;
  std::vector<std::string> m_RichMatFresARNames;

  int m_RichMatNumTypes{};

  int m_RichNumMatWithOptProp{};
  int m_RichMatNumAbsLenPropNames{};
  int m_RichMatNumRindexPropNames{};
  int m_RichMatNumCkvRndxPropNames{};
  int m_RichMatNumScintPropNames{};
  int m_RichMatNumQePropNames{};
  int m_RichMatNumHvNames{};
  int m_RichMatNumFresARNames{};

  int m_NumTabCol{};

  std::vector<double> m_RichMatStdEnVect;

  std::vector<double> m_RichMatStdCkvEnVect;

  int m_RichMatNumStdEnBins{};

  int m_RichMatNumStdCkvEnBins{};
};
