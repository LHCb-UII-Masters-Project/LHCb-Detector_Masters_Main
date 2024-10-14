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

#include "Detector/Muon/TSMap.h"
//#include "Detector/Muon/DeMuon.h"
#include "Core/yaml_converters.h"

#include "DD4hep/Printout.h"

#include <iostream>
#include <vector>

LHCb::Detector::Muon::detail::TSMap::TSMap( const YAML::Node& muon_condition, unsigned int reg, unsigned int map ) {
  // add printout
  std::string name_to_print = "Muon Cond Loader inside TSMap";
  dd4hep::printout( dd4hep::DEBUG, name_to_print, "TSMap reg and map: %d %d", reg, map );

  auto TSmaps       = ( muon_condition )["TSMap"];
  auto TS_in_region = TSmaps[reg]["TSMapInRegion"];
  auto TS_map       = TS_in_region[map];
  // now fill the relevant value
  m_NumberLogLayout   = TS_map["NumberLogLayout"].as<unsigned int>();
  auto         TSGrid = TS_map["GridList"].as<std::vector<std::vector<unsigned int>>>();
  unsigned int ii     = 0;
  for ( auto tt : TSGrid ) {
    m_GridXLayout[ii] = tt[0];
    m_GridYLayout[ii] = tt[1];

    ii++;
    //      m_GridXLayout[ii]=s;

    // std::cout<<s<<std::endl;}
  }
  // std::cout<<"ciao "<<m_GridXLayout[0]<<" "<<m_GridYLayout[0]<<std::endl;
  // if(m_NumberLogLayout>1)std::cout<<m_GridXLayout[1]<<" "<<m_GridYLayout[1]<<std::endl;

  m_OutputSignal                   = TS_map["ActiveChannels"].as<unsigned int>();
  std::vector<unsigned int> data_1 = TS_map["OutputLayoutSeq"].as<std::vector<unsigned int>>();

  std::copy( data_1.begin(), data_1.end(), m_OutputLayoutSequence.begin() );

  std::vector<unsigned int> data_2 = TS_map["OutputGridXSeq"].as<std::vector<unsigned int>>();
  std::copy( data_2.begin(), data_2.end(), m_OutputGridXSequence.begin() );
  std::vector<unsigned int> data_3 = TS_map["OutputGridYSeq"].as<std::vector<unsigned int>>();
  std::copy( data_3.begin(), data_3.end(), m_OutputGridYSequence.begin() );
  std::vector<unsigned int> data_4 = TS_map["OutputSequence"].as<std::vector<unsigned int>>();
  std::copy( data_4.begin(), data_4.end(), m_OutputSynchSequence.begin() );

  m_SynchOutputSignal = data_4.size();
}
LHCb::Detector::Muon::detail::TSMap::TSMap( const LHCb::Detector::Muon::detail::TSMap& old ) {

  m_NumberLogLayout = old.m_NumberLogLayout;
  m_GridXLayout[0]  = old.m_GridXLayout[0];
  m_GridXLayout[1]  = old.m_GridXLayout[1];
  m_GridYLayout[0]  = old.m_GridYLayout[0];
  m_GridYLayout[1]  = old.m_GridYLayout[1];

  m_OutputSignal         = old.m_OutputSignal;
  m_SynchOutputSignal    = old.m_SynchOutputSignal;
  m_OutputLayoutSequence = old.m_OutputLayoutSequence;
  m_OutputGridXSequence  = old.m_OutputGridXSequence;
  m_OutputGridYSequence  = old.m_OutputGridYSequence;
  m_OutputSynchSequence  = old.m_OutputSynchSequence;
}
