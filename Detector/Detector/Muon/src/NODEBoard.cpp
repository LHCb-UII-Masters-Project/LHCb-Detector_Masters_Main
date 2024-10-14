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

#include "Detector/Muon/NODEBoard.h"
//#include "Detector/Muon/DeMuon.h"
#include "Core/yaml_converters.h"

#include "DD4hep/Printout.h"

#include <iostream>
#include <vector>

using namespace LHCb::Detector::Muon;

LHCb::Detector::Muon::detail::NODEBoard::NODEBoard( const YAML::Node& muon_condition, unsigned int odenum ) {
  // add printout
  std::string name_to_print = "Muon Cond Loader inside NODEBoard";
  dd4hep::printout( dd4hep::DEBUG, name_to_print, "ODE num: %d", odenum );

  auto NODEBoards_list = ( muon_condition )["NODEBoard"];
  auto NODEBoard       = NODEBoards_list[odenum];
  m_ECSName            = NODEBoard["ODEECSName"].as<std::string>();
  m_NODENumber         = NODEBoard["ODESerialNumber"].as<unsigned int>();
  m_region             = NODEBoard["RegionNumber"].as<unsigned int>();
  m_TSNumber           = NODEBoard["TSNumber"].as<unsigned int>();
  // now reserve the vector
  m_TSGridX.reserve( m_TSNumber );
  m_TSGridY.reserve( m_TSNumber );
  m_TSQuadrant.reserve( m_TSNumber );
  m_TSName.reserve( m_TSNumber );
  m_TSMapNumber.reserve( m_TSNumber );

  m_TSGridX     = NODEBoard["TSGridXList"].as<std::vector<unsigned int>>();
  m_TSGridY     = NODEBoard["TSGridYList"].as<std::vector<unsigned int>>();
  m_TSQuadrant  = NODEBoard["TSGridQList"].as<std::vector<unsigned int>>();
  m_TSName      = NODEBoard["TSNameList"].as<std::vector<std::string>>();
  m_TSMapNumber = NODEBoard["TSMapNumberList"].as<std::vector<unsigned int>>();

  for ( unsigned int i = 0; i < m_TSNumber; i++ ) {
    unsigned int a = m_TSQuadrant[i];
    if ( i == 0 ) m_unique_quadrant = a;
    m_quadrant[a] = 1;
  }
}

LHCb::Detector::Muon::detail::NODEBoard::NODEBoard( const LHCb::Detector::Muon::detail::NODEBoard& old ) {

  m_region          = old.m_region;
  m_NODENumber      = old.m_NODENumber;
  m_TSLayoutX       = old.m_TSLayoutX;
  m_TSLayoutY       = old.m_TSLayoutY;
  m_TSNumber        = old.m_TSNumber;
  m_TSGridX         = old.m_TSGridX;
  m_TSGridY         = old.m_TSGridY;
  m_TSQuadrant      = old.m_TSQuadrant;
  m_TSName          = old.m_TSName;
  m_TSMapNumber     = old.m_TSMapNumber;
  m_ECSName         = old.m_ECSName;
  m_unique_quadrant = old.m_unique_quadrant;
  m_quadrant        = old.m_quadrant;
}

bool LHCb::Detector::Muon::detail::NODEBoard::isQuadrantContained( unsigned int quadrant ) const {
  if ( m_quadrant[quadrant] > 0 ) {
    return true;
  } else {
    return false;
  }
}

bool LHCb::Detector::Muon::detail::NODEBoard::isTSContained( TileID TSTile ) const {
  bool debug    = false;
  long quadrant = (long)TSTile.quarter();
  long gridx    = (long)TSTile.nX();
  long gridy    = (long)TSTile.nY();
  if ( debug ) {
    std::cout << " entro" << std::endl;
    std::cout << m_TSNumber << std::endl;
    std::cout << TSTile << std::endl;
    std::cout << getNODESerialNumber() << " " << getTSNumber() << std::endl;
  }
  for ( unsigned int i = 0; i < m_TSNumber; i++ ) {
    //  std::cout<<" ts in ode "<<i<<" "<<
    //  m_TSQuadrant[i]<< " "<<
    //  m_TSGridX[i]<<" "<<
    //   m_TSGridY[i]<<std::endl;
    if ( m_TSQuadrant[i] == quadrant ) {
      if ( m_TSGridX[i] == gridx ) {
        if ( m_TSGridY[i] == gridy ) { return true; }
      }
    }
  }
  return false;
}
