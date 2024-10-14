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

#include "Detector/Muon/DAQConstants.h"
#include "Detector/Muon/L1Board.h"
#include "Detector/Muon/MuonConstants.h"
#include "Detector/Muon/NODEBoard.h"
#include "Detector/Muon/ODEBoard.h"
#include "Detector/Muon/TSMap.h"
#include "Detector/Muon/Tell40PCIBoard.h"
#include "Detector/Muon/TileID.h"

#include <optional>
#include <string>
#include <vector>

#include "Core/DeIOV.h"

namespace LHCb::Detector::Muon {
  using namespace std;
  static constexpr std::size_t N_OF_QUADRANTS  = 4;
  static constexpr std::size_t N_OF_PARTITIONS = N_OF_STATIONS * N_OF_REGIONS;
  enum class Station { M2 = 0, M3, M4, M5 };
  const std::array<std::string, N_OF_STATIONS> stationName = {"M2", "M3", "M4", "M5"};

  struct DAQHelper {
    DAQHelper() = default; // only needed for backward compatibility, should be dropped with all "Notimplemented
                           // functions"
    DAQHelper( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt );
    vector<vector<detail::TSMap>>  v_tsmap = vector<vector<detail::TSMap>>( 16 );
    vector<detail::NODEBoard>      v_node; //= vector<detail:NODEBoard>(DAQHelper_maxNODENumber);
    vector<detail::Tell40PCIBoard> v_pci;

    std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_layoutX{{}};
    std::array<std::array<unsigned int, Muon::nPartitions>, Muon::maxReadoutType> m_layoutY{{}};
    std::array<std::array<unsigned int, Muon::nRegions>, Muon::nStations>         m_TSlayoutX{{}};
    std::array<std::array<unsigned int, Muon::nRegions>, Muon::nStations>         m_TSlayoutY{{}};

    static constexpr unsigned int maxTell1Number = 14;
    static constexpr unsigned int maxODENumber   = 180;

    TileID getADDInODE( long, long ) const { throw NotImplemented(); }
    TileID getADDInODENoHole( long, long ) const { throw NotImplemented(); }
    TileID getADDInTell1( unsigned int, long ) const { throw NotImplemented(); }

    const std::vector<TileID>& getADDInTell1( unsigned int ) const { throw NotImplemented(); }
    TileID                     getPadInTell1DC06( unsigned int, long ) const { throw NotImplemented(); }
    TileID                     getPadInTell1V1( unsigned int, long ) const { throw NotImplemented(); }
    unsigned int               getPPNumber( unsigned int, unsigned int ) const { throw NotImplemented(); }
    inline unsigned int        TotTellNumber() const { throw NotImplemented(); }
    inline unsigned int        M1TellNumber() const { throw NotImplemented(); }
    inline std::string         Tell1Name( unsigned int ) const { throw NotImplemented(); }
    inline unsigned int        ODEInTell1( unsigned int ) const { throw NotImplemented(); }
    inline unsigned int        getODENumberInTell1( unsigned int, unsigned int ) const { throw NotImplemented(); }

    unsigned int     getODENumberInLink( unsigned int, unsigned int ) const { throw NotImplemented(); }
    Muon::L1Board    getL1Board( unsigned int ) { throw NotImplemented(); }
    Muon::ODEBoard   getODEBoard( Muon::L1Board, unsigned int ) const { throw NotImplemented(); }
    detail::TSMap    getTSMap( Muon::L1Board, Muon::ODEBoard, unsigned int ) const { throw NotImplemented(); }
    std::vector<int> getTell1InStation( int ) { throw NotImplemented(); }

    struct DAQAddress {
      unsigned int position;
      long         ODENumber;
    };

    DAQAddress  DAQaddressInODE( TileID, bool = true ) const { throw NotImplemented(); }
    DAQAddress  DAQaddressInL1( TileID, bool = true ) const { throw NotImplemented(); }
    int         findL1Index( std::string_view ) const { throw NotImplemented(); }
    std::string findL1( TileID ) const { throw NotImplemented(); }

    std::string               getBasePath( std::string ) const { throw NotImplemented(); }
    std::vector<unsigned int> padsinTS( std::vector<unsigned int>&, std::string ) const { throw NotImplemented(); }

    std::optional<DAQAddress>                findHWNumber( TileID ) const { throw NotImplemented(); }
    std::optional<std::pair<TileID, TileID>> findStrips( TileID ) const { throw NotImplemented(); }

    inline TileID TileInTell40FrameNoHole( unsigned int Tell40Number, unsigned int Tell40PCINumber,
                                           unsigned int Tell40PCILinkNumber, unsigned int chInFrame ) const {
      return m_mapTileInTell40[Tell40Number - 1][Tell40PCINumber]
                              [Tell40PCILinkNumber * DAQHelper_ODEFrameSize + chInFrame];
    };

    TileID TileInTell40Frame( unsigned int Tell40Number, unsigned int Tell40PCINumber, unsigned int Tell40PCILinkNumber,
                              unsigned int chInFrame ) const;

    inline unsigned int stationOfLink( long Tell40Number, unsigned int Tell40PCINumber,
                                       unsigned int Tell40PCILinkNumber ) const {
      return m_mapStationOfLink[Tell40Number - 1][Tell40PCINumber][Tell40PCILinkNumber];
    };
    inline unsigned int regionOfLink( long Tell40Number, unsigned int Tell40PCINumber,
                                      unsigned int Tell40PCILinkNumber ) const {
      return m_mapRegionOfLink[Tell40Number - 1][Tell40PCINumber][Tell40PCILinkNumber];
    };
    inline unsigned int quarterOfLink( long Tell40Number, unsigned int Tell40PCINumber,
                                       unsigned int Tell40PCILinkNumber ) const {
      return m_mapQuarterOfLink[Tell40Number - 1][Tell40PCINumber][Tell40PCILinkNumber];
    };

    inline unsigned int whichstationTell40( long Tell40Number ) const { return m_which_stationIsTell40[Tell40Number]; };
    void                getSynchConnection( unsigned int, unsigned int, long&, unsigned int&, unsigned int& ) const;

    inline unsigned int getNumberOfTell40Boards( unsigned int station ) const {
      return m_NumberOfTell40Boards[station];
    }

    inline TileID getTileIDInNODE( unsigned int ODENumber, unsigned int Tell40PCILinkNumber ) const {
      return m_mapTileInNODE[ODENumber][Tell40PCILinkNumber];
    }

    inline unsigned int getODENumberNoHole( long Tell40Number, unsigned int Tell40PCINumber,
                                            unsigned int Tell40PCILinkNumber ) const {
      return m_ODENumberNoHole[Tell40Number][Tell40PCINumber][Tell40PCILinkNumber];
    }

    inline unsigned int getODEFrameNumberNoHole( long Tell40Number, unsigned int Tell40PCINumber,
                                                 unsigned int Tell40PCILinkNumber ) const {
      return m_ODEFrameNumberNoHole[Tell40Number][Tell40PCINumber][Tell40PCILinkNumber];
    }

    unsigned int getNumberOfActiveLinkInTell40PCI( unsigned int Tell40Number, unsigned int Tell40PCINumber ) const {
      return m_Tell40PCINumberOfActiveLink[Tell40Number - 1][Tell40PCINumber];
    }
    std::string getODENameInECS( unsigned int ODENumber ) const { return m_ODENameInECS[ODENumber - 1]; }
    void        getODENumberAndSynch( long, unsigned int, unsigned int, int&, int& ) const;

    int getSourceID( long Tell40Number, unsigned int Tell40PCINumber ) const {

      return m_sourceID[Tell40Number - 1][Tell40PCINumber];
    };

    unsigned int getDeclaredLinks( long Tell40Number, unsigned int Tell40PCINumber ) const {
      return m_declaredLinks[Tell40Number - 1][Tell40PCINumber];
    };
    TileID       findTS( const TileID digit ) const;
    unsigned int findODENumber( TileID TS ) const;

    void findTSNumber( unsigned int odeNum, TileID TSTile, unsigned int& TSNumber, unsigned int& TSMapNum ) const;
    unsigned int findDigitInTS( unsigned int TSPosInNODE, unsigned int TSMapNumber, TileID TSTile, TileID digit,
                                bool hole, unsigned int& TSSize ) const;

    void DAQaddressInODE( TileID digitTile, unsigned int& ODENumber, unsigned int& frame,
                          unsigned int& channelInFrame ) const;

    void         dumpData();
    void         initNODE();
    void         initMaps();
    void         initSourceID();
    void         testInitialization();
    unsigned int getTSLayoutX( unsigned int s, unsigned int r ) const { return m_TSlayoutX[s][r]; };
    unsigned int getTSLayoutY( unsigned int s, unsigned int r ) const { return m_TSlayoutY[s][r]; };

    const vector<detail::Tell40PCIBoard> getTell40s() const { return v_pci; };

    unsigned int m_startNODEInStation[N_OF_STATIONS]; // numbered from zero 0 68 100 120
    unsigned int m_stopNODEInStation[N_OF_STATIONS];  // numbered from 1 (so stop at 68 100 120 144
    unsigned int m_ODENameSRQStart[N_OF_STATIONS][N_OF_REGIONS][N_OF_QUADRANTS] = {};
    unsigned int m_ODENameSRQStop[N_OF_STATIONS][N_OF_REGIONS][N_OF_QUADRANTS]  = {};
    unsigned int m_which_stationIsTell40[DAQHelper_maxTell40Number]; // numbered from zero

    unsigned int m_TotTell40 = 0; // TELL1 counter
    // not used check
    // unsigned int m_NODENumberInTell40[MuonUpgradeDAQHelper_maxTell40Number];
    // not used check
    // std::vector<unsigned int> m_NODEInTell40[MuonUpgradeDAQHelper_maxTell40Number];

    // Tiles in each ODE  ODENumber as index starting from zero (NODESerialNumber-1) returned vector is 192
    std::vector<TileID> m_mapTileInNODE[DAQHelper_maxNODENumber];

    // map to know ODE/synch frame ->Tell40  Tell40input connection
    // to which Tell40 a specif nsync is connected Tell40 is numbered from 1
    unsigned int m_Tell40Number[DAQHelper_maxNODENumber][DAQHelper_frameNumber] = {};

    // map to know ODE/synch frame ->Tell40 PCI Tell40input connection
    // to which Tell40 a specif nsync is connected Tell40 PCI is numbered from 0
    unsigned int m_Tell40PCINumber[DAQHelper_maxNODENumber][DAQHelper_frameNumber] = {};

    // map to know ODE/synch frame ->Tell40 PCI link Tell40input connection
    // to which Tell40 a specif nsync is connected Tell40 PCI link is numbered from 0 (link counts also the holes)
    unsigned int m_Tell40PCILinkNumber[DAQHelper_maxNODENumber][DAQHelper_frameNumber] = {};

    // map Tell40 PCI Tell40input connection to  ODE/synch frame

    // map which ODE is connected to Tell40 (numbered from 0) PCINumber (numbered from 0) link (hole are not counted)
    unsigned int m_ODENumberNoHole[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber][DAQHelper_linkNumber] = {};

    // map which ODE frame is connected to Tell40 (numbered from 0) PCINumber (numbered from 0) link (hole are not
    // counted)
    unsigned int m_ODEFrameNumberNoHole[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber][DAQHelper_linkNumber] =
        {};

    // map which ODE is connected to Tell40 (numbered from 0) PCINumber (numbered from 0) link (hole are counted)
    unsigned int m_ODENumberWithHole[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber][DAQHelper_linkNumber] =
        {};

    // map which ODE frame is connected to Tell40 (numbered from 0) PCINumber (numbered from 0) link (hole are  counted)
    unsigned int m_ODEFrameNumberWithHole[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber]
                                         [DAQHelper_linkNumber] = {};

    //
    unsigned int m_Tell40LinkToActiveLink[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber]
                                         [DAQHelper_linkNumber] = {};

    // map Tell40frame ->MuonTileID
    TileID m_mapTileInTell40[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber]
                            [DAQHelper_linkNumber * DAQHelper_ODEFrameSize];

    unsigned int m_Tell40PCINumberOfActiveLink[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber]            = {};
    unsigned int m_mapStationOfLink[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber][DAQHelper_linkNumber] = {};
    unsigned int m_mapRegionOfLink[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber][DAQHelper_linkNumber]  = {};
    unsigned int m_mapQuarterOfLink[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber][DAQHelper_linkNumber] = {};

    std::string                         m_ODENameInECS[DAQHelper_maxNODENumber];
    std::array<unsigned int, nStations> m_NumberOfTell40Boards;
    int                                 m_sourceID[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber]      = {};
    unsigned int                        m_declaredLinks[DAQHelper_maxTell40Number][DAQHelper_maxTell40PCINumber] = {};
  };
  //  using DAQHelper = DAQHelperElement<detail::DAQHelperObject>;

} // End namespace LHCb::Detector::Muon
