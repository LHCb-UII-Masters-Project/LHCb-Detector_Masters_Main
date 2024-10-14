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

#include "Detector/Rich1/RichMatOPD.h"
#include "Detector/Rich1/RichMatPropData.h"
#include <cassert>
#include <memory>
#include <mutex>

using namespace LHCb::Detector;

//===========================================================================================//

std::string RichMatOPD::GetQEGeneralTableName( RichMatPropType aPropType, RichMatQE aQEType ) {

  std::string aTableName = "aDummyQETableName";

  if ( aPropType == RichMatPropType::pmtQE ) {
    if ( aQEType == RichMatQE::qePmtNominal ) {
      aTableName = getPmtQETableName( RichMatQE::qePmtNominal );
    } else if ( aQEType == RichMatQE::qePmtCBAUV ) {
      aTableName = getPmtQETableName( RichMatQE::qePmtCBAUV );
    } else if ( aQEType == RichMatQE::qePmtMeanMeas ) {
      aTableName = getPmtQETableName( RichMatQE::qePmtMeanMeas );
    } else if ( aQEType == RichMatQE::qePmtPre2020 ) {
      aTableName = getPmtQETableName( RichMatQE::qePmtPre2020 );
    }
  }
  return aTableName;
}
//===========================================================================================//

std::string RichMatOPD::GetTableName( RichMatNameWOP aMatNameE, RichMatPropType aPropType ) {
  std::string aTableName = "aDummyTableName";
  if ( aMatNameE == RichMatNameWOP::Rhair ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alAir );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riAir );
    }
  } else if ( ( aMatNameE == RichMatNameWOP::R1GasQw ) || ( aMatNameE == RichMatNameWOP::R2GasQw ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alGasQw );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riGasQw );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aTableName = getCkvRndxTableName( RichMatCkvRndx::ciGasQw );
    }
  } else if ( aMatNameE == RichMatNameWOP::PmtVac ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alPmtVac );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riPmtVac );
    }
  } else if ( ( aMatNameE == RichMatNameWOP::R1M2Glass ) || ( aMatNameE == RichMatNameWOP::R2MGlass ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alMirrQw );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riMirrQw );
    }
  } else if ( aMatNameE == RichMatNameWOP::PmtQw ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alPmtQw );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riPmtQw );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aTableName = getCkvRndxTableName( RichMatCkvRndx::ciPmtQw );
    }
  } else if ( aMatNameE == RichMatNameWOP::PmtPhc ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alPmtPhCath );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riPmtPhCath );
    }

  } else if ( ( aMatNameE == RichMatNameWOP::R1c4f10 ) || ( aMatNameE == RichMatNameWOP::R1RGas ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alC4F10 );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riC4F10 );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aTableName = getCkvRndxTableName( RichMatCkvRndx::ciC4F10 );
    }
  } else if ( ( aMatNameE == RichMatNameWOP::R1Nitrogen ) || ( aMatNameE == RichMatNameWOP::R2Nitrogen ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alNitrogen );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riNitrogen );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aTableName = getCkvRndxTableName( RichMatCkvRndx::ciNitrogen );
    }

  } else if ( ( aMatNameE == RichMatNameWOP::R2cf4 ) || ( aMatNameE == RichMatNameWOP::R2RGas ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alCF4 );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riCF4 );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aTableName = getCkvRndxTableName( RichMatCkvRndx::ciCF4 );
    }

  } else if ( aMatNameE == RichMatNameWOP::Rhco2 ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aTableName = getAbLeTableName( RichMatAbLe::alCO2 );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aTableName = getRindexTableName( RichMatRindex::riCO2 );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aTableName = getCkvRndxTableName( RichMatCkvRndx::ciCO2 );
    }
  }

  return aTableName;
}
//===========================================================================================//

void RichMatOPD::AttachTableToMaterial( TGDMLMatrix* aTableT, RichMatPropType aPropType, std::string aTableName,
                                        dd4hep::Material& aRichCurMat ) {
  if ( ( aRichCurMat->GetId() ) != 0 ) {
    if ( aTableT ) {
      auto aMType = RichMatPropTypeName( aPropType );
      aRichCurMat->GetMaterial()->AddProperty( aMType.c_str(), aTableName.c_str() );
    } else {
      throw std::runtime_error( "RichMatOPD Material: " + std::string( aRichCurMat.name() ) +
                                " Property Table missing  " + aTableName + ".  Hence unable to attach Table. " );
    }
  } else {
    throw std::runtime_error( "RichMatOPD: Material: " + std::string( aRichCurMat.name() ) +
                              " does not exist. Hence a Property cannot be attached to it " );
  }
}

int RichMatOPD::RichMatNumGasQWAbleBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumGasQWAbleDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumGasQWEnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumGasQWEnDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumGasQWCkvEnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumGasQWCkvEnDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumPmtQwRiEnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumPmtQwRiEnDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumPmtQwCkvRiEnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumPmtQwCkvRiEnDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumPmtPhCathRiEnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumPmtPhCathRiEnDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumGasEnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumGasEnDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumGasCkvEnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumGasCkvEnDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumCO2EnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumCO2EnDBins();
}
//==============================================================================================//
int RichMatOPD::RichMatNumCO2CkvEnBins() const {
  return RichMatPropData::getRichMatPropDataInstance()->RichMatNumCO2CkvEnDBins();
}
//==============================================================================================//
void RichMatOPD::InitRichMPEnVal() {

  // Number of colums in a Table of properties
  m_NumTabCol = 2;

  // standard energy vector values for the tables

  m_RichMatStdEnVect.clear();

  m_RichMatStdCkvEnVect.clear();

  const double ueV = 1.0 * dd4hep::eV;

  // Modification for DD4HEP structure
  // m_RichMatStdEnVect = {1.2 * ueV, 1.5 * ueV, 2.0 * ueV, 2.5 * ueV, 3.0 * ueV, 3.5 * ueV, 4.0 * ueV,
  //                      4.5 * ueV, 5.0 * ueV, 5.5 * ueV, 6.0 * ueV, 6.5 * ueV, 7.0 * ueV, 7.5 * ueV};

  m_RichMatStdEnVect = {1.6 * ueV, 2.0 * ueV, 2.5 * ueV, 3.0 * ueV, 3.5 * ueV, 4.0 * ueV,
                        4.5 * ueV, 5.0 * ueV, 5.5 * ueV, 6.0 * ueV, 6.5 * ueV};

  m_RichMatNumStdEnBins = (int)m_RichMatStdEnVect.size(); // Standard table with 11 bins
                                                          // 14 in the old scheme.

  // m_RichMatStdCkvEnVect = {1.75 * ueV, 2.0 * ueV, 2.5 * ueV, 3.0 * ueV, 3.5 * ueV, 4.0 * ueV,
  //                         4.5 * ueV,  5.0 * ueV, 5.5 * ueV, 6.0 * ueV, 6.5 * ueV};

  m_RichMatStdCkvEnVect = {1.6 * ueV, 2.0 * ueV, 2.5 * ueV, 3.0 * ueV, 3.5 * ueV, 4.0 * ueV,
                           4.5 * ueV, 5.0 * ueV, 5.5 * ueV, 6.0 * ueV, 6.5 * ueV};

  m_RichMatNumStdCkvEnBins = (int)m_RichMatStdCkvEnVect.size();

  auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();

  aRichMatPropData->InitRichMatPropertyValues();
  aRichMatPropData->InitPmtQwPhRiVal();
  aRichMatPropData->CreateGasSellRefIndexValues();
  aRichMatPropData->InitRichCO2RiVal();
  aRichMatPropData->InitGasQWAbleVal();
  aRichMatPropData->InitPmtQEValues();
  aRichMatPropData->InitPmtHVValues();
  aRichMatPropData->InitCF4ScintValues();
  aRichMatPropData->InitGasQWARCoatReflValues();
}
//===========================================================================================//

int RichMatOPD::GetQENumBinsInGeneralTable( RichMatPropType aPropType, RichMatQE aQEType ) {
  auto aNumBin          = m_RichMatNumStdEnBins;
  auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();
  if ( aPropType == RichMatPropType::pmtQE ) {
    if ( aQEType == RichMatQE::qePmtNominal ) {
      aNumBin = aRichMatPropData->RichPmtNumNominalQEBins();
    } else if ( aQEType == RichMatQE::qePmtCBAUV ) {
      aNumBin = aRichMatPropData->RichPmtNumCBAUVQEBins();
    } else if ( aQEType == RichMatQE::qePmtMeanMeas ) {
      aNumBin = aRichMatPropData->RichPmtNumMeanMeasQEBins();
    } else if ( aQEType == RichMatQE::qePmtHamamatsuNominal ) {
      aNumBin = aRichMatPropData->RichPmtNumNominalHamamatsuQEBins();
    } else if ( aQEType == RichMatQE::qePmtPre2020 ) {
      aNumBin = aRichMatPropData->RichPmtNumPre2020QEBins();
    }
  }
  return aNumBin;
}
//===========================================================================================//

int RichMatOPD::GetHVNumBinsInGeneralTable( RichMatPropType aPropType, RichMatHV ahvType ) {
  int  aNumBin          = 1;
  auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();
  if ( aPropType == RichMatPropType::pmtHV ) {
    if ( ahvType == RichMatHV::hvPmtNominal ) {
      aNumBin = aRichMatPropData->RichPmtNumHVNominalBins();
    } else if ( ahvType == RichMatHV::hvPmtClassic ) {
      aNumBin = aRichMatPropData->RichPmtNumHVClassicBins();
    }
  }
  return aNumBin;
}
//=============================================================//

int RichMatOPD::GetQWARNumBinsInGeneralTable( RichMatPropType aPropType, RichSurfCoat aQWARType ) {
  int  aNumBin          = 1;
  auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();
  if ( aPropType == RichMatPropType::FresAR ) {
    if ( aQWARType == RichSurfCoat::arR1GasQW ) { aNumBin = aRichMatPropData->Rich1GasQWARReflNumBins(); }
  }
  return aNumBin;
}
//===========================================================================================//

int RichMatOPD::GetSCNumBinsInGeneralTable( RichMatPropType aPropType, RichMatScint aScintType ) {
  int  aNumBin          = 1;
  auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();
  if ( aPropType == RichMatPropType::scintGeneral ) {
    if ( aScintType == RichMatScint::scFast ) {
      aNumBin = aRichMatPropData->RichCF4NumSCFastBins();
    } else if ( aScintType == RichMatScint::scFastTime ) {
      aNumBin = aRichMatPropData->RichCF4NumScFaTimeBins();
    } else if ( aScintType == RichMatScint::scYield ) {
      aNumBin = aRichMatPropData->RichCF4NumScYieldBins();
    } else if ( aScintType == RichMatScint::scRes ) {
      aNumBin = aRichMatPropData->RichCF4NumScResBins();
    }
  }
  return aNumBin;
}

//===========================================================================================//

int RichMatOPD::GetNumBinsInTable( RichMatNameWOP aMatNameW, RichMatPropType aPropType ) {
  auto aNumBin = m_RichMatNumStdEnBins;
  if ( ( aMatNameW == RichMatNameWOP::Rhair ) || ( aMatNameW == RichMatNameWOP::PmtVac ) ||
       ( aMatNameW == RichMatNameWOP::R1M2Glass ) || ( aMatNameW == RichMatNameWOP::R2MGlass ) ) {
    aNumBin = m_RichMatNumStdEnBins;
  } else if ( ( aMatNameW == RichMatNameWOP::R1GasQw ) || ( aMatNameW == RichMatNameWOP::R2GasQw ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aNumBin = RichMatNumGasQWAbleBins();
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aNumBin = RichMatNumGasQWEnBins();
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aNumBin = RichMatNumGasQWCkvEnBins();
    }
  } else if ( aMatNameW == RichMatNameWOP::PmtQw ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aNumBin = m_RichMatNumStdEnBins;
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aNumBin = RichMatNumPmtQwRiEnBins();
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aNumBin = RichMatNumPmtQwCkvRiEnBins();
    }
  } else if ( aMatNameW == RichMatNameWOP::PmtPhc ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aNumBin = m_RichMatNumStdEnBins;
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aNumBin = RichMatNumPmtPhCathRiEnBins();
    }
  } else if ( ( aMatNameW == RichMatNameWOP::R1c4f10 ) || ( aMatNameW == RichMatNameWOP::R1RGas ) ||
              ( aMatNameW == RichMatNameWOP::R1Nitrogen ) || ( aMatNameW == RichMatNameWOP::R2Nitrogen ) ||
              ( aMatNameW == RichMatNameWOP::R2cf4 ) || ( aMatNameW == RichMatNameWOP::R2RGas ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aNumBin = m_RichMatNumStdEnBins;
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aNumBin = RichMatNumGasEnBins();
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aNumBin = RichMatNumGasCkvEnBins();
    }
  } else if ( aMatNameW == RichMatNameWOP::Rhco2 ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aNumBin = m_RichMatNumStdEnBins;
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aNumBin = RichMatNumCO2EnBins();
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aNumBin = RichMatNumCO2CkvEnBins();
    }
  }

  return aNumBin;
}

//===========================================================================================//
void RichMatOPD::FillGeneralTable( dd4hep::PropertyTable& aTable, RichMatNameWOP aMatNameW,
                                   RichMatPropType aPropType ) {

  auto aRichMatPropData = RichMatPropData::getRichMatPropDataInstance();

  if ( aMatNameW == RichMatNameWOP::Rhair ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alAir );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      FillGeneralRindexTable( aTable, RichMatRindex::riAir );
    }

  } else if ( ( aMatNameW == RichMatNameWOP::R1GasQw ) || ( aMatNameW == RichMatNameWOP::R2GasQw ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      aRichMatPropData->SelectAndFillAbleTable( aTable, RichMatAbLe::alGasQw );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aRichMatPropData->FillGasQwRiTable( aTable );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aRichMatPropData->FillGasQwCkvRiTable( aTable );
    }

  } else if ( aMatNameW == RichMatNameWOP::PmtVac ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alPmtVac );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      FillGeneralRindexTable( aTable, RichMatRindex::riPmtVac );
    }
  } else if ( ( aMatNameW == RichMatNameWOP::R1M2Glass ) || ( aMatNameW == RichMatNameWOP::R2MGlass ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alMirrQw );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      FillGeneralRindexTable( aTable, RichMatRindex::riMirrQw );
    }
  } else if ( aMatNameW == RichMatNameWOP::PmtQw ) {

    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alPmtQw );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aRichMatPropData->SelectAndFillRindexTables( aTable, RichMatRindex::riPmtQw );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aRichMatPropData->SelectAndFillCkvRndxTables( aTable, RichMatCkvRndx::ciPmtQw );
    }

  } else if ( aMatNameW == RichMatNameWOP::PmtPhc ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alPmtPhCath );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aRichMatPropData->SelectAndFillRindexTables( aTable, RichMatRindex::riPmtPhCath );
    }

  } else if ( ( aMatNameW == RichMatNameWOP::R1c4f10 ) || ( aMatNameW == RichMatNameWOP::R1RGas ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alC4F10 );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aRichMatPropData->SelectAndFillRindexTables( aTable, RichMatRindex::riC4F10 );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aRichMatPropData->SelectAndFillCkvRndxTables( aTable, RichMatCkvRndx::ciC4F10 );
    }
  } else if ( ( aMatNameW == RichMatNameWOP::R1Nitrogen ) || ( aMatNameW == RichMatNameWOP::R2Nitrogen ) ) {
    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alNitrogen );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aRichMatPropData->SelectAndFillRindexTables( aTable, RichMatRindex::riNitrogen );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aRichMatPropData->SelectAndFillCkvRndxTables( aTable, RichMatCkvRndx::ciNitrogen );
    }

  } else if ( ( aMatNameW == RichMatNameWOP::R2cf4 ) || ( aMatNameW == RichMatNameWOP::R2RGas ) ) {

    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alCF4 );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aRichMatPropData->SelectAndFillRindexTables( aTable, RichMatRindex::riCF4 );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aRichMatPropData->SelectAndFillCkvRndxTables( aTable, RichMatCkvRndx::ciCF4 );
    }

  } else if ( aMatNameW == RichMatNameWOP::Rhco2 ) {

    if ( aPropType == RichMatPropType::AbLe ) {
      FillGeneralAbLeTable( aTable, RichMatAbLe::alCO2 );
    } else if ( aPropType == RichMatPropType::Rindex ) {
      aRichMatPropData->SelectAndFillRindexTables( aTable, RichMatRindex::riCO2 );
    } else if ( aPropType == RichMatPropType::CkvRndx ) {
      aRichMatPropData->SelectAndFillCkvRndxTables( aTable, RichMatCkvRndx::ciCO2 );
    }
  }
}
//===========================================================================================//

void RichMatOPD::FillGeneralAbLeTable( dd4hep::PropertyTable& aTable, RichMatAbLe aAbLeType ) {
  if ( aAbLeType == RichMatAbLe::alAir ) {
    double aAbsLen = detail::dd4hep_param<double>( "RichAirAbsLengthValue" );
    FillStdSingleValueTable( aTable, aAbsLen );
  } else if ( aAbLeType == RichMatAbLe::alPmtVac ) {
    double aAbsLenPmtVac = detail::dd4hep_param<double>( "RichPmtVacuumAbsLengthValue" );
    FillStdSingleValueTable( aTable, aAbsLenPmtVac );
  } else if ( aAbLeType == RichMatAbLe::alMirrQw ) {
    double aAbsLenMirrQw = detail::dd4hep_param<double>( "RichMirrorQuartzAbsLengthValue" );
    FillStdSingleValueTable( aTable, aAbsLenMirrQw );
  } else if ( aAbLeType == RichMatAbLe::alPmtQw ) {
    double aAbsLenPmtQw = detail::dd4hep_param<double>( "RichPmtWindowQuartzAbsLengthValue" );
    FillStdSingleValueTable( aTable, aAbsLenPmtQw );
  } else if ( aAbLeType == RichMatAbLe::alPmtPhCath ) {
    double aAbsLenPmtPhC = detail::dd4hep_param<double>( "RichPmtWindowPhCathAbsLengthValue" );
    FillStdSingleValueTable( aTable, aAbsLenPmtPhC );
  } else if ( aAbLeType == RichMatAbLe::alC4F10 ) {
    double aAbsLenC4F10 = detail::dd4hep_param<double>( "Rich1RadiatorGasAbsLengthNominalValue" );
    FillStdSingleValueTable( aTable, aAbsLenC4F10 );
  } else if ( aAbLeType == RichMatAbLe::alNitrogen ) {
    double aAbsLenNitrogen = detail::dd4hep_param<double>( "RichNitrogenGasAbsLengthNominalValue" );
    FillStdSingleValueTable( aTable, aAbsLenNitrogen );
  } else if ( aAbLeType == RichMatAbLe::alCF4 ) {
    double aAbsLenCF4 = detail::dd4hep_param<double>( "Rich2RadiatorGasAbsLengthNominalValue" );
    FillStdSingleValueTable( aTable, aAbsLenCF4 );
  } else if ( aAbLeType == RichMatAbLe::alCO2 ) {
    double aAbsLenCO2 = detail::dd4hep_param<double>( "RichCO2GasAbsLengthNominalValue" );
    FillStdSingleValueTable( aTable, aAbsLenCO2 );
  }
}
//==============================================================================================//
void RichMatOPD::FillGeneralRindexTable( dd4hep::PropertyTable& aTable, RichMatRindex aRindexType ) {
  if ( aRindexType == RichMatRindex::riAir ) {
    double aRindex = detail::dd4hep_param<double>( "RichAirRindexValue" );
    FillStdSingleValueTable( aTable, aRindex );
  } else if ( aRindexType == RichMatRindex::riPmtVac ) {
    double aRindexPmtVac = detail::dd4hep_param<double>( "RichPmtVacuumRindexValue" );
    FillStdSingleValueTable( aTable, aRindexPmtVac );
  } else if ( aRindexType == RichMatRindex::riMirrQw ) {
    double aRindexMirrQw = detail::dd4hep_param<double>( "RichMirrorQuartzRindexValue" );
    FillStdSingleValueTable( aTable, aRindexMirrQw );
  }
}

//==============================================================================================//
void RichMatOPD::FillStdSingleValueTable( dd4hep::PropertyTable& aTable, double aValue ) {
  for ( int i = 0; i < m_RichMatNumStdEnBins; ++i ) {
    aTable->Set( i, 0, m_RichMatStdEnVect[i] );
    aTable->Set( i, 1, aValue );
  }
}
//==============================================================================================//
void RichMatOPD::PrintAGDMLMatrix( TGDMLMatrix* aTableG ) {
  const char* aTableTitleC = aTableG->GetName();
  int         aNumColC     = (int)( aTableG->GetCols() );
  int         aNumRowC     = (int)( aTableG->GetRows() );

  printout( dd4hep::ALWAYS, "RichMatOPD", "Current GDMLMatrix tableName  NumColC  NumRowC : %s %i %i", aTableTitleC,
            aNumColC, aNumRowC );

  for ( size_t row = 0; row < (size_t)aNumRowC; ++row ) {
    printf( "   " );
    for ( size_t col = 0; col < (size_t)aNumColC; ++col ) { printf( "%15.8g", aTableG->Get( row, col ) ); }
    printf( "\n" );
  }

  printout( dd4hep::ALWAYS, "RichMatOPD", "End of the Table PrintOut for %s", aTableTitleC );
}
//==============================================================================================//
RichMatOPD::TabData RichMatOPD::GetARichGeneralGDMLMatrix( TGDMLMatrix* aTableG ) {
  auto    aNumRowC = (int)( aTableG->GetRows() );
  TabData aGeneralTab;
  for ( size_t row = 0; row < (size_t)aNumRowC; ++row ) {
    const auto aEnValM = aTableG->Get( row, 0 );
    const auto aPropM  = aTableG->Get( row, 1 );
    aGeneralTab.emplace( aEnValM, aPropM );
  }
  return aGeneralTab;
}

//==============================================================================================//
void RichMatOPD::PrintRichMatProperty( const dd4hep::Material& aMat, RichMatPropType aTableType ) {

  std::string aMType        = RichMatPropTypeName( aTableType );
  std::string aMaterialName = aMat->GetName();

  dd4hep::Material::Property RC       = aMat.property( aMType.c_str() );
  int                        aNumColC = (int)( RC->GetCols() );
  int                        aNumRowC = (int)( RC->GetRows() );

  const char* aMatTitleC = RC->GetName();
  std::cout << "Current Material Name PropertyType NumColc NumRowC: " << aMaterialName << "  " << aMatTitleC << "  "
            << aMType << "    " << aNumColC << "   " << aNumRowC << std::endl;
  // RC->Print(); // This does not print with sufficent number of decimal places.
  //                 Hence the printing repeated below from TGDMLMatrix.cpp
  for ( size_t row = 0; row < (size_t)aNumRowC; ++row ) {
    printf( "   " );
    for ( size_t col = 0; col < (size_t)aNumColC; ++col ) { printf( "%15.8g", RC->Get( row, col ) ); }
    printf( "\n" );
  }
  std::cout << " End of the Table PrintOut for " << aMatTitleC << std::endl;
}
//==============================================================================================//
RichMatOPD::TabData RichMatOPD::GetRichMatProperty( const dd4hep::Material& aMat, RichMatPropType aTableType ) {

  auto aMType = RichMatPropTypeName( aTableType );
  // std::string aMaterialName = aMat->GetName();
  TabData aMatTab;

  auto RC = aMat.property( aMType.c_str() );
  // int                        aNumColC = (int)( RC->GetCols() );
  auto aNumRowC = (int)( RC->GetRows() );

  for ( size_t row = 0; row < (size_t)aNumRowC; ++row ) {
    const auto aEnValM = RC->Get( row, 0 );
    const auto aPropM  = RC->Get( row, 1 );
    aMatTab.emplace( aEnValM, aPropM );
  }
  return aMatTab;
}

//==============================================================================================//

void RichMatOPD::InitRichFillMatNames( const Rich::Detector::NamedVector<RichMatNameWOP>& aRichMatN ) {
  for ( const auto& A : aRichMatN ) {
    const auto& aIndex                  = A.second;
    const int   aIndexNum               = toInTy( aIndex );
    const auto& aLabel                  = A.first;
    m_RichMatNameWithOptProp[aIndexNum] = aLabel;
  }
}
//==============================================================================================//

void RichMatOPD::InitRichFillMatPropNames( const Rich::Detector::NamedVector<int>& aRichMatC, RichMatPropType aType ) {
  for ( const auto& M : aRichMatC ) {
    const auto& mIndexNum = M.second;
    const auto& mLabel    = M.first;
    if ( aType == ( RichMatPropType::TpName ) ) {
      m_RichMatPropTypeName[mIndexNum] = mLabel;
    } else if ( aType == ( RichMatPropType::AbLe ) ) {
      m_RichMatAbsLengthNames[mIndexNum] = mLabel;
    } else if ( aType == RichMatPropType::Rindex ) {
      m_RichMatRindexNames[mIndexNum] = mLabel;
    } else if ( aType == RichMatPropType::CkvRndx ) {
      m_RichMatCkvRndxNames[mIndexNum] = mLabel;
    } else if ( aType == RichMatPropType::scintGeneral ) {
      m_RichMatScintillationNames[mIndexNum] = mLabel;
    } else if ( aType == RichMatPropType::pmtQE ) {
      m_RichMatPmtQENames[mIndexNum] = mLabel;
    } else if ( aType == RichMatPropType::pmtHV ) {
      m_RichMatPmtHVNames[mIndexNum] = mLabel;
    } else if ( aType == RichMatPropType::FresAR ) {
      m_RichMatFresARNames[mIndexNum] = mLabel;
    }
  }
}
//==============================================================================================//
void RichMatOPD::InitRichMatOPD() {
  m_RichMatNameWithOptProp.clear();
  m_RichMatPropTypeName.clear();
  m_RichMatAbsLengthNames.clear();
  m_RichMatRindexNames.clear();
  m_RichMatCkvRndxNames.clear();
  m_RichMatScintillationNames.clear();
  m_RichMatPmtQENames.clear();
  m_RichMatPmtHVNames.clear();
  m_RichMatFresARNames.clear();

  const Rich::Detector::NamedVector<int> aRichMTCat{{"ABSLENGTH", toInTy( RichMatPropType::AbLe )},
                                                    {"RINDEX", toInTy( RichMatPropType::Rindex )},
                                                    {"CKVRNDX", toInTy( RichMatPropType::CkvRndx )},
                                                    {"FASTCOMPONENT", toInTy( RichMatPropType::scFast )},
                                                    {"FASTTIMECONSTANT", toInTy( RichMatPropType::scFastTime )},
                                                    {"SCINTILLATIONYIELD", toInTy( RichMatPropType::scYield )},
                                                    {"RESOLUTIONSCALE", toInTy( RichMatPropType::scRes )},
                                                    {"SCINTGENERAL", toInTy( RichMatPropType::scintGeneral )},
                                                    {"PMTQE", toInTy( RichMatPropType::pmtQE )},
                                                    {"PMTHV", toInTy( RichMatPropType::pmtHV )},
                                                    {"FRESAR", toInTy( RichMatPropType::FresAR )}};
  m_RichMatNumTypes = (int)aRichMTCat.size();
  m_RichMatPropTypeName.resize( m_RichMatNumTypes );
  InitRichFillMatPropNames( aRichMTCat, RichMatPropType::TpName );

  const Rich::Detector::NamedVector<int> aRichMTAble{{"RichAirAbsLengthPT", toInTy( RichMatAbLe::alAir )},
                                                     {"RichPmtVacuumAbsLengthPT", toInTy( RichMatAbLe::alPmtVac )},
                                                     {"MirrorQuartzAbsLengthPT", toInTy( RichMatAbLe::alMirrQw )},
                                                     {"GasWindowQuartzAbsLengthPT", toInTy( RichMatAbLe::alGasQw )},
                                                     {"PMTWindowQuartzAbsLengthPT", toInTy( RichMatAbLe::alPmtQw )},
                                                     {"Rich1NitrogenAbsLengthPT", toInTy( RichMatAbLe::alNitrogen )},
                                                     {"C4F10AbsLengthPT", toInTy( RichMatAbLe::alC4F10 )},
                                                     {"CF4AbsLengthPT", toInTy( RichMatAbLe::alCF4 )},
                                                     {"RichCO2AbsLengthPT", toInTy( RichMatAbLe::alCO2 )},
                                                     {"PmtPhCathodeAbsLengthPT", toInTy( RichMatAbLe::alPmtPhCath )}};
  m_RichMatNumAbsLenPropNames = (int)aRichMTAble.size();
  m_RichMatAbsLengthNames.resize( m_RichMatNumAbsLenPropNames );
  InitRichFillMatPropNames( aRichMTAble, RichMatPropType::AbLe );

  const Rich::Detector::NamedVector<int> aRichMTRi{{"RichAirRIndexPT", toInTy( RichMatRindex::riAir )},
                                                   {"RichPmtVacuumRIndexPT", toInTy( RichMatRindex::riPmtVac )},
                                                   {"MirrorQuartzRindexPT", toInTy( RichMatRindex::riMirrQw )},
                                                   {"GasWindowQuartzRIndexPT", toInTy( RichMatRindex::riGasQw )},
                                                   {"PmtWindowQuartzRIndexPT", toInTy( RichMatRindex::riPmtQw )},
                                                   {"PmtPhCathodeRIndexPT", toInTy( RichMatRindex::riPmtPhCath )},
                                                   {"Rich1NitrogenRIndexPT", toInTy( RichMatRindex::riNitrogen )},
                                                   {"C4F10SellParamRIndexPT", toInTy( RichMatRindex::riC4F10 )},
                                                   {"CF4RIndexPT", toInTy( RichMatRindex::riCF4 )},
                                                   {"RichCO2RIndexPT", toInTy( RichMatRindex::riCO2 )}};
  m_RichMatNumRindexPropNames = (int)aRichMTRi.size();
  m_RichMatRindexNames.resize( m_RichMatNumRindexPropNames );
  InitRichFillMatPropNames( aRichMTRi, RichMatPropType::Rindex );

  const Rich::Detector::NamedVector<int> aRichMTCkr{{"GasWindowQuartzCkvRIndexPT", toInTy( RichMatCkvRndx::ciGasQw )},
                                                    {"PmtWindowQuartzCkvRIndexPT", toInTy( RichMatCkvRndx::ciPmtQw )},
                                                    {"Rich1NitrogenCkvRIndexPT", toInTy( RichMatCkvRndx::ciNitrogen )},
                                                    {"C4F10SellParamCkvRIndexPT", toInTy( RichMatCkvRndx::ciC4F10 )},
                                                    {"CF4CkvRIndexPT", toInTy( RichMatCkvRndx::ciCF4 )},
                                                    {"RichCO2CkvRIndexPT", toInTy( RichMatCkvRndx::ciCO2 )}};
  m_RichMatNumCkvRndxPropNames = (int)aRichMTCkr.size();
  m_RichMatCkvRndxNames.resize( m_RichMatNumCkvRndxPropNames );
  InitRichFillMatPropNames( aRichMTCkr, RichMatPropType::CkvRndx );

  const Rich::Detector::NamedVector<int> aRichMTsc{
      {"CF4ScintilFastPT", toInTy( RichMatScint::scFast )},
      {"CF4ScintillationFastTimeConstPT", toInTy( RichMatScint::scFastTime )},
      {"CF4ScintillationYieldPT", toInTy( RichMatScint::scYield )},
      {"CF4ScintillationResolutionPT", toInTy( RichMatScint::scRes )}};
  m_RichMatNumScintPropNames = (int)aRichMTsc.size();
  m_RichMatScintillationNames.resize( m_RichMatNumScintPropNames );
  InitRichFillMatPropNames( aRichMTsc, RichMatPropType::scintGeneral );

  const Rich::Detector::NamedVector<int> aRichMTqe{
      {"NominalPmtQuantumEff", toInTy( RichMatQE::qePmtNominal )},
      {"NominalCBAUVGlassPmtQuantumEff", toInTy( RichMatQE::qePmtCBAUV )},
      {"NominalMeanMeasuredPmtQuantumEff", toInTy( RichMatQE::qePmtMeanMeas )},
      {"NominalHamaMatsuPmtQuantumEff", toInTy( RichMatQE::qePmtHamamatsuNominal )},
      {"NominalBefore2020PmtQuantumEff", toInTy( RichMatQE::qePmtPre2020 )}};
  m_RichMatNumQePropNames = (int)aRichMTqe.size();
  m_RichMatPmtQENames.resize( m_RichMatNumQePropNames );
  InitRichFillMatPropNames( aRichMTqe, RichMatPropType::pmtQE );

  const Rich::Detector::NamedVector<int> aRichMThv{{"PmtHighVoltageNominal", toInTy( RichMatHV::hvPmtNominal )},
                                                   {"PmtHighVoltageClassic", toInTy( RichMatHV::hvPmtClassic )}};
  m_RichMatNumHvNames = (int)aRichMThv.size();
  m_RichMatPmtHVNames.resize( m_RichMatNumHvNames );
  InitRichFillMatPropNames( aRichMThv, RichMatPropType::pmtHV );

  const Rich::Detector::NamedVector<int> aRichMTAR{{"R1GasQWARCoatRefl", toInTy( RichSurfCoat::arR1GasQW )}};
  m_RichMatNumFresARNames = (int)aRichMTAR.size();
  m_RichMatFresARNames.resize( m_RichMatNumFresARNames );
  InitRichFillMatPropNames( aRichMTAR, RichMatPropType::FresAR );

  const Rich::Detector::NamedVector<RichMatNameWOP> aRichMTN{{"Rich:Rich1GasWindowQuartz", RichMatNameWOP::R1GasQw},
                                                             {"Rich:Rich2GasWindowQuartz", RichMatNameWOP::R2GasQw},
                                                             {"Rich:RichPMTQuartzMaterial", RichMatNameWOP::PmtQw},
                                                             {"Rich:RichPMTVacuum", RichMatNameWOP::PmtVac},
                                                             {"Rich:RichPMTPhCathodeMaterial", RichMatNameWOP::PmtPhc},
                                                             {"Rich:Rich1Nitrogen", RichMatNameWOP::R1Nitrogen},
                                                             {"Rich:Rich2Nitrogen", RichMatNameWOP::R2Nitrogen},
                                                             {"Rich:R1RadiatorGas", RichMatNameWOP::R1RGas},
                                                             {"Rich:C4F10", RichMatNameWOP::R1c4f10},
                                                             {"Rich:R2RadiatorGas", RichMatNameWOP::R2RGas},
                                                             {"Rich:CF4", RichMatNameWOP::R2cf4},
                                                             {"Rich:CO2", RichMatNameWOP::Rhco2},
                                                             {"Rich:Air", RichMatNameWOP::Rhair},
                                                             {"Rich:Rich1MirrorGlassSimex", RichMatNameWOP::R1M2Glass},
                                                             {"Rich:Rich2MirrorGlass", RichMatNameWOP::R2MGlass}};
  m_RichNumMatWithOptProp = (int)aRichMTN.size();
  m_RichMatNameWithOptProp.resize( m_RichNumMatWithOptProp );
  InitRichFillMatNames( aRichMTN );
}

//==================================================================================//

RichMatOPD* RichMatOPD::getRichMatOPDInstance() {
  static std::once_flag              alloc_once;
  static std::unique_ptr<RichMatOPD> RichMatOPDInstance;
  std::call_once( alloc_once, []() { RichMatOPDInstance = std::make_unique<RichMatOPD>(); } );
  assert( RichMatOPDInstance.get() );
  return RichMatOPDInstance.get();
}

//==================================================================================//
