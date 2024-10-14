/*****************************************************************************\
 * (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
 *                                                                             *
 * This software is distributed under the terms of the GNU General Public      *
 * Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
 *                                                                             *
 * In applying this licence, CERN does not waive the privileges and immunities *
 * granted to it by virtue of its status as an Intergovernmental Organization  *
 * or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "Detector/Muon/DAQHelper.h"
#include "Detector/Muon/DeMuonChamber.h"
#include "Detector/Muon/DeMuonGasGap.h"
#include "Detector/Muon/DeMuonStation.h"
#include "Detector/Muon/FrontEndID.h"
#include "Detector/Muon/MuonChamberGrid.h"
#include "Detector/Muon/MuonConstants.h"
#include "Detector/Muon/MuonVolumeID.h"
#include "Detector/Muon/NODEBoard.h"
#include "Detector/Muon/PackMCHit.h"
#include "Detector/Muon/TSMap.h"
#include "Detector/Muon/TileID.h"
#include <Core/DeIOV.h>
#include <DD4hep/DetElement.h>
#include <Detector/Muon/ReadoutCond.h>

#include <array>
#include <deque>
#include <fmt/format.h>
#include <map>
#include <string>
#include <vector>

namespace LHCb::Detector {

  // using namespace LHCb::Detector::Muon;

  namespace detail {
    /**
     *  Muon detector element data
     *  \author
     *  \date    2018-11-08
     *  \version  1.0
     */
    struct DeMuonObject : DeIOVObject {

      using NotImplemented  = LHCb::Detector::NotImplemented;
      using MuonChamberGrid = LHCb::Detector::Muon::detail::MuonChamberGrid;
      using FrontEndID      = LHCb::Detector::Muon::FrontEndID;
      using TileID          = LHCb::Detector::Muon::TileID;
      using Layout          = LHCb::Detector::Muon::Layout;
      using Chamber         = LHCb::Detector::DeMuonChamber;

      // geometry & readout info info
      std::array<float, Muon::nPartitions>        m_sensitiveAreaX{{}}; // filled by fillGeoInfo
      std::array<float, Muon::nPartitions>        m_sensitiveAreaY{{}}; // filled by fillGeoInfo
      std::array<float, Muon::nPartitions>        m_sensitiveAreaZ{{}}; // filled by fillGeoInfo
      std::array<float, Muon::nPartitions>        m_chamberAreaX{{}};
      std::array<float, Muon::nPartitions>        m_chamberAreaY{{}};
      std::array<float, Muon::nPartitions>        m_chamberAreaZ{{}};
      std::array<double, Muon::nPartitions>       m_areaChamber{{}}; // filled by fillGeoInfo
      std::array<int, Muon::nStations>            m_regsperSta{{}};  // filled by CountDetEls
      std::array<double, Muon::nStations>         m_posZstation{{}};
      std::array<unsigned int, Muon::nPartitions> m_chamberPerRegion{{}};
      //
      std::array<int, Muon::nPartitions>                                            m_readoutNumber{{}};
      std::array<unsigned int, Muon::nPartitions>                                   m_gapPerRegion{{}};
      std::array<unsigned int, Muon::nPartitions>                                   m_gapPerFE{{}};
      std::array<unsigned int, Muon::nPartitions>                                   m_LogMapPerRegion{{}};
      std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_readoutType{{}};
      std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_LogMapRType{{}};
      std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_LogMapMergex{{}};
      std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_LogMapMergey{{}};
      std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_phChannelNX{{}};
      std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_phChannelNY{{}};
      std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_phCardiacORNX{{}};
      std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_phCardiacORNY{{}};
      //
      std::array<float, Muon::nPartitions> m_padSizeX{{}};
      std::array<float, Muon::nPartitions> m_padSizeY{{}};
      //
      std::array<Layout, LHCb::Detector::Muon::N_OF_REGIONS> m_layout{{}};

      // grid of Logical Channels and Pads initialised in FillSystemGrids
      // Size of logical channels
      std::array<unsigned int, Muon::nPartitions> m_logVertGridX{{}};
      std::array<unsigned int, Muon::nPartitions> m_logVertGridY{{}};
      std::array<unsigned int, Muon::nPartitions> m_logHorizGridX{{}};
      std::array<unsigned int, Muon::nPartitions> m_logHorizGridY{{}};

      // Size of pads
      std::array<unsigned int, Muon::nPartitions> m_padGridX{{}};
      std::array<unsigned int, Muon::nPartitions> m_padGridY{{}};

      // grid Chamber Layout --> grid definition in MuonConstants.h
      std::array<unsigned int, Muon::N_OF_REGIONS> m_cgX{{}};
      std::array<unsigned int, Muon::N_OF_REGIONS> m_cgY{{}};
      //
      std::map<std::string, MuonChamberGrid> m_chambersGrids{{}};
      //
      struct ChamberIndex {
        int          index   = -1;
        unsigned int side    = 0;
        unsigned int quarter = 0;
        unsigned int nx      = 0;
        unsigned int ny      = 0;

        int          getIndex() const { return index; }
        unsigned int getQuarter() const { return quarter; }
      };
      //
      std::array<std::array<std::map<unsigned int, ChamberIndex>, Muon::nRegions>, Muon::nStations>
          m_chamberNumber_Index{{}};

      DeMuonObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      // using namespace LHCb::Detector::Muon;
      // Indexes of the stations by name
      enum class Station { M2 = 0, M3, M4, M5 };
      // static constexpr std::size_t N_OF_STATIONS = 4;
      struct StationDimensions {
        double innerX, innerY, outerX, outerY;
      };
      std::array<StationDimensions, LHCb::Detector::Muon::N_OF_STATIONS>   m_stationBox{{}};
      std::array<DeMuonStationObject, LHCb::Detector::Muon::N_OF_STATIONS> m_stations;
      //
      int m_stanum = 0;
      std::array<std::array<std::array<unsigned int, 2>, LHCb::Detector::Muon::N_OF_REGIONS>,
                 LHCb::Detector::Muon::N_OF_STATIONS>
          m_TSLayout = {{{}}};
      //
      Muon::DAQHelper m_daq;
      //
      unsigned int getStationFromZ( const double z ) const;
      //
      struct ChamberInfo {
        const DeMuonChamber detCh;
        const DeMuonGasGap  detFirstgap;
        unsigned int        ChNum = 0;
        double              xCh   = 0.0;
        double              yCh   = 0.0;
        double              zCh   = 0.0;
        double              Dx    = 0.0;
        double              Dy    = 0.0;
        double              Dz    = 0.0;
      };
      //

      class TilePosition {

      private:
        ROOT::Math::XYZPoint m_position{};
        double               m_dX = 0.0, m_dY = 0.0, m_dZ = 0.0;

      public:
        TilePosition( double x, double y, double z, double dX, double dY, double dZ )
            : m_position( x, y, z ), m_dX( dX ), m_dY( dY ), m_dZ( dZ ) {}
        auto position() const { return m_position; }
        auto x() const { return m_position.x(); }
        auto y() const { return m_position.y(); }
        auto z() const { return m_position.z(); }
        auto dX() const { return m_dX; }
        auto dY() const { return m_dY; }
        auto dZ() const { return m_dZ; }
      };

      // method to fill a vector of vector of maps that given station and region returns the map of the chambers:
      // m_chamberNumber_Index[ista][ireg]
      void FillChamberIndexMap();
      void FillGeoInfo();
      //
      // **************************************************************************************
      //
      // methods to get the DeMuonChamber Detector Element and/or the related chamber infos
      //
      // **************************************************************************************
      //
      std::optional<ChamberInfo> GetChamberInfoFromNum( int ista, int ireg, unsigned int chamberNumber ) const;
      //
      int     getChamberVectorIndex( int nx, int ny, int ireg, int iqua ) const;
      int     getChamberIdxFromTile( const TileID& tile ) const;
      int     getChamberNumber( const TileID& tile ) const;
      Chamber getChamberFromTile( const TileID& tile ) const;
      Chamber getChamber( unsigned int ista, unsigned int ireg,
                          unsigned int chamberNumber ) const; // icha <-> chamberNumber .. bisogna cambiare Boole ..
      //
      // **************************************************************************************
      //
      // Returns the list of physical channels for a given chamber
      std::vector<std::pair<FrontEndID, std::array<float, 4>>>
      listOfPhysChannels( ROOT::Math::XYZPoint my_entry, ROOT::Math::XYZPoint my_exit, int ireg, int icha ) const;
      //
      std::optional<ROOT::Math::XYZPoint> getPCCenter( FrontEndID fe, int icha, int ista, int ireg ) const;
      //
      void Hit2ChamberNumber( ROOT::Math::XYZPoint myPoint, int ista, unsigned int& chamberNumber, int& ireg ) const;
      //
      void chamberMostLikely( float x, float y, int ista, int& isid, int& ireg, int& icha ) const;
      //
      std::vector<unsigned int> chambersAround( int ista, int jsid, int jreg, float xref, float yref, float x,
                                                float y ) const;
      std::vector<unsigned int> chambersOut( int ista, int jsid, int jreg, float x, float y ) const;
      bool loopOnChambers( ROOT::Math::XYZPoint myPoint, std::vector<unsigned int> chaInfos, int ista,
                           unsigned int& chamberNumber, int& jreg ) const;
      //
      void        FillSystemGrids( unsigned int stationNumber, unsigned int regionNumber );
      std::string gridTag( unsigned int stationNumber, unsigned int regionNumber ) const;
      //
      std::optional<TilePosition> position( const TileID& tile );
      std::optional<TilePosition> getXYZChamber( const TileID& tile, bool toGlob );
      std::optional<TilePosition> getXYZPad( const TileID& tile );
      std::optional<TilePosition> getXYZLogical( const TileID& tile );
      std::optional<TilePosition> getXYZ( const Chamber& chamber, const int& gapNumber, bool toGlob );
      void                        localToGlobal( const Chamber* chamber, const ROOT::Math::XYZPoint& center,
                                                 const ROOT::Math::XYZPoint& corner, ROOT::Math::XYZPoint& delta );
      //
      //
      std::map<std::string, Muon::ReadoutCond> m_readoutModules;
      //
      //
      unsigned int              Tile2ChamberNumber( const TileID& tile ) const;
      std::vector<unsigned int> Logical2ChamberNumber( const TileID& tile ) const;
      //
      unsigned int Tile2FirstChamberNumber( const TileID& Tile ) const;
      //
      //
      int     Hit2ChamberRegionNumber( ROOT::Math::XYZPoint myPoint ) const;
      Chamber pos2StChamber( const double x, const double y, unsigned int ista ) const;
      int     regNum( const double x, const double y, unsigned int ista ) const;
      TileID  tileChamber( const Chamber chamber ) const;
      TileID  Chamber2Tile( unsigned int icha, unsigned int ista, unsigned int ireg ) const;
      //
      int  sensitiveVolumeID( const ROOT::Math::XYZPoint& point ) const;
      void Pos2ChamberTile( const double x, const double y, const double z, TileID& tile ) const;
      void Hit2GapNumber( ROOT::Math::XYZPoint point, unsigned int stationNumber, unsigned int& gapNumber,
                          unsigned int& chamberNumber, unsigned int& regionNumber ) const;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& station : m_stations ) { func( &station ); };
      }
    };

  } // End  namespace detail

  struct DeMuon : DeIOVElement<detail::DeMuonObject> {
    using DeIOVElement::DeIOVElement;

    using NotImplemented = LHCb::Detector::NotImplemented;

    using Station      = LHCb::Detector::detail::DeMuonObject::Station;
    using TileID       = LHCb::Detector::Muon::TileID;
    using TilePosition = LHCb::Detector::detail::DeMuonObject::TilePosition;
    using FrontEndID   = LHCb::Detector::Muon::FrontEndID;
    using Chamber      = LHCb::Detector::DeMuonChamber;
    using ChamberInfo  = LHCb::Detector::detail::DeMuonObject::ChamberInfo;

    const DeMuonStation getStation( unsigned int iS ) const {
      return DeMuonStation( &this->access()->m_stations[iS] );
    };

    const std::array<DeMuonStation, 4> getStations() const {
      const auto obj = this->access();
      return {DeMuonStation( &obj->m_stations[0] ), DeMuonStation( &obj->m_stations[1] ),
              DeMuonStation( &obj->m_stations[2] ), DeMuonStation( &obj->m_stations[3] )};
    };

    //  unsigned int gapID( int sensDetID ) const { return ( sensDetID & PackMCHit::maskGapID ) >>
    //  PackMCHit::shiftGapID; } unsigned int chamberID( int sensDetID ) const {
    //    return ( sensDetID & PackMCHit::maskChamberID ) >> PackMCHit::shiftChamberID;
    //  }
    //  unsigned int regionID( int sensDetID ) const {
    //    return ( sensDetID & PackMCHit::maskRegionID ) >> PackMCHit::shiftRegionID;
    //  }
    //  unsigned int stationID( int sensDetID ) const {
    //    return ( sensDetID & PackMCHit::maskStationID ) >> PackMCHit::shiftStationID;
    //  }
    //  unsigned int quadrantID( int sensDetID ) const {
    //    return ( sensDetID & PackMCHit::maskQuadrantID ) >> PackMCHit::shiftQuadrantID;
    //  }

    unsigned int gapID( int sensDetID ) const { return MuonVolumeID( sensDetID ).gap().value(); }
    unsigned int chamberID( int sensDetID ) const { return MuonVolumeID( sensDetID ).chamber().value(); }
    unsigned int regionID( int sensDetID ) const { return MuonVolumeID( sensDetID ).region().value(); }
    unsigned int stationID( int sensDetID ) const { return MuonVolumeID( sensDetID ).station().value(); }
    unsigned int quadrantID( int sensDetID ) const { return MuonVolumeID( sensDetID ).quadrant().value(); }
    unsigned int sideID( int sensDetID ) const { return MuonVolumeID( sensDetID ).side().value(); }

    double getInnerX( const Station station ) const {
      return this->access()->m_stationBox[static_cast<std::size_t>( station )].innerX;
    }
    double getInnerY( const Station station ) const {
      return this->access()->m_stationBox[static_cast<std::size_t>( station )].innerY;
    }
    double getOuterX( const Station station ) const {
      return this->access()->m_stationBox[static_cast<std::size_t>( station )].outerX;
    }
    double getOuterY( const Station station ) const {
      return this->access()->m_stationBox[static_cast<std::size_t>( station )].outerY;
    }

    bool upgradeReadout() const { return true; }
    int  stations() const { return Muon::nStations; }
    int  regions() const { return Muon::nPartitions; }
    int  regions( int ista ) const {
      // Number of regions in each station
      return this->access()->m_regsperSta[ista];
    }
    //
    double getStationZ( const unsigned int ista ) const { return this->access()->m_stations[ista].m_Zstation; }
    //
    // Returns the station index starting from the z position
    // unsigned int StationFromZ( const double z ) const { return this->access()->StationFromZ( z ); }
    unsigned int getStationFromZ( const double z ) const { return this->access()->getStationFromZ( z ); }
    //
    float getSensAreaX( const unsigned int ista, const unsigned int ireg ) const {
      return this->access()->m_sensitiveAreaX[ista * 4 + ireg];
    }
    float getSensAreaY( const unsigned int ista, const unsigned int ireg ) const {
      return this->access()->m_sensitiveAreaY[ista * 4 + ireg];
    }
    float getSensAreaZ( const unsigned int ista, const unsigned int ireg ) const {
      return this->access()->m_sensitiveAreaZ[ista * 4 + ireg];
    }
    double areaChamber( const unsigned int ista, const unsigned int ireg ) const {
      return this->access()->m_areaChamber[ista * 4 + ireg];
    }

    unsigned int chamberInRegion( const unsigned int ista, const unsigned int ireg ) const {
      return this->access()->m_chamberPerRegion[ista * 4 + ireg];
    }
    unsigned int gapsInRegion( const unsigned int ista, const unsigned int ireg ) const {
      return this->access()->m_gapPerRegion[ista * 4 + ireg];
    }

    std::optional<TilePosition> position( const Muon::TileID& tile ) const { return this->access()->position( tile ); }

    std::optional<TilePosition> getXYZChamber( const TileID& tile, bool toGlob = true ) const {
      return this->access()->getXYZChamber( tile, toGlob );
    }

    std::optional<TilePosition> getXYZPad( const TileID& tile ) const { return this->access()->getXYZPad( tile ); }

    std::optional<TilePosition> getXYZLogical( const TileID& tile ) const {
      return this->access()->getXYZLogical( tile );
    }

    std::optional<TilePosition> getXYZ( const Chamber& chamber, const int& gapNumber, bool toGlob ) const {
      return this->access()->getXYZ( chamber, gapNumber, toGlob );
    }
    //
    void CountDetEls() { throw NotImplemented(); }
    //
    std::string getStationName( int ista ) const {
      return std::string( this->access()->m_stations[ista].detector().name() ).substr( 0, 2 );
    }
    std::string getRegionName( int ista, int ireg ) const {
      return fmt::format( "{}R{}", getStationName( ista ), ireg + Muon::firstRegion );
    }
    //
    int gapsPerFE( const int ista, const int ireg ) const { return this->access()->m_gapPerFE[ista * 4 + ireg]; }
    //
    int readoutInRegion( const int ista, const int ireg ) const {
      return this->access()->m_readoutNumber[ista * 4 + ireg];
    }
    int mapInRegion( const int ista, const int ireg ) const {
      return this->access()->m_LogMapPerRegion[ista * 4 + ireg];
    }
    unsigned int getPhChannelNX( const int ireadout, const int ista, const int ireg ) const {
      return this->access()->m_phChannelNX[ireadout][ista * 4 + ireg];
    }
    unsigned int getPhChannelNY( const int ireadout, const int ista, const int ireg ) const {
      return this->access()->m_phChannelNY[ireadout][ista * 4 + ireg];
    }
    float getPadSizeX( const int ista, const int ireg ) const { return this->access()->m_padSizeX[ista * 4 + ireg]; }
    float getPadSizeY( const int ista, const int ireg ) const { return this->access()->m_padSizeY[ista * 4 + ireg]; }
    unsigned int getReadoutType( const int ireadout, const int ista, const int ireg ) const {
      return this->access()->m_readoutType[ireadout][ista * 4 + ireg];
    }
    unsigned int getLogMapInRegion( const int ista, const int ireg ) const {
      return this->access()->m_LogMapPerRegion[ista * 4 + ireg];
    }
    unsigned int getLogMapRType( const int ireadout, const int ista, const int ireg ) const {
      return this->access()->m_LogMapRType[ireadout][ista * 4 + ireg];
    }
    unsigned int getLogMapMergex( const int ireadout, const int ista, const int ireg ) const {
      return this->access()->m_LogMapMergex[ireadout][ista * 4 + ireg];
    }
    unsigned int getLogMapMergey( const int ireadout, const int ista, const int ireg ) const {
      return this->access()->m_LogMapMergey[ireadout][ista * 4 + ireg];
    }
    unsigned int getLayoutX( const int ireadout, const int ista, const int ireg ) const {
      return this->access()->m_daq.m_layoutX[ireadout][ista * 4 + ireg];
    }
    unsigned int getLayoutY( const int ireadout, const int ista, const int ireg ) const {
      return this->access()->m_daq.m_layoutY[ireadout][ista * 4 + ireg];
    }

    double getInnerX( const int ista ) const { return this->access()->m_stationBox[ista].innerX; }
    double getInnerY( const int ista ) const { return this->access()->m_stationBox[ista].innerY; }
    double getOuterX( const int ista ) const { return this->access()->m_stationBox[ista].outerX; }
    double getOuterY( const int ista ) const { return this->access()->m_stationBox[ista].outerY; }

    unsigned int getCardiacORX( const int par, const unsigned int ireadout ) const {
      return this->access()->m_phCardiacORNX[ireadout][par];
    }
    unsigned int getCardiacORY( const int par, const unsigned int ireadout ) const {
      return this->access()->m_phCardiacORNY[ireadout][par];
    }

    // Return the number of the first chamber from the Muon::TileID
    int Tile2FirstChamberNumber( const Muon::TileID Tile ) const {
      return this->access()->Tile2FirstChamberNumber( Tile );
    }

    // Returns the chamber's region number from a Hit
    int Hit2ChamberRegionNumber( const ROOT::Math::XYZPoint point ) const {
      return this->access()->Hit2ChamberRegionNumber( point );
    }
    int regNum( const double x, const double y, int stationNumber ) const {
      return this->access()->regNum( x, y, stationNumber );
    }

    struct ChamberNotFound {};

    Chamber pos2StChamber( const double x, const double y, unsigned int ista ) const {
      return this->access()->pos2StChamber( x, y, ista );
    }
    Muon::TileID Chamber2Tile( unsigned int icha, unsigned int ista, unsigned int ireg ) const {
      return this->access()->Chamber2Tile( icha, ista, ireg );
    }
    int sensitiveVolumeID( const ROOT::Math::XYZPoint& point ) const override {
      return this->access()->sensitiveVolumeID( point );
    }
    //
    // **************************************************************************************
    //
    // methods to get the DeMuonChamber Detector Element and/or the related chamber infos
    //
    // **************************************************************************************
    //
    std::optional<ChamberInfo> GetChamberInfoFromNum( int istat, int ireg, unsigned int chamberNumber ) const {
      return this->access()->GetChamberInfoFromNum( istat, ireg, chamberNumber );
    }
    //
    int getChamberVectorIndex( int nx, int ny, int ireg, int iqua ) const {
      return this->access()->getChamberVectorIndex( nx, ny, ireg, iqua );
    }

    int     getChamberIdxFromTile( const TileID& tile ) const { return this->access()->getChamberIdxFromTile( tile ); }
    int     getChamberNumber( const TileID& tile ) const { return this->access()->getChamberNumber( tile ); }
    Chamber getChamberFromTile( const TileID& tile ) const { return this->access()->getChamberFromTile( tile ); }
    Chamber getChamber( unsigned int ista, unsigned int ireg, unsigned int icha ) const {
      return this->access()->getChamber( ista, ireg, icha );
    }
    //
    // **************************************************************************************
    //
    // Returns the list of physical channels for a given chamber
    //
    std::vector<std::pair<FrontEndID, std::array<float, 4>>>
    listOfPhysChannels( ROOT::Math::XYZPoint my_entry, ROOT::Math::XYZPoint my_exit, int ireg, int icha ) const {
      return this->access()->listOfPhysChannels( my_entry, my_exit, ireg, icha );
    }
    //
    void Hit2ChamberNumber( ROOT::Math::XYZPoint myPoint, int ista, unsigned int& chamberNumber, int& ireg ) const {
      this->access()->Hit2ChamberNumber( myPoint, ista, chamberNumber, ireg );
    }
    //
    std::optional<ROOT::Math::XYZPoint> getPCCenter( FrontEndID fe, int icha, int ista, int ireg ) const {
      return this->access()->getPCCenter( fe, icha, ista, ireg );
    }
    //
    void chamberMostLikely( float x, float y, int ista, int& isid, int& ireg, int& icha ) const {
      return this->access()->chamberMostLikely( x, y, ista, isid, ireg, icha );
    }
    //
    std::vector<unsigned int> chambersAround( int ista, int jsid, int jreg, float xref, float yref, float x,
                                              float y ) const {
      return this->access()->chambersAround( ista, jsid, jreg, xref, yref, x, y );
    }
    //
    std::vector<unsigned int> chambersOut( int ista, int jsid, int jreg, float x, float y ) const {
      return this->access()->chambersOut( ista, jsid, jreg, x, y );
    }
    //
    bool loopOnChambers( ROOT::Math::XYZPoint myPoint, std::vector<unsigned int> chaInfos, int ista,
                         unsigned int& chamberNumber, int& jreg ) const {
      return this->access()->loopOnChambers( myPoint, chaInfos, ista, chamberNumber, jreg );
    }
    //
    void fillSystemGrids( unsigned int stationNumber, unsigned int regionNumber ) {
      return this->access()->FillSystemGrids( stationNumber, regionNumber );
    }
    std::string gridTag( unsigned int stationNumber, unsigned int regionNumber ) const {
      return this->access()->gridTag( stationNumber, regionNumber );
    }
    void Pos2ChamberTile( const double x, const double y, const double z, TileID& tile ) const {
      return this->access()->Pos2ChamberTile( x, y, z, tile );
    }
    void Hit2GapNumber( ROOT::Math::XYZPoint point, unsigned int stationNumber, unsigned int& gapNumber,
                        unsigned int& chamberNumber, unsigned int& regionNumber ) const {
      return this->access()->Hit2GapNumber( point, stationNumber, gapNumber, chamberNumber, regionNumber );
    }
    Muon::DAQHelper*       getDAQInfo() { return &this->access()->m_daq; }
    const Muon::DAQHelper* getDAQInfo() const { return &this->access()->m_daq; }
    Muon::DAQHelper*       getUpgradeDAQInfo() { return &this->access()->m_daq; }
    const Muon::DAQHelper* getUpgradeDAQInfo() const { return &this->access()->m_daq; }

    const Muon::ReadoutCond& getReadoutCond( const std::string& id ) const {
      if ( auto it = this->access()->m_readoutModules.find( id ); it != this->access()->m_readoutModules.end() ) {
        return it->second;
      } else {
        throw std::runtime_error( fmt::format( "cannot find readout condition for {}", id ) );
      }
    }
    const Muon::ReadoutCond& getReadoutCond( int ista, int ireg ) const {
      return getReadoutCond( getRegionName( ista, ireg ) );
    }
  };
} // End namespace LHCb::Detector
