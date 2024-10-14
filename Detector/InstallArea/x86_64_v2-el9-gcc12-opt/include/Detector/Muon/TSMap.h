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

#include "Core/DeIOV.h"

#include <vector>
#include <yaml-cpp/yaml.h>

namespace LHCb::Detector::Muon {

  namespace detail {

    struct TSMap {
      TSMap( const YAML::Node& muon_condition, unsigned int reg, unsigned int map );
      TSMap( const LHCb::Detector::Muon::detail::TSMap& old );

      // TSMap(){};

      unsigned int m_NumberLogLayout;
      unsigned int m_GridXLayout[2];
      unsigned int m_GridYLayout[2];
      unsigned int m_OutputSignal;
      unsigned int m_SynchOutputSignal;

      std::array<unsigned int, 96> m_OutputLayoutSequence;
      std::array<unsigned int, 96> m_OutputGridXSequence;
      std::array<unsigned int, 96> m_OutputGridYSequence;
      std::array<unsigned int, 96> m_OutputSynchSequence;

      inline unsigned int numberOfLayout() { return m_NumberLogLayout; }
      inline unsigned int gridXLayout( unsigned int i ) { return m_GridXLayout[i]; }
      inline unsigned int gridYLayout( unsigned int i ) { return m_GridYLayout[i]; }

      inline unsigned int numberOfOutputSignal() { return m_OutputSignal; }
      inline unsigned int layoutOutputChannel( unsigned int i ) { return m_OutputLayoutSequence[i]; }
      inline unsigned int gridXOutputChannel( unsigned int i ) { return m_OutputGridXSequence[i]; }
      inline unsigned int gridYOutputChannel( unsigned int i ) { return m_OutputGridYSequence[i]; }
      inline unsigned int synchChSize() { return m_SynchOutputSignal; }
      inline bool         synchChUsed( unsigned int i ) { return m_OutputSynchSequence[i] == 0 ? false : true; }
    };

  } // namespace detail
} // namespace LHCb::Detector::Muon
