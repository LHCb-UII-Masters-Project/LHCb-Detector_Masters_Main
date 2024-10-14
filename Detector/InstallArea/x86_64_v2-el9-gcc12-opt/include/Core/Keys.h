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

#include "Core/ParameterMap.h"

#include "DD4hep/DetElement.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Objects.h"
#include "DD4hep/Printout.h"
#include "DD4hep/detail/ConditionsInterna.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#include <map>

namespace LHCb::Detector {

  /**
   * Helper class that hashes the item name and keeps
   * the original string for logging purpose.
   *
   * To be used as a parameter to construct a ConditionKey
   */
  struct ItemKey {

    ItemKey( std::string item_name ) : name( std::move( item_name ) ), hash{dd4hep::ConditionKey::itemCode( name )} {
      dd4hep::printout( dd4hep::DEBUG, "item_key", "Key %016lX  maps to %s", hash, name.c_str() );
    }

    ItemKey( const ItemKey& key ) = default;
    ItemKey( ItemKey&& key )      = default;
    ItemKey& operator=( const ItemKey& key ) = delete;
    ItemKey& operator=( ItemKey&& key ) = delete;

    // Allow implicit conversion to DD4hep itemkey_type
    operator dd4hep::Condition::itemkey_type() const { return hash; }

    // The members themselves
    const std::string                 name;
    const dd4hep::Condition::key_type hash;
  };

  /**
   * Utility methods to compute the hash
   */
  dd4hep::Condition::key_type hash_key( const dd4hep::DetElement& det, const std::string& value );

  dd4hep::Condition::key_type hash_key( const dd4hep::DetElement& det, const ItemKey& item_key );

  /**
   * Helpers to deal with condition keys. Condition keys are 64 bit integers,
   * with the top 32 bits containing a DetElement key, and the lower 32 bits
   * containing an item_key.
   *
   * Condition key = DetElement key | Item Key
   *
   */
  struct ConditionKey {

    ConditionKey( const std::string& det_name, const ItemKey& item_key )
        : ConditionKey( dd4hep::Detector::getInstance().detector( det_name ), std::move( item_key ) ) {
      if ( !det_element.isValid() ) { throw std::runtime_error( det_name + ": not a valid detector element path" ); }
    }

    ConditionKey( const dd4hep::DetElement& det, const std::string& item_name )
        : ConditionKey( det, std::move( ItemKey{item_name} ) ) {}

    ConditionKey( const dd4hep::DetElement& det, const ItemKey& item_key )
        : det_element{det}, item_key{std::move( item_key )} {
      dd4hep::printout( dd4hep::DEBUG, "cond_key", "Key %016lX  maps to %s:%s. Precalc. item hash.", hash(),
                        det_element.path().c_str(), item_key.name.c_str() );
    }

    ConditionKey( const ConditionKey& key ) = default;
    ConditionKey( ConditionKey&& key )      = default;
    ConditionKey& operator=( const ConditionKey& key ) = delete;
    ConditionKey& operator=( ConditionKey&& key ) = delete;

    dd4hep::Condition::key_type hash() const { return LHCb::Detector::hash_key( det_element, item_key ); }

    // Allow implicit conversion to DD4hep key_type
    operator dd4hep::Condition::key_type() const { return hash(); }

    // The members themselves
    const dd4hep::DetElement det_element;
    const ItemKey            item_key;
  };

  /**
   * Perform the conversion for an item_key (lower 32 bits of the condition key)
   */
  dd4hep::Condition::itemkey_type item_key( const std::string& name );

  /// Static identifiers computed once. Use the hash code whenever possible!
  struct Keys {
    /// Static key name: "DetElement-Info-IOV"
    static const std::string deKeyName;
    /// Static key: 32 bit hash of "DetElement-Info-IOV". Must be unique for one DetElement
    static const ItemKey deKey;

    /// Key name  of a delta condition "alignment_delta".
    static const std::string deltaName;
    /// Key value of a delta condition "alignment_delta".
    static const ItemKey deltaKey;

    /// Static key name: "alignment"
    static const std::string alignmentKeyName;
    /// Static key: 32 bit hash of "alignment". Must be unique for one DetElement
    static const ItemKey alignmentKey;

    /// Static key name: "Alignments-Computed"
    static const std::string alignmentsComputedKeyName;
    /// Static key: 32 bit hash of "Alignments-Computed". Must be unique for the world
    static const dd4hep::Condition::key_type alignmentsComputedKey;
  };

} // namespace LHCb::Detector