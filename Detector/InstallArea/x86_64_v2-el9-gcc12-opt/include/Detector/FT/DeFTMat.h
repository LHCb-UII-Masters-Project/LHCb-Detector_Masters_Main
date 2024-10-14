/*****************************************************************************\
* (c) Copyright 2000-2023 CERN for the benefit of the LHCb Collaboration      *
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
#include "Core/LineTraj.h"
#include "Core/Units.h"
#include "Detector/FT/FTChannelID.h"
#include "Detector/FT/FTConstants.h"

#include <Core/Keys.h>
#include <yaml-cpp/yaml.h>

#include <array>
#include <fmt/format.h>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    /**
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeFTMatObject : DeIOVObject {

      FTChannelID  m_elementID; ///< element ID
      unsigned int m_matID;     // FIXME: needed?
      //      dd4hep::_toDouble
      int m_nChannelsInSiPM = {dd4hep::_toInt( "FT:nChannelsInSiPM" )}; ///< number of channels per SiPM // FIXME: it
                                                                        ///< is constant
      int m_nChannelsInDie;                                             ///< number of channels per die
      int m_nSiPMsInMat = FT::nSiPM; ///< number of SiPM arrays per mat // FIXME: it is constant

      // https://gitlab.cern.ch/lhcb/Detector/-/commit/10912b05b6a9c96d1f4c0a8cdeaf9445dc5fa43a
      int m_nDiesInSiPM = 2; ///< number of dies per SiPM // WHY was this set to nChannels?!?!

      ROOT::Math::Plane3D   m_plane;     ///< xy-plane in the z-middle of the module
      ROOT::Math::XYZPointF m_sipmPoint; ///< Location of end of fibres at x=z=0
      float                 m_globalZ;   ///< Global z position of module closest to y-axis
      float                 m_airGap = detail::toLHCbLengthUnits( dd4hep::_toFloat( "FT:airGap" ) ); ///< air gap
      float m_deadRegion = detail::toLHCbLengthUnits( dd4hep::_toFloat( "FT:deadRegion" ) );         ///< dead region
      float m_channelPitch =
          detail::toLHCbLengthUnits( dd4hep::_toFloat( "FT:channelPitch" ) ); ///< readout channel pitch (250 micron)
      float m_diePitch;                                                       ///< pitch between dies in SiPM
      float m_sizeX = detail::toLHCbLengthUnits( dd4hep::_toFloat( "FT:MatSizeX" ) ); ///< Width in x of the mat
      float m_sizeY =
          detail::toLHCbLengthUnits( dd4hep::_toFloat( "FT:MatSizeY" ) ); ///< Length in y of the fibre in the mat
      float m_sizeZ = detail::toLHCbLengthUnits( dd4hep::_toFloat( "FT:MatSizeZ" ) ); ///< Thickness of the fibre mat
                                                                                      ///< (nominal: 1.3 mm)

      // Parameters needed for decoding
      ROOT::Math::XYZPointF  m_mirrorPoint;      ///< Location of end of fibres at x=z=0
      ROOT::Math::XYZVectorF m_ddx;              ///< Global direction vector for a local displacement in unit x
      float                  m_uBegin;           ///< start in local u-coordinate of sensitive SiPM
      float                  m_halfChannelPitch; ///< half of the readout channel pitch (125 micron)
      float m_dieGap = detail::toLHCbLengthUnits( dd4hep::_toFloat( "FT:dieGap" ) ); ///< gap between channel 63 and 64
      float m_sipmPitch;                                                             ///< pitch between SiPMs in mat
      float m_dxdy;     ///< Global slope dx/dy for a fibre mat
      float m_dzdy;     ///< Global slope dz/dy for a fibre mat
      float m_globaldy; ///< Length of a fibre projected along global y

      // Parameters needed for mat contraction calibration
      dd4hep::Condition   m_matContractionCondition;
      std::vector<double> m_matContractionParameterVector;

      // Get mat name as string
      std::string getMatName() const {
        auto                         channelID = m_elementID;
        static constexpr const char* layers[4] = {"X1", "U", "V", "X2"};
        return fmt::format( "T{station}{layer}Q{quarter}M{module}Mat{mat}",
                            fmt::arg( "station", to_unsigned( channelID.station() ) ),
                            fmt::arg( "layer", layers[to_unsigned( channelID.layer() )] ),
                            fmt::arg( "quarter", to_unsigned( channelID.quarter() ) ),
                            fmt::arg( "module", to_unsigned( channelID.module() ) ),
                            fmt::arg( "mat", to_unsigned( channelID.mat() ) ) );
      }

      // Get contraction condition name as string
      [[nodiscard]] std::string getContractionConditionName( /*FTChannelID channelID*/ ) const {
        return "matContraction" + getMatName();
      }

      DeFTMatObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt, unsigned int iMat,
                     unsigned int iModule );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {
        // no children
      }
    };
  } // End namespace detail
  struct DeFTMat : DeIOVElement<detail::DeFTMatObject> {
    using DeIOVElement::DeIOVElement;

    // Getters
    FTChannelID elementID() const { return this->access()->m_elementID; }
    float       airGap() const { return this->access()->m_airGap; }
    float       deadRegion() const { return this->access()->m_deadRegion; }
    float       channelPitch() const { return this->access()->m_channelPitch; }
    int         sensitiveVolumeID( const ROOT::Math::XYZPoint& ) const override { return this->access()->m_elementID; }
    float       sipmPitch() const { return this->access()->m_sipmPitch; };
    float       halfChannelPitch() const { return this->access()->m_halfChannelPitch; };
    float       dieGap() const { return this->access()->m_dieGap; }
    ROOT::Math::XYZPointF  mirrorPoint() const { return this->access()->m_mirrorPoint; }
    ROOT::Math::XYZVectorF ddx() const { return this->access()->m_ddx; }
    float                  dxdy() const { return this->access()->m_dxdy; }
    float                  dzdy() const { return this->access()->m_dzdy; }
    float                  globaldy() const { return this->access()->m_globaldy; }
    float                  globalZ() const { return this->access()->m_globalZ; }
    float                  uBegin() const { return this->access()->m_uBegin; }
    /** Returns the width of the fibre mat */
    [[nodiscard]] float fibreMatWidth() const { return this->access()->m_sizeX; }

    /** Get the length of the fibre in this mat */
    [[nodiscard]] float fibreLength() const { return this->access()->m_sizeY; }

    /** Returns the thickness of the fibre mat (1.3 mm) */
    [[nodiscard]] float fibreMatThickness() const { return this->access()->m_sizeZ; }

    // Element ID operations
    FTChannelID::StationID stationID() const { return this->elementID().station(); }
    FTChannelID::LayerID   layerID() const { return this->elementID().layer(); }
    FTChannelID::QuarterID quarterID() const { return this->elementID().quarter(); }
    FTChannelID::ModuleID  moduleID() const { return this->elementID().module(); }
    FTChannelID::MatID     matID() const { return FTChannelID::MatID{this->access()->m_matID}; }
    bool                   isBottom() const { return this->access()->m_elementID.isBottom(); }
    bool                   isTop() const { return this->access()->m_elementID.isTop(); }
    bool                   hasGapLeft( const FTChannelID thisChannel ) const {
      return ( thisChannel.channel() == 0u || int( thisChannel.channel() ) == this->access()->m_nChannelsInDie );
    }
    bool hasGapRight( const FTChannelID thisChannel ) const {
      return ( int( thisChannel.channel() ) == this->access()->m_nChannelsInSiPM - 1 ||
               int( thisChannel.channel() ) == this->access()->m_nChannelsInDie - 1 );
    }

    // Geometrical operations
    LineTraj<double> trajectory( FTChannelID channelID, float frac ) const {
      float                localX = localXfromChannel( channelID, frac );
      auto                 obj    = this->access();
      ROOT::Math::XYZPoint mirrorPoint( obj->m_mirrorPoint.x() + localX * obj->m_ddx.x(),
                                        obj->m_mirrorPoint.y() + localX * obj->m_ddx.y(),
                                        obj->m_mirrorPoint.z() + localX * obj->m_ddx.z() );
      ROOT::Math::XYZPoint sipmPoint( obj->m_sipmPoint.x() + localX * obj->m_ddx.x(),
                                      obj->m_sipmPoint.y() + localX * obj->m_ddx.y(),
                                      obj->m_sipmPoint.z() + localX * obj->m_ddx.z() );
      return {mirrorPoint, sipmPoint};
    }

    // Others
    std::pair<FTChannelID, float> calculateChannelAndFrac( float localX ) const {
      const auto& obj = this->access();
      // Correct for the starting point of the sensitive area
      float xInMat = localX - obj->m_uBegin;

      // Find the sipm that is hit and the local position within the sipm
      int   hitSiPM = std::clamp( int( xInMat / obj->m_sipmPitch ), 0, obj->m_nSiPMsInMat - 1 );
      float xInSiPM = fma( -obj->m_sipmPitch, hitSiPM, xInMat );

      // Find the die that is hit and the local position within the die
      int   hitDie    = std::clamp( int( xInSiPM / obj->m_diePitch ), 0, obj->m_nDiesInSiPM - 1 );
      float chanInDie = fma( -obj->m_diePitch, hitDie, xInSiPM ) / obj->m_channelPitch;

      // Find the channel that is hit and the local position within the channel
      int   hitChan = std::clamp( int( chanInDie ), 0, obj->m_nChannelsInDie - 1 );
      float frac    = chanInDie - hitChan - 0.5f;

      // Construct channelID
      return std::make_pair( FTChannelID( stationID(), layerID(), quarterID(), moduleID(), matID(), hitSiPM,
                                          hitChan + ( hitDie * obj->m_nChannelsInDie ) ),
                             frac );
    }
    [[nodiscard]] std::vector<std::pair<FTChannelID, float>> calculateChannels( FTChannelID thisChannel,
                                                                                FTChannelID endChannel ) const {
      // Reserve memory
      std::vector<std::pair<FTChannelID, float>> channelsAndLeftEdges;
      channelsAndLeftEdges.reserve( endChannel - thisChannel );

      // Loop over the intermediate channels
      bool keepAdding = true;
      while ( keepAdding ) {
        float channelLeftEdge = localXfromChannel( thisChannel, -0.5f );
        // Add channel and left edge to output vector.
        channelsAndLeftEdges.emplace_back( thisChannel, channelLeftEdge );
        if ( thisChannel == endChannel ) keepAdding = false;
        thisChannel.advance();
      }
      return channelsAndLeftEdges;
    }

    /** Get the list of SiPM channels traversed by the hit.
     *  The particle trajectory is a straight line defined by:
     *  @param provide local entry and exit point
     *  @param provide the number of additional channels to add
     *  Fills a vector of FTChannelIDs, and a vector of the
     *  corresponding left edges (along x) in the local frame.
     */
    [[nodiscard]] std::vector<std::pair<FTChannelID, float>>
    calculateChannels( const float localEntry, const float localExit,
                       const unsigned int numOfAdditionalChannels ) const {
      // set ordering in increasing local x
      float xBegin = std::min( localEntry, localExit );
      float xEnd   = std::max( localEntry, localExit );

      // Find the first and last channels that are involved
      float xOffset                       = numOfAdditionalChannels * this->access()->m_channelPitch;
      const auto [thisChannel, fracBegin] = calculateChannelAndFrac( xBegin - xOffset );
      const auto [endChannel, fracEnd]    = calculateChannelAndFrac( xEnd + xOffset );

      // return empty vector when both channels are the same gap
      if ( thisChannel.channelID() == endChannel.channelID() && std::abs( fracBegin ) > 0.5f &&
           std::abs( fracEnd ) > 0.5f && fracBegin * fracEnd > 0.25f )
        return std::vector<std::pair<FTChannelID, float>>();

      return this->calculateChannels( thisChannel, endChannel );
    }

    /** Get the local x from a channelID and its fraction */
    [[nodiscard]] float localXfromChannel( const FTChannelID channelID, const float frac ) const {
      const auto& obj = this->access();

      float uFromChannel = obj->m_uBegin + ( float( channelID.channel() ) + 0.5f + frac ) * obj->m_channelPitch;
      if ( int( channelID.channel() ) >= obj->m_nChannelsInDie ) uFromChannel += obj->m_dieGap;
      uFromChannel += channelID.sipm() * obj->m_sipmPitch;
      return uFromChannel;
    }

    // Get the distance between a 3D global point and a channel+fraction
    float distancePointToChannel( const ROOT::Math::XYZPoint& globalPoint, const FTChannelID channelID,
                                  const float frac ) const {
      ROOT::Math::XYZPoint localPoint = this->toLocal( globalPoint );
      return localXfromChannel( channelID, frac ) - localPoint.x();
    }

    /** Get the distance from the hit to the SiPM
     *  @param localPoint is the position of the half module in local coordinates
     *  @return the distance to the SiPM
     */
    [[nodiscard]] float distanceToSiPM( const ROOT::Math::XYZPoint& localPoint ) const {
      return 0.5f * this->access()->m_sizeY - localPoint.y();
    };

    /** Set the coldbox alignment conditions **/
    void setmatContractionParameterVector( const std::vector<double>& paramValue ) {
      this->access()->m_matContractionParameterVector = paramValue;
    }

    [[nodiscard]] const std::vector<double>& getmatContractionParameterVector() const {
      return this->access()->m_matContractionParameterVector;
    }

    ROOT::Math::Plane3D plane() const { return this->access()->m_plane; }
  };
} // End namespace LHCb::Detector
