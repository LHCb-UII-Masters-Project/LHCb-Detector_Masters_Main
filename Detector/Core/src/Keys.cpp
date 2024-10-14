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

// Framework include files
#include "Core/Keys.h"
#include "DD4hep/Alignments.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"

namespace LHCb::Detector {
  /// Static key name: "DetElement-Info-IOV"
  const std::string Keys::deKeyName( "DetElement-Info-IOV" );
  /// Static key: 32 bit hash of "DetElement-Info-IOV". Must be unique for one DetElement
  const ItemKey Keys::deKey{Keys::deKeyName};
  /// Key name  of a delta condition "alignment_delta".
  const std::string Keys::deltaName = dd4hep::align::Keys::deltaName;
  /// Key value of a delta condition "alignment_delta".
  const ItemKey Keys::deltaKey{dd4hep::align::Keys::deltaName};

  /// Static key name: "alignment"
  const std::string Keys::alignmentKeyName = dd4hep::align::Keys::alignmentName;
  /// Static key: 32 bit hash of "alignment". Must be unique for one DetElement
  const ItemKey Keys::alignmentKey{dd4hep::align::Keys::alignmentName};

  const std::string Keys::alignmentsComputedKeyName( "Alignments-Computed" );
  /// Static key: 32 bit hash of "Alignments-Computed". Must be unique for the world
  /// Keeping the KeyMaker here as hash_key does not accept a plain int
  const dd4hep::Condition::key_type Keys::alignmentsComputedKey =
      dd4hep::ConditionKey::KeyMaker( 0, Keys::alignmentsComputedKeyName ).hash;

  /**
   * Utility methods to compute the hash
   */
  dd4hep::Condition::key_type hash_key( const dd4hep::DetElement& det, const std::string& value ) {
    dd4hep::Condition::key_type ret = (dd4hep::Condition::key_type)det.key() << 32;
    ret |= dd4hep::detail::hash32( value );
    return ret;
  }

  dd4hep::Condition::key_type hash_key( const dd4hep::DetElement& det, const LHCb::Detector::ItemKey& item_key ) {
    dd4hep::Condition::key_type ret = (dd4hep::Condition::key_type)det.key() << 32;
    ret |= item_key.hash;
    return ret;
  }

  /**
   * Perform the conversion for an item_key (lower 32 bits of the condition key)
   */
  dd4hep::Condition::itemkey_type item_key( const std::string& name ) { return dd4hep::ConditionKey::itemCode( name ); }

} // namespace LHCb::Detector
