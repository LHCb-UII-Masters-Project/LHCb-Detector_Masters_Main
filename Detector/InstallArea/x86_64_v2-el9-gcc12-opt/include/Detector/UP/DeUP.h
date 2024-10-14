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

#include "Core/LineTraj.h"
#include "Detector/UP/DeUPSide.h"
#include "Detector/UP/UPChannelID.h"

#include "Core/DeIOV.h"

#include "fmt/format.h"

#include <array>
#include <vector>

namespace LHCb::Detector {

  namespace detail {

    struct DeUPObject : LHCb::Detector::detail::DeIOVObject {
      DeUPObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );
      std::array<DeUPSideObject, 2> sides; // A side at index 0, C side at index 1;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& side : sides ) { func( &side ); };
      }
    };
  } // End namespace detail

  inline std::string const  Location = "/world/BeforeMagnetRegion/UP:DetElement-Info-IOV";
  inline std::string const& location() { return Location; }

  /**
   *  This object defines the behaviour of the objects's data
   *
   *  \author  Markus Frank
   *  \date    2018-03-08
   *  \version  1.0
   */
  template <typename ObjectType>
  struct DeUPElement : DeIOVElement<ObjectType> {
    using DeIOVElement<ObjectType>::DeIOVElement;

    using Sectors = std::vector<const DeUPSector*>;
    using Layers  = std::vector<const DeUPLayer*>;
    using Modules = std::vector<const DeUPModule*>;
    using Sides   = std::vector<const DeUPSide*>;
    struct LayerGeom {
      float z{};
      int   nColsPerSide{};
      int   nRowsPerSide{};
      float invHalfSectorYSize{};
      float invHalfSectorXSize{};
      float dxDy{};
    };
    enum class LayerID_t : unsigned int {};
    enum class Side_t : unsigned int {};
    enum class Layer_t : unsigned int {};
    enum class Stave_t : unsigned int {};
    enum class Face_t : unsigned int {};
    enum class Module_t : unsigned int {};
    enum class Sector_t : unsigned int {};
    enum class GeoVersion { ErrorVersion = 0, v0, v1 };
    /** Implementation of sensitive volume identifier for a given point in the
        global reference frame. This is the sensor number defined in the xml.
    */
    int sensitiveVolumeID( const ROOT::Math::XYZPoint& ) const { throw NotImplemented(); }

    /// locate layer based on a point
    const std::optional<DeUPLayer> findLayer( const ROOT::Math::XYZPoint& ) const { throw NotImplemented(); }

    /// check contains channel
    bool contains( ChannelID ) const { throw NotImplemented(); }

    /// Workaround to prevent hidden base class function
    bool isValid() const { throw NotImplemented(); }

    /// check channel number is valid
    bool isValid( ChannelID ) const { throw NotImplemented(); }

    /// flat vector of sectors
    const DeUPSector& sector( unsigned int ) const { throw NotImplemented(); }

    const Sectors   sectors( LayerID_t ) const { throw( "Not implemented" ); }
    const Sectors   sectors() const { throw( "Not implemented" ); }
    const LayerGeom getLayerGeom( unsigned int ) const { throw( "Not implemented" ); }

    bool SectorsSwapped() const { return true; }

    double zStation( unsigned int ) const { throw( "Not implemented" ); }

    auto version() const { return GeoVersion::v1; }

    /// @return number of sectors
    unsigned int nSectors() const { throw NotImplemented(); }

    /// apply given callable to all sectors
    void applyToAllSectors( const std::function<void( DeUPSector const& )>& ) const { throw NotImplemented(); }

    /// apply given callable to all sectors
    void applyToAllSectorsAllen( const std::function<void( DeUPSector const& )>& ) const { throw NotImplemented(); }

    /// check no sector returns true for the given callable
    bool none_of_sectors( const std::function<bool( DeUPSector const& )>& ) const { throw NotImplemented(); }

    /**
     * access to a given layer
     * Note that there is no checks on the validity of the index.
     * use at your own risks
     */
    const DeUPLayer& layer( unsigned int ) const { throw NotImplemented(); }

    /// apply given callable to all layers
    void applyToAllLayers( const std::function<void( DeUPLayer const& )>& ) const { throw NotImplemented(); }

    /// short cut to pick up the wafer corresponding to a channel
    const std::optional<DeUPSector> findSector( ChannelID ) const { throw NotImplemented(); }

    const std::optional<DeUPSector> findSector( const ROOT::Math::XYZPoint& ) const { throw NotImplemented(); }

    /// get the sector corresponding to the input channel
    const DeUPSector& getSector( ChannelID ) const { throw NotImplemented(); }

    /// get the next channel left
    ChannelID nextLeft( ChannelID ) const { throw NotImplemented(); }

    /// get the next channel right
    ChannelID nextRight( ChannelID ) const { throw NotImplemented(); }

    /// get the trajectory
    // LHCb::LineTraj<double> trajectory( LHCb::LHCbID id, double offset ) const { return {}; }

    /// get the number of pixels in detector
    unsigned int nPixel() const { throw NotImplemented(); }

    /// get the number of layers
    unsigned int nLayer() const { throw NotImplemented(); }

    /// get the number of readout sectors
    unsigned int nReadoutSector() const { throw NotImplemented(); }

    /// number of layers per station
    unsigned int nLayersPerStation() const { throw NotImplemented(); }

    /// fraction active channels
    double fractionActive() const { throw NotImplemented(); };

    void setOffset(){};

    /// get list of all disabled sectors
    std::vector<DeUPSector> disabledSectors() const { throw NotImplemented(); }

    /// get list of disabled beetles
    std::vector<ChannelID> disabledBeetles() const { throw NotImplemented(); }

    /// beetle as a string
    std::string uniqueBeetle( const ChannelID& chan ) const {
      auto const& sector = findSector( chan );
      return fmt::format( "{}Beetle{}", sector->nickname(), sector->beetle( chan ) );
    };

    /// port
    std::string uniquePort( const ChannelID& ) const { throw NotImplemented(); };

    inline unsigned int firstStation() const { throw NotImplemented(); }
    void applyToSectors( const std::vector<ChannelID>&, const std::function<void( DeUPSector const& )>& ) const {
      throw NotImplemented();
    }

    std::optional<DeUPLayer> findLayer( ChannelID ) const { throw NotImplemented(); }
    std::optional<DeUPSide>  findStation( ChannelID ) const { throw NotImplemented(); }

    const DeUPSide& station( unsigned int ) const { throw NotImplemented(); }
    const DeUPSide& side( unsigned int ) const { throw NotImplemented(); }
    unsigned int    nStation() const { throw NotImplemented(); }
    bool            layerSizeOK() const { throw NotImplemented(); }
  };

  using DeUP = DeUPElement<detail::DeUPObject>;

} // namespace LHCb::Detector
