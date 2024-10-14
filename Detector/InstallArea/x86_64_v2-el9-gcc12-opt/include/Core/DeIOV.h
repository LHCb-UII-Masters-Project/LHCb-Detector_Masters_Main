/*****************************************************************************\
* (c) Copyright 2000-2022 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "Core/GeometryTools.h"
#include "Core/Units.h"
#include "Core/Utils.h"

#include "DD4hep/ConditionDerived.h"
#include "DD4hep/Conditions.h"
#include "DD4hep/ConditionsMap.h"
#include "DD4hep/Handle.h"
#include "DD4hep/detail/ConditionsInterna.h"
#include "DD4hep/detail/DetectorInterna.h"

#include <cstddef>
#include <optional>

namespace LHCb::Detector {

  namespace detail {
    struct DeIOVObject;
  } // namespace detail

  // We need a forward declaration as IAlignable uses it
  // We have to review this design
  template <typename ObjectType>
  struct DeIOVElement;

  /**
   * Generic incterface defined for compatibility with DetDesc.
   * IGeometryInfo * are passed in Rec but never used.
   */
  struct IGeometryInfo {

    virtual ~IGeometryInfo() = default;
  };

  /**
   * Generic functionality of alignable objects.
   * Detector equivalent of DetDesc IGeometryInfo
   */
  struct IAlignable : IGeometryInfo {

    /// Name of the volume
    virtual std::string lVolumeName() const = 0;

    /// return a pointer to the volume
    virtual IGeometryInfo*       geometry()       = 0;
    virtual const IGeometryInfo* geometry() const = 0;

    /// Access to the alignmant object to transformideal coordinates
    virtual dd4hep::Alignment detectorAlignment() const = 0;

    /// Checker whether global point is inside this detector element.
    virtual bool isInside( const ROOT::Math::XYZPoint& globalPoint ) const = 0;

    /// Access to transformation matrices
    virtual const ROOT::Math::Transform3D toLocalMatrix() const         = 0;
    virtual const ROOT::Math::Transform3D toGlobalMatrix() const        = 0;
    virtual const ROOT::Math::Transform3D toLocalMatrixNominal() const  = 0;
    virtual const ROOT::Math::Transform3D toGlobalMatrixNominal() const = 0;

    /// Local -> Global and Global -> Local transformations
    virtual ROOT::Math::XYZPoint    toLocal( const ROOT::Math::XYZPoint& global ) const           = 0;
    virtual ROOT::Math::XYZPoint    toGlobal( const ROOT::Math::XYZPoint& local ) const           = 0;
    virtual ROOT::Math::XYZVector   toLocal( const ROOT::Math::XYZVector& globalDirection ) const = 0;
    virtual ROOT::Math::XYZVector   toGlobal( const ROOT::Math::XYZVector& localDirection ) const = 0;
    virtual ROOT::Math::Transform3D toLocal( const ROOT::Math::Transform3D& transform ) const     = 0;
    virtual ROOT::Math::Transform3D toGlobal( const ROOT::Math::Transform3D& transform ) const    = 0;

    /// Methods used by the Alignment
    virtual const ROOT::Math::Transform3D ownToOffNominalMatrix() const                                        = 0;
    virtual std::vector<double>           elemDeltaTranslations() const                                        = 0;
    virtual std::vector<double>           elemDeltaRotations() const                                           = 0;
    virtual dd4hep::Delta                 delta() const                                                        = 0;
    virtual const ROOT::Math::Transform3D localDeltaMatrix( const ROOT::Math::Transform3D& globalDelta ) const = 0;

    /// returns motionSystemTransform if we are dealing with the Velo
    virtual std::optional<ROOT::Math::Transform3D> motionSystemTransform() const = 0;

    virtual void applyToAllChildren( const std::function<void( DeIOVElement<detail::DeIOVObject> )>& func ) const = 0;
    virtual std::size_t size() const                                                                              = 0;
  }; // namespace LHCb::Detector

  template <typename ObjectType>
  struct DeIOVElement : dd4hep::Handle<ObjectType>, IAlignable {
    using dd4hep::Handle<ObjectType>::Handle;

    /// Accessor to the geometry structure of this detector element
    std::string lVolumeName() const override { return placement().volume().name(); }

    /// Print the detector element's information to stdout
    void print( int indent, int flg ) const { this->access()->print( indent, flg ); }
    /// Compute key value for caching
    static auto key( const std::string& value ) { return dd4hep::ConditionKey::itemCode( value ); }
    /// Compute key value for caching
    static auto key( const char* value ) { return dd4hep::ConditionKey::itemCode( value ); }

    /// Detector element Class ID, as given by DetDesc, for backward compatibility
    auto clsID() const { return this->access()->clsID; }
    /// Accessor to detector structure

    auto detector() const { return this->access()->detector(); }
    /// Accessor to the geometry structure of this detector element
    IGeometryInfo*       geometry() override { return this->access(); }
    const IGeometryInfo* geometry() const override { return this->access(); }
    /// Return the placed volume
    auto placement() const { return this->access()->placement(); }

    /// Access to the alignmant object to transformideal coordinates
    dd4hep::Alignment detectorAlignment() const override { return this->access()->detectorAlignment(); }

    /// Checker whether global point is inside this detector element.
    bool isInside( const ROOT::Math::XYZPoint& globalPoint ) const override {
      return this->access()->isInside( globalPoint );
    }

    /// default implementation of sensitiveVolumeID, returning 0. Should be overwritten by children
    virtual int sensitiveVolumeID( ROOT::Math::XYZPoint const& ) const { return 0; }

    // Finding most precise IAlignable hosting given point
    DeIOVElement<ObjectType> findChildForPoint( const ROOT::Math::XYZPoint& point ) const {
      return this->access()->findChildForPoint( point );
    }

    /// Access to transformation matrices
    const ROOT::Math::Transform3D toLocalMatrix() const override { return this->access()->toLocalMatrix(); }
    const ROOT::Math::Transform3D toGlobalMatrix() const override { return this->access()->toGlobalMatrix(); }
    const ROOT::Math::Transform3D toLocalMatrixNominal() const override {
      return this->access()->toLocalMatrixNominal();
    }
    const ROOT::Math::Transform3D toGlobalMatrixNominal() const override {
      return this->access()->toGlobalMatrixNominal();
    }

    // Local -> Global and Global -> Local transformations
    ROOT::Math::XYZPoint toLocal( const ROOT::Math::XYZPoint& global ) const override {
      return this->access()->toLocal( global );
    }
    ROOT::Math::XYZPoint toGlobal( const ROOT::Math::XYZPoint& local ) const override {
      return this->access()->toGlobal( local );
    }
    ROOT::Math::XYZVector toLocal( const ROOT::Math::XYZVector& globalDirection ) const override {
      return this->access()->toLocal( globalDirection );
    }
    ROOT::Math::XYZVector toGlobal( const ROOT::Math::XYZVector& localDirection ) const override {
      return this->access()->toGlobal( localDirection );
    }
    ROOT::Math::Transform3D toLocal( const ROOT::Math::Transform3D& transform ) const override {
      return this->access()->toLocal( transform );
    }
    ROOT::Math::Transform3D toGlobal( const ROOT::Math::Transform3D& transform ) const override {
      return this->access()->toGlobal( transform );
    }

    const ROOT::Math::Transform3D ownToOffNominalMatrix() const override {
      return this->access()->ownToOffNominalMatrix();
    }

    std::vector<double> elemDeltaTranslations() const override { return this->access()->elemDeltaTranslations(); }

    std::vector<double> elemDeltaRotations() const override { return this->access()->elemDeltaRotations(); }

    dd4hep::Delta delta() const override { return this->access()->delta(); }

    const ROOT::Math::Transform3D localDeltaMatrix( const ROOT::Math::Transform3D& globalDelta ) const override {
      return this->access()->localDeltaMatrix( globalDelta );
    }

    /// returns motionSystemTransform if we are dealing with the Velo
    std::optional<ROOT::Math::Transform3D> motionSystemTransform() const override {
      return this->access()->motionSystemTransform();
    }

    void applyToAllChildren( const std::function<void( DeIOVElement<detail::DeIOVObject> )>& func ) const override {
      return this->access()->applyToAllChildren( func );
    }

    std::size_t size() const override { return this->access()->size(); }
  };

  namespace detail {

    /**
     *  Base class for interval of validity dependent data
     *
     *  \author  Markus Frank
     *  \date    2018-03-08
     *  \version  1.0
     */
    struct DeIOVObject : dd4hep::detail::ConditionObject, IAlignable {

      DeIOVObject( dd4hep::DetElement de, dd4hep::cond::ConditionUpdateContext& ctxt, int classID = 6,
                   bool hasAlignment = true );

      /// Detector element Class ID, as given by DetDesc, for backward compatibility
      int clsID{6}; // matches DetDesc CLID_AlignmentCondition

    private:
      /// The dd4hep detector element reference of this gaudi detector element
      dd4hep::DetElement m_detector;
      /// The alignment object
      dd4hep::Alignment m_detectorAlignment;
      /// We cache a reference to the geometry information of this gaudi detector element
      dd4hep::PlacedVolume m_geometry;
      /// We want to cache here the matrix from local to world as original one is a TGeoHMatrix in ROOT system of units
      /// and we want a Transform3D with LHCb system of units
      ROOT::Math::Transform3D m_toGlobalMatrix;
      /// We want to cache here the nominal matrix from local to world as original one is a TGeoHMatrix in ROOT
      /// system of units and we want a Transform3D with LHCb system of units
      ROOT::Math::Transform3D m_toGlobalMatrixNominal;
      /// We want to cache here the matrix from world to local (i.e. inverse toGlobalMatrix)
      ROOT::Math::Transform3D m_toLocalMatrix;
      /// We want to cache here the nominal matrix from world to local (i.e. inverse toGlobalMatrixNominal)
      ROOT::Math::Transform3D m_toLocalMatrixNominal;
      /// We want to cache here the delta matrix, that is the alignment
      ROOT::Math::Transform3D m_delta;

    public:
      /// Accessor to the geometry structure of this detector element
      std::string lVolumeName() const override { return placement().volume().name(); }

      // Pointer to the IGeometryInfo i.e. itself...
      IGeometryInfo* geometry() override { return this; }

      // Pointer to the IGeometryInfo i.e. itself...
      const IGeometryInfo* geometry() const override { return this; }

      // Accessor for the Detector
      dd4hep::DetElement detector() const { return m_detector; }

      // Get the actual geometry
      dd4hep::PlacedVolume placement() const { return this->m_geometry; }

      /// Access to the alignmant object to transformideal coordinates
      dd4hep::Alignment detectorAlignment() const override { return this->m_detectorAlignment; }

      /// Checker whether global point is inside this detector element.
      bool isInside( const ROOT::Math::XYZPoint& globalPoint ) const override;

      // Finding most precise IAlignable hosting given point
      DeIOVElement<DeIOVObject> findChildForPoint( const ROOT::Math::XYZPoint& point ) const;

      const ROOT::Math::Transform3D toLocalMatrix() const override {
        return detail::toLHCbLengthUnits( this->m_toLocalMatrix );
      }
      const ROOT::Math::Transform3D toGlobalMatrix() const override {
        return detail::toLHCbLengthUnits( this->m_toGlobalMatrix );
      }
      const ROOT::Math::Transform3D toLocalMatrixNominal() const override {
        return detail::toLHCbLengthUnits( this->m_toLocalMatrixNominal );
      }
      const ROOT::Math::Transform3D toGlobalMatrixNominal() const override {
        return detail::toLHCbLengthUnits( this->m_toGlobalMatrixNominal );
      }

      /// Local -> Global and Global -> Local transformations
      ROOT::Math::XYZPoint toLocal( const ROOT::Math::XYZPoint& global ) const override {
        // The convertion to XYZVector is due to the fact that DD4hep uses that type in AlignmentData
        return ROOT::Math::XYZPoint( detail::toLHCbLengthUnits(
            this->m_detectorAlignment.worldToLocal( detail::toDD4hepUnits( ROOT::Math::XYZVector( global ) ) ) ) );
      }
      ROOT::Math::XYZPoint toGlobal( const ROOT::Math::XYZPoint& local ) const override {
        // The convertion to XYZVector is due to the fact that DD4hep uses that type in AlignmentData
        return ROOT::Math::XYZPoint( detail::toLHCbLengthUnits(
            this->m_detectorAlignment.localToWorld( detail::toDD4hepUnits( ROOT::Math::XYZVector( local ) ) ) ) );
      }
      ROOT::Math::XYZVector toLocal( const ROOT::Math::XYZVector& globalDirection ) const override {
        // The DD4hep worldToLocal considers XYZVectors as points. It returns the coordinates of the
        // point with coordinates "globalDirection" in the global frame, in the local frame
        // We therefore subtract the vector corresponding to  {0, 0, 0}
        return detail::toLHCbLengthUnits(
            this->m_detectorAlignment.worldToLocal( detail::toDD4hepUnits( globalDirection ) ) -
            this->m_detectorAlignment.worldToLocal( {0, 0, 0} ) );
      }
      ROOT::Math::XYZVector toGlobal( const ROOT::Math::XYZVector& localDirection ) const override {
        // The DD4hep worldToLocal considers XYZVectors as points. It returns the coordinates of the
        // point with coordinates "globalDirection" in the global frame, in the local frame
        // We therefore subtract the vector corresponding to  {0, 0, 0}
        return detail::toLHCbLengthUnits(
            this->m_detectorAlignment.localToWorld( detail::toDD4hepUnits( localDirection ) ) -
            this->m_detectorAlignment.localToWorld( {0, 0, 0} ) );
      }

      ROOT::Math::Transform3D toLocal( const ROOT::Math::Transform3D& transform ) const override {
        return toLocalMatrix() * transform;
      }
      ROOT::Math::Transform3D toGlobal( const ROOT::Math::Transform3D& transform ) const override {
        return toGlobalMatrix() * transform;
      }

      /// Methods used by the alignment
      std::vector<double> elemDeltaTranslations() const override {
        std::vector<double> res{0, 0, 0};
        ownToOffNominalMatrix().Translation().GetComponents( res[0], res[1], res[2] );
        return res;
      }

      std::vector<double> elemDeltaRotations() const override {
        const auto zyx = ownToOffNominalMatrix().Rotation<ROOT::Math::RotationZYX>();
        return {zyx.Psi(), zyx.Theta(), zyx.Phi()};
      }

      dd4hep::Delta delta() const override { return detail::toLHCbLengthUnits( detectorAlignment().delta() ); }

      const ROOT::Math::Transform3D ownToOffNominalMatrix() const override {
        return detail::toLHCbLengthUnits( m_delta );
      }

      /// Obtain the 3D transformation that applies misalignment from nominal
      /// position in the frame of the detector element's parent.
      const ROOT::Math::Transform3D localDeltaMatrix( const ROOT::Math::Transform3D& globalDelta ) const override {
        const ROOT::Math::Transform3D d_0 = ownToOffNominalMatrix();
        const ROOT::Math::Transform3D T   = toGlobalMatrix();
        return d_0 * T.Inverse() * globalDelta * T;
      }

      std::optional<ROOT::Math::Transform3D> motionSystemTransform() const override { return {}; }

      std::size_t size() const override { return 0; }

      /// Printout method to stdout
      virtual void print( int indent, int flags ) const;
    };
  } // End namespace detail

  using DeIOV = DeIOVElement<detail::DeIOVObject>;

} // End namespace LHCb::Detector
