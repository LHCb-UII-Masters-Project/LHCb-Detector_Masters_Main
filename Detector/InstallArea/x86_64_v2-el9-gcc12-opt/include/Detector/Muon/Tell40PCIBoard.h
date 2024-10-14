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
#include "DAQConstants.h"
#include "Detector/Muon/TileID.h"
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace LHCb::Detector::Muon {
  namespace detail {
    struct Tell40PCIBoard {
      Tell40PCIBoard( const YAML::Node& muon_condition, unsigned int station, unsigned int TellNum, unsigned PCInum );

      long Tell40Number() const { return m_Tell40Number; };

      long Tell40PCINumber() const { return m_PCINumber; };

      int getStation() const { return m_station; };
      int activeLinkNumber() const { return m_numberOfActiveLink; };

      std::string  getODEName( int i ) const { return m_ODEName[i]; };
      unsigned int getODENumber( int i ) const { return m_ODENumberList[i]; };
      unsigned int getLinkNumber( int i ) const { return m_linkConnection[i]; };

      // void addLayout( long region, long X, long Y );
      long getTSLayoutX( long i ) { return m_regionLayout[i][0]; };
      long getTSLayoutY( long i ) { return m_regionLayout[i][1]; };
      // long        getODEPosition( long ode, long link, bool hole = true );
      long getODEAndLinkPosition( long ode, long link, bool hole = true );
      // void addLinkAndODE( long index, long ode, long link );
      void         dumpPCI();
      void         setSourceID( long number ) { m_sourceID = number; };
      long         getSourceID() const { return m_sourceID; };
      unsigned int connectedLinks() const { return ( m_declaredLink & 0xFFFFFF ); };
      unsigned int m_station      = 0;
      unsigned int m_PCINumber    = 0;
      unsigned int m_Tell40Number = 0;

      std::string                                    m_ODEName[DAQHelper_linkNumber];
      unsigned int                                   m_regionLayout[4][2];
      std::array<unsigned int, DAQHelper_linkNumber> m_ODENumberList      = {};
      std::array<unsigned int, DAQHelper_linkNumber> m_linkConnection     = {};
      unsigned int                                   m_numberOfActiveLink = 0;
      long                                           m_sourceID           = 0;
      unsigned int                                   m_declaredLink       = 0;
    };
  } // namespace detail
} // End namespace LHCb::Detector::Muon
