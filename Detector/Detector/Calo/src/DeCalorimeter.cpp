/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include <Core/GeometryTools.h>
#include <Core/Keys.h>
#include <Core/Units.h>
#include <Detector/Calo/CaloVector.h>
#include <Detector/Calo/CardParam.h>
#include <Detector/Calo/DeCalorimeter.h>
#include <nlohmann/json.hpp>
#include <string>

using namespace LHCb::Detector::Calo;
using LHCb::Detector::hash_key;
// using SubCalos = std::vector<dd4hep::DetElement>;
using CellParam           = LHCb::Detector::Calo::CellParam;
namespace CaloCellQuality = LHCb::Detector::Calo::CellQuality;

detail::DeCaloObject::DeCaloObject( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt )
    : DeIOVObject( de, ctxt ), index( CellCode::caloNum( de.name() ) ), caloName( std::string( de.name() ) + "Det" ) {
  // get the dimensions from the geometry
  dd4hep::Box box = de.solid();
  if ( !box ) throw std::runtime_error( std::string( de.name() ) + " shape is not a box" );
  xBound = box.x();
  yBound = box.y();
  xSize  = LHCb::Detector::detail::toLHCbLengthUnits( 2. * xBound );
  ySize  = LHCb::Detector::detail::toLHCbLengthUnits( 2. * yBound );

  // these were defined in the DDDB XML of the detectors
  bool isEcal = index == CellCode::Index::EcalCalo;
  if ( isEcal ) {
    zOffset = LHCb::Detector::detail::toLHCbLengthUnits(
        dd4hep::_toDouble( "-0.5 * EcalMidModLength + 0.5 * EcalStackLength + EcalMidFrontCoverLength" ) );
    zSize = LHCb::Detector::detail::toLHCbLengthUnits( dd4hep::_toDouble( "EcalStackLength" ) );
  } else if ( index == CellCode::Index::HcalCalo ) {
    zOffset = LHCb::Detector::detail::toLHCbLengthUnits(
        dd4hep::_toDouble( "-0.5 * HcalTotSpace + HcalFwFaceLength + 3 * HcalMsPlStepLength" ) );
    zSize = LHCb::Detector::detail::toLHCbLengthUnits( dd4hep::_toDouble( "6 * HcalMsPlStepLength" ) );
  } else {
    throw std::runtime_error( std::string( "DeCalorimeter does not support detector " ) + de.name() );
  }

  const auto& hardware = ctxt.condition( hash_key( de, "Hardware" ), true ).get<nlohmann::json>();
  adcMax               = hardware["AdcMax"].get<int>();
  auto coding          = hardware["CodingBit"].get<int>();
  maxRowCol            = ( 1 << coding ) - 1;                   // 63   (31   for HCAL)
  firstRowUp           = maxRowCol / 2 + 1;                     // 32   (16   for HCAL)
  centerRowCol         = .5 * static_cast<double>( maxRowCol ); // 31.5 (15.5 for HCAL)
  centralHoleX         = hardware["centralHoleX"].get<int>();
  centralHoleY         = hardware["centralHoleY"].get<int>();

  auto gain              = ctxt.condition( hash_key( de, "Gain" ), true ).get<nlohmann::json>();
  pedestalShift          = gain.contains( "PedShift" ) ? gain["PedShift"].get<double>() : 0;
  pinPedestalShift       = gain.contains( "PinPedShift" ) ? gain["PinPedShift"].get<double>() : 0;
  l0Et                   = gain.contains( "L0EtBin" ) ? gain["L0EtBin"].get<double>() : 0.;
  coherentNoise          = gain.contains( "CoherentNoise" ) ? gain["CoherentNoise"].get<double>() : -1;
  incoherentNoise        = gain.contains( "IncoherentNoise" ) ? gain["IncoherentNoise"].get<double>() : -1;
  gainError              = gain.contains( "GainError" ) ? gain["GainError"].get<double>() : -1;
  stochasticTerm         = gain.contains( "StochasticTerm" ) ? gain["StochasticTerm"].get<double>() : -1;
  zSupMethod             = gain.contains( "ZeroSupMethod" ) ? gain["ZeroSupMethod"].get<int>() : 1;
  zSupThreshold          = gain.contains( "ZeroSup" ) ? gain["ZeroSup"].get<double>() : 0.5;
  zSupNeighbourThreshold = gain.contains( "ZeroSupNeighbour" ) ? gain["ZeroSupNeighbour"].get<double>() : -5;
  activeToTotal          = gain.contains( "ActiveToTotal" ) ? gain["ActiveToTotal"].get<double>() : 1.;
  mip                    = gain.contains( "MipDeposit" ) ? gain["MipDeposit"].get<double>() : 0.;
  dyn                    = gain.contains( "DynamicsSaturation" ) ? gain["DynamicsSaturation"].get<double>() : 1.;
  prev                   = gain.contains( "PreviousFraction" ) ? gain["PreviousFraction"].get<double>() : 0.;
  l0Thresh               = gain.contains( "L0Threshold" ) ? gain["L0Threshold"].get<double>() : 0.5;
  if ( gain.contains( "phePerMip" ) ) phe = gain["phePerMip"].get<std::vector<double>>();

  auto reco  = ctxt.condition( hash_key( de, "Reco" ), true ).get<nlohmann::json>();
  zShowerMax = reco["ZShowerMax"].get<double>();

  geometry = de.nominal();
  detElem  = de;

  // for use of global calo alignment
  dd4hep::Alignment align = this->detectorAlignment();
  TGeoHMatrix       align_delta;
  align.delta().computeMatrix( align_delta );

  auto toGlobalMatrix = align_delta * de.nominal().worldTransformation();
  toGlobalTransform = LHCb::Detector::detail::toLHCbLengthUnits( lhcb::geometrytools::toTransform3D( toGlobalMatrix ) );
  toLocalTransform  = toGlobalTransform.Inverse();

  //----------------------------------------------------------------------------
  // ** Builds the cells from the geometry of the Detector Element
  //----------------------------------------------------------------------------
  for ( auto& i : de.children() ) {
    dd4hep::DetElement sub = i.second;

    auto& subSubCalos = i.second.children();
    nArea             = subSubCalos.size();

    auto& subcalo = subcalos[sub.id()];
    subcalo.subsubcalos.reserve( nArea );
    subcalo.side = ( sub.id() == 1 ) ? -1 : +1; // ID, Left:0 , Right: 1 ;

    for ( auto& j : subSubCalos ) {

      dd4hep::DetElement subSub = j.second;
      dd4hep::Volume     lv     = subSub.volume();

      // add global calo alignment delta to nominal calo subdetector local to world transformations
      auto subSub_toGlobalMatrix = align_delta * subSub.nominal().worldTransformation();
      auto subSub_toGlobalTransform =
          LHCb::Detector::detail::toLHCbLengthUnits( lhcb::geometrytools::toTransform3D( subSub_toGlobalMatrix ) );

      unsigned int Area =
          isEcal ? ( ( std::string( subSub.name() ).find( "Inn" ) != std::string::npos )
                         ? 2
                         : ( ( std::string( subSub.name() ).find( "Mid" ) != std::string::npos ) ? 1 : 0 ) )
                 : ( std::string( subSub.name() ).find( "Inn" ) != std::string::npos ) ? 1 : 0;

      double xOffset = -subcalo.side * lv.boundingBox().x();

      auto& subsubcalo            = subcalo.subsubcalos.emplace_back( detail::DeCaloObject::DeSubSubCaloObject() );
      subsubcalo.area             = Area;
      subsubcalo.xOffset          = xOffset;
      subsubcalo.xBound           = lv.boundingBox().x();
      subsubcalo.yBound           = lv.boundingBox().y();
      subsubcalo.toLocalTransform = subSub_toGlobalTransform.Inverse();

      if ( cellSizes.size() <= Area ) cellSizes.resize( Area + 1, 0 );

      if ( isEcal )
        cellSizes[Area] = ( Area == 2 ) ? ( dd4hep::_toDouble( "EcalModXSize" ) / 3 )
                                        : ( ( Area == 1 ) ? ( dd4hep::_toDouble( "EcalModXSize" ) / 2 )
                                                          : ( dd4hep::_toDouble( "EcalModXSize" ) ) );
      else
        cellSizes[Area] =
            ( Area == 0 ) ? dd4hep::_toDouble( "HcalOutCellXSize" ) : dd4hep::_toDouble( "HcalInnCellXSize" );

      dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter",
                        "SubSubCalo : Area %d | cellSize %f | local center X-offset: %f", Area, cellSizes[Area],
                        xOffset );

      ROOT::Math::XYZPoint pointLocal( 0, 0, 0 ), pointGlobal( 0, 0, 0 );
      for ( int Row = 0; maxRowCol >= Row; ++Row ) {
        pointLocal.SetY( cellSizes[Area] * ( Row - centerRowCol ) );
        for ( int Column = 0; maxRowCol >= Column; ++Column ) {
          pointLocal.SetX( cellSizes[Area] * ( Column - centerRowCol ) + xOffset );

          ROOT::Math::XYZPoint localPoint = LHCb::Detector::detail::toDD4hepUnits( pointLocal );
          Double_t             p[3]{localPoint.x(), localPoint.y(), localPoint.z()};
          if ( !lv->Contains( p ) ) continue;
          // Mask the non connected calorimeter cells.
          // Should be only for central part, but is OK also for middle and
          // outer as the hole is quite small...
          if ( ( centralHoleX > fabs( Column - centerRowCol ) ) && ( centralHoleY > fabs( Row - centerRowCol ) ) )
            continue;

          LHCb::Detector::Calo::CellID id( index, Area, Row, Column );
          if ( 0 > cells.index( id ) ) {
            cells.addEntry( CellParam( id ), id ); // store the new cell if it does not already exist
          }
          pointGlobal = subSub_toGlobalTransform( localPoint );

          cells[id].setCenterSize( pointGlobal, cellSizes[Area] );
          cells[id].setValid( true );
          cells[id].setZshower( zShowerMax );
        } // loop over columns
      }   // loop over rows

    } // loop over half-areas
  }   // loop over half-calos

  // ** Compute neighboring cells
  for ( auto& cell : cells ) {

    LHCb::Detector::Calo::CellID id     = cell.cellID();
    int                          Column = id.col();
    int                          Row    = id.row();
    unsigned int                 Area   = id.area();
    int                          nNeigh = 0;

    // ** Defines the standard 8 neighbouring cells:
    // ** +-1 X and Y in the same area
    // ** One may also take care (in the future) of
    // ** the central vertical separation
    // ** between the two calorimeter halves

    for ( int iColumn = Column - 1; iColumn <= Column + 1; ++iColumn ) {
      if ( iColumn >= 0 && iColumn <= maxRowCol ) { // inside calorimeter

        for ( int iRow = Row - 1; iRow <= Row + 1; ++iRow ) {
          if ( ( iRow >= 0 ) && ( iRow <= maxRowCol ) && // inside calorimeter
               ( iRow != Row || iColumn != Column ) ) {  // not itself...

            LHCb::Detector::Calo::CellID id2( index, Area, iRow, iColumn );
            if ( cells[id2].valid() ) {
              cell.addZsupNeighbor( id2 );
              cell.addNeighbor( id2 );
              nNeigh++;
            }
          }
        } // Loop on rows
      }
    } // Loop on columns

    if ( 8 != nNeigh ) {

      // ** Find neighbours from different areas,
      // ** if not 8 same-area neighbors.
      // ** For cells in different area to be neighbors,
      // ** the distance in X and Y of
      // ** the two cells center should be less than half
      // ** the sum of the cell sizes
      // ** To avoid rounding problems, one uses .55 instead of 1/2

      double x    = cells[id].x();
      double y    = cells[id].y();
      double size = cells[id].size();

      for ( auto pArea = cellSizes.begin(); cellSizes.end() != pArea; ++pArea ) {
        unsigned int iArea = pArea - cellSizes.begin();

        if ( ( iArea == Area ) || ( iArea == Area + 2 ) || ( iArea == Area - 2 ) ) continue; // Only in nearby area.

        double sizeArea = *pArea;
        double margin   = 0.55 * ( sizeArea + size );

        int cc = (int)( x / sizeArea + centerRowCol );
        int rc = (int)( y / sizeArea + centerRowCol );

        // ** To be fast, one checks only cells near
        // ** the expected position (cc and rc)
        // ** We limit to +-4, i.e. the size ratio should be less than 4

        for ( int iRow = rc - 4; iRow <= rc + 4; ++iRow ) {
          if ( ( 0 <= iRow ) && ( maxRowCol >= iRow ) ) {

            for ( int iColumn = cc - 4; iColumn <= cc + 4; ++iColumn ) {
              if ( ( 0 <= iColumn ) && ( maxRowCol >= iColumn ) ) {

                LHCb::Detector::Calo::CellID id2( index, iArea, iRow, iColumn );

                if ( cells[id2].valid() ) {
                  if ( ( fabs( cells[id2].x() - x ) <= margin ) && ( fabs( cells[id2].y() - y ) <= margin ) ) {
                    cell.addNeighbor( id2 );
                  }
                }
              }
            } // Loop on columns
          }
        } // Loop on Rows
      }   // end of loop over all areas
    }
  } // end of loop ovel all cells

  auto readout = ctxt.condition( hash_key( de, "Readout" ), true ).get<nlohmann::json>();

  //----------------------------------------------------------------------------
  // ** Construct the Front End card list for each source_id
  //----------------------------------------------------------------------------
  auto tell40links = readout["TELL40Link"].get<std::vector<std::vector<unsigned int>>>();

  source_ids.clear();
  for ( auto& tell40link : tell40links ) {
    int              feb1        = tell40link[0];
    int              feb2        = tell40link[1];
    int              feb3        = tell40link[2];
    int              feb4        = tell40link[3];
    int              feb5        = tell40link[4];
    int              feb6        = tell40link[5];
    int              source_0_id = tell40link[6];
    int              source_1_id = tell40link[7];
    std::vector<int> vecFEBs0    = {feb1, feb2, feb3};
    std::vector<int> vecFEBs1    = {feb4, feb5, feb6};
    source_ids.insert( std::make_pair( source_0_id, vecFEBs0 ) );
    source_ids.insert( std::make_pair( source_1_id, vecFEBs1 ) );
  }

  //----------------------------------------------------------------------------
  // ** Construct the Front End card information for each cell
  //----------------------------------------------------------------------------
  feCards.clear();
  pinArea = readout.contains( "pinArea" ) ? readout["pinArea"].get<int>() : -1;

  // decode cards array
  // ------------------
  auto aSize = readout["size"].get<int>();
  auto febs  = readout["FEB"].get<std::vector<std::vector<unsigned int>>>();
  // int firstCrate = febs[0][6];
  for ( auto& feb : febs ) {
    int cardNum = feb[0];
    int area    = feb[1];
    int fCol    = feb[2];
    int fRow    = feb[3];
    int lCol    = feb[4];
    int lRow    = feb[5];
    int crate   = feb[6];
    int slot    = feb[7];
    int mapType = 0;
    if ( aSize >= 9 ) mapType = feb[9];

    // build the FEcard
    CardParam myCard( area, fRow, fCol, lRow, lCol, cardNum, crate, slot );
    myCard.setMapping( (CardParam::Mapping)mapType );
    if ( pinArea == area ) myCard.setIsPin( true );

    std::vector<LHCb::Detector::Calo::CellID> cellids;
    // Update CellParam
    LHCb::Detector::Calo::CellID dummy{};
    for ( int col = fCol; lCol >= col; ++col ) {
      for ( int row = fRow; lRow >= row; ++row ) {
        LHCb::Detector::Calo::CellID id( index, area, row, col );
        if ( !cells[id].valid() ) {
          ( mapType == CardParam::None ) ? myCard.addID( dummy ) : cellids.push_back( dummy );
        } else {
          cells[id].setFeCard( cardNum, col - fCol, row - fRow );
          ( mapType == CardParam::None ) ? myCard.addID( id ) : cellids.push_back( id );
        }
      }
    }
    //
    feCards.push_back( myCard ); // add card
  }

  // 2nd loop on FE-Cards -
  //== Find the cards TO WHICH this card sends L0 data + Validation board
  for ( auto& card : feCards ) {

    int downCard     = -1;
    int leftCard     = -1;
    int cornerCard   = -1;
    int previousCard = -1;

    int area = card.area();
    int fRow = card.firstRow();
    int lRow = card.lastRow();
    int fCol = card.firstColumn();
    int lCol = card.lastColumn();

    for ( int row = fRow; lRow >= row; ++row ) {
      for ( int col = fCol; lCol >= col; ++col ) {
        LHCb::Detector::Calo::CellID id( index, area, row, col );
        if ( !cells[id].valid() ) continue;

        //== previous card in crate: only non zero row number
        if ( ( row == fRow ) && ( 0 < fRow ) ) {
          LHCb::Detector::Calo::CellID testID( index, area, row - 1, col );
          if ( cells[testID].cardNumber() >= 0 ) { downCard = cells[testID].cardNumber(); }
        }

        //== Left card:  As the two halves are independent, no 'Left' neighbors for
        //== the central column. Test on 'm_firstRowUp' which is also the first Column Left
        //== Find also the corner card.

        if ( ( col == fCol ) && ( 0 < col ) && ( firstRowUp != col ) ) {
          LHCb::Detector::Calo::CellID testID( index, area, row, col - 1 );
          if ( cells[testID].cardNumber() >= 0 ) { leftCard = cells[testID].cardNumber(); }
          if ( ( row == fRow ) && ( 0 < row ) ) {
            LHCb::Detector::Calo::CellID testID2( index, area, row - 1, col - 1 );
            if ( cells[testID2].cardNumber() >= 0 ) { cornerCard = cells[testID2].cardNumber(); }
          }
        }
      }
    }
    if ( leftCard == -1 ) cornerCard = -1; // WARNING : corner is transmitted via left !!

    if ( area != pinArea && !card.isParasitic() ) {
      card.setNeighboringCards( downCard, leftCard, cornerCard, previousCard );
    }
  }

  //--------------------------------
  // ** Construct the Tell1 Readout
  //--------------------------------
  auto tell40s = readout["Tell40"].get<std::vector<std::vector<unsigned int>>>();
  tell1Boards.clear();

  for ( auto& tell40 : tell40s ) {
    int        num = tell40[0];
    Tell1Param tell1( num );

    for ( unsigned int k = 2; k < 8; k++ ) {
      unsigned int feCard = tell40[k];
      if ( feCard == 0 ) continue; // skip links with 0 as FEB number (used to locate the board with source_id)
      auto i = std::find_if( feCards.begin(), feCards.end(),
                             [&]( const CardParam& c ) { return c.number() == static_cast<int>( feCard ); } );
      feCard = i != feCards.end() ? i - feCards.begin() : -1;

      feCards[feCard].setTell1( num );
      tell1.addFeCard( feCard );
      if ( feCards[feCard].isPinCard() ) tell1.setReadPin( true );
    }
    if ( tell1.readPin() ) pinTell1s.push_back( num );
    tell1Boards.push_back( tell1 );
  }

  auto monitoring = ctxt.condition( hash_key( de, "Monitoring" ), true ).get<nlohmann::json>();

  //----------------------------------------------
  // ** Construct the monitoring readout (LED/PIN)
  //----------------------------------------------
  auto pinsVec = monitoring["PIN"].get<std::vector<std::vector<unsigned int>>>();

  pins.clear();
  leds.clear();
  for ( auto& pinVec : pinsVec ) {
    int box      = pinVec[0];
    int pinIndex = pinVec[1];
    int side     = pinVec[2]; // 0 = Left ; 1 = Right
    int crate    = pinVec[3];
    int slot     = pinVec[4];
    int channel  = pinVec[5]; // == readout channel numbering ( WARNING != ADC numbering)
    int cLed     = pinVec[6];
    int rLed     = pinVec[7];
    int nReg     = pinVec[8];
    // floating size  sub-array
    int area  = pinVec[9];
    int fCol  = pinVec[10];
    int fRow  = pinVec[11];
    int lCol  = pinVec[12];
    int lRow  = pinVec[13];
    int area2 = -1;
    int fCol2 = -1;
    int fRow2 = -1;
    int lCol2 = -1;
    int lRow2 = -1;
    if ( 2 == nReg ) {
      area2 = pinVec[14];
      fCol2 = pinVec[15];
      fRow2 = pinVec[16];
      lCol2 = pinVec[17];
      lRow2 = pinVec[18];
    }
    // Define a CaloCellId for PIN-diode :
    // area = m_pinArea,
    // relative PinRow/PinCol derives from channel according to standard FEB usage
    // absolute PinRow/PinCol according to Card condDB

    // Relative position within card (8x4)
    int pinRow = (int)channel / nColCaloCard;
    int pinCol = channel - pinRow * nColCaloCard;
    // Absolute Row & Col wrt to CardParam

    auto indCard = std::find_if( feCards.begin(), feCards.end(),
                                 [&]( const CardParam& c ) { return c.crate() == crate && c.slot() == slot; } );
    int  iCard   = indCard != feCards.end() ? indCard - feCards.begin() : -1;
    if ( 0 > iCard )
      dd4hep::printout( dd4hep::ERROR, "DeCalorimeter", "No FE-Card defined in crate/slot %d/%d", crate, slot );

    LHCb::Detector::Calo::CellID pinId( index, pinArea, feCards[iCard].firstRow() + pinRow,
                                        feCards[iCard].firstColumn() + pinCol );

    Pin pin;
    if ( 0 <= pins.index( pinId ) ) {
      // the caloPin already exist
      pin = pins[pinId];
      if ( box != pin.box() || pinIndex != pin.index() || side != pin.side() )
        dd4hep::printout( dd4hep::ERROR, "DeCalorimeter",
                          "PIN-diode with same ID but different location already exists" );
      dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Extended definition for an existing  PIN-diode is found" );
    } else {
      // create a new one and store it
      pin = Pin( pinId );
      pin.setPinLocation( box, pinIndex, side );
      pins.addEntry( pin, pinId );
      // built a new VALID 'virtual' cell associated with the PIN if it does not exists !!
      if ( 0 > cells.index( pinId ) ) cells.addEntry( CellParam( pinId ), pinId );
      cells[pinId].addPin( pinId );
      cells[pinId].setValid( true );
      cells[pinId].setFeCard( feCards[iCard].number(), pinCol, pinRow );
      // update FE-Card
      auto& ids = const_cast<std::vector<LHCb::Detector::Calo::CellID>&>( feCards[iCard].ids() );
      ids[pinCol * nRowCaloCard + pinRow] = pinId;
    }
    // update CaloRegion
    pins[pinId].addCaloRegion( area, fCol, fRow, lCol, lRow );
    if ( area2 >= 0 ) pins[pinId].addCaloRegion( area2, fCol2, fRow2, lCol2, lRow2 );
    // Hcal Leds can be  distributed on 2 areas

    int nleds = pins[pinId].leds().size();

    // Built the Led system
    int rSize = ( lRow - fRow + 1 ) / rLed;
    int cSize = ( lCol - fCol + 1 ) / cLed;
    for ( int ir = 1; ir != rLed + 1; ir++ ) {
      for ( int ic = 1; ic != cLed + 1; ic++ ) {

        // Local numbering (ic,ir) -> ic + (ir-1)*cLed
        // +---+---+
        // | 3 | 4 |
        // +-------+
        // | 1 | 2 |
        // +-------+

        // LED numbering (jc,jr) -> index - as in Irina document
        //   C-side     A-side
        // +---+---+  +---+---+
        // | 2 | 1 |  | 1 | 2 |
        // +-------+  +-------+
        // | 4 | 3 |  | 3 | 4 |
        // +-------+  +-------+

        int jr = rLed + 1 - ir;
        int jc = ic;
        if ( 0 == side ) jc = cLed + 1 - ic;
        int ledIndex = jc + ( jr - 1 ) * cLed + nleds; // increment !!

        int fColLed = fCol + ( ic - 1 ) * cSize;
        int lColLed = fColLed + cSize - 1;
        int fRowLed = fRow + ( ir - 1 ) * rSize;
        int lRowLed = fRowLed + rSize - 1;

        // Build the LED
        unsigned int ledNum = leds.size();
        Led          led( ledNum );
        led.setIndex( ledIndex );
        led.addCaloRegion( area, fColLed, fRowLed, lColLed, lRowLed );
        led.setPin( pinId );
        pins[pinId].addLed( ledNum );  // update CaloPin
        cells[pinId].addLed( ledNum ); // add the Led to the virtual pin cell
        // Hcal Leds can be distributed on 2 areas
        if ( 0 <= area2 ) {
          if ( 1 != rLed * cLed )
            dd4hep::printout( dd4hep::ERROR, "DeCalorimeter",
                              "Don't know how to distributed Leds array on several Calo area" );
          led.addCaloRegion( area2, fCol2, fRow2, lCol2, lRow2 );
        }
        leds.push_back( led ); // store Led
      }
    }

    // Link to cell (if not done yet)
    for ( int ledNum : pins[pinId].leds() ) {
      Led& led = leds[ledNum];
      dd4hep::printout( dd4hep::VERBOSE, "DeCalorimeter", "LED %d", ledNum );

      for ( unsigned int ireg = 0; ireg < led.areas().size(); ++ireg ) {
        int ar = led.areas()[ireg];
        int fr = led.firstRows()[ireg];
        int lr = led.lastRows()[ireg];
        int fc = led.firstColumns()[ireg];
        int lc = led.lastColumns()[ireg];
        for ( int row = fr; lr >= row; ++row ) {
          for ( int col = fc; lc >= col; ++col ) {
            LHCb::Detector::Calo::CellID id( index, ar, row, col );
            dd4hep::printout( dd4hep::VERBOSE, "DeCalorimeter", "Connected cells to LED/Pin %s valid ? %d",
                              id.asString().c_str(), cells[id].valid() );
            if ( cells[id].valid() ) {
              cells[id].addPin( pinId );
              cells[id].addLed( ledNum );
              leds[ledNum].addCell( id ); // update led
              pins[pinId].addCell( id );  // update pin
            }
          }
        }
      }
    }
  }

  // check all cells (including virtual) are connected to a Led
  for ( auto& elem : cells ) {
    dd4hep::printout( dd4hep::VERBOSE, "DeCalorimeter", "Cell %s <- Led %d -> Pin %d", elem.cellID().asString().c_str(),
                      elem.leds().size(), elem.pins().size() );
    if ( elem.leds().empty() )
      dd4hep::printout( dd4hep::ERROR, "DeCalorimeter", "Cell %s is not connect to the monitoring system.",
                        elem.cellID().asString().c_str() );
  }
  dd4hep::printout( dd4hep::VERBOSE, "DeCalorimeter", "Initialized, %d monitoring LEDs", leds.size() );
  dd4hep::printout( dd4hep::VERBOSE, "DeCalorimeter", "Initialized, %d PIN-diodes.", pins.size() );

  auto quality = ctxt.condition( hash_key( de, "Quality" ), true ).get<nlohmann::json>();

  //----------------------------------------------
  // ** Quality and current LED signal for relative calibration
  //----------------------------------------------
  auto cellsQuality = quality["data"].get<std::vector<std::vector<double>>>();
  auto sizeQual     = quality["size"].get<int>();

  int countQual = 0;
  int bad       = 0;
  int badLED    = 0;
  int masked    = 0;
  for ( auto& cellQuality : cellsQuality ) {
    double                       cell       = cellQuality[0];
    double                       qFlag      = cellQuality[1];
    double                       ledData    = ( sizeQual > 2 ) ? cellQuality[2] : 0.;
    double                       ledDataRMS = ( sizeQual > 3 ) ? cellQuality[3] : 0.;
    double                       ledMoni    = ( sizeQual > 4 ) ? cellQuality[4] : 1.;
    double                       ledMoniRMS = ( sizeQual > 5 ) ? cellQuality[5] : 0.;
    LHCb::Detector::Calo::CellID id         = LHCb::Detector::Calo::CellID( (int)cell );
    id.setCalo( index );
    // get cell
    if ( cells[id].valid() ) {
      cells[id].addQualityFlag( (int)qFlag );
      cells[id].setLedData( ledData, ledDataRMS );
      cells[id].setLedMoni( ledMoni, ledMoniRMS );
      dd4hep::printout( dd4hep::VERBOSE, "DeCalorimeter", "Added quality for channel %s : quality = %f",
                        id.asString().c_str(), qFlag );
      dd4hep::printout( dd4hep::VERBOSE, "DeCalorimeter", "    current <PMT> +- RMS from LED signal : %f +- %f",
                        ledData, ledDataRMS );
      dd4hep::printout( dd4hep::VERBOSE, "DeCalorimeter", "    current <PMT/PIN> +- RMS  from LED signal : %f +- %f",
                        ledMoni, ledMoniRMS );

      if ( CaloCellQuality::OK != (int)qFlag ) {
        dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Quality %s : %s", std::to_string( id.index() ).c_str(),
                          cells[id].cellStatus().c_str() );
        int  iFlag = (int)qFlag;
        bool bLed  = false;
        bool bRO   = false;
        bool bMask = false;
        int  d     = 1;
        while ( iFlag > 0 ) {
          if ( ( iFlag & 0x1 ) == 1 ) {
            if ( CaloCellQuality::qName( d ).find( "LED" ) != std::string::npos )
              bLed = true;
            else if ( (CaloCellQuality::Flag)iFlag == CaloCellQuality::OfflineMask )
              bMask = true;
            else
              bRO = true;
          }
          iFlag /= 2;
          d += 1;
        }
        if ( bLed ) badLED++;
        if ( bRO ) bad++;
        if ( bMask ) masked++;
      }
      countQual++;
    } else {
      dd4hep::printout( dd4hep::WARNING, "DeCalorimeter", "Trying to add quality on non-valid channel : %s",
                        id.asString().c_str() );
    }
  }
  dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Quality constant added for %d channel(s)", countQual );
  if ( bad > 0 ) dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Found %d problematic readout channel(s)", bad );
  if ( masked > 0 )
    dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Found %d channel(s) to be masked offline", masked );
  if ( badLED > 0 )
    dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Found %d channel(s) with 'LED monitoring problem'", badLED );

  //----------------------------------------------
  // ** pileUp subtraction
  //----------------------------------------------
  if ( isEcal ) {

    auto pileup = ctxt.condition( hash_key( de, "PileUpOffset" ), true ).get<nlohmann::json>();

    //----------------------------------------------
    // ** Quality and current LED signal for relative calibration
    //----------------------------------------------
    auto cellsPileUp = pileup["data"].get<std::vector<std::vector<double>>>();
    puMeth           = pileup["Method"].get<int>();
    puBin            = pileup["Bin"].get<int>();
    puMin            = pileup["Min"].get<int>();

    dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Pileup offset substraction : method = %d ; Min = %d ; Bin = %d",
                      puMeth, puMin, puBin );

    auto sizePU  = pileup["size"].get<int>();
    int  countPU = 0;

    for ( auto& cellPileUp : cellsPileUp ) {

      double cell  = cellPileUp[0];
      double offs  = cellPileUp[1];
      double eoffs = ( sizePU > 2 ) ? cellPileUp[2] : 0;

      LHCb::Detector::Calo::CellID id = LHCb::Detector::Calo::CellID( (int)cell );
      id.setCalo( index );
      if ( cells[id].valid() ) {
        cells[id].setPileUpOffset( offs, eoffs );
        dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter",
                          "Added pileup offset for channel %s : <offset> = %f ( RMS = %f )", id.asString().c_str(),
                          offs, eoffs );
      } else {
        dd4hep::printout( dd4hep::WARNING, "DeCalorimeter", "Trying to add pileup offset on non-valid channel : %s",
                          id.asString().c_str() );
      }
      countPU++;
    }
    dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Pileup offset added for %d channel(s)", countPU );
  }

  bool nominal   = false;
  int  countGain = 0;
  // Nominal Gain parameters
  auto cellsGain = gain["data"].get<std::vector<std::vector<double>>>();
  if ( cellsGain.size() > 0 ) {
    nominal = true;
    for ( auto& cellGain : cellsGain ) {
      double                       cell    = cellGain[0];
      double                       gainVal = cellGain[1];
      LHCb::Detector::Calo::CellID id      = LHCb::Detector::Calo::CellID( (int)cell );
      id.setCalo( index );
      if ( cells[id].valid() ) {
        cells[id].setNominalGain( gainVal );
        countGain++;
      }
    }
  }
  // Theoretical gain parameters
  auto etSlope    = gain["EtSlope"].get<std::vector<double>>();
  auto etInCenter = gain["EtInCenter"].get<std::vector<double>>();
  maxEtInCenter   = etInCenter;
  maxEtSlope      = etSlope;
  if ( ( maxEtInCenter.size() != nArea && maxEtInCenter.size() != 1 ) ||
       ( maxEtSlope.size() != nArea && maxEtSlope.size() != 1 ) ) {
    dd4hep::printout( dd4hep::ERROR, "DeCalorimeter",
                      "Found %d gain parameters, there should be %d parameters (or a single one for all regions)",
                      maxEtInCenter.size(), nArea );
  }
  // ** update gain/channel with theoretical value (if needed)
  if ( !nominal ) {
    dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter",
                      "Apply theoretical nominal gain as EtInCenter+sin(Theta)*EtSlope" );
    countGain = 0;
    for ( auto& cell : cells ) {
      LHCb::Detector::Calo::CellID id = cell.cellID();
      if ( id.isPin() ) continue;
      unsigned int Area    = id.area();
      double       maxEt   = ( Area < maxEtInCenter.size() ) ? maxEtInCenter[Area] : maxEtInCenter[0];
      double       maxEtSl = ( Area < maxEtSlope.size() ) ? maxEtSlope[Area] : maxEtSlope[0];

      double gainVal = ( maxEt / cell.sine() ) + maxEtSl;
      gainVal        = gainVal / (double)adcMax;
      cell.setNominalGain( gainVal );
      ++countGain;
    }
  }
  std::string nom = ( nominal ) ? "Nominal" : "Theoretical";

  dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "%s gain constant added for %d / %d channel(s)", nom.c_str(),
                    countGain, cells.size() - pins.size() );
  if ( countGain != (int)( cells.size() - pins.size() ) )
    dd4hep::printout( dd4hep::ERROR, "DeCalorimeter", "%s gains are missing for %d channels!", nom.c_str(),
                      (int)( cells.size() - pins.size() ) - countGain );

  // Calibration constants
  auto calibration = ctxt.condition( hash_key( de, "Calibration" ), true ).get<nlohmann::json>();
  auto cellsCalib  = calibration["data"].get<std::vector<std::vector<double>>>();
  if ( cellsCalib.size() > 0 ) {
    for ( auto& cellCalib : cellsCalib ) {
      double                       cell = cellCalib[0];
      double                       coef = cellCalib[1];
      LHCb::Detector::Calo::CellID id( (int)cell );
      id.setCalo( index );
      // get cell
      if ( cells[id].valid() ) {
        cells[id].setCalibration( coef );
        dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Added calibration coefficient for channel %s : %f",
                          id.asString().c_str(), coef );
      }
    }
  } else {
    dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter", "Calibration coefficients are not set" );
  }
}

CellID DeCalorimeter::cellIDfromVolumeID( const dd4hep::DDSegmentation::VolumeID& volumeID ) const {
  // decode the volume ID
  const unsigned int detectorMask = ( ( ( (unsigned int)1 ) << 8 ) - 1 ) << 0;
  const unsigned int detectorID   = ( volumeID & detectorMask );
  if ( detectorID == 60 ) {
    if ( this->index() != CellCode::Index::EcalCalo ) {
      dd4hep::printout( dd4hep::ERROR, "DeCalorimeter",
                        "The volumeID says this DeCalorimeter object is a Ecal, while its 'index' is %d.\nThere's "
                        "probabaly an error in the decoding of the volumeID\n",
                        this->index() );
    }
    return cellIDfromVolumeID_ECAL( volumeID );
  } else if ( detectorID == 70 ) {
    if ( this->index() != CellCode::Index::HcalCalo ) {
      dd4hep::printout( dd4hep::ERROR, "DeCalorimeter",
                        "The volumeID says this DeCalorimeter object is a Hcal, while its 'index' is %d.\nThere's "
                        "probabaly an error in the decoding of the volumeID\n",
                        this->index() );
    }
    return cellIDfromVolumeID_HCAL( volumeID );
  } else {
    dd4hep::printout( dd4hep::ERROR, "DeCalorimeter",
                      "The volumeID says this DeCalorimeter object is a neither and Ecal or a Hcal.\nThere's "
                      "probabaly an error in the decoding of the volumeID\n" );
    return CellID();
  }
}

CellID DeCalorimeter::cellIDfromVolumeID_ECAL( const dd4hep::DDSegmentation::VolumeID& volumeID ) const {
  const unsigned int sideMask   = ( ( ( (unsigned int)1 ) << 1 ) - 1 ) << 8;
  const unsigned int regionMask = ( ( ( (unsigned int)1 ) << 2 ) - 1 ) << 9;
  const unsigned int blockMask  = ( ( ( (unsigned int)1 ) << 6 ) - 1 ) << 11;
  const unsigned int moduleMask = ( ( ( (unsigned int)1 ) << 5 ) - 1 ) << 17;
  const unsigned int cellMask   = ( ( ( (unsigned int)1 ) << 4 ) - 1 ) << 22;
  const unsigned int sideID     = ( volumeID & sideMask ) >> 8;
  const unsigned int regionID   = ( volumeID & regionMask ) >> 9;
  const unsigned int blockID    = ( volumeID & blockMask ) >> 11;
  const unsigned int moduleID   = ( volumeID & moduleMask ) >> 17;
  const unsigned int cellID     = ( volumeID & cellMask ) >> 22;

  unsigned int region = regionID;
  unsigned int row    = 0;
  unsigned int col    = 0;
  if ( region == 2 ) {
    unsigned int row_inModule = cellID % 3;
    unsigned int col_inModule = cellID / 3;
    unsigned int row_inBlock  = moduleID % 2;
    unsigned int col_inBlock  = moduleID / 2;
    unsigned int row_inRegion = blockID;
    unsigned int col_offset   = ( sideID ? 8 : 32 );
    unsigned int row_offset   = 14;
    row                       = row_inModule + 3 * row_inBlock + 6 * row_inRegion + row_offset;
    col                       = col_inModule + 3 * col_inBlock + col_offset;
    if ( col > 23 && col < 40 && row > 25 && row < 38 )
      return LHCb::Detector::Calo::CellID(); // mask the not instrumented central cells
  } else if ( region == 1 ) {
    unsigned int row_inModule = cellID % 2;
    unsigned int col_inModule = cellID / 2;
    unsigned int row_inBlock  = moduleID % 2;
    unsigned int col_inBlock  = moduleID / 2;
    unsigned int row_inRegion = blockID % 10;
    unsigned int col_inRegion = blockID / 10;
    unsigned int col_offset   = ( sideID ? 0 : 32 );
    unsigned int row_offset   = 12;
    row                       = row_inModule + 2 * row_inBlock + 4 * row_inRegion + row_offset;
    col                       = col_inModule + 2 * col_inBlock + 16 * col_inRegion + col_offset;
  } else if ( region == 0 ) {
    unsigned int row_inBlock  = moduleID % 2;
    unsigned int col_inBlock  = moduleID / 2;
    unsigned int row_inRegion = blockID % 26;
    unsigned int col_inRegion = blockID / 26;
    unsigned int col_offset   = ( sideID ? 0 : 32 );
    unsigned int row_offset   = 6;
    row                       = row_inBlock + 2 * row_inRegion + row_offset;
    col                       = col_inBlock + 16 * col_inRegion + col_offset;
  }
  return CellID( this->index(), region, row, col );
}

CellID DeCalorimeter::cellIDfromVolumeID_HCAL( const dd4hep::DDSegmentation::VolumeID& volumeID ) const {
  // decode the volumeID
  const unsigned int sideMask      = ( ( ( (unsigned int)1 ) << 1 ) - 1 ) << 8;
  const unsigned int regionMask    = ( ( ( (unsigned int)1 ) << 1 ) - 1 ) << 9;
  const unsigned int moduleMask    = ( ( ( (unsigned int)1 ) << 7 ) - 1 ) << 10;
  const unsigned int submoduleMask = ( ( ( (unsigned int)1 ) << 4 ) - 1 ) << 17;
  const unsigned int cellMask      = ( ( ( (unsigned int)1 ) << 3 ) - 1 ) << 21;
  const unsigned int sideID        = ( volumeID & sideMask ) >> 8;
  const unsigned int regionID      = ( volumeID & regionMask ) >> 9;
  const unsigned int moduleID      = ( volumeID & moduleMask ) >> 10;
  const unsigned int submoduleID   = ( volumeID & submoduleMask ) >> 17;
  const unsigned int cellID        = ( volumeID & cellMask ) >> 21;

  unsigned int region = regionID;
  unsigned int row    = 0;
  unsigned int col    = 0;
  if ( region == 1 ) {
    unsigned int row_inSubmodule = ( sideID ? 1 - cellID % 2 : cellID % 2 );
    unsigned int col_inSubmodule = ( sideID ? 1 - cellID / 2 : cellID / 2 );
    unsigned int col_inModule    = ( sideID ? 7 - submoduleID : submoduleID );
    unsigned int row_inRegion    = moduleID;
    unsigned int col_offset      = ( sideID ? 0 : 16 );
    unsigned int row_offset      = 2;
    row                          = row_inSubmodule + 2 * row_inRegion + row_offset;
    col                          = col_inSubmodule + 2 * col_inModule + col_offset;
    // if ( col > 13 && col < 18 && row > 13 && row < 18 ) return CellID(); //<--unnecessary since no HCAL cells is
    // present in this region
  } else if ( region == 0 ) {
    unsigned int col_inModule = submoduleID;
    unsigned int row_inRegion = moduleID % 26;
    unsigned int col_inRegion = moduleID / 26;
    unsigned int col_offset   = ( sideID ? 0 : 16 );
    unsigned int row_offset   = 3;
    row                       = row_inRegion + row_offset;
    col                       = col_inModule + 8 * col_inRegion + col_offset;
  }
  return CellID( this->index(), region, row, col );
}
