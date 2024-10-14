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
#include <deque>
#include <functional>

#include "Core/DeIOV.h"
#include "Detector/FT/DeFTModule.h"

namespace LHCb::Detector {

  namespace detail {

    /**
     *  Generic FT iov dependent detector element of a FT quarter
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeFTQuarterObject : DeIOVObject {
      /// Reference to the static information of modules
      unsigned int                 m_id{};
      std::deque<DeFTModuleObject> m_modules{};
      ROOT::Math::Plane3D          m_plane{};
      float                        m_meanModuleZ{};
      float                        m_meanModuleDzdy{};
      float                        m_meanModuleDxdy{};

      DeFTQuarterObject( const dd4hep::DetElement& de, dd4hep::cond::ConditionUpdateContext& ctxt,
                         unsigned int iQuarter, unsigned int layerID );
      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& func ) const override {
        for ( auto& module : m_modules ) { func( &module ); };
      };
      template <typename F>
      void applyToAllMats( F func ) const {
        for ( auto& module : m_modules ) module.applyToAllChildren( func );
      }
    };
  } // End namespace detail

  struct DeFTQuarter : DeIOVElement<detail::DeFTQuarterObject> {
    using DeIOVElement::DeIOVElement;
    unsigned int quarterID() const { return this->access()->m_id; }
    unsigned int quarterIdx() const { return this->access()->m_id - FTConstants::quarterShift; }

    /** Find the FT Module corresponding to the point
     *  @return Pointer to the relevant module
     */
    [[nodiscard]] const std::optional<DeFTModule> findModule( const ROOT::Math::XYZPoint& aPoint ) const {
      auto iM =
          std::find_if( this->access()->m_modules.begin(), this->access()->m_modules.end(),
                        [&aPoint]( const detail::DeFTModuleObject& m ) { return DeFTModule{&m}.isInside( aPoint ); } );
      return iM != this->access()->m_modules.end() ? DeFTModule{&*iM} : std::optional<DeFTModule>{}; // DeFTModule{};
    }
    /** Const method to return the module for a given channel id
     * @param  aChannel an FT channel id
     * @return pointer to detector element
     */
    [[nodiscard]] const std::optional<DeFTModule> findModule( const FTChannelID& id ) const {
      return ( to_unsigned( id.module() ) < this->access()->m_modules.size() )
                 ? &( this->access()->m_modules[to_unsigned( id.module() )] )
                 : std::optional<DeFTModule>{};
    }

    /// Find a module from global X and Y coordinates
    const std::optional<DeFTModule> findModuleWithXY( const ROOT::Math::XYZPoint& globalPoint ) const {
      const auto iter =
          std::find_if( this->access()->m_modules.begin(), this->access()->m_modules.end(),
                        [&globalPoint]( const detail::DeFTModuleObject& m ) {
                          auto localPoint = m.toLocal( globalPoint );
                          return m.isInside( m.toGlobal( ROOT::Math::XYZPoint( localPoint.x(), localPoint.y(), 0. ) ) );
                        } );
      return iter != this->access()->m_modules.end() ? DeFTModule{&*iter} : std::optional<DeFTModule>{};
    }

    /** Flat vector of all FT modules
     * @return vector of modules
     */
    [[nodiscard]] const auto&         modules() const { return this->access()->m_modules; }
    [[nodiscard]] float               meanModuleZ() const { return this->access()->m_meanModuleZ; }
    [[nodiscard]] ROOT::Math::Plane3D plane() const { return this->access()->m_plane; }
    [[nodiscard]] float               meanModuleDxdy() const { return this->access()->m_meanModuleDxdy; }
    [[nodiscard]] float               meanModuleDzdy() const { return this->access()->m_meanModuleDzdy; }
  };
} // End namespace LHCb::Detector
