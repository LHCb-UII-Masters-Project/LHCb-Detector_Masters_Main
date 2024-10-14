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
//==========================================================================//

#include "Detector/Rich1/RichMatPropData.h"
#include <array>
#include <cassert>
#include <memory>
#include <mutex>

using namespace LHCb::Detector;

//==========================================================================//
void RichMatPropData::SelectAndFillAbleTable( dd4hep::PropertyTable& aTable, RichMatAbLe aAbleType ) {
  if ( aAbleType == RichMatAbLe::alGasQw ) { FillATableFromMap( aTable, m_RichMatGasQwEnAble ); }
}

//==========================================================================//
void RichMatPropData::FillGasQwRiTable( dd4hep::PropertyTable& aTable ) {

  // This may be generalized into single value table filling in the future for different tables.

  double aQwRiValue = detail::dd4hep_param<double>( "RichGasQwRindexValue" );

  for ( int i = 0; i < m_RichMatNumGasQWEnDBins; ++i ) {

    aTable->Set( i, 0, m_RichMatGasQWEnVect[i] );
    aTable->Set( i, 1, aQwRiValue );
  }
}
//==========================================================================//
void RichMatPropData::FillGasQwCkvRiTable( dd4hep::PropertyTable& aTable ) {

  double aQwRiValue = detail::dd4hep_param<double>( "RichGasQwRindexValue" );
  int    iW         = 0;
  for ( int i = 0; i < m_RichMatNumGasQWEnDBins; ++i ) {
    double aEnVal = m_RichMatGasQWEnVect[i];
    if ( ( aEnVal >= m_minCkvRndxEn ) && ( aEnVal <= m_maxCkvRndxEn ) ) {
      aTable->Set( iW, 0, aEnVal );
      aTable->Set( iW, 1, aQwRiValue );
      iW++;
    }
  }
}
//==========================================================================//

void RichMatPropData::SelectAndFillQETable( dd4hep::PropertyTable& aTable, RichMatQE aQEType ) {

  if ( aQEType == RichMatQE::qePmtNominal ) {
    FillATableFromMap( aTable, m_RichPmtNominal_QE_Percent );
  } else if ( aQEType == RichMatQE::qePmtCBAUV ) {
    FillATableFromMap( aTable, m_RichPmtCBAUVGlass_QE_Percent );
  } else if ( aQEType == RichMatQE::qePmtMeanMeas ) {
    FillATableFromMap( aTable, m_RichPmtMeanMeas_QE_Percent );
  } else if ( aQEType == RichMatQE::qePmtHamamatsuNominal ) {
    FillATableFromMap( aTable, m_RichPmtNominal_Hamamatsu_QE_Percent );
  } else if ( aQEType == RichMatQE::qePmtPre2020 ) {
    FillATableFromMap( aTable, m_RichPmtPre2020_QE_Percent );
  }
}
//==========================================================================//
void RichMatPropData::SelectAndFillHVTable( dd4hep::PropertyTable& aTable, RichMatHV ahvType ) {
  if ( ahvType == RichMatHV::hvPmtNominal ) {
    FillATableFromMap( aTable, m_RichPmtHVNominal );
  } else if ( ahvType == RichMatHV::hvPmtClassic ) {
    FillATableFromMap( aTable, m_RichPmtHVClassic );
  }
}
//==========================================================================//
void RichMatPropData::SelectAndFillQWARTable( dd4hep::PropertyTable& aTable, RichSurfCoat aARType ) {
  if ( aARType == RichSurfCoat::arR1GasQW ) { FillATableFromMap( aTable, m_Rich1GasQWARCoatingRefl ); }
}
//==========================================================================//

void RichMatPropData::SelectAndFillScTable( dd4hep::PropertyTable& aTable, RichMatScint aScType ) {

  if ( aScType == RichMatScint::scFast ) {
    FillATableFromMap( aTable, m_RichCF4ScintFast );
  } else if ( aScType == RichMatScint::scFastTime ) {
    FillATableFromMap( aTable, m_RichCF4ScFaTimeConst );
  } else if ( aScType == RichMatScint::scYield ) {
    FillATableFromMap( aTable, m_RichCF4ScintYield );
  } else if ( aScType == RichMatScint::scRes ) {
    FillATableFromMap( aTable, m_RichCF4ScintRes );
  }
}
//==========================================================================//

void RichMatPropData::SelectAndFillRindexTables( dd4hep::PropertyTable& aTable, RichMatRindex aRindexType ) {
  if ( aRindexType == RichMatRindex::riC4F10 ) {
    FillATableFromMap( aTable, m_RichMatC4F10EnRi );
  } else if ( aRindexType == RichMatRindex::riNitrogen ) {
    FillATableFromMap( aTable, m_RichMatNitrogenEnRi );
  } else if ( aRindexType == RichMatRindex::riCF4 ) {
    FillATableFromMap( aTable, m_RichMatCF4EnRi );
  } else if ( aRindexType == RichMatRindex::riCO2 ) {
    FillATableFromMap( aTable, m_RichMatCO2EnRi );
  } else if ( aRindexType == RichMatRindex::riPmtPhCath ) {
    FillATableFromMap( aTable, m_RichMatPmtPhCathRi );
  } else if ( aRindexType == RichMatRindex::riPmtQw ) {
    FillATableFromMap( aTable, m_RichMatPmtQwEnRi );
  }
}
//==========================================================================//

void RichMatPropData::SelectAndFillCkvRndxTables( dd4hep::PropertyTable& aTable, RichMatCkvRndx aCkvRndxType ) {

  if ( aCkvRndxType == RichMatCkvRndx::ciC4F10 ) {
    FillACkvRndxTable( aTable, m_RichMatC4F10EnRi );
  } else if ( aCkvRndxType == RichMatCkvRndx::ciNitrogen ) {
    FillACkvRndxTable( aTable, m_RichMatNitrogenEnRi );
  } else if ( aCkvRndxType == RichMatCkvRndx::ciCF4 ) {
    FillACkvRndxTable( aTable, m_RichMatCF4EnRi );
  } else if ( aCkvRndxType == RichMatCkvRndx::ciCO2 ) {
    FillACkvRndxTable( aTable, m_RichMatCO2EnRi );
  } else if ( aCkvRndxType == RichMatCkvRndx::ciPmtQw ) {
    FillACkvRndxTable( aTable, m_RichMatPmtQwEnRi );
  }
}

//==========================================================================//

void RichMatPropData::FillATableFromMap( dd4hep::PropertyTable& aTable, TabData& aRiMap ) {

  // void RichMatPropData::FillARindexOrALTable(dd4hep::PropertyTable & aTable,
  //                                     std::map<double,double> & aRiMap ){

  int iH = 0;
  for ( auto itH = aRiMap.begin(); itH != aRiMap.end(); ++itH ) {
    double aEnValH = itH->first;
    double aRiValH = itH->second;

    aTable->Set( iH, 0, aEnValH );
    aTable->Set( iH, 1, aRiValH );
    iH++;
  }
}
//==========================================================================//
void RichMatPropData::FillACkvRndxTable( dd4hep::PropertyTable& aTable, TabData& aCkvRiMap ) {

  int iJ = 0;
  for ( auto itR = aCkvRiMap.begin(); itR != aCkvRiMap.end(); ++itR ) {
    double aEnVal = itR->first;
    if ( ( aEnVal >= m_minCkvRndxEn ) && ( aEnVal <= m_maxCkvRndxEn ) ) {
      double aRiVal = itR->second;
      aTable->Set( iJ, 0, aEnVal );
      aTable->Set( iJ, 1, aRiVal );
      iJ++;
    }
  }
}

//==========================================================================//
void RichMatPropData::InitRichMatPropertyValues() {

  m_RichMatGasQWEnVect.clear();

  const auto ueV = 1.0 * dd4hep::eV;

  // m_minCkvRndxEn = 1.6 * ueV;
  // m_maxCkvRndxEn = 6.5 * ueV;
  // To match DetDesc for testing
  m_minCkvRndxEn = 1.50 * ueV;
  m_maxCkvRndxEn = 7.48 * ueV;

  //  m_RichMatGasQWEnVect = {1.2 * ueV,      1.5 * ueV,       1.75 * ueV,      2.0 * ueV,     2.5 * ueV, 3.0 * ueV,
  //                        3.5 * ueV,      4.0 * ueV,       4.5 * ueV,       5.0 * ueV,     5.5 * ueV, 5.6505682 * ueV,
  //                        6.215625 * ueV, 6.5427632 * ueV, 6.7195946 * ueV, 6.90625 * ueV, 7.0 * ueV, 7.5 * ueV};

  m_RichMatGasQWEnVect = {1.6 * ueV, 1.75 * ueV, 2.0 * ueV, 2.5 * ueV,       3.0 * ueV,      3.5 * ueV, 4.0 * ueV,
                          4.5 * ueV, 5.0 * ueV,  5.5 * ueV, 5.6505682 * ueV, 6.215625 * ueV, 6.5 * ueV};

  // m_RichMatNumGasQWEnDBins = (int)m_RichMatGasQWEnVect.size(); // This table has 18 bins
  m_RichMatNumGasQWEnDBins = (int)m_RichMatGasQWEnVect.size(); // This table has 13 bins

  int iNumGasQwCkvBins = 0;
  for ( const auto aE : m_RichMatGasQWEnVect ) {
    // if ( ( aE > m_minCkvRndxEn ) && ( aE < m_maxCkvRndxEn ) ) { iNumGasQwCkvBins++; }
    if ( ( aE >= m_minCkvRndxEn ) && ( aE <= m_maxCkvRndxEn ) ) { iNumGasQwCkvBins++; }
  }

  m_RichMatNumGasQWCkvEnDBins = iNumGasQwCkvBins;

  m_RichMatNumGasEnDBins = 300;
  m_RichMatGasEnVect.reserve( m_RichMatNumGasEnDBins );
  const auto enIncr = ( m_maxCkvRndxEn - m_minCkvRndxEn ) / ( m_RichMatNumGasEnDBins - 1 );
  for ( int i = 0; i < m_RichMatNumGasEnDBins; ++i ) {
    m_RichMatGasEnVect.emplace_back( m_minCkvRndxEn + ( i * enIncr ) );
  }

  // m_RichMatGasEnVect = {1.6 * ueV,  2.0 * ueV,  2.25 * ueV, 2.5 * ueV,  2.75 * ueV, 3.0 * ueV,  3.25 * ueV,
  //                      3.5 * ueV,  3.75 * ueV, 4.0 * ueV,  4.25 * ueV, 4.5 * ueV,  4.75 * ueV, 5.0 * ueV,
  //                      5.25 * ueV, 5.5 * ueV,  5.75 * ueV, 6.0 * ueV,  6.25 * ueV, 6.5 * ueV};

  int iNumGasCkvEnBins = 0;
  for ( const auto aG : m_RichMatGasEnVect ) {
    //    if ( ( aG > m_minCkvRndxEn ) && ( aG < m_maxCkvRndxEn ) ) { iNumGasCkvEnBins++; }
    if ( ( aG >= m_minCkvRndxEn ) && ( aG <= m_maxCkvRndxEn ) ) { iNumGasCkvEnBins++; }
  }

  m_RichMatNumGasCkvEnDBins = iNumGasCkvEnBins;
}

RichMatPropData::TabData                                                    //
RichMatPropData::CreateRefIndxTable( const Rich::RadiatorType gName,        //
                                     const double             aPressure,    //
                                     const double             aTemperature, //
                                     const double             aScaleFactor ) const {

  const auto ueV            = 1.0 * dd4hep::eV;
  const auto aSellLorGasFac = detail::dd4hep_param<double>( "SellLorGasFac" );

  double aSellE1       = 0.0;
  double aSellE2       = 0.0;
  double aSellF1       = 0.0;
  double aSellF2       = 0.0;
  double aGasMolWeight = 0.0;
  double RefRho        = 0.0;

  // the classic sellmeir formulae here are all at reference temperature of 293K

  if ( gName == Rich::Nitrogen ) {
    aSellE1       = detail::dd4hep_param<double>( "SellN2E1" );
    aSellE2       = detail::dd4hep_param<double>( "SellN2E2" );
    aSellF1       = detail::dd4hep_param<double>( "SellN2F1" );
    aSellF2       = detail::dd4hep_param<double>( "SellN2F2" );
    aGasMolWeight = detail::dd4hep_param<double>( "GasMolWeightN2" );
    RefRho        = detail::dd4hep_param<double>( "N2GasRhoCur" );
  } else if ( gName == Rich::C4F10 ) {
    aSellE1       = detail::dd4hep_param<double>( "SellC4F10E1" );
    aSellE2       = detail::dd4hep_param<double>( "SellC4F10E2" );
    aSellF1       = detail::dd4hep_param<double>( "SellC4F10F1" );
    aSellF2       = detail::dd4hep_param<double>( "SellC4F10F2" );
    aGasMolWeight = detail::dd4hep_param<double>( "GasMolWeightC4F10" );
    RefRho        = detail::dd4hep_param<double>( "RhoEffectiveSellC4F10" );
  } else if ( gName == Rich::CF4 ) {
    aSellE1       = detail::dd4hep_param<double>( "SellCF4E1" );
    aSellE2       = detail::dd4hep_param<double>( "SellCF4E2" );
    aSellF1       = detail::dd4hep_param<double>( "SellCF4F1" );
    aSellF2       = detail::dd4hep_param<double>( "SellCF4F2" );
    aGasMolWeight = detail::dd4hep_param<double>( "GasMolWeightCF4" );
    RefRho        = detail::dd4hep_param<double>( "CF4GasRhoCur" );
  }

  // scale for requested P/T values
  const auto refPressure    = getRefPressure( gName );
  const auto refTemperature = getRefTemperature( gName );
  const auto aGasRhoCur     = RefRho * ( aPressure / refPressure ) * ( refTemperature / aTemperature );

  const auto lev = dd4hep::DEBUG;
  dd4hep::printout( lev, "CreateRefIndxTable", "GasType=%i", (int)gName );
  dd4hep::printout( lev, "CreateRefIndxTable", "RefPres=%f RequestedPres=%f", refPressure, aPressure );
  dd4hep::printout( lev, "CreateRefIndxTable", "RefTemp=%f RequestedTemp=%f", refTemperature, aTemperature );
  dd4hep::printout( lev, "CreateRefIndxTable", "E1=%f E2=%f F1=%f F2=%f MolW=%f RefRho=%f RhoCur=%f", //
                    aSellE1, aSellE2, aSellF1, aSellF2, aGasMolWeight, RefRho, aGasRhoCur );

  assert( (std::size_t)m_RichMatNumGasEnDBins == m_RichMatGasEnVect.size() );

  TabData refTable;

  for ( const auto ePho : m_RichMatGasEnVect ) {
    const auto ePhoeV  = ePho / ueV;
    const auto pfe     = ( ( ( aSellF1 / ( ( aSellE1 * aSellE1 ) - ( ePhoeV * ePhoeV ) ) ) +
                         ( aSellF2 / ( ( aSellE2 * aSellE2 ) - ( ePhoeV * ePhoeV ) ) ) ) );
    const auto cpfe    = aSellLorGasFac * ( aGasRhoCur / aGasMolWeight ) * pfe;
    const auto nMinus1 = aScaleFactor * ( std::sqrt( ( 1.0 + ( 2.0 * cpfe ) ) / ( 1.0 - cpfe ) ) - 1.0 );
    dd4hep::printout( dd4hep::VERBOSE, "CreateRefIndxTable", " -> En=%.6e RefIndx=%f", ePho, ( nMinus1 + 1.0 ) );
    refTable.emplace( ePho, ( nMinus1 + 1.0 ) );
  }

  return refTable;
}

void RichMatPropData::CreateGasSellRefIndexValues() {
  m_RichMatNitrogenEnRi = CreateRefIndxTable( Rich::Nitrogen );
  m_RichMatC4F10EnRi    = CreateRefIndxTable( Rich::C4F10 );
  m_RichMatCF4EnRi      = CreateRefIndxTable( Rich::CF4 );
}

void RichMatPropData::InitRichCO2RiVal() {
  m_RichMatCO2EnRi.clear();
  // m_RichMatCO2EnCkvRi.clear();
  const double ueV = 1.0 * dd4hep::eV;
  // Temporary modif for DD4HEP structure
  //  m_RichMatCO2EnRi.insert( raPair( 1.5 * ueV, 1.000363563 ) );
  // m_RichMatCO2EnRi.emplace( 1.6 * ueV, 1.000364 );
  m_RichMatCO2EnRi = {{1.6 * ueV, 1.000364},    {2.1 * ueV, 1.000366881}, {2.7 * ueV, 1.0003714},
                      {3.3 * ueV, 1.000377208}, {3.9 * ueV, 1.000384422}, {4.5 * ueV, 1.000393195},
                      {5.1 * ueV, 1.000403724}, {5.7 * ueV, 1.000416265}, {6.3 * ueV, 1.000431145},
                      {6.5 * ueV, 1.000437028}};

  //  m_RichMatCO2EnRi.insert( raPair( 6.9 * ueV, 1.000448793 ) );
  // m_RichMatCO2EnRi.insert( raPair( 7.5 * ueV, 1.00046901 ) );
  m_RichMatNumCO2EnDBins = (int)m_RichMatCO2EnRi.size();

  int iK = 0;
  for ( auto itC = m_RichMatCO2EnRi.begin(); itC != m_RichMatCO2EnRi.end(); ++itC ) {
    double aEnVal = itC->first;
    if ( ( aEnVal >= m_minCkvRndxEn ) && ( aEnVal <= m_maxCkvRndxEn ) ) { iK++; }
  }
  m_RichMatNumCO2CkvEnDBins = iK;
}
//==========================================================================//
void RichMatPropData::InitPmtQwPhRiVal() {
  m_RichMatPmtQwEnRi.clear();
  m_RichMatPmtPhCathRi.clear();

  double ueV = 1.0 * dd4hep::eV;

  // Now for the PmtQw Rindex
  // Modif for DD4HEP structure

  //  m_RichMatPmtQwEnRi.emplace(  1.55 * ueV, 1.453371 );
  m_RichMatPmtQwEnRi = {
      {1.6 * ueV, 1.45337},   {1.78 * ueV, 1.455347}, {1.80 * ueV, 1.455579},
      {1.83 * ueV, 1.455818}, {1.86 * ueV, 1.456066}, {1.89 * ueV, 1.45646},
      {2.11 * ueV, 1.45851},  {2.12 * ueV, 1.45857},  {2.28 * ueV, 1.46021},
      {2.56 * ueV, 1.46324},  {2.85 * ueV, 1.46679},  {3.12 * ueV, 1.47028},
      {3.44 * ueV, 1.4752},   {3.59 * ueV, 1.47766},  {3.65 * ueV, 1.47877},
      {3.79 * ueV, 1.48183},  {4.17 * ueV, 1.48859},  {4.52 * ueV, 1.49634},
      {4.83 * ueV, 1.50397},  {4.90 * ueV, 1.5059},   {4.97 * ueV, 1.50762},
      {5.34 * ueV, 1.51834},  {5.49 * ueV, 1.52318},  {5.80 * ueV, 1.53385},
      {6.03 * ueV, 1.54269},  {6.14 * ueV, 1.54729},  {6.42 * ueV, 1.56071},
      {6.5 * ueV, 1.56286}

      //  {  6.70 * ueV, 1.57464 },
      // {  7.5 * ueV, 1.57464 }
  };
  m_RichMatNumPmtQwRiEnDBins = (int)m_RichMatPmtQwEnRi.size();

  int iJ = 0;
  for ( auto itR = m_RichMatPmtQwEnRi.begin(); itR != m_RichMatPmtQwEnRi.end(); ++itR ) {
    double aEnVal = itR->first;
    if ( ( aEnVal >= m_minCkvRndxEn ) && ( aEnVal <= m_maxCkvRndxEn ) ) { iJ++; }
  }
  m_RichMatNumPmtQwCkvRiEnDBins = iJ;

  // Now for PhCath Rindex
  // Modif for DD4HEP structure
  //  m_RichMatPmtPhCathRi.emplace(  1.50 * ueV, ( 2.64 + 0.066 ) );
  // m_RichMatPmtPhCathRi.emplace(  1.55 * ueV, ( 2.65 + 0.11 ) );
  m_RichMatPmtPhCathRi = {
      {1.6 * ueV, ( 2.65 + 0.11 )},   {1.66 * ueV, ( 2.67 + 0.16 )},  {1.78 * ueV, ( 2.68 + 0.188 )},
      {1.94 * ueV, ( 2.72 + 0.22 )},  {2.07 * ueV, ( 2.76 + 0.264 )}, {2.22 * ueV, ( 2.84 + 0.337 )},
      {2.39 * ueV, ( 2.91 + 0.411 )}, {2.59 * ueV, ( 3.1 + 0.608 )},  {2.83 * ueV, ( 3.58 + 1.18 )},
      {3.11 * ueV, ( 3.59 + 1.09 )},  {3.45 * ueV, ( 3.33 + 0.86 )},  {3.88 * ueV, ( 3.33 + 0.86 )},
      {4.14 * ueV, ( 3.33 + 0.86 )},  {4.60 * ueV, ( 3.33 + 0.86 )},  {5.18 * ueV, ( 3.33 + 0.86 )},
      {6.22 * ueV, ( 3.33 + 0.86 )},  {6.5 * ueV, ( 3.33 + 0.86 )}
      //  {  7.5 * ueV, ( 3.33 + 0.86 ) }
  };
  m_RichMatNumPmtPhCathRiEnDBins = (int)m_RichMatPmtPhCathRi.size();
}
//==========================================================================//

void RichMatPropData::InitPmtQEValues() {

  m_RichPmtNominal_QE_Percent.clear();
  m_RichPmtNominal_Hamamatsu_QE_Percent.clear();
  m_RichPmtCBAUVGlass_QE_Percent.clear();
  m_RichPmtMeanMeas_QE_Percent.clear();
  m_RichPmtPre2020_QE_Percent.clear();

  double ueV = 1.0 * dd4hep::eV;

  // Nominal QE
  // Nominal QE changed from Hamamatsu sheets to measured values below.

  m_RichPmtPre2020_QE_Percent = {
      {1.61445 * ueV, 0.0},    {1.63569 * ueV, 0.0},    {1.6575 * ueV, 0.0007}, {1.6799 * ueV, 0.0014},
      {1.70291 * ueV, 0.0028}, {1.72656 * ueV, 0.0056}, {1.75088 * ueV, 0.01},  {1.77589 * ueV, 0.0181},
      {1.80163 * ueV, 0.03},   {1.82812 * ueV, 0.06},   {1.85541 * ueV, 0.1},   {1.88352 * ueV, 0.18},
      {1.9125 * ueV, 0.3},     {1.94238 * ueV, 0.49},   {1.97321 * ueV, 0.77},  {2.00504 * ueV, 1.21},
      {2.03791 * ueV, 1.71},   {2.07187 * ueV, 2.29},   {2.10699 * ueV, 3.0},   {2.14332 * ueV, 3.7},
      {2.18092 * ueV, 4.5},    {2.21987 * ueV, 5.2},    {2.26023 * ueV, 6.1},   {2.30208 * ueV, 7.0},
      {2.34552 * ueV, 8.3},    {2.39062 * ueV, 10.8},   {2.4375 * ueV, 13.6},   {2.48625 * ueV, 17.5},
      {2.53699 * ueV, 19.6},   {2.58984 * ueV, 21.7},   {2.64495 * ueV, 23.8},  {2.70245 * ueV, 25.9},
      {2.7625 * ueV, 28.0},    {2.82528 * ueV, 30.0},   {2.89099 * ueV, 31.2},  {2.95982 * ueV, 32.0},
      {3.03201 * ueV, 32.5},   {3.10781 * ueV, 33.0},   {3.1875 * ueV, 33.5},   {3.27138 * ueV, 34.0},
      {3.3598 * ueV, 34.5},    {3.45312 * ueV, 35.0},   {3.55179 * ueV, 35.0},  {3.65625 * ueV, 35.0},
      {3.76705 * ueV, 34.0},   {3.88477 * ueV, 33.0},   {4.01008 * ueV, 32.0},  {4.14375 * ueV, 31.0},
      {4.28664 * ueV, 27.2},   {4.43973 * ueV, 23.6},   {4.60417 * ueV, 20.0},  {4.78125 * ueV, 16.5},
      {4.9725 * ueV, 13.0},    {5.17969 * ueV, 10.0},   {5.40489 * ueV, 1.5},   {5.65057 * ueV, 1.0},
      {5.91964 * ueV, 0.5},    {6.21563 * ueV, 0.0},    {6.54276 * ueV, 0.0},   {6.90625 * ueV, 0.0}};
  m_RichPmtNumNominalHamamatsuQEBins = (int)m_RichPmtNominal_Hamamatsu_QE_Percent.size();

  // CBAUV version

  m_RichPmtCBAUVGlass_QE_Percent = {
      {1.61445 * ueV, 0.0},    {1.63569 * ueV, 0.0},    {1.6575 * ueV, 0.0007}, {1.6799 * ueV, 0.0014},
      {1.70291 * ueV, 0.0028}, {1.72656 * ueV, 0.0056}, {1.75088 * ueV, 0.01},  {1.77589 * ueV, 0.0181},
      {1.80163 * ueV, 0.03},   {1.82812 * ueV, 0.06},   {1.85541 * ueV, 0.1},   {1.88352 * ueV, 0.18},
      {1.9125 * ueV, 0.3},     {1.94238 * ueV, 0.49},   {1.97321 * ueV, 0.77},  {2.00504 * ueV, 1.21},
      {2.03791 * ueV, 1.71},   {2.07187 * ueV, 2.29},   {2.10699 * ueV, 3.0},   {2.14332 * ueV, 3.7},
      {2.18092 * ueV, 4.5},    {2.21987 * ueV, 5.2},    {2.26023 * ueV, 6.1},   {2.30208 * ueV, 7.0},
      {2.34552 * ueV, 8.3},    {2.39062 * ueV, 10.3},   {2.4375 * ueV, 13.1},   {2.48625 * ueV, 15.2},
      {2.53699 * ueV, 16.3},   {2.58984 * ueV, 17.2},   {2.64495 * ueV, 18.4},  {2.70245 * ueV, 19.7},
      {2.7625 * ueV, 20.9},    {2.82528 * ueV, 21.7},   {2.89099 * ueV, 22.4},  {2.95982 * ueV, 23.3},
      {3.03201 * ueV, 23.6},   {3.10781 * ueV, 24.0},   {3.1875 * ueV, 24.5},   {3.27138 * ueV, 24.5},
      {3.3598 * ueV, 24.8},    {3.45312 * ueV, 25.1},   {3.55179 * ueV, 25.0},  {3.65625 * ueV, 25.0},
      {3.76705 * ueV, 24.0},   {3.88477 * ueV, 22.9},   {4.01008 * ueV, 21.8},  {4.14375 * ueV, 20.2},
      {4.28664 * ueV, 18.5},   {4.43973 * ueV, 16.7},   {4.60417 * ueV, 14.8},  {4.78125 * ueV, 12.7},
      {4.9725 * ueV, 10.5},    {5.17969 * ueV, 9.0},    {5.40489 * ueV, 7.8},   {5.65057 * ueV, 6.1},
      {5.91964 * ueV, 5.2},    {6.21563 * ueV, 3.9},    {6.54276 * ueV, 2.7},   {6.90625 * ueV, 0.0},
      {7.0 * ueV, 0.0}};
  m_RichPmtNumCBAUVQEBins = (int)m_RichPmtCBAUVGlass_QE_Percent.size();

  m_RichPmtPre2020_QE_Percent = {
      {1.61445 * ueV, 0.00},  {1.65750 * ueV, 0.00},  {1.70291 * ueV, 0.00},  {1.72656 * ueV, 0.01},
      {1.77589 * ueV, 0.02},  {1.82813 * ueV, 0.16},  {1.88352 * ueV, 0.50},  {1.94238 * ueV, 1.23},
      {2.00504 * ueV, 2.61},  {2.07188 * ueV, 3.99},  {2.14332 * ueV, 5.30},  {2.21987 * ueV, 6.94},
      {2.30208 * ueV, 9.14},  {2.39063 * ueV, 12.19}, {2.48625 * ueV, 18.20}, {2.58984 * ueV, 22.59},
      {2.70245 * ueV, 25.65}, {2.82528 * ueV, 29.18}, {2.95982 * ueV, 32.93}, {3.10781 * ueV, 34.36},
      {3.27138 * ueV, 36.11}, {3.45313 * ueV, 38.29}, {3.65625 * ueV, 38.40}, {3.88477 * ueV, 37.53},
      {4.14375 * ueV, 32.94}, {4.43973 * ueV, 26.75}, {4.78125 * ueV, 25.61}, {5.17969 * ueV, 22.37},
      {5.65057 * ueV, 16.87}, {6.21563 * ueV, 9.63},  {6.2300 * ueV, 0.00},   {6.2500 * ueV, 0.00},
      {6.5000 * ueV, 0.00},   {6.5428 * ueV, 0.00},   {6.9063 * ueV, 0.00}};

  m_RichPmtNumPre2020QEBins    = (int)m_RichPmtPre2020_QE_Percent.size();
  m_RichPmtMeanMeas_QE_Percent = {
      {1.61445 * ueV, 0.00},  {1.65750 * ueV, 0.00},  {1.70291 * ueV, 0.00},  {1.72656 * ueV, 0.01},
      {1.77589 * ueV, 0.01},  {1.82813 * ueV, 0.11},  {1.88352 * ueV, 0.36},  {1.94238 * ueV, 0.91},
      {2.00504 * ueV, 1.93},  {2.07188 * ueV, 3.32},  {2.14332 * ueV, 5.02},  {2.21987 * ueV, 6.97},
      {2.30208 * ueV, 9.47},  {2.39063 * ueV, 12.31}, {2.48625 * ueV, 17.96}, {2.58984 * ueV, 22.80},
      {2.70245 * ueV, 25.57}, {2.82528 * ueV, 28.72}, {2.95982 * ueV, 31.32}, {3.10781 * ueV, 33.03},
      {3.27138 * ueV, 33.93}, {3.45313 * ueV, 37.01}, {3.65625 * ueV, 38.29}, {3.88477 * ueV, 39.29},
      {4.14375 * ueV, 38.72}, {4.43973 * ueV, 34.83}, {4.78125 * ueV, 32.23}, {5.17969 * ueV, 26.65},
      {5.65057 * ueV, 20.72}, {6.21563 * ueV, 15.29}, {6.23000 * ueV, 0.00},  {6.25000 * ueV, 0.00},
      {6.50000 * ueV, 0.00},  {6.54276 * ueV, 0.00},  {6.90625 * ueV, 0.00}};

  m_RichPmtNumMeanMeasQEBins = (int)m_RichPmtMeanMeas_QE_Percent.size();

  m_RichPmtNominal_QE_Percent = m_RichPmtMeanMeas_QE_Percent;
  m_RichPmtNumNominalQEBins   = (int)m_RichPmtNominal_QE_Percent.size();
}
//==========================================================================//
void RichMatPropData::InitPmtHVValues() {
  m_RichPmtHVNominal.clear();
  m_RichPmtHVClassic.clear();
  typedef std::pair<double, double> hvPair;
  double                            ueV = 1.0 * dd4hep::eV;

  m_RichPmtHVClassic.emplace( hvPair( 1.0, 20000 * ueV ) );
  m_RichPmtNumHVClassicBins = (int)m_RichPmtHVClassic.size();

  m_RichPmtHVNominal.emplace( hvPair( 1.0, 1000 * ueV ) );
  m_RichPmtNumHVNominalBins = (int)m_RichPmtHVNominal.size();
}
//==========================================================================//
void RichMatPropData::InitGasQWARCoatReflValues() {

  m_Rich1GasQWARCoatingRefl.clear();

  const double ueV = 1.0 * dd4hep::eV;

  m_Rich1GasQWARCoatingRefl = {
      {0.9000 * ueV, 0.03046}, {1.0000 * ueV, 0.03046}, {1.2431 * ueV, 0.03046}, {1.5000 * ueV, 0.02866},
      {1.5539 * ueV, 0.02835}, {2.0000 * ueV, 0.02544}, {2.5000 * ueV, 0.02247}, {3.0000 * ueV, 0.01996},
      {3.5000 * ueV, 0.01830}, {4.0000 * ueV, 0.01821}, {4.5000 * ueV, 0.02020}, {5.0000 * ueV, 0.02403},
      {5.5000 * ueV, 0.03131}, {5.6506 * ueV, 0.03381}, {6.2156 * ueV, 0.03712}, {6.5428 * ueV, 0.03793},
      {6.7196 * ueV, 0.01984}, {6.9063 * ueV, 0.00000}, {7.0000 * ueV, 0.00000}, {7.5000 * ueV, 0.00000},
      {8.0000 * ueV, 0.00000}, {8.5000 * ueV, 0.00000}, {9.0000 * ueV, 0.00000}, {9.5000 * ueV, 0.00000},
      {10.0000 * ueV, 0.00000}};

  m_Rich1GasQWARReflNumBins = (int)m_Rich1GasQWARCoatingRefl.size();
}
//==========================================================================//
void RichMatPropData::InitGasQWAbleVal() {

  const double umm = 1.0 * dd4hep::mm;

  const auto aLowEnAbLeValue = detail::dd4hep_param<double>( "RichGasQwAbsLengthLowEnValue" );
  // double aHighEnAbLeValue = detail::dd4hep_param<double>( "RichGasQwAbsLengthHighEnValue" );

  m_RichMatGasQwEnAble.clear();

  // Modif for DD4HEP structure
  //  std::vector<double>               aSpAble = {1.0e18 * umm,  1.0e18 * umm,  494.983 * umm,    194.957 * umm,
  //                               94.9122 * umm, 44.8142 * umm, aHighEnAbLeValue, aHighEnAbLeValue};

  std::vector<double> aSpAble = {1.0e18 * umm, 1.0e18 * umm, 494.983 * umm, 194.957 * umm};

  //  int                               iLowLim = 10;
  int iLowLim = 9;
  for ( int i = 0; i < iLowLim; i++ ) { m_RichMatGasQwEnAble.emplace( m_RichMatGasQWEnVect[i], aLowEnAbLeValue ); }
  for ( int j = 0; j < (int)aSpAble.size(); j++ ) {
    const auto k = j + iLowLim;
    m_RichMatGasQwEnAble.emplace( m_RichMatGasQWEnVect[k], aSpAble[j] );
  }

  m_RichMatNumGasQWAbleDBins = (int)m_RichMatGasQwEnAble.size();
}

//==========================================================================//
void RichMatPropData::InitCF4ScintValues() {
  m_RichCF4ScintFast.clear();
  m_RichCF4ScFaTimeConst.clear();
  m_RichCF4ScintYield.clear();
  m_RichCF4ScintRes.clear();

  const double ueV = 1.0 * dd4hep::eV;
  // Modif for DD4HEP structure
  m_RichCF4ScintFast = {
      {1.6 * ueV, 0.0005}, {1.75 * ueV, 0.001}, {2.00 * ueV, 0.18}, {2.25 * ueV, 0.05}, {2.50 * ueV, 0.01},
      {2.75 * ueV, 0.05},  {3.00 * ueV, 0.10},  {3.25 * ueV, 0.12}, {3.50 * ueV, 0.15}, {3.75 * ueV, 0.25},
      {4.00 * ueV, 0.48},  {4.25 * ueV, 0.65},  {4.50 * ueV, 0.35}, {4.75 * ueV, 0.28}, {5.00 * ueV, 0.22},
      {5.25 * ueV, 0.38},  {5.50 * ueV, 0.37},  {5.75 * ueV, 0.10}, {6.00 * ueV, 0.01}, {6.25 * ueV, 0.0004},
      {6.50 * ueV, 0.0001}
      //  {  6.75 * ueV, 0.00001 },
      // {  7.00 * ueV, 0.0 }
  };

  m_RichCF4NumSCFastBins = (int)m_RichCF4ScintFast.size();

  double uNanoS = 1.0 * dd4hep::ns;

  double aCF4ScintillationFastTimeConstantInNanoSec =
      detail::dd4hep_param<double>( "CF4ScintillationFastTimeConstantInNanoSec" );
  //  m_RichCF4ScFaTimeConst.emplace(  1.75 * ueV, aCF4ScintillationFastTimeConstantInNanoSec * uNanoS );
  // m_RichCF4ScFaTimeConst.emplace(  7.00 * ueV, aCF4ScintillationFastTimeConstantInNanoSec * uNanoS );
  m_RichCF4ScFaTimeConst.emplace( 1.6 * ueV, aCF4ScintillationFastTimeConstantInNanoSec * uNanoS );
  m_RichCF4ScFaTimeConst.emplace( 6.5 * ueV, aCF4ScintillationFastTimeConstantInNanoSec * uNanoS );
  m_RichCF4NumScFaTimeBins = (int)m_RichCF4ScFaTimeConst.size();

  double aCF4ScintillationYieldPerMeV = detail::dd4hep_param<double>( "CF4ScintillationYieldPerMeV" );
  //  m_RichCF4ScintYield.emplace(  1.75 * ueV, aCF4ScintillationYieldPerMeV );
  // m_RichCF4ScintYield.emplace(  7.00 * ueV, aCF4ScintillationYieldPerMeV );
  m_RichCF4ScintYield.emplace( 1.6 * ueV, aCF4ScintillationYieldPerMeV );
  m_RichCF4ScintYield.emplace( 6.5 * ueV, aCF4ScintillationYieldPerMeV );
  m_RichCF4NumScYieldBins = (int)m_RichCF4ScintYield.size();

  double aCF4ScintillationResolutionScale = detail::dd4hep_param<double>( "CF4ScintillationResolutionScale" );
  //  m_RichCF4ScintRes.emplace(  1.75 * ueV, aCF4ScintillationResolutionScale );
  // m_RichCF4ScintRes.emplace(  7.00 * ueV, aCF4ScintillationResolutionScale );
  m_RichCF4ScintRes.emplace( 1.6 * ueV, aCF4ScintillationResolutionScale );
  m_RichCF4ScintRes.emplace( 6.5 * ueV, aCF4ScintillationResolutionScale );
  m_RichCF4NumScResBins = (int)m_RichCF4ScintRes.size();
}
//==========================================================================//

RichMatPropData* RichMatPropData::getRichMatPropDataInstance() {
  static std::once_flag                   alloc_instance_once;
  static std::unique_ptr<RichMatPropData> RichMatPropDataInstance;
  std::call_once( alloc_instance_once, []() { RichMatPropDataInstance = std::make_unique<RichMatPropData>(); } );
  assert( RichMatPropDataInstance.get() );
  return RichMatPropDataInstance.get();
}
