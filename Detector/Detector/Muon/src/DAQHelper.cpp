/***************************************************************************** \
 * (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
 *                                                                             *
 * This software is distributed under the terms of the GNU General Public      *
 * Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
 *                                                                             *
 * In applying this licence, CERN does not waive the privileges and immunities *
 * granted to it by virtue of its status as an Intergovernmental Organization  *
 * or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include "Detector/Muon/DAQHelper.h"
#include "Core/Utils.h"

#include "DD4hep/Printout.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>

LHCb::Detector::Muon::DAQHelper::DAQHelper( dd4hep::DetElement const& de, dd4hep::cond::ConditionUpdateContext& ctxt ) {
  bool debug = false;
  // add printout
  std::string name_to_print = "DAQHelper";
  // dd4hep::printout( dd4hep::DEBUG, "qui quiq", "StationNumber: %d", condTS.size() );
  unsigned int node_counter   = 0;
  unsigned int Tell40_counter = 0;
  for ( unsigned int sta = 0; sta < N_OF_STATIONS; sta++ ) {

    const auto& m2_load =
        ctxt.condition( dd4hep::ConditionKey::KeyMaker( de, stationName[sta] ).hash ).get<nlohmann::json>();
    auto m2_load_yaml = utils::json2yaml( m2_load );

    unsigned int stationNumber = m2_load["StationNumber"].get<unsigned int>() - 1;
    if ( sta != stationNumber )
      dd4hep::printout( dd4hep::DEBUG, name_to_print, "StationNumber: %d non equal to map number %d", sta,
                        stationNumber );

    // load TS description
    auto TSLayout = m2_load["TSLayout"];
    for ( auto const& item : TSLayout ) { dd4hep::printout( dd4hep::DEBUG, name_to_print, "Item: %d", item.size() ); }
    int reg = 0;
    for ( auto const& item : TSLayout ) {
      std::vector TSReg               = item.get<std::vector<unsigned int>>();
      m_TSlayoutX[stationNumber][reg] = TSReg[1];
      m_TSlayoutY[stationNumber][reg] = TSReg[2];
      dd4hep::printout( dd4hep::DEBUG, name_to_print, "New TS Layout  %d   %d %d %d %d     %d", stationNumber, reg,
                        TSReg[1], m_TSlayoutX[stationNumber][reg], TSReg[2], m_TSlayoutY[stationNumber][reg] );

      reg++;
    }

    auto TSmaps = m2_load["TSMap"];
    for ( unsigned int reg = 0; reg < 4; reg++ ) {
      const unsigned int ts_reg = TSmaps[reg]["RegionNumber"].get<unsigned int>();
      if ( ts_reg != reg ) dd4hep::printout( dd4hep::DEBUG, name_to_print, "problem in TS region %d", reg );
      const unsigned int ts_number = TSmaps[reg]["TSNumber"].get<unsigned int>();
      for ( unsigned int map = 0; map < ts_number; map++ ) {
        detail::TSMap temp( m2_load_yaml, reg, map );
        v_tsmap[stationNumber * N_OF_REGIONS + reg].push_back( temp );
      }
    }
    // now calculate the logical layout (2 possible layouts per region)
    for ( unsigned int r = 0; r < N_OF_REGIONS; r++ ) {
      unsigned int p     = stationNumber * N_OF_REGIONS + r;
      unsigned int n_lay = ( ( v_tsmap[p] )[0] ).numberOfLayout();
      for ( unsigned l = 0; l < n_lay; l++ ) {
        m_layoutX[l][p] = ( ( v_tsmap[p] )[0] ).gridXLayout( l ) * m_TSlayoutX[stationNumber][r];
        m_layoutY[l][p] = ( ( v_tsmap[p] )[0] ).gridYLayout( l ) * m_TSlayoutY[stationNumber][r];
      }
    }
    // test

    auto         ODEList = m2_load["NODEBoard"];
    unsigned int numODE  = 0;
    for ( auto ode : ODEList ) {
      detail::NODEBoard temp( m2_load_yaml, numODE );
      v_node.push_back( temp );

      // unsigned int serialnumber=temp.getNODESerialNumber()-1;
      if ( numODE == 0 ) m_startNODEInStation[stationNumber] = node_counter;
      m_stopNODEInStation[stationNumber] = node_counter;
      numODE++;
      node_counter++;
    }
    auto         tellList = m2_load["TELL40Board"];
    unsigned int localT   = 0;
    // read the number of PCI per Tell40 and create the obj
    for ( auto tb : tellList ) {
      const unsigned int enabledPCI = tb["UsedPCINumber"].get<unsigned int>();
      for ( unsigned int pci = 0; pci < enabledPCI; pci++ ) {
        v_pci.emplace_back( detail::Tell40PCIBoard( m2_load_yaml, sta, localT, pci ) );
      }
      m_which_stationIsTell40[Tell40_counter] = sta;
      localT++;
      Tell40_counter++;
    }
    // Tell40PCIBoard
    m_NumberOfTell40Boards[sta] = Tell40_counter;
  }

  initNODE();
  initMaps();
  initSourceID();

  // these two only for testing scope
  if ( debug ) {
    dumpData();
    testInitialization();
  }
}

void LHCb::Detector::Muon::DAQHelper::initNODE() {

  std::string  name_to_print                                = "DAQHelper";
  bool         connectedODE[DAQHelper_maxNODENumber]        = {};
  bool         connectedLinkODE[DAQHelper_maxNODENumber][4] = {};
  unsigned int NODECounter                                  = 0;
  for ( auto pci : v_pci ) {
    unsigned int station = pci.getStation();
    // loop on link and enstablish if it is connected
    for ( unsigned int link = 0; link < DAQHelper_linkNumber; link++ ) {
      unsigned int node = pci.getODENumber( link );
      if ( node > 0 ) {
        connectedODE[node - 1] = 1;
        if ( node < m_startNODEInStation[station] ) m_startNODEInStation[station] = node;
        if ( node > m_stopNODEInStation[station] ) m_stopNODEInStation[station] = node;

        unsigned int nlink = pci.getLinkNumber( link );
        if ( connectedLinkODE[node - 1][nlink] > 0 ) {
          dd4hep::printout( dd4hep::ERROR, name_to_print, "same link %d for nODE %d already connected", nlink, node );
          throw std::invalid_argument( "check nODE connection" );
        } else {
          connectedLinkODE[node - 1][nlink] = 1;
        }
      }
    }
  }

  for ( unsigned int i = 0; i < DAQHelper_maxNODENumber; i++ ) {
    if ( connectedODE[i] ) NODECounter++;
  }
  if ( NODECounter > DAQHelper_maxNODENumber ) {
    dd4hep::printout( dd4hep::ERROR, name_to_print,
                      " Number of retrieved NODEs in CondDB exceeds max allowed in software: %d", NODECounter );
    throw std::invalid_argument( "Too much NODEBoard in maps" );
  }

  for ( unsigned int s = 0; s < N_OF_STATIONS; s++ ) {
    // check thatNODE number are sequantially assigned
    if ( m_stopNODEInStation[s] < m_startNODEInStation[s] ) {
      dd4hep::printout( dd4hep::DEBUG, name_to_print, "NODE number not assigned sequantially correct the maps" );
      throw std::invalid_argument( "check nODE numbering" );
    }
    for ( unsigned int is = s + 1; is < N_OF_STATIONS; is++ ) {
      if ( m_stopNODEInStation[s] > m_startNODEInStation[is] ) {
        dd4hep::printout( dd4hep::DEBUG, name_to_print, "NODE number not assigned sequantially correct the maps" );
        throw std::invalid_argument( "check nODE numbering" );
      }
    }
  }
}

void LHCb::Detector::Muon::DAQHelper::dumpData() {
  std::string name_to_print = "DAQHelper dumper";
  for ( unsigned int i = 0; i < N_OF_STATIONS; i++ ) {
    for ( unsigned j = 0; j < 2; j++ ) {
      for ( unsigned int r = 0; r < N_OF_REGIONS; r++ ) {
        dd4hep::printout( dd4hep::DEBUG, name_to_print, "sta lay reg LayoutX %d %d %d %d", i, j, r,
                          m_layoutX[j][i * N_OF_REGIONS + r] );
        dd4hep::printout( dd4hep::DEBUG, name_to_print, "sta lay reg LayoutY %d %d %d %d", i, j, r,
                          m_layoutY[j][i * N_OF_REGIONS + r] );
      }
    }
    for ( unsigned int r = 0; r < N_OF_REGIONS; r++ ) {
      for ( unsigned int q = 0; q < 4; q++ ) {
        dd4hep::printout( dd4hep::DEBUG, name_to_print, "start and stop ODE in s r q %d %d %d %d %d", i, r, q,
                          m_ODENameSRQStart[i][r][q], m_ODENameSRQStop[i][r][q] );
      }
    }
  }
}

void LHCb::Detector::Muon::DAQHelper::initMaps() {
  std::string name_to_print = "DAQHelper initMaps";
  // set the correspondance among NODE link and Tell40 input
  for ( auto pci : v_pci ) {
    dd4hep::printout( dd4hep::DEBUG, name_to_print, "reading Tell40 # %d", pci.Tell40Number() );
    unsigned int  noHoleCounter       = 0;
    unsigned long TNumber             = pci.Tell40Number();
    unsigned int  PNumber             = pci.Tell40PCINumber();
    unsigned int  active_link_per_PCI = 0;
    dd4hep::printout( dd4hep::DEBUG, name_to_print, "Tell 40 %d %d %d", TNumber, PNumber, pci.connectedLinks() );
    m_declaredLinks[TNumber - 1][PNumber] = pci.connectedLinks();
    for ( unsigned int iLink = 0; iLink < DAQHelper_linkNumber; iLink++ ) {
      unsigned int node = pci.getODENumber( iLink );
      dd4hep::printout( dd4hep::DEBUG, name_to_print, "examining link connected to NODE # %d", node );
      if ( node > 0 ) {
        unsigned int nlink                                          = pci.getLinkNumber( iLink );
        m_Tell40Number[node - 1][nlink]                             = TNumber;
        m_Tell40PCINumber[node - 1][nlink]                          = PNumber;
        m_Tell40PCILinkNumber[node - 1][nlink]                      = iLink;
        m_ODENumberNoHole[TNumber - 1][PNumber][noHoleCounter]      = node;
        m_ODEFrameNumberNoHole[TNumber - 1][PNumber][noHoleCounter] = nlink;
        m_ODENumberWithHole[TNumber - 1][PNumber][iLink]            = node;
        m_ODEFrameNumberWithHole[TNumber - 1][PNumber][iLink]       = nlink;
        m_Tell40LinkToActiveLink[TNumber - 1][PNumber][iLink]       = noHoleCounter;
        noHoleCounter++;
        active_link_per_PCI++;
        m_Tell40PCINumberOfActiveLink[TNumber - 1][PNumber] = active_link_per_PCI;
        dd4hep::printout( dd4hep::DEBUG, name_to_print, "reading nODE %d %d %d %d %d %d ", TNumber - 1, PNumber, iLink,
                          node, nlink, noHoleCounter );
      }
    }
  }
  // fill the map of which  MuonTileID is inside an ODE channel
  //  return;
  for ( unsigned int s = 0; s < N_OF_STATIONS; s++ ) {
    for ( unsigned int r = 0; r < N_OF_REGIONS; r++ ) {
      for ( unsigned int q = 0; q < 4; q++ ) {
        m_ODENameSRQStart[s][r][q] = 999;
        m_ODENameSRQStop[s][r][q]  = 0;
      }
    }
  }
  for ( unsigned int station = 0; station < N_OF_STATIONS; station++ ) {

    unsigned int region          = 0;
    unsigned int unique_quadrant = 99;

    dd4hep::printout( dd4hep::DEBUG, name_to_print, "start stop in station %d %d %d", station,
                      m_startNODEInStation[station], m_stopNODEInStation[station] );
    for ( unsigned int i = m_startNODEInStation[station]; i < m_stopNODEInStation[station]; i++ ) {

      unsigned int node_number = i;

      LHCb::Detector::Muon::detail::NODEBoard nodeboard( v_node[node_number] );
      unsigned int                            NODESerialNumber = nodeboard.getNODESerialNumber();
      m_ODENameInECS[NODESerialNumber - 1]                     = nodeboard.ECSName();
      region                                                   = nodeboard.region();
      unique_quadrant                                          = nodeboard.quadrant();

      if ( m_ODENameSRQStart[station][region][unique_quadrant] > nodeboard.getNODESerialNumber() )
        m_ODENameSRQStart[station][region][unique_quadrant] = nodeboard.getNODESerialNumber();
      if ( m_ODENameSRQStop[station][region][unique_quadrant] < nodeboard.getNODESerialNumber() )
        m_ODENameSRQStop[station][region][unique_quadrant] = nodeboard.getNODESerialNumber();

      unsigned int TSLayoutX = getTSLayoutX( station, region );
      unsigned int TSLayoutY = getTSLayoutY( station, region );

      for ( unsigned int iTS = 0; iTS < nodeboard.getTSNumber(); iTS++ ) {

        unsigned int                        tsmapnum = nodeboard.getTSMapNumber( iTS );
        LHCb::Detector::Muon::detail::TSMap myts( v_tsmap[station * N_OF_REGIONS + region][tsmapnum - 1] );

        unsigned int quadrant = nodeboard.getTSQuadrant( iTS );
        unsigned int TSGridX  = nodeboard.getTSGridX( iTS );
        unsigned int TSGridY  = nodeboard.getTSGridY( iTS );

        int iusedch = 0;
        for ( unsigned int isynch = 0; isynch < myts.synchChSize(); isynch++ ) {
          if ( myts.synchChUsed( isynch ) ) {
            unsigned int layout       = myts.layoutOutputChannel( iusedch );
            unsigned int layoutX      = myts.gridXLayout( layout );
            unsigned int layoutY      = myts.gridYLayout( layout );
            unsigned int digitOffSetX = layoutX * TSGridX;
            unsigned int digitOffSetY = layoutY * TSGridY;
            unsigned int digitX       = digitOffSetX + myts.gridXOutputChannel( iusedch );
            unsigned int digitY       = digitOffSetY + myts.gridYOutputChannel( iusedch );

            LHCb::Detector::Muon::Layout lay( TSLayoutX * layoutX,
                                              TSLayoutY * layoutY ); // layout in terms of H- or V- strips
            LHCb::Detector::Muon::TileID muontile( station, lay, region, quadrant, digitX, digitY );

            m_mapTileInNODE[nodeboard.getNODESerialNumber() - 1].push_back( muontile );
            iusedch++;
          } else {
            // empty tiles for unused channels
            LHCb::Detector::Muon::TileID muontile;
            m_mapTileInNODE[nodeboard.getNODESerialNumber() - 1].push_back( muontile );
          }
        }
      }
    }
  }

  for ( unsigned int t = 0; t < DAQHelper_maxTell40Number; t++ ) {
    for ( unsigned int p = 0; p < DAQHelper_maxTell40PCINumber; p++ ) {
      for ( unsigned int l = 0; l < DAQHelper_linkNumber; l++ ) {
        unsigned int ode   = m_ODENumberNoHole[t][p][l];
        unsigned int frame = m_ODEFrameNumberNoHole[t][p][l];
        dd4hep::printout( dd4hep::DEBUG, name_to_print, "Tell40 %d pci %d link %d ode %d frame %d", t, p, l, ode,
                          frame );
        if ( ode > 0 ) {
          for ( unsigned int ch = 0; ch < DAQHelper_ODEFrameSize; ch++ ) {
            LHCb::Detector::Muon::TileID pippo = m_mapTileInNODE[ode - 1][frame * 48 + ch];
            if ( pippo.isValid() ) {
              m_mapStationOfLink[t][p][l] = pippo.station();
              m_mapRegionOfLink[t][p][l]  = pippo.region();
              m_mapQuarterOfLink[t][p][l] = pippo.quarter();
            }

            m_mapTileInTell40[t][p][(l)*DAQHelper_ODEFrameSize + ch] = pippo;
            dd4hep::printout( dd4hep::DEBUG, name_to_print, "map filling %d %d %d %d era in ode frame %d %d %s", t, p,
                              l, (l)*DAQHelper_ODEFrameSize + ch, frame, (frame)*DAQHelper_ODEFrameSize + ch,
                              pippo.toString().c_str() );
          }
        }
      }
    }
  }
}

LHCb::Detector::Muon::TileID LHCb::Detector::Muon::DAQHelper::TileInTell40Frame( unsigned int Tell40Number,
                                                                                 unsigned int Tell40PCINumber,
                                                                                 unsigned int Tell40PCILinkNumber,
                                                                                 unsigned int chInFrame ) const {

  unsigned int linkNoHole = m_Tell40LinkToActiveLink[Tell40Number - 1][Tell40PCINumber][Tell40PCILinkNumber];

  return m_mapTileInTell40[Tell40Number - 1][Tell40PCINumber][linkNoHole * DAQHelper_ODEFrameSize + chInFrame];
}

LHCb::Detector::Muon::TileID LHCb::Detector::Muon::DAQHelper::findTS( const LHCb::Detector::Muon::TileID digit ) const {

  unsigned int station = digit.station();
  unsigned int region  = digit.region();

  unsigned int                 TSLayoutX = getTSLayoutX( station, region );
  unsigned int                 TSLayoutY = getTSLayoutY( station, region );
  LHCb::Detector::Muon::Layout TSLayout( TSLayoutX, TSLayoutY );

  return TSLayout.contains( digit );
}

unsigned int LHCb::Detector::Muon::DAQHelper::findODENumber( LHCb::Detector::Muon::TileID TS ) const {
  unsigned int station  = TS.station();
  unsigned int region   = TS.region();
  unsigned int quadrant = TS.quarter();
  unsigned int odeStart = m_ODENameSRQStart[station][region][quadrant];
  unsigned int odeEnd   = m_ODENameSRQStop[station][region][quadrant] + 1;
  for ( unsigned int ode = odeStart; ode < odeEnd; ode++ ) {
    if ( ( v_node[ode - 1] ).isTSContained( TS ) ) return ( v_node[ode - 1] ).getNODESerialNumber();
  }

  throw std::invalid_argument( "the TS has not contained in nODE" );
  return 0;
}

void LHCb::Detector::Muon::DAQHelper::findTSNumber( unsigned int odeNum, LHCb::Detector::Muon::TileID TSTile,
                                                    unsigned int& TSNumber, unsigned int& TSMapNum ) const {
  unsigned int                            quadrant = TSTile.quarter();
  unsigned int                            gridx    = TSTile.nX();
  unsigned int                            gridy    = TSTile.nY();
  LHCb::Detector::Muon::detail::NODEBoard nodeboard( v_node[odeNum - 1] );

  for ( unsigned int TS = 0; TS < nodeboard.getTSNumber(); TS++ ) {
    if ( nodeboard.getTSQuadrant( TS ) == quadrant ) {
      if ( nodeboard.getTSGridX( TS ) == gridx ) {
        if ( nodeboard.getTSGridY( TS ) == gridy ) {
          TSNumber = TS;
          TSMapNum = nodeboard.getTSMapNumber( TS );
          return;
        }
      }
    }
  }

  dd4hep::printout( dd4hep::ERROR, "DAQHelper", "the TS has not been found where expected" );
  throw std::invalid_argument( "the TS has not contained in nODE" );

  return;
}

unsigned int LHCb::Detector::Muon::DAQHelper::findDigitInTS( unsigned int TSPosInNODE, unsigned int TSMapNumber,
                                                             LHCb::Detector::Muon::TileID TSTile,
                                                             LHCb::Detector::Muon::TileID digit, bool hole,
                                                             unsigned int& TSSize ) const {
  unsigned int                        station = TSTile.station();
  unsigned int                        region  = TSTile.region();
  LHCb::Detector::Muon::detail::TSMap TS( v_tsmap[station * N_OF_REGIONS + region][TSMapNumber - 1] );

  TSSize = TS.synchChSize();

  unsigned int TSLayoutX = TSTile.layout().xGrid();
  unsigned int TSLayoutY = TSTile.layout().yGrid();

  unsigned int layoutX = digit.layout().xGrid();
  unsigned int layoutY = digit.layout().yGrid();

  // calculate the relative position of the digit in the TS
  unsigned int xScaleFactor = layoutX / TSLayoutX;
  unsigned int yScaleFactor = layoutY / TSLayoutY;

  unsigned int gridx = digit.nX() - xScaleFactor * TSTile.nX();
  unsigned int gridy = digit.nY() - yScaleFactor * TSTile.nY();

  unsigned int layout = 100;

  // calculate the relative position of the digit in the TS

  for ( unsigned int i = 0; i < TS.numberOfLayout(); i++ ) {
    if ( (unsigned int)TS.gridXLayout( i ) == xScaleFactor && (unsigned int)TS.gridYLayout( i ) == yScaleFactor ) {
      layout = i;
    }
  }
  if ( layout > 2 ) {
    throw std::invalid_argument( "the TS does not contain the digit" );

    /*for ( int i = 0; i < TS.numberOfLayout(); i++ ) {
      if ( (unsigned int)TS.gridXLayout( i ) == xScaleFactor && (unsigned int)TS.gridYLayout( i ) == yScaleFactor ) {
        layout = i;
      }
      }*/
  }

  unsigned int ch_no_hole = 0;
  for ( unsigned int i = 0; i < TS.synchChSize(); i++ ) {

    if ( TS.synchChUsed( i ) ) {

      if ( TS.layoutOutputChannel( ch_no_hole ) == layout ) {
        if ( (unsigned int)TS.gridXOutputChannel( ch_no_hole ) == gridx ) {
          if ( (unsigned int)TS.gridYOutputChannel( ch_no_hole ) == gridy ) { return hole ? i : ch_no_hole; }
        }
      }
      ch_no_hole++;
    }
  }
  dd4hep::printout( dd4hep::ERROR, "DAQHelper", "the digit has not inside the  TS where it should be %d", TSPosInNODE );
  throw std::invalid_argument( "the digit is not contained in TS" );

  return 100000;
}

void LHCb::Detector::Muon::DAQHelper::DAQaddressInODE( LHCb::Detector::Muon::TileID digitTile, unsigned int& ODENumber,
                                                       unsigned int& frame, unsigned int& channelInFrame ) const {

  bool hole = true;
  //  unsigned int station = digitTile.station();
  //  digitTile.Layout()
  dd4hep::printout( dd4hep::DEBUG, "Muon::DAQaddressInODE", "Digit to place %i", (int)digitTile );
  LHCb::Detector::Muon::TileID TSTile = findTS( digitTile );
  dd4hep::printout( dd4hep::DEBUG, "Muon::DAQaddressInODE", "Assigned to TS Tile %i", (int)TSTile );

  if ( !TSTile.isValid() ) {
    dd4hep::printout( dd4hep::DEBUG, "Muon::DAQaddressInODE", "wrong TSTile" );
    throw std::invalid_argument( "the digit is not contained in TS" );
  }
  ODENumber = findODENumber( TSTile );
  dd4hep::printout( dd4hep::DEBUG, "Muon::DAQaddressInODE", "find in NODE %i", ODENumber );
  if ( ODENumber < 1 ) {
    dd4hep::printout( dd4hep::DEBUG, "Muon::DAQaddressInODE", "ODENumber <1 error %i", ODENumber );
    throw std::invalid_argument( "the TS is not contained in NODEs" );
  }

  unsigned int TSNum    = 99;
  unsigned int TSMapNum = 99;
  unsigned int TSSize   = 99;
  findTSNumber( ODENumber, TSTile, TSNum, TSMapNum );
  dd4hep::printout( dd4hep::DEBUG, "Muon::DAQaddressInODE", "%i %i %i", ODENumber, TSNum, TSMapNum );

  unsigned int digitPositionInTS = findDigitInTS( TSNum, TSMapNum, TSTile, digitTile, hole, TSSize );
  dd4hep::printout( dd4hep::DEBUG, "Muon::DAQaddressInODE", "ecco %i", TSSize );

  unsigned int digitInODE          = TSNum * TSSize;
  unsigned int DigitOutputPosition = digitInODE + digitPositionInTS;

  frame          = DigitOutputPosition / DAQHelper_ODEFrameSize;
  channelInFrame = DigitOutputPosition % DAQHelper_ODEFrameSize;
}

// to test that the maps are ok. It means all channels are described. To run only when new yml file are produced not in
// normal run mode
void LHCb::Detector::Muon::DAQHelper::testInitialization() {

  for ( unsigned int s = 0; s < N_OF_STATIONS; s++ ) {
    for ( unsigned int r = 0; r < N_OF_REGIONS; r++ ) {
      for ( unsigned int q = 0; q < 4; q++ ) {
        for ( unsigned int readout = 0; readout < 2; readout++ ) {
          if ( m_layoutX[readout][s * N_OF_REGIONS + r] > 0 ) {

            unsigned int layx = m_layoutX[readout][s * N_OF_REGIONS + r];
            unsigned int layy = m_layoutY[readout][s * N_OF_REGIONS + r];
            for ( unsigned int x = 0; x < 2 * layx; x++ ) {
              for ( unsigned int y = 0; y < 2 * layy; y++ ) {
                if ( x < layx && y < layy ) continue;
                LHCb::Detector::Muon::Layout lay( layx, layy );
                LHCb::Detector::Muon::TileID tile( s, lay, r, q, x, y );
                // now test that the code to find which  Tell40 channel is fire works
                unsigned int ODENumber      = 999;
                unsigned int frame          = 99;
                unsigned int channelInFrame = 99;
                DAQaddressInODE( tile, ODENumber, frame, channelInFrame );
                dd4hep::printout( dd4hep::DEBUG, "Muon::DAQaddressInODE", "%i %i %i %i", tile, ODENumber, frame,
                                  channelInFrame );
              }
            }
          }
        }
      }
    }
  }
}

void LHCb::Detector::Muon::DAQHelper::getSynchConnection( unsigned int NODE, unsigned int frame, long& Tell40Number,
                                                          unsigned int& Tell40PCINumber,
                                                          unsigned int& Tell40PCILinkNumber ) const {

  if ( NODE < 1 || frame > 3 ) {
    Tell40Number        = 0;
    Tell40PCINumber     = 0;
    Tell40PCILinkNumber = 0;
    dd4hep::printout( dd4hep::ERROR, "DAQHelper::getSynchConnection", "invoked with invalid nODE or frame number %i %i",
                      NODE, frame );

  } else {
    Tell40Number        = m_Tell40Number[NODE - 1][frame];
    Tell40PCINumber     = m_Tell40PCINumber[NODE - 1][frame];
    Tell40PCILinkNumber = m_Tell40PCILinkNumber[NODE - 1][frame];
  }
}

void LHCb::Detector::Muon::DAQHelper::getODENumberAndSynch( long Tell40Number, unsigned int Tell40PCINumber,
                                                            unsigned int Tell40PCILinkNumber, int& ODENumber,
                                                            int& SynchNumber ) const {
  ODENumber   = -1;
  SynchNumber = -1;

  if ( Tell40Number > 0 && Tell40Number <= DAQHelper_maxTell40Number ) {
    if ( Tell40PCINumber == 0 || Tell40PCINumber == 1 ) {
      if ( Tell40PCILinkNumber < DAQHelper_linkNumber ) {
        ODENumber   = m_ODENumberNoHole[Tell40Number - 1][Tell40PCINumber][Tell40PCILinkNumber];
        SynchNumber = m_ODEFrameNumberNoHole[Tell40Number - 1][Tell40PCINumber][Tell40PCILinkNumber];
      }
    }
  }
}

void LHCb::Detector::Muon::DAQHelper::initSourceID() {
  std::string name_to_print = "SourceID initialization";
  for ( auto& pci : v_pci ) {
    dd4hep::printout( dd4hep::DEBUG, name_to_print, "reading Tell40 # %d", pci.Tell40Number() );

    unsigned long TNumber = pci.Tell40Number();
    unsigned int  PNumber = pci.Tell40PCINumber();
    // unsigned int  active_link_per_PCI = 0;
    for ( unsigned int iLink = 0; iLink < DAQHelper_linkNumber; iLink++ ) {
      unsigned int node = pci.getODENumber( iLink );
      dd4hep::printout( dd4hep::DEBUG, name_to_print, "examining link connected to NODE # %d", node );
      if ( node > 0 ) {

        LHCb::Detector::Muon::detail::NODEBoard nodeboard( v_node[node - 1] );

        long quadrant = nodeboard.quadrant();
        long sourceID = ( TNumber - 1 ) * 2 + PNumber;
        if ( quadrant == 0 || quadrant == 1 ) {
          sourceID = sourceID + 0x6800;
          if ( sourceID > 0x6FFF ) {
            dd4hep::printout( dd4hep::ERROR, "DAQHelper::initSourceID", "the source ID is outside the allowed range %d",
                              sourceID );
            throw std::invalid_argument( "the sourceID value is too large" );
          }
        } else {
          sourceID = sourceID + 0x7000;
          if ( sourceID > 0x77FF ) {
            dd4hep::printout( dd4hep::ERROR, "DAQHelper::initSourceID", "the source ID is outside the allowed range %d",
                              sourceID );
            throw std::invalid_argument( "the sourceID value is too large" );
          }
        }
        pci.setSourceID( sourceID );
        m_sourceID[TNumber - 1][PNumber] = sourceID;

        break;
      }
    }
  }
}
