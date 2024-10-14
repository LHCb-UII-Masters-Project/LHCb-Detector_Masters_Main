/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*******************************************************************************/

#pragma once

#include "Core/DeIOV.h"
#include "Detector/Rich/Types.h"
#include "Detector/Rich/Utilities.h"
#include <utility>

namespace LHCb::Detector {

  namespace detail {

    struct DeRichMirrorSegObject : DeIOVObject {

    public:
      int                   m_mirrorNumber = -1;                ///< id number which is same as the copy number.
      Rich::Side            m_mirrorSide   = Rich::InvalidSide; ///< side number 0 for top and 1 for bottom
      double                m_SegmentROC   = 0;                 ///< Mirror radius of curvature
      double                m_SegmentXSize = 0;                 ///< Mirror X size
      double                m_SegmentYSize = 0;                 ///< Mirror Y size
      dd4hep::Position      m_SegmentLocalCOC;        ///< local COC in the corresponding Rich1Mirro2Master system
      dd4hep::Position      m_SegmentGlobalCOC;       ///< COC in the global coordinate system
      dd4hep::Position      m_MirrorSurfCentrePtn;    ///< Centre point of mirror segment surface in global frame
      ROOT::Math::XYZVector m_MirrorSurfCentreNormal; ///< Normal vector to mirror surface at centre point, global frame
      XYTable               m_Reflectivity;           ///< Mirror reflectivity data

    public:
      // inherit base constructor
      using DeIOVObject::DeIOVObject;

      void applyToAllChildren( const std::function<void( LHCb::Detector::DeIOV )>& ) const override {
        // no children
      }
      /// common initialisation for all mirrors
      void commonMirrorInit() {
        // COC position in the Master volume.
        m_SegmentLocalCOC = detector().placement().position();
        // CoC in the global LHCb frame
        m_SegmentGlobalCOC = detectorAlignment().localToWorld( {0, 0, 0} );
        // Get the mirror centre point and normal vector at that point
        // unit vector in local frame to mirror CoC
        // Sajan - DeRichSphMirror in RichDet does a more complex calculation here to determine this vector
        //         Using the mirror min/max theta angles etc. However, what it ends up with in all cases is
        //         pretty much {1,0,0} .... Do we need something better here or is this OK ?
        const ROOT::Math::XYZVector toMirrorCentreLocal( 1, 0, 0 );
        // centre point of mirror segment surface in local frame
        const dd4hep::Position loc_ptn( m_SegmentROC * toMirrorCentreLocal );
        // normal to mirror at centre point, in local frame
        const auto loc_norm = ROOT::Math::XYZVector( -loc_ptn.x(), -loc_ptn.y(), -loc_ptn.z() ).Unit();
        // ... and finally then convert to global frame
        m_MirrorSurfCentrePtn = detectorAlignment().localToWorld( loc_ptn );
        // Note, 'localToWorld' applies the full translation+rotation to go fom the local to global frame.
        // However, for the unit vector we only want the rotation. Subtracting the transformed (0,0,0) origin
        // point removes the translation, leaving just the rotation.
        m_MirrorSurfCentreNormal = ( detectorAlignment().localToWorld( loc_norm ) - m_SegmentGlobalCOC ).Unit();
        // finally print yourself...
        detail::print( *this );
      }

    public:
      /// Checks if the direction intersects with the mirror
      template <typename POINT, typename VECTOR>
      bool intersects( const POINT& /*globalP*/, //
                       const VECTOR& /*globalV*/ ) const {
        throw NotImplemented( "DeRichMirrorSegObject::intersects" );
        return false;
      }

      /// Checks if the direction intersects with the mirror and returns the intersction point
      template <typename POINT, typename VECTOR>
      bool intersects( const POINT& /*globalP*/,  //
                       const VECTOR& /*globalV*/, //
                       POINT& /*intersectionPoint*/ ) const {
        throw NotImplemented( "DeRichMirrorSegObject::intersects" );
        return false;
      }
    };

    template <typename ObjectType>
    struct DeRichMirrorSegElement : DeIOVElement<ObjectType> {

      using DeIOVElement<ObjectType>::DeIOVElement;

      auto mirrorNumber() const noexcept { return this->access()->m_mirrorNumber; }
      auto mirrorSide() const noexcept { return this->access()->m_mirrorSide; }

      auto SegXSize() const noexcept { return toLHCbLengthUnits( this->access()->m_SegmentXSize ); }
      auto SegYSize() const noexcept { return toLHCbLengthUnits( this->access()->m_SegmentYSize ); }
      auto ROC() const noexcept { return toLHCbLengthUnits( this->access()->m_SegmentROC ); }

      auto LocalCOC() const noexcept { return toLHCbLengthUnits( this->access()->m_SegmentLocalCOC ); }
      auto GlobalCOC() const noexcept { return toLHCbLengthUnits( this->access()->m_SegmentGlobalCOC ); }

      auto RetrieveReflectivity() const noexcept { return scalePhotonEnergies( this->access()->m_Reflectivity ); }

      auto detector() const noexcept { return this->access()->detector; }
      auto detectorParent() const noexcept { return detector().parent(); }

      /// A point in global coordinates, that correspeonds to a point in the
      /// mirror surface at its centre point.
      auto mirrorCentre() const noexcept { return toLHCbLengthUnits( this->access()->m_MirrorSurfCentrePtn ); }

      /// A vector, in global coordinates, that is normal to the surface of the mirror at its centre point.
      auto centreNormal() const noexcept { return this->access()->m_MirrorSurfCentreNormal; }

      /// Volume intersection methods
      template <typename... ARGS>
      inline auto intersects( ARGS&&... args ) const {
        return this->access()->intersects( std::forward<ARGS>( args )... );
      }
    };

  } // namespace detail

  using DeRichBaseMirror = detail::DeRichMirrorSegObject;

} // namespace LHCb::Detector
