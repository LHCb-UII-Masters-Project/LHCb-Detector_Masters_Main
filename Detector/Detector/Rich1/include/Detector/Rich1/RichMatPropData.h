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

#include "DD4hep/Objects.h"
#include "DD4hep/PropertyTable.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich1/RichMatOPD.h"
#include "XML/Utilities.h"
#include <map>

class RichMatPropData {

public:
  using TabData = LHCb::Detector::XYTable;

public:
  void InitRichMatPropertyValues();
  void InitPmtQwPhRiVal();
  void CreateGasSellRefIndexValues();
  void InitRichCO2RiVal();
  void InitGasQWAbleVal();
  void InitPmtQEValues();
  void InitPmtHVValues();
  void InitCF4ScintValues();
  void InitGasQWARCoatReflValues();

  inline auto getRefTemperature( const Rich::RadiatorType gName ) const noexcept {
    using namespace LHCb::Detector;
    double refTemperature = 0.0;
    if ( gName == Rich::Nitrogen ) {
      refTemperature = detail::dd4hep_param<double>( "Rich1NitrogenTemperature" );
    } else if ( gName == Rich::C4F10 ) {
      refTemperature = detail::dd4hep_param<double>( "C4F10ReferenceTemperature" );
    } else if ( gName == Rich::CF4 ) {
      refTemperature = detail::dd4hep_param<double>( "CF4ReferenceTemperatureForSellmeirClassic" );
    }
    return refTemperature;
  }

  inline auto getRefPressure( const Rich::RadiatorType gName ) const noexcept {
    using namespace LHCb::Detector;
    double refPressure = 0.0;
    if ( gName == Rich::Nitrogen ) {
      refPressure = detail::dd4hep_param<double>( "Rich1NitrogenPressure" );
    } else if ( gName == Rich::C4F10 ) {
      refPressure = detail::dd4hep_param<double>( "C4F10ReferencePressure" );
    } else if ( gName == Rich::CF4 ) {
      refPressure = detail::dd4hep_param<double>( "CF4Pressure" );
    }
    return refPressure;
  }

  TabData CreateRefIndxTable( const Rich::RadiatorType gName, const double aPressure, const double aTemperature,
                              const double aScaleFactor = 1.0 ) const;

  inline auto CreateRefIndxTable( const Rich::RadiatorType gName ) const {
    return CreateRefIndxTable( gName, getRefPressure( gName ), getRefTemperature( gName ) );
  }

  void SelectAndFillAbleTable( dd4hep::PropertyTable& aTable, RichMatAbLe aAbleType );
  void FillGasQwRiTable( dd4hep::PropertyTable& aTable );
  void FillGasQwCkvRiTable( dd4hep::PropertyTable& aTable );
  void FillATableFromMap( dd4hep::PropertyTable& aTable, TabData& aRiMap );
  //  void FillARindexOrALTable(dd4hep::PropertyTable & aTable, TabData & aRiMap );
  void FillACkvRndxTable( dd4hep::PropertyTable& aTable, TabData& aCkvRiMap );
  void SelectAndFillRindexTables( dd4hep::PropertyTable& aTable, RichMatRindex aRindexType );
  void SelectAndFillCkvRndxTables( dd4hep::PropertyTable& aTable, RichMatCkvRndx aCkvRndxType );
  void SelectAndFillQETable( dd4hep::PropertyTable& aTable, RichMatQE aQEType );
  void SelectAndFillHVTable( dd4hep::PropertyTable& aTable, RichMatHV ahvType );
  void SelectAndFillScTable( dd4hep::PropertyTable& aTable, RichMatScint aScType );
  void SelectAndFillQWARTable( dd4hep::PropertyTable& aTable, RichSurfCoat aARType );

  auto RichMatNumGasQWAbleDBins() const { return m_RichMatNumGasQWAbleDBins; }
  auto RichMatNumGasQWEnDBins() const { return m_RichMatNumGasQWEnDBins; }
  auto RichMatNumGasQWCkvEnDBins() const { return m_RichMatNumGasQWCkvEnDBins; }
  auto RichMatNumPmtQwRiEnDBins() const { return m_RichMatNumPmtQwRiEnDBins; }
  auto RichMatNumPmtQwCkvRiEnDBins() const { return m_RichMatNumPmtQwCkvRiEnDBins; }
  auto RichMatNumPmtPhCathRiEnDBins() const { return m_RichMatNumPmtPhCathRiEnDBins; }

  auto RichMatNumGasEnDBins() const { return m_RichMatNumGasEnDBins; }
  auto RichMatNumGasCkvEnDBins() const { return m_RichMatNumGasCkvEnDBins; }

  auto RichMatNumCO2EnDBins() const { return m_RichMatNumCO2EnDBins; }
  auto RichMatNumCO2CkvEnDBins() const { return m_RichMatNumCO2CkvEnDBins; }

  auto RichPmtNumNominalQEBins() const { return m_RichPmtNumNominalQEBins; }
  auto RichPmtNumNominalHamamatsuQEBins() const { return m_RichPmtNumNominalHamamatsuQEBins; }
  auto RichPmtNumCBAUVQEBins() const { return m_RichPmtNumCBAUVQEBins; }
  auto RichPmtNumMeanMeasQEBins() const { return m_RichPmtNumMeanMeasQEBins; }
  auto RichPmtNumPre2020QEBins() const { return m_RichPmtNumPre2020QEBins; }

  auto RichPmtNumHVNominalBins() const { return m_RichPmtNumHVNominalBins; }
  auto RichPmtNumHVClassicBins() const { return m_RichPmtNumHVClassicBins; }

  auto RichCF4NumSCFastBins() const { return m_RichCF4NumSCFastBins; }
  auto RichCF4NumScFaTimeBins() const { return m_RichCF4NumScFaTimeBins; }
  auto RichCF4NumScYieldBins() const { return m_RichCF4NumScYieldBins; }
  auto RichCF4NumScResBins() const { return m_RichCF4NumScResBins; }
  auto Rich1GasQWARReflNumBins() const { return m_Rich1GasQWARReflNumBins; }

  const auto& RichPmtNominal_QE_Percent() const { return m_RichPmtNominal_QE_Percent; }
  const auto& RichPmtNominal_Hamamatsu_QE_Percent() const { return m_RichPmtNominal_Hamamatsu_QE_Percent; }
  const auto& RichPmtCBAUVGlass_QE_Percent() const { return m_RichPmtCBAUVGlass_QE_Percent; }
  const auto& RichPmtMeanMeas_QE_Percent() const { return m_RichPmtMeanMeas_QE_Percent; }
  const auto& RichPmtPre2020_QE_Percent() const { return m_RichPmtPre2020_QE_Percent; }

  const auto& Rich1GasQWARCoatingRefl() const { return m_Rich1GasQWARCoatingRefl; }

  const auto& RichMatNitrogenEnRi() const { return m_RichMatNitrogenEnRi; }
  const auto& RichMatC4F10EnRi() const { return m_RichMatC4F10EnRi; }
  const auto& RichMatCF4EnRi() const { return m_RichMatCF4EnRi; }
  const auto& RichMatCO2EnRi() const { return m_RichMatCO2EnRi; }

  static RichMatPropData* getRichMatPropDataInstance();

private:
  std::vector<double> m_RichMatGasQWEnVect;

  std::vector<double> m_RichMatGasEnVect;

  TabData m_RichMatGasQwEnAble;

  TabData m_RichMatPmtQwEnRi;
  TabData m_RichMatPmtPhCathRi;

  TabData m_RichMatNitrogenEnRi;
  TabData m_RichMatC4F10EnRi;
  TabData m_RichMatCF4EnRi;
  TabData m_RichMatCO2EnRi;

  TabData m_RichPmtNominal_QE_Percent;
  TabData m_RichPmtNominal_Hamamatsu_QE_Percent;
  TabData m_RichPmtCBAUVGlass_QE_Percent;
  TabData m_RichPmtMeanMeas_QE_Percent;
  TabData m_RichPmtPre2020_QE_Percent;

  TabData m_RichPmtHVNominal;
  TabData m_RichPmtHVClassic;

  TabData m_RichCF4ScintFast;
  TabData m_RichCF4ScFaTimeConst;
  TabData m_RichCF4ScintYield;
  TabData m_RichCF4ScintRes;

  TabData m_Rich1GasQWARCoatingRefl;

  double m_minCkvRndxEn{};
  double m_maxCkvRndxEn{};

  int m_RichMatNumGasQWAbleDBins{};
  int m_RichMatNumGasQWEnDBins{};
  int m_RichMatNumGasQWCkvEnDBins{};
  int m_RichMatNumPmtQwRiEnDBins{};
  int m_RichMatNumPmtQwCkvRiEnDBins{};
  int m_RichMatNumPmtPhCathRiEnDBins{};

  int m_RichMatNumGasEnDBins{};
  int m_RichMatNumGasCkvEnDBins{};

  int m_RichMatNumCO2EnDBins{};
  int m_RichMatNumCO2CkvEnDBins{};

  int m_RichPmtNumNominalQEBins{};
  int m_RichPmtNumNominalHamamatsuQEBins{};
  int m_RichPmtNumCBAUVQEBins{};
  int m_RichPmtNumMeanMeasQEBins{};
  int m_RichPmtNumPre2020QEBins{};

  int m_RichPmtNumHVNominalBins{};
  int m_RichPmtNumHVClassicBins{};

  int m_RichCF4NumSCFastBins{};
  int m_RichCF4NumScFaTimeBins{};
  int m_RichCF4NumScYieldBins{};
  int m_RichCF4NumScResBins{};

  int m_Rich1GasQWARReflNumBins{};
};
