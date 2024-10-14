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

#include "Detector/Muon/TileID.h"

#include "Core/DeIOV.h"
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

// using LHCb::Muon;

namespace LHCb::Detector::Muon {

  namespace detail {
    // using LHCb::Muon;
    struct NODEBoard {
      NODEBoard( const YAML::Node& muon_condition, unsigned int odenum );
      NODEBoard( const NODEBoard& old );

      unsigned int                m_region;
      unsigned int                m_NODENumber;
      unsigned int                m_TSLayoutX;
      unsigned int                m_TSLayoutY;
      unsigned int                m_TSNumber;
      std::vector<unsigned int>   m_TSGridX;
      std::vector<unsigned int>   m_TSGridY;
      std::vector<unsigned int>   m_TSQuadrant;
      std::vector<std::string>    m_TSName;
      std::vector<unsigned int>   m_TSMapNumber;
      std::string                 m_ECSName;
      unsigned int                m_unique_quadrant;
      std::array<unsigned int, 4> m_quadrant;

      unsigned int getNODESerialNumber() const { return m_NODENumber; }
      // unsigned int        getTSLayoutX() { return m_TSLayoutX; }
      // unsigned int        getTSLayoutY() { return m_TSLayoutY; }
      unsigned int getTSNumber() const { return m_TSNumber; }
      unsigned int getTSGridX( unsigned int i ) const { return m_TSGridX[i]; }
      unsigned int getTSGridY( unsigned int i ) const { return m_TSGridY[i]; }
      unsigned int getTSQuadrant( unsigned int i ) const { return m_TSQuadrant[i]; }
      std::string  getTSName( unsigned int i ) const { return m_TSName[i]; }
      unsigned int getTSMapNumber( unsigned int i ) const { return m_TSMapNumber[i]; }
      unsigned int region() const { return m_region; };
      unsigned int quadrant() const { return m_unique_quadrant; };
      bool         isQuadrantContained( unsigned int quadrant ) const;
      bool         isTSContained( TileID TSTile ) const;

      inline std::string ECSName() const { return m_ECSName; }
    };
  } // namespace detail
} // End namespace LHCb::Detector::Muon
