/*****************************************************************************\
* (c) Copyright 2000-2021 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "DD4hep/Printout.h"
#include <Core/DeIOV.h>
#include <Detector/Calo/CaloCardParams.h>
#include <Detector/Calo/CaloCellID.h>
#include <Detector/Calo/CaloLed.h>
#include <Detector/Calo/CaloPin.h>
#include <Detector/Calo/CaloVector.h>
#include <Detector/Calo/CardParam.h>
#include <Detector/Calo/CellParam.h>
#include <Detector/Calo/Tell1Param.h>
#include <vector>

namespace CaloCellQuality = LHCb::Detector::Calo::CellQuality;

namespace LHCb::Detector::Calo {

  /// definition of calorimeter planes
  enum Plane { Front = 0, Middle, ShowerMax, Back };

  inline std::string toString( Plane e ) {
    switch ( e ) {
    case Plane::Front:
      return "Front";
    case Plane::ShowerMax:
      return "ShowerMax";
    case Plane::Middle:
      return "Middle";
    case Plane::Back:
      return "Back";
    default:
      throw "Not a correct plane in Calo";
    }
  }

  inline std::ostream& toStream( Plane e, std::ostream& os ) { return os << std::quoted( toString( e ), '\'' ); }

  inline std::ostream& operator<<( std::ostream& s, Plane e ) { return toStream( e, s ); }

  namespace detail {

    struct DeCaloObject : LHCb::Detector::detail::DeIOVObject {
      DeCaloObject( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {
        // no children
      }

      CellCode::Index index{LHCb::Detector::Calo::CellCode::Index::Undefined};
      std::string     caloName{"UNDEFINED"};

      struct DeSubSubCaloObject {
        unsigned int            area{0u};
        double                  xBound{0.};
        double                  yBound{0.};
        double                  xOffset{0.};
        ROOT::Math::Transform3D toLocalTransform;
      };

      struct DeSubCaloObject {
        int                             side{0};
        std::vector<DeSubSubCaloObject> subsubcalos;
      };

      std::array<DeSubCaloObject, 2> subcalos;

      unsigned int nArea{0};

      double xSize{0};
      double ySize{0};
      double zSize{0};
      double zOffset{0};

      // for internal checks, in DD4HEP units
      double xBound{0};
      double yBound{0};

      // Hardware parameters (from conditions)
      int    adcMax{-1};
      int    centralHoleX{0};
      int    centralHoleY{0};
      int    maxRowCol{-1};
      int    firstRowUp{-1};
      double centerRowCol{0};

      // Gain parameters (from conditions)
      std::vector<double> maxEtInCenter;
      std::vector<double> maxEtSlope;
      double              pedestalShift{0};
      double              pinPedestalShift{0};
      double              l0Et{0};
      double              coherentNoise{0};
      double              incoherentNoise{0};
      double              stochasticTerm{0};
      double              gainError{0};
      int                 zSupMethod{-1};
      double              zSupThreshold{0};
      double              zSupNeighbourThreshold{0};
      double              activeToTotal{0};
      double              mip{0};
      double              dyn{0};
      double              prev{0};
      double              l0Thresh{0};
      std::vector<double> phe;

      // Reconstruction parameters (from conditions)
      double zShowerMax{0};

      // readout
      int pinArea{-1};

      // Collections
      std::vector<LHCb::Detector::Calo::CellID> empty;
      CaloVector<CellParam>                     cells;
      std::vector<double>                       cellSizes;
      std::vector<CardParam>                    feCards;
      std::map<int, std::vector<int>>           source_ids;
      std::vector<Tell1Param>                   tell1Boards;
      std::vector<int>                          pinTell1s;
      CaloVector<Pin>                           pins;
      std::vector<Led>                          leds;

      // pileup
      int puMeth{0};
      int puBin{0};
      int puMin{0};

      dd4hep::Alignment  geometry;
      dd4hep::DetElement detElem;

      ROOT::Math::Transform3D toGlobalTransform;
      ROOT::Math::Transform3D toLocalTransform;
    };

  } // namespace detail

  namespace DeCalorimeterLocation {
    inline const std::string Ecal = "/world/DownstreamRegion/Ecal:DetElement-Info-IOV";
    inline const std::string Hcal = "/world/DownstreamRegion/Hcal:DetElement-Info-IOV";
  } // namespace DeCalorimeterLocation

  class DeCalorimeter : public DeIOVElement<detail::DeCaloObject> {

  public:
    using DeIOVElement::DeIOVElement;

    // general
    const std::string& caloName() const { return this->access()->caloName; }
    /// calorimeter index, @see namespace CaloCellCode
    CellCode::Index index() const { return this->access()->index; }
    // accessing the geometry parameters
    double       xSize() const { return this->access()->xSize; }
    double       ySize() const { return this->access()->ySize; }
    double       zSize() const { return this->access()->zSize; }
    double       zOffset() const { return this->access()->zOffset; }
    unsigned int numberOfAreas() const { return this->access()->nArea; }

    // reference plane in the global frame
    // ============================================================================
    // Return a reference (tilted) plane
    // ============================================================================
    inline ROOT::Math::Plane3D plane( const double dz ) const {
      if ( fabs( dz - this->access()->zOffset ) > this->access()->zSize / 2. ) {
        dd4hep::printout( dd4hep::DEBUG, "DeCalorimeter",
                          "THE REQUESTED PLANE IS OUTSIDE THE ACTIVE VOLUME of : %s  dz = %f z-enveloppe of the active "
                          "area = [%f ; %f]",
                          this->access()->detElem.name(), dz, this->access()->zOffset - this->access()->zSize / 2.,
                          this->access()->zOffset + this->access()->zSize / 2. );
      }
      ROOT::Math::XYZPoint point( 0, 0, dz ), goff( 0, 0, dz - 1 ); // arbitrary but non-zero z-offset
      point = this->access()->toGlobalTransform( point );
      goff  = this->access()->toGlobalTransform( goff );
      return ROOT::Math::Plane3D( point - goff, point );
    }

    // ============================================================================
    // Return a reference (tilted) plane
    // ============================================================================
    inline ROOT::Math::Plane3D plane( const Plane pos ) const {
      switch ( pos ) {
      case Plane::Front:
        return plane( this->access()->zOffset - this->access()->zSize / 2. );
      case Plane::ShowerMax:
        return plane( this->access()->zShowerMax );
      case Plane::Middle:
        return plane( this->access()->zOffset );
      case Plane::Back:
        return plane( this->access()->zOffset + this->access()->zSize / 2. );
      default:
        return plane( this->access()->zOffset );
      }
    }
    // ============================================================================
    // return a 3D-plane, which contain the given 3D-point in the global system
    // ============================================================================
    inline ROOT::Math::Plane3D plane( const ROOT::Math::XYZPoint& global ) const {
      return plane( this->access()->toLocalTransform( global ).Z() );
    }

    // accessing the calibration parameters
    double maxEtInCenter( unsigned int reg = 0 ) const {
      return ( reg < this->access()->maxEtInCenter.size() ) ? this->access()->maxEtInCenter[reg]
                                                            : this->access()->maxEtInCenter[0];
    }
    double maxEtSlope( unsigned int reg = 0 ) const {
      return ( reg < this->access()->maxEtSlope.size() ) ? this->access()->maxEtSlope[reg]
                                                         : this->access()->maxEtSlope[0];
    }

    double pedestalShift() const { return this->access()->pedestalShift; }
    double pinPedestalShift() const { return this->access()->pinPedestalShift; }
    double L0EtGain() const { return this->access()->l0Et; }
    double coherentNoise() const { return this->access()->coherentNoise; }
    double incoherentNoise() const { return this->access()->incoherentNoise; }
    double stochasticTerm() const { return this->access()->stochasticTerm; }
    double gainError() const { return this->access()->gainError; }

    double getCalibration( const CellID& ID ) const { return this->access()->cells[ID].calibration(); }

    // for simulation only
    int    zSupMethod() const { return this->access()->zSupMethod; }
    double zSupThreshold() const { return this->access()->zSupThreshold; }
    double zSupNeighbourThreshold() const { return this->access()->zSupNeighbourThreshold; }

    bool hasQuality( const CellID& ID, CellQuality::Flag flag ) const {
      int quality = this->access()->cells[ID].quality();
      return flag == CaloCellQuality::OK ? quality == 0 : ( ( quality & flag ) != 0 );
    }

    double l0Threshold() const { return this->access()->l0Thresh; }
    double mipDeposit() const { return this->access()->mip; }
    double dynamicsSaturation() const { return this->access()->dyn; }
    double fractionFromPrevious() const { return this->access()->prev; }

    double numberOfPhotoElectrons( unsigned int area = 0 ) const {
      return ( area < this->access()->phe.size() ) ? this->access()->phe[area] : 0.;
    }

    double activeToTotal() const { return this->access()->activeToTotal; }

    // accessing the hardware parameter(s)
    unsigned int adcMax() const { return this->access()->adcMax; }

    // accessing the reconstruction parameter(s)
    double zShowerMax() const { return this->access()->zShowerMax; }

    // accessing readout parameters
    unsigned int numberOfCells() const { return this->access()->cells.size() - numberOfPins(); }
    unsigned int numberOfCards() const { return this->access()->feCards.size(); }
    unsigned int numberOfTell1s() const { return this->access()->tell1Boards.size(); }
    unsigned int numberOfPins() const { return this->access()->pins.size(); }
    unsigned int numberOfLeds() const { return this->access()->leds.size(); }
    unsigned int numberOfInvalidCells() const {
      return std::count_if( this->access()->cells.begin(), this->access()->cells.end(),
                            []( const CellParam& c ) { return !c.cellID().isPin() && !c.valid(); } );
    }
    unsigned int pinArea() const { return this->access()->pinArea; }

    //  Cell Parameters
    bool   valid( const CellID& ID ) const { return this->access()->cells[ID].valid(); }
    double cellX( const CellID& ID ) const { return this->access()->cells[ID].x(); }
    double cellY( const CellID& ID ) const { return this->access()->cells[ID].y(); }
    double cellZ( const CellID& ID ) const { return this->access()->cells[ID].z(); }
    double cellSize( unsigned int area ) const { return this->access()->cellSizes[area]; }
    double cellSize( const CellID& ID ) const { return this->access()->cells[ID].size(); }
    double cellSine( const CellID& ID ) const { return this->access()->cells[ID].sine(); }
    double cellGain( const CellID& ID ) const { return this->access()->cells[ID].gain(); }
    // convert ADC to energy in MeV for a given cellID
    double cellEnergy( int adc, CellID ID ) const {
      double offset = ( static_cast<unsigned int>( this->access()->pinArea ) == ID.area() )
                          ? this->access()->pinPedestalShift
                          : this->access()->pedestalShift;
      return this->access()->cells[ID].gain() * ( (double)adc - offset );
    }
    // reverse operation : convert energy in MeV to ADC
    double cellADC( double energy, CellID ID ) const {
      double offset = ( static_cast<unsigned int>( this->access()->pinArea ) == ID.area() )
                          ? this->access()->pinPedestalShift
                          : this->access()->pedestalShift;
      double gain = this->access()->cells[ID].gain();
      return ( gain > 0 ) ? floor( energy / gain + offset + 0.5 ) : 0;
    }
    bool isSaturated( double energy, CellID id ) const { return cellADC( energy, id ) + 256 >= adcMax(); }

    double                     cellTime( const CellID& ID ) const { return this->access()->cells[ID].time(); }
    const ROOT::Math::XYZPoint cellCenter( const CellID& ID ) const { return this->access()->cells[ID].center(); }
    const std::vector<CellID>& neighborCells( const CellID& ID ) const { return this->access()->cells[ID].neighbors(); }
    const std::vector<CellID>& zsupNeighborCells( const CellID& ID ) const {
      return this->access()->cells[ID].zsupNeighbors();
    }

    bool isDead( const CellID& ID ) const { return this->hasQuality( ID, CaloCellQuality::Dead ); }
    bool isNoisy( const CellID& ID ) const { return hasQuality( ID, CaloCellQuality::Noisy ); }
    bool isShifted( const CellID& ID ) const { return hasQuality( ID, CaloCellQuality::Shifted ); }
    bool hasDeadLED( const CellID& ID ) const { return hasQuality( ID, CaloCellQuality::DeadLED ); }
    bool isVeryNoisy( const CellID& ID ) const { return hasQuality( ID, CaloCellQuality::VeryNoisy ); }
    bool isVeryShifted( const CellID& ID ) const { return hasQuality( ID, CaloCellQuality::VeryShifted ); }

    // from cellId to  serial number and vice-versa
    int    cellIndex( const CellID& ID ) const { return this->access()->cells.index( ID ); }
    CellID cellIdByIndex( const unsigned int num ) const {
      return ( ( num < this->access()->cells.size() ) ? ( this->access()->cells.begin() + num )->cellID()
                                                      : LHCb::Detector::Calo::CellID() );
    }
    // from cell to FEB
    bool isReadout( const CellID& ID ) const { return ( this->access()->cells[ID].cardNumber() >= 0 ) ? true : false; }
    int  cardNumber( const CellID& ID ) const { return this->access()->cells[ID].cardNumber(); }
    int  cardRow( const CellID& ID ) const { return this->access()->cells[ID].cardRow(); }
    int  cardColumn( const CellID& ID ) const { return this->access()->cells[ID].cardColumn(); }
    void cardAddress( const CellID& ID, int& card, int& row, int& column ) const {
      card   = this->access()->cells[ID].cardNumber();
      row    = this->access()->cells[ID].cardRow();
      column = this->access()->cells[ID].cardColumn();
    }
    //  FEB card
    int nCards() const { return this->access()->feCards.size(); }
    int downCardNumber( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->downNumber() : -1;
    }
    int leftCardNumber( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->leftNumber() : -1;
    }
    int cornerCardNumber( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->cornerNumber() : -1;
    }
    int previousCardNumber( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->previousNumber() : -1;
    }
    void cardNeighbors( const int card, int& down, int& left, int& corner ) const {
      auto feb = getFEB( card );
      down     = feb ? feb->downNumber() : -1;
      left     = feb ? feb->leftNumber() : -1;
      corner   = feb ? feb->cornerNumber() : -1;
    }
    int validationNumber( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->validationNumber() : -1;
    }
    int selectionType( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->selectionType() : -1;
    }
    int cardArea( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->area() : -1;
    }
    int cardFirstRow( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->firstRow() : -1;
    }
    int cardLastColumn( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->lastColumn() : -1;
    }
    int cardLastRow( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->lastRow() : -1;
    }
    int cardFirstColumn( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->firstColumn() : -1;
    }

    int cardFirstValidRow( const int card ) const {
      LHCb::Detector::Calo::CellID cell = this->firstCellID( card );
      if ( this->access()->cells[cell].valid() ) return cell.row();
      for ( int irow = this->cardFirstRow( card ); irow <= this->cardLastRow( card ); ++irow ) {
        for ( int icol = this->cardFirstColumn( card ); icol <= this->cardLastColumn( card ); ++icol ) {
          LHCb::Detector::Calo::CellID test( cell.calo(), cell.area(), irow, icol );
          if ( valid( test ) ) return irow;
        }
      }
      return -1;
    }
    int cardLastValidRow( const int card ) const {
      LHCb::Detector::Calo::CellID cell = this->lastCellID( card );
      if ( this->valid( cell ) ) return cell.row();

      for ( int irow = this->cardLastRow( card ); irow >= this->cardFirstRow( card ); --irow ) {
        for ( int icol = this->cardLastColumn( card ); icol >= this->cardFirstColumn( card ); --icol ) {
          LHCb::Detector::Calo::CellID test( cell.calo(), cell.area(), irow, icol );
          if ( valid( test ) ) return irow;
        }
      }
      return -1;
    }
    int cardFirstValidColumn( const int card ) const {
      LHCb::Detector::Calo::CellID cell = this->firstCellID( card );
      if ( this->valid( cell ) ) return cell.col();
      for ( int irow = this->cardFirstRow( card ); irow <= this->cardLastRow( card ); ++irow ) {
        for ( int icol = this->cardFirstColumn( card ); icol <= this->cardLastColumn( card ); ++icol ) {
          LHCb::Detector::Calo::CellID test( cell.calo(), cell.area(), irow, icol );
          if ( valid( test ) ) return icol;
        }
      }
      return -1;
    }
    int cardLastValidColumn( const int card ) const {
      LHCb::Detector::Calo::CellID cell = this->lastCellID( card );
      if ( this->valid( cell ) ) return cell.col();
      for ( int irow = this->cardLastRow( card ); irow >= this->cardFirstRow( card ); --irow ) {
        for ( int icol = this->cardLastColumn( card ); icol >= this->cardFirstColumn( card ); --icol ) {
          LHCb::Detector::Calo::CellID test( cell.calo(), cell.area(), irow, icol );
          if ( valid( test ) ) return icol;
        }
      }
      return -1;
    }

    CellID firstCellID( const int card ) const {
      auto feb = getFEB( card );
      return feb ? LHCb::Detector::Calo::CellID( this->access()->index, feb->area(), feb->firstRow(),
                                                 feb->firstColumn() )
                 : LHCb::Detector::Calo::CellID();
    }
    CellID lastCellID( const int card ) const {
      auto feb = getFEB( card );
      return feb ? LHCb::Detector::Calo::CellID( this->access()->index, feb->area(), feb->firstRow() + nRowCaloCard - 1,
                                                 feb->firstColumn() + nColCaloCard - 1 )
                 : LHCb::Detector::Calo::CellID();
    }
    CellID cardCellID( const int card, const int row, const int col ) const {
      auto feb = getFEB( card );
      return feb ? LHCb::Detector::Calo::CellID( this->access()->index, feb->area(), feb->firstRow() + row,
                                                 feb->firstColumn() + col )
                 : LHCb::Detector::Calo::CellID();
    }
    int cardCrate( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->crate() : -1;
    }
    int cardSlot( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->slot() : -1;
    }
    int cardCode( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->code() : -1;
    }
    const std::vector<CellID>& cardChannels( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->ids() : this->access()->empty;
    }
    const std::vector<CellID>& pinChannels( const CellID id ) const { return this->access()->pins[id].cells(); }
    const std::vector<CellID>& ledChannels( const int led ) const {
      return led < (int)this->access()->leds.size() ? this->access()->leds[led].cells() : this->access()->empty;
    }
    const std::vector<int>& pinLeds( const CellID id ) const { return this->access()->pins[id].leds(); }
    const std::vector<int>& cellLeds( const CellID id ) const { return this->access()->cells[id].leds(); }
    int                     cardIndexByCode( const int crate, const int slot ) const {
      auto i = std::find_if( this->access()->feCards.begin(), this->access()->feCards.end(),
                             [&]( const CardParam& c ) { return c.crate() == crate && c.slot() == slot; } );
      return i != this->access()->feCards.end() ? i - this->access()->feCards.begin() : -1;
    }
    int cardToTell1( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->tell1() : -1;
    }

    // Source_ID -> Vector of FEB
    std::vector<int> sourceID2FEBs( const int source_id ) const {
      std::map<int, std::vector<int>> map = this->access()->source_ids;
      auto                            it  = map.find( source_id );
      if ( it != map.end() ) {
        const auto& vec = it->second;
        return vec;
      }
      return {};
    }

    // FEB number -> FEB vector index (m_feCards)
    int getFEBindex( const int card ) const {
      auto i   = std::find_if( this->access()->feCards.begin(), this->access()->feCards.end(),
                             [&]( const CardParam& c ) { return c.number() == card; } );
      int  ind = i != this->access()->feCards.end() ? i - this->access()->feCards.begin() : -1;
      return ind;
    }

    const std::map<int, std::vector<int>>& getSourceIDsMap() const { return this->access()->source_ids; }

    // Tell1s
    int                 nTell1s() const { return this->access()->tell1Boards.size(); }
    Tell1Param::FECards tell1ToCards( const int tell1 ) const {
      return ( tell1 >= 0 && static_cast<unsigned int>( tell1 ) < this->access()->tell1Boards.size() )
                 ? this->access()->tell1Boards[tell1].feCards()
                 : Tell1Param::FECards{};
    }
    int nSourceIDs() const { return this->access()->source_ids.size(); }

    // CardParam/Tell1Param/CellParam
    CardParam cardParam( const int card ) const {
      auto feb = getFEB( card );
      return feb ? *feb : CardParam();
    }
    Tell1Param tell1Param( const unsigned int tell1 ) const {
      return ( tell1 < this->access()->tell1Boards.size() ) ? this->access()->tell1Boards[tell1] : Tell1Param( -1 );
    }
    CellParam cellParam( CellID id ) const { return this->access()->cells[id]; }
    Pin       caloPin( CellID id ) const { return this->access()->pins[id]; }
    Led       caloLed( const unsigned int led ) const {
      return ( led < this->access()->leds.size() ) ? this->access()->leds[led] : -1;
    }

    //  More complex functions
  private:
    CardParam const* getFEB( const int card ) const {
      auto itr = std::find_if( this->access()->feCards.begin(), this->access()->feCards.end(),
                               [&]( const CardParam& c ) { return c.number() == card; } );
      return itr != this->access()->feCards.end() ? &( *itr ) : nullptr;
    }

    detail::DeCaloObject::DeSubSubCaloObject const* getSubSubCalo( const ROOT::Math::XYZPoint& globalPoint,
                                                                   ROOT::Math::XYZPoint&       localPoint ) const {
      // if the point is outside calorimeter
      auto local = this->access()->toLocalTransform( globalPoint );
      if ( fabs( local.x() ) > this->access()->xBound || fabs( local.y() ) > this->access()->yBound ) return nullptr;

      // select side (left or right)
      auto& sc = this->access()->subcalos[local.x() > 0. ? 0 : 1];

      // select area (assume ordered from inner to outer)
      for ( auto& ssc : sc.subsubcalos ) {
        auto sub_local = ssc.toLocalTransform( globalPoint );
        if ( fabs( sub_local.x() ) > ssc.xBound || fabs( sub_local.y() ) > ssc.yBound ) continue;
        localPoint = sub_local;
        return &ssc;
      }
      return nullptr;
    }

  public:
    //----------------------------------------------------------------------------
    // ** Return the cell at the specified position
    //----------------------------------------------------------------------------
    CellID Cell( const ROOT::Math::XYZPoint& globalPoint ) const {
      auto pars = Cell_( globalPoint );
      return pars ? pars->cellID() : LHCb::Detector::Calo::CellID();
    }
    // ============================================================================
    /** return parameters of cell, which contains the global point.
     *  the function should be a bit more fast and efficient,
     *  @param globalPoint point to be checked
     *  @return cell parameters (null if point is not in Calorimeter)
     */
    // ============================================================================
    const CellParam* Cell_( const ROOT::Math::XYZPoint& globalPoint ) const {
      ROOT::Math::XYZPoint local;
      auto                 ssc = getSubSubCalo( globalPoint, local );
      if ( !ssc ) return nullptr;

      auto inv_cs = 1. / this->access()->cellSizes[ssc->area];
      int  col    = (int)( ( local.x() - ssc->xOffset ) * inv_cs + this->access()->firstRowUp );
      int  row    = (int)( local.y() * inv_cs + this->access()->firstRowUp );
      auto cellid = LHCb::Detector::Calo::CellID( this->index(), ssc->area, row, col );
      return LHCb::Detector::Calo::isValid( cellid ) ? &( this->access()->cells[cellid] ) : nullptr;
    }

    // ============================================================================
    /** return calorimeter area (e.g. Inner, Middle, Outer), which contains the global point.
     *  @param globalPoint point to be checked
     *  @return area id (CaloArea::UndefinedArea if outside of calorimeter - beam hole is treated as part of Inner)
     */
    // ============================================================================
    CellCode::CaloArea Area( const ROOT::Math::XYZPoint& globalPoint ) const {
      ROOT::Math::XYZPoint local;
      auto                 ssc = getSubSubCalo( globalPoint, local );
      return ssc ? LHCb::Detector::Calo::CellCode::CaloArea( ssc->area )
                 : LHCb::Detector::Calo::CellCode::CaloArea::UndefinedArea;
    }

    // Collections
    const CaloVector<CellParam>&   cellParams() const { return this->access()->cells; }
    const CaloVector<Pin>&         caloPins() const { return this->access()->pins; }
    const std::vector<Led>&        caloLeds() const { return this->access()->leds; }
    const std::vector<CardParam>&  cardParams() const { return this->access()->feCards; }
    const std::vector<Tell1Param>& tell1Params() const { return this->access()->tell1Boards; }
    // PIN flag
    bool isParasiticCard( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->isParasitic() : false;
    }
    bool isPmtCard( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->isPmtCard() : false;
    }
    bool isPinCard( const int card ) const {
      auto feb = getFEB( card );
      return feb ? feb->isPinCard() : false;
    }
    bool isPinTell1( const unsigned int tell1 ) const {
      return ( tell1 < this->access()->tell1Boards.size() ) ? this->access()->tell1Boards[tell1].readPin() : false;
    }
    bool isPinId( CellID ID ) const { return (unsigned)this->access()->pinArea == ID.area(); }

    // pileUp subtraction parameters
    int pileUpSubstractionMethod() const { return this->access()->puMeth; }
    int pileUpSubstractionBin() const { return this->access()->puBin; }
    int pileUpSubstractionMin() const { return this->access()->puMin; }

    CellID cellIDfromVolumeID( const dd4hep::DDSegmentation::VolumeID& volumeID ) const;
    CellID cellIDfromVolumeID_ECAL( const dd4hep::DDSegmentation::VolumeID& volumeID ) const;
    CellID cellIDfromVolumeID_HCAL( const dd4hep::DDSegmentation::VolumeID& volumeID ) const;
  };
} // End namespace LHCb::Detector::Calo
