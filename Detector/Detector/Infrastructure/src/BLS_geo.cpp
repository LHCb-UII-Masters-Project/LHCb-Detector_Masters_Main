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
// Specialized generic detector constructor
//
//==========================================================================

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "XML/Utilities.h"

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::Ref_t sens_det ) {
  xml_det_t                         x_det = e;
  dd4hep::xml::tools::VolumeBuilder builder( description, x_det, sens_det );
  dd4hep::DetElement                bls_det = builder.detector;
  dd4hep::Assembly                  bls_vol( std::string( "lv" ) + bls_det.name() );

  builder.buildTransformations( e );
  builder.buildVolumes( e );
  builder.placeDaughters( bls_det, bls_vol, e );
  /// Now we have to attach the volume ids for the sensitive volumes for DDG4
  const auto& children = bls_det.children();
  for ( const auto& c : children ) {
    dd4hep::PlacedVolume pv = c.second.placement();
    pv.addPhysVolID( "module", c.second.id() );
    for ( Int_t idau = 0, ndau = pv->GetNdaughters(); idau < ndau; ++idau ) {
      dd4hep::PlacedVolume p( pv->GetDaughter( idau ) );
      if ( p.volIDs().empty() && p.volume().isSensitive() ) { p.addPhysVolID( "sensor", idau ); }
    }
  }
  dd4hep::PlacedVolume pv = builder.placeDetector( bls_vol );
  pv.addPhysVolID( "system", x_det.id() );
  return bls_det;
}
// first argument is the type from the xml file
DECLARE_DETELEMENT( LHCb_Bls_v1_0, create_element )
