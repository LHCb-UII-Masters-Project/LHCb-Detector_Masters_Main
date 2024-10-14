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
#include "Detector/UT/ChannelID.h"
#include "Detector/UT/DeUTFace.h"
#include "Detector/UT/DeUTLayer.h"
#include "Detector/UT/DeUTModule.h"
#include "Detector/UT/DeUTSector.h"
#include "Detector/UT/DeUTSensor.h"
#include "Detector/UT/DeUTSide.h"
#include "Detector/UT/DeUTStave.h"
#include "Detector/UT/UTConstants.h"

#include "Core/DeIOV.h"

#include "fmt/format.h"

#include <array>
#include <deque>
#include <vector>

namespace LHCb::Detector::UT {

  namespace detail {

    struct DeUTObject : LHCb::Detector::detail::DeIOVObject {

      std::array<DeUTSideObject, 2> m_sides;
      /// Flatten list of sectors
      std::vector<DeUTSectorObject*> m_sectors;
      unsigned int                   m_version = 2;

      DeUTObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt );

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& side : m_sides ) { func( &side ); };
      }

      void applyToAllSides( const std::function<void( DeUTSide )>& func ) const { applyToAllChildren( func ); }

      void applyToAllLayers( const std::function<void( DeUTLayer )>& func ) const {
        for ( auto& side : m_sides ) { side.applyToAllLayers( func ); };
      }

      void applyToAllStaves( const std::function<void( DeUTStave )>& func ) const {
        for ( auto& side : m_sides ) side.applyToAllStaves( func );
      }

      void applyToAllFaces( const std::function<void( DeUTFace )>& func ) const {
        for ( auto& side : m_sides ) side.applyToAllFaces( func );
      }

      void applyToAllModules( const std::function<void( DeUTModule )>& func ) const {
        for ( auto& side : m_sides ) side.applyToAllModules( func );
      }

      void applyToAllSectors( const std::function<void( DeUTSector )>& func ) const {
        for ( auto& side : m_sides ) side.applyToAllSectors( func );
      }

      /// returns nb of active sectors and total number of sectors, in this order
      std::pair<double, unsigned int> countSectors() {
        std::pair<double, unsigned int> ret{0, 0};
        auto& [active, total] = ret;
        for ( auto& side : m_sides ) {
          auto [a, t] = side.countSectors();
          active += a;
          total += t;
        }
        return ret;
      }

      unsigned int                         m_nStrip = 0u;
      void                                 setNstrip( unsigned int nStrip ) { m_nStrip = nStrip; }
      std::map<unsigned int, unsigned int> m_STCBmap   = {{0, 0}, {2, 1}, {4, 2},  {5, 3},  {6, 4},  {7, 5},
                                                        {8, 6}, {9, 7}, {10, 8}, {12, 9}, {14, 10}};
      std::map<unsigned int, unsigned int> m_STCFmap   = {{0, 0}, {2, 1},  {4, 2},  {6, 3},  {7, 4},  {8, 5},
                                                        {9, 6}, {10, 7}, {11, 8}, {12, 9}, {14, 10}};
      std::map<unsigned int, unsigned int> m_STBBmap   = {{0, 0}, {2, 1},  {4, 2},  {5, 3}, {8, 4},
                                                        {9, 5}, {10, 6}, {12, 7}, {14, 8}};
      std::map<unsigned int, unsigned int> m_STBFmap   = {{0, 0},  {2, 1},  {4, 2},  {6, 3}, {7, 4},
                                                        {10, 5}, {11, 6}, {12, 7}, {14, 8}};
      std::map<unsigned int, unsigned int> m_STABmap   = {{0, 0}, {2, 1}, {4, 2}, {8, 3}, {10, 4}, {12, 5}, {14, 6}};
      std::map<unsigned int, unsigned int> m_STAFmap   = {{0, 0}, {2, 1}, {4, 2}, {6, 3}, {10, 4}, {12, 5}, {14, 6}};
      static constexpr unsigned int        NBSTATION   = 2;
      static constexpr unsigned int        NBLAYER     = 2;  // nbr layers per station
      static constexpr unsigned int        NBREGION    = 3;  // nbr regions per layer
      static constexpr unsigned int        NBSECTOR    = 98; // nbr sectors per region
      static constexpr unsigned int        NBSIDE      = 2;
      static constexpr unsigned int        NBHALFLAYER = 4; // nbr halflayers per side
      static constexpr unsigned int        NBSTAVE     = 9; // nbr staves per halflayer
      static constexpr unsigned int        NBFACE      = 2; // nbr face per stave
      static constexpr unsigned int        NBMODULE    = 8; // nbr module per face
      static constexpr unsigned int        NBSUBSECTOR = 2; // nbr subsector per module
      static constexpr unsigned int        NTOTSECTOR  = 1048;
      static constexpr unsigned int        NSECTSTA    = 124;
      static constexpr unsigned int        NSECTSTB    = 138;
      static constexpr unsigned int        NSECTSTVC   = 22;
      static constexpr unsigned int        NSECTSTVB   = 18;
      static constexpr unsigned int        NSECTSTVA   = 14;
    };
  } // End namespace detail

  inline std::string const  Location = "/world/BeforeMagnetRegion/UT:DetElement-Info-IOV";
  inline std::string const& location() { return Location; }

  struct DeUT : public DeIOVElement<detail::DeUTObject> {
    using DeIOVElement::DeIOVElement;

    struct LayerGeom {
      float z;
      int   nColsPerSide;
      int   nRowsPerSide;
      float invHalfSectorYSize;
      float invHalfSectorXSize;
      float dxDy;
    };
    enum class LayerID_t : unsigned int {};
    enum class Side_t : unsigned int {};
    enum class Layer_t : unsigned int {};
    enum class Stave_t : unsigned int {};
    enum class Face_t : unsigned int {};
    enum class Module_t : unsigned int {};
    enum class Sector_t : unsigned int {};

    enum class GeoVersion { ErrorVersion = 0, v0, v1 };

    auto detector() const noexcept { return this->access()->detector(); }
    /** vector of sides (stations)
     * @return vector of sides
     */
    const std::vector<DeUTSide> sides() const {
      std::vector<DeUTSide> tsides;
      for ( const auto& side : this->access()->m_sides ) { tsides.push_back( DeUTSide( &side ) ); }
      return tsides;
    }
    const std::vector<DeUTSide> stations() const { return sides(); }

    std::vector<DeUTSector> sectors( LayerID_t layerID ) const {
      std::vector<DeUTSector> tsectors;
      auto&                   layerC = this->access()->m_sides[0].m_layers[(unsigned int)layerID];
      auto&                   layerA = this->access()->m_sides[1].m_layers[(unsigned int)layerID];
      layerC.applyToAllSectors( [&tsectors]( DeUTSector s ) { tsectors.push_back( s ); } );
      layerA.applyToAllSectors( [&tsectors]( DeUTSector s ) { tsectors.push_back( s ); } );
      return tsectors;
    }

    int sensitiveVolumeID( const ROOT::Math::XYZPoint& point ) const override {
      auto sector = findSector( point );
      if ( !sector.isValid() ) {
        dd4hep::printout( dd4hep::ERROR, "DeUT::sensitiveVolumeID",
                          "sensitiveVolumeID: no sensitive volume at (%.2f, %.2f, %.2f)", point.x(), point.y(),
                          point.z() );
        return -1;
      }
      return sector.elementID();
    }

    const DeUTSide findSide( const ROOT::Math::XYZPoint& aPoint ) const {
      const auto& sides = this->access()->m_sides;
      const auto  iside = std::find_if( sides.begin(), sides.end(), [&]( const detail::DeUTSideObject& s ) {
        return DeUTSide{&s}.isInside( aPoint );
      } );
      return iside != sides.end() ? &( *iside ) : nullptr;
    }
    const DeUTSide findSide( const ChannelID aChannel ) const {
      return ( (unsigned int)aChannel.side() < this->access()->m_sides.size() )
                 ? &( this->access()->m_sides[(unsigned int)aChannel.side()] )
                 : nullptr;
    }

    const DeUTSide findStation( const ChannelID aChannel ) const { return findSide( aChannel ); }

    const DeUTLayer findLayer( const ROOT::Math::XYZPoint& aPoint ) const {
      auto side = findSide( aPoint );
      return side.isValid() ? side.findLayer( aPoint ) : DeUTLayer{};
    }

    const DeUTLayer findLayer( const ChannelID aChannel ) const {
      auto side = findSide( aChannel );
      return side.isValid() ? side.findLayer( aChannel ) : DeUTLayer{};
    }

    unsigned int m_firstSide = 0u;
    unsigned int firstSide() const { return m_firstSide; }
    unsigned int firstStation() const { return firstSide(); }
    unsigned int lastSide() const { return m_firstSide + this->access()->m_sides.size() - 1u; }

    /// check contains channel
    [[nodiscard]] bool contains( LHCb::Detector::UT::ChannelID aChannel ) const {
      return ( ( aChannel.side() >= firstSide() ) && ( aChannel.side() < lastSide() ) );
    };

    // what is Gaudi::Time in dd4hep
    // TODO: looks no one use isValid (Time) ?? need to check!!
    // bool isValid( const Gaudi::Time& t ) const override { return ValidDataObject::isValid( t ); }

    /// check channel number is valid
    bool isValid( const ChannelID aChannel ) const {
      const auto aSector = findSector( aChannel );
      return ( aSector.isValid() ? aSector.isStrip( aChannel.strip() ) : false );
    }

    const DeUTSide side( unsigned int index ) const { return &this->access()->m_sides.at( index ); }

    const DeUTSide station( unsigned int index ) const { return side( index ); }

    const DeUTSector sector( unsigned int index ) const { return DeUTSector( this->access()->m_sectors[index] ); }

    /// @return number of sectors
    unsigned int nSectors() const { return this->access()->m_sectors.size(); }

    /// apply given callable to all sectors
    void applyToAllSectors( const std::function<void( const DeUTSector& )>& func ) const {
      return this->access()->applyToAllSectors( func );
    }

    void applyToAllSectorsAllen( const std::function<void( DeUTSector const& )>& func ) const {
      applyToAllSectors( func );
    }

    bool SectorsSwapped() const {
      // UT in DD4hep supports only GeoVersion::v1 which
      // does not have swapped sectors
      return false;
    }

    bool layerSizeOK() const {
      return static_cast<unsigned int>( version() ) != static_cast<unsigned int>( GeoVersion::v0 );
    }

    const LayerGeom getLayerGeom( unsigned int layerid ) const {
      LayerGeom    layergeom;
      auto         SectorsInLayer = sectors( LayerID_t{layerid} );
      auto         stationID      = layerid / 2;
      unsigned int keysectorID    = ( stationID == 0 ) ? 31 : 11;
      layergeom.z                 = SectorsInLayer[keysectorID].globalCentre().z();

      // columns = staves, counting from -x to +x
      // -> From 1 to 16 for UTaX/U layers (stationID = 0)
      // -> From 1 to 18 for UTbX/V layers (stationID = 1)
      // rows = typeA/B 10cmx10cm sectors, rows = 2 x typeC/D sectors
      unsigned int                     lastColumn        = ( stationID == 0 ) ? 16u : 18u;
      float                            yFirstRow         = 0; // y of bottommost sector of outmost stave
      float                            yLastRow          = 0; // y of topmost sector of outmost stave
      float                            xFirstColFirstRow = 0; // x of bottommost sector of Cside outmost stave
      float                            xFirstColLastRow  = 0; // x of topmost sector of Cside outmost stave
      float                            xLastColFirstRow  = 0; // x of bottommost sector of Aside outmost stave
      unsigned int                     maxStaveID        = ( stationID == 0 ) ? 7u : 8u;
      unsigned int                     firstColumn       = 1u;  // count from 1
      unsigned int                     lastRow           = 14u; // top most sector
      unsigned int                     firstRow          = 1u;  // bottom most sector
      std::map<std::string, ChannelID> cornerSectorChanIDs{
          {// Cside, LayerID, maxStaveID, Face1, Module0
           {"FirstColFirstRow", ChannelID( ChannelID::detType::typeUT, 0, layerid, maxStaveID, 1, 0, 0, 0 )},
           // Cside, LayerID, maxStaveID, Face0, Module7
           {"FirstColLastRow", ChannelID( ChannelID::detType::typeUT, 0, layerid, maxStaveID, 0, 7, 0, 0 )},
           // Aside, LayerID, maxStaveID, Face1, Module0
           {"LastColFirstRow", ChannelID( ChannelID::detType::typeUT, 1, layerid, maxStaveID, 1, 0, 0, 0 )}}};
      // loop over sectors within this layer to get x, y of corner sectors
      std::for_each( SectorsInLayer.begin(), SectorsInLayer.end(), [&]( DeUTSector const& utSector ) {
        if ( utSector.channelID() == cornerSectorChanIDs["FirstColFirstRow"] ) {
          yFirstRow         = utSector.globalCentre().y();
          xFirstColFirstRow = utSector.globalCentre().x();
        } else if ( utSector.channelID() == cornerSectorChanIDs["FirstColLastRow"] ) {
          yLastRow         = utSector.globalCentre().y();
          xFirstColLastRow = utSector.globalCentre().x();
        } else if ( utSector.channelID() == cornerSectorChanIDs["LastColFirstRow"] ) {
          xLastColFirstRow = utSector.globalCentre().x();
        }
      } );

      dd4hep::printout( dd4hep::DEBUG, "DeUT::getLayerGeom", "xFirstColFirstRow = %.3f", xFirstColFirstRow );
      dd4hep::printout( dd4hep::DEBUG, "DeUT::getLayerGeom", "xFirstColLastRow = %.3f", xFirstColLastRow );
      dd4hep::printout( dd4hep::DEBUG, "DeUT::getLayerGeom", "xLastColFirstRow = %.3f", xLastColFirstRow );
      dd4hep::printout( dd4hep::DEBUG, "DeUT::getLayerGeom", "yFirstRow = %.3f", yFirstRow );

      // gather all information into the corresponding LayerInfo object
      auto ncols                   = lastColumn - firstColumn + 1;
      auto nrows                   = lastRow - firstRow + 1;
      layergeom.nColsPerSide       = ncols / 2;
      layergeom.nRowsPerSide       = nrows / 2;
      layergeom.invHalfSectorYSize = 2 * ( nrows - 1 ) / ( yLastRow - yFirstRow );
      layergeom.invHalfSectorXSize = 2 * ( ncols - 1 ) / ( xLastColFirstRow - xFirstColFirstRow );
      layergeom.dxDy               = ( xFirstColLastRow - xFirstColFirstRow ) / ( yLastRow - yFirstRow );

      return layergeom;
    }

    double zStation( unsigned int stationid ) const {
      if ( stationid > 2 ) throw std::runtime_error( "wrong stationID in DeUTDetector::zStation" );
      return ( sides()[0].layers()[stationid * 2].globalCentre().z() +
               sides()[0].layers()[stationid * 2 + 1].globalCentre().z() ) /
             2;
    }

    /// apply given callable to all layers
    void applyToAllLayers( const std::function<void( DeUTLayer const& )> func ) const {
      return this->access()->applyToAllLayers( func );
    }

    const DeUTSector findSector( const ChannelID aChannel ) const {
      auto side = findSide( aChannel );
      return side.isValid() ? side.findSector( aChannel ) : DeUTSector{};
    }

    const DeUTSector findSector( const ROOT::Math::XYZPoint& aPoint ) const {
      auto side = findSide( aPoint );
      return side.isValid() ? side.findSector( aPoint ) : DeUTSector{};
    }

    /// get the sector corresponding to the input channel
    const DeUTSector getSector( Side_t side, Layer_t layer, Stave_t stave, Face_t face, Module_t module,
                                Sector_t sec ) const {
      if ( (unsigned int)side < this->access()->NBSIDE && (unsigned int)layer < this->access()->NBHALFLAYER &&
           (unsigned int)stave < this->access()->NBSTAVE && (unsigned int)face < this->access()->NBFACE &&
           (unsigned int)module < this->access()->NBMODULE && (unsigned int)sec < this->access()->NBSUBSECTOR ) {
        const unsigned int id_Layer =
            (unsigned int)side * this->access()->NTOTSECTOR / 2 +
            ( ( (unsigned int)layer < 2 )
                  ? (unsigned int)layer * this->access()->NSECTSTA
                  : ( 2 * this->access()->NSECTSTA + ( (unsigned int)layer - 2 ) * this->access()->NSECTSTB ) );
        const unsigned int id_Stave =
            ( ( (unsigned int)stave > 0 ) ? this->access()->NSECTSTVC : 0 ) +
            ( ( (unsigned int)stave > 1 ) ? this->access()->NSECTSTVB : 0 ) +
            ( ( (unsigned int)stave > 2 ) ? ( (unsigned int)stave - 2 ) * this->access()->NSECTSTVA : 0 );
        const unsigned int id_Face =
            ( ( (unsigned int)stave == 0 ) ? (unsigned int)face * ( this->access()->NSECTSTVC / 2 ) : 0 ) +
            ( ( (unsigned int)stave == 1 ) ? ( (unsigned int)face ) * ( this->access()->NSECTSTVB / 2 ) : 0 ) +
            ( ( (unsigned int)stave > 1 ) ? ( (unsigned int)face ) * ( this->access()->NSECTSTVA / 2 ) : 0 );
        const unsigned int id_Module = 2 * (unsigned int)module + (unsigned int)sec;
        unsigned int       id_Sector;
        if ( (unsigned int)face == 0 ) {
          if ( (unsigned int)stave == 0 )
            id_Sector = this->access()->m_STCBmap.find( id_Module )->second;
          else if ( (unsigned int)stave == 1 )
            id_Sector = this->access()->m_STBBmap.find( id_Module )->second;
          else
            id_Sector = this->access()->m_STABmap.find( id_Module )->second;
        } else {
          if ( (unsigned int)stave == 0 )
            id_Sector = this->access()->m_STCFmap.find( id_Module )->second;
          else if ( (unsigned int)stave == 1 )
            id_Sector = this->access()->m_STBFmap.find( id_Module )->second;
          else
            id_Sector = this->access()->m_STAFmap.find( id_Module )->second;
        }
        const auto i = id_Sector + id_Face + id_Stave + id_Layer;
        return ( i < nSectors() ? sector( i ) : nullptr );
      } else {
        return {};
      }
    }

    const DeUTSector getSector( LHCb::Detector::UT::ChannelID chan ) const {
      auto sec = getSector( Side_t{chan.side()}, Layer_t{chan.layer()}, Stave_t{chan.stave()}, Face_t{chan.face()},
                            Module_t{chan.module()}, Sector_t{chan.sector()} );
      if ( !sec.isValid() ) throw std::runtime_error{"DeUTDetector::getSector returned nullptr"};
      return sec;
    }

    /// get the next channel left
    ChannelID nextLeft( const ChannelID aChannel ) const {
      const auto aSector = findSector( aChannel );
      return aSector.isValid() ? aSector.nextLeft( aChannel ) : ChannelID( 0u );
    }

    /// get the next channel right
    ChannelID nextRight( const ChannelID aChannel ) const {
      const auto aSector = findSector( aChannel );
      return ( aSector.isValid() ? aSector.nextRight( aChannel ) : ChannelID( 0u ) );
    }

    /// get the trajectory
    LineTraj<double> trajectory( const LHCb::Detector::UT::ChannelID channelID, const double offset ) const {
      if ( !channelID.isUT() ) {
        dd4hep::printout( dd4hep::ERROR, "DeUT", "The ChannelID is not of UT type!" );
        throw std::runtime_error( "The ChannelID is not of UT type!" );
      }
      const auto aSector = findSector( channelID );
      if ( !aSector.isValid() ) {
        dd4hep::printout( dd4hep::ERROR, "DeUT", "Failed to find sector" );
        throw std::runtime_error( "Failed to find sector" );
      }
      return aSector.trajectory( channelID, offset );
    }

    /// get the number of sides in detector
    unsigned int nSide() const { return this->access()->m_sides.size(); }

    /// get the number of strips in detector
    unsigned int nStrip() const { return this->access()->m_nStrip; }

    /// get the number of layers
    unsigned int nLayer() const { return 8; }

    /// get the number of readout sectors
    unsigned int nReadoutSector() const { return nSectors(); }

    /// number of layers per station
    unsigned int nLayersPerStation() const { return nLayer() / nStation(); }

    /// number of layers per station
    unsigned int nLayersPerSide() const { return nLayer() / nSide(); }

    /** Geometry version **/
    GeoVersion version() const { return GeoVersion( this->access()->m_version ); }

    /// fraction active channels
    double fractionActive() const {
      auto [active, total] = this->access()->countSectors();
      return active / total;
      ;
    }

    /// get list of all disabled sectors
    std::vector<DeUTSector> disabledSectors() const {
      std::vector<DeUTSector> disabled;
      // disabled.reserve( nReadoutSector() );
      applyToAllSectors( [&disabled]( DeUTSector const& utSector ) {
        if ( utSector.sectorStatus() == Status::ReadoutProblems ) { disabled.push_back( utSector ); }
      } );
      return disabled;
    }

    /// get list of disabled beetles
    std::vector<ChannelID> disabledBeetles() const {
      std::vector<ChannelID> disabledBeetles;
      applyToAllSectors( [&disabledBeetles]( DeUTSector const& s ) {
        auto bStatus = s.beetleStatus();
        for ( unsigned int i = 0; i < bStatus.size(); ++i ) {
          if ( bStatus[i] == Status::ReadoutProblems ) {
            const unsigned int firstStripOnBeetle = ( i * nStripsInSector ) + 1;
            disabledBeetles.push_back( s.stripToChan( firstStripOnBeetle ) );
          }
        } // i
      } );
      return disabledBeetles;
    }

    // beetle as a string
    std::string uniqueBeetle( const ChannelID& chan ) const {
      const auto sector = findSector( chan );
      if ( sector.isValid() ) {
        return fmt::format( "{}Beetle{}", sector.nickname(), sector.beetle( chan ) );
      } else {
        return fmt::format( "Cannot find sector with ChannelID: {}", chan.toString() );
      }
    };

    // port
    std::string uniquePort( const ChannelID& chan ) const {
      const unsigned int port = ( ( chan.strip() - 1u ) / nStripsInPort ) +
                                1u; // FOR HANGYI: LHCbConstants::nStripsInPort should be defined somewhere in DeUT.h
      return fmt::format( "{}Port{}", uniqueBeetle( chan ), port );
    };

    void applyToSectors( const std::vector<ChannelID>&                                       vec,
                         const std::function<void( const LHCb::Detector::UT::DeUTSector& )>& func ) const {
      std::vector<DeUTSector> sectors;
      sectors.reserve( vec.size() );
      for ( auto i : vec ) {
        auto aSector = findSector( i );
        if ( aSector.isValid() ) { sectors.push_back( aSector ); }
      }
      // ensure unique list
      sectors.erase( std::unique( sectors.begin(), sectors.end() ), sectors.end() );
      for ( const DeUTSector& sector : sectors ) func( sector );
    }

    unsigned int nStation() const { return 2u; }
  };

} // namespace LHCb::Detector::UT
