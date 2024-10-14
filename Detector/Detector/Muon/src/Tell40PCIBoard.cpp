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

#include "Detector/Muon/Tell40PCIBoard.h"
//#include "Detector/Muon/DeMuon.h"
#include "Core/yaml_converters.h"

#include "DD4hep/Printout.h"

#include <iostream>
#include <vector>

// using namespace LHCb::Muon;

LHCb::Detector::Muon::detail::Tell40PCIBoard::Tell40PCIBoard( const YAML::Node& muon_condition, unsigned int station,
                                                              unsigned int TellNum, unsigned PCINum ) {
  // add printout
  std::string name_to_print = "Muon Cond Loader inside Tell40PCIBoard";
  dd4hep::printout( dd4hep::DEBUG, name_to_print, "Tell num: %d PCI num: %d", TellNum, PCINum );

  m_station              = station;
  auto Tell40Boards_list = ( muon_condition )["TELL40Board"];
  auto Tell40Board       = Tell40Boards_list[TellNum];
  m_Tell40Number         = Tell40Board["Tell40Number"].as<unsigned int>();

  auto Tell40PCIBoards_list = Tell40Board["PCI"];
  auto Tell40PCIBoard       = Tell40PCIBoards_list[PCINum];
  m_PCINumber               = Tell40PCIBoard["PCINumber"].as<unsigned int>();

  m_ODENumberList      = Tell40PCIBoard["ODEList"].as<std::array<unsigned int, 24>>();
  m_linkConnection     = Tell40PCIBoard["ODELink"].as<std::array<unsigned int, 24>>();
  m_numberOfActiveLink = Tell40PCIBoard["ActiveLinkNumber"].as<unsigned int>();

  for ( unsigned int i = 0; i < DAQHelper_linkNumber; i++ ) {
    if ( m_ODENumberList[i] > 0 ) { m_declaredLink = m_declaredLink | ( ( 0x1 ) << i ); }
  }
}
