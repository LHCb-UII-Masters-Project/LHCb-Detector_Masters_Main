//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
//
// DDDB is a detector description convention developed by the LHCb experiment.
// For further information concerning the DTD, please see:
// http://lhcb-comp.web.cern.ch/lhcb-comp/Frameworks/DetDesc/Documents/lhcbDtd.pdf
//
//==========================================================================

// Framework includes
#include "Core/ConditionsRepository.h"
#include "DDCond/ConditionsContent.h"

#include <iostream>

dd4hep::Condition::key_type LHCb::Detector::location_key( const ConditionIdentifier* entry ) {
  dd4hep::ConditionKey::KeyMaker loc( entry->sys_id_hash, entry->object_hash );
  return loc.hash;
}

// C/C++ include files
template <>
std::string dd4hep::cond::ConditionsContent::LoadInfo<LHCb::Detector::ConditionIdentifier>::toString() const {
  char text[64];
  ::snprintf( text, sizeof( text ), "[%016llX]", info.hash );
  std::stringstream str;
  str << text << " DE:" << info.detector.path() << " SysID:" << info.sys_id << "#" << info.object << std::endl;
  return str.str();
}

std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionsLoadInfo*>
LHCb::Detector::ConditionsRepository::addLocation( dd4hep::DetElement de, dd4hep::Condition::itemkey_type item,
                                                   const std::string& sys_id, const std::string& object,
                                                   bool is_optional ) {
  ConditionIdentifier            ident;
  dd4hep::ConditionKey::KeyMaker km( de.key(), item );
  ident.hash        = km.hash;
  ident.detector    = de;
  ident.object      = object;
  ident.sys_id      = sys_id;
  ident.sys_id_hash = dd4hep::detail::hash32( sys_id );
  ident.object_hash = dd4hep::detail::hash32( object );
  ident.repository  = this;
  ident.is_optional = is_optional;
  auto* info        = new LoadInfo<ConditionIdentifier>( ident );
  auto  ret         = addLocationInfo( km.hash, info );
  locations.insert( &info->info );
  return ret;
}

std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionsLoadInfo*>
LHCb::Detector::ConditionsRepository::addGlobal( dd4hep::DetElement de, dd4hep::Condition::itemkey_type item,
                                                 const std::string& sys_id, const std::string& object,
                                                 bool is_optional ) {
  ConditionIdentifier            ident;
  dd4hep::ConditionKey::KeyMaker km( 0, item );
  ident.hash        = km.hash;
  ident.detector    = de;
  ident.object      = object;
  ident.sys_id      = sys_id;
  ident.sys_id_hash = dd4hep::detail::hash32( sys_id );
  ident.object_hash = dd4hep::detail::hash32( object );
  ident.repository  = this;
  ident.is_optional = is_optional;
  auto* info        = new LoadInfo<ConditionIdentifier>( ident );
  auto  ret         = addLocationInfo( km.hash, info );
  locations.insert( &info->info );
  return ret;
}

/// Merge the content of "to_add" into the this content
void LHCb::Detector::ConditionsRepository::merge( const LHCb::Detector::ConditionsRepository& to_add ) {
  this->dd4hep::cond::ConditionsContent::merge( to_add );
  for ( const auto& c : to_add.locations ) { this->locations.insert( c ); }
}

std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
LHCb::Detector::ConditionsRepository::addDependency( dd4hep::cond::ConditionDependency* dep ) {
  return this->dd4hep::cond::ConditionsContent::addDependency( dep );
}

/// Add a new conditions dependency (Built internally from arguments)
std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
LHCb::Detector::ConditionsRepository::addDependency( dd4hep::DetElement de, dd4hep::Condition::itemkey_type item,
                                                     std::shared_ptr<dd4hep::cond::ConditionUpdateCall> callback ) {
  dd4hep::cond::ConditionDependency* dep = new dd4hep::cond::ConditionDependency( de, item, callback );
  return addDependency( dep );
}

/// Add a new global conditions dependency (Built internally from arguments)
std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
LHCb::Detector::ConditionsRepository::addGlobal( dd4hep::DetElement de, dd4hep::Condition::itemkey_type item,
                                                 std::shared_ptr<dd4hep::cond::ConditionUpdateCall> callback ) {
  dd4hep::cond::ConditionDependency* dep = new dd4hep::cond::ConditionDependency( de, item, callback );
  return addGlobal( dep );
}

/// Add a new global conditions dependency (Built internally from arguments)
std::pair<dd4hep::Condition::key_type, dd4hep::cond::ConditionDependency*>
LHCb::Detector::ConditionsRepository::addGlobal( dd4hep::cond::ConditionDependency* dep ) {
  dd4hep::ConditionKey::KeyMaker km( dep->target.hash );
  km.values.det_key = 0;
  dep->target.hash  = km.hash;
  return addDependency( dep );
}

/// Clear the container. Destroys the contained stuff
void LHCb::Detector::ConditionsRepository::clear() {
  this->dd4hep::cond::ConditionsContent::clear();
  locations.clear();
}
