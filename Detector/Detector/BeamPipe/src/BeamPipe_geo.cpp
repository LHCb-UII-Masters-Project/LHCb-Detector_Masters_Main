/*****************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"

// static Volume add_vaccuum(Cone c, Material mat, std::string name, Detector &description)
// {
//   Material vaccuum(description.material("Vacuum"));
//   Cone ext_cone(c->GetDz(),
//                 0, c->GetRmax1(),
//                 0, c->GetRmax2());
//   Volume ext_cone_vol(name + "_vaccuum", ext_cone, vaccuum);
//   ext_cone_vol.setVisAttributes(description, "VacuumVis");
//   Volume int_cone_vol(name, c, mat);
//   int_cone_vol.setVisAttributes(description, "BeamPipeVis");
//   Transform3D trb(Position(0.0, 0.0, 0.0));
//   ext_cone_vol.placeVolume(int_cone_vol, trb);
//   return ext_cone_vol;
// }

static dd4hep::Volume add_vaccuum( dd4hep::Cone c, dd4hep::Material mat, std::string name,
                                   dd4hep::Detector& description ) {
  dd4hep::Material vacuum( description.material( "Vacuum" ) );
  dd4hep::Cone     int_cone( c->GetDz(), 0, c->GetRmin1(), 0, c->GetRmin2() );
  dd4hep::Volume   int_cone_vol( name + "_vaccuum", int_cone, vacuum );
  int_cone_vol.setVisAttributes( description, "VacuumVis" );

  dd4hep::Volume ext_cone_vol( name, c, mat );
  ext_cone_vol.setVisAttributes( description, "BeamPipeVis" );
  dd4hep::Transform3D trb( dd4hep::Position( 0.0, 0.0, 0.0 ) );
  ext_cone_vol.placeVolume( int_cone_vol, trb );
  return ext_cone_vol;
}

static dd4hep::Ref_t create_section1( dd4hep::Detector& description ) {
  // Now creating a cone with Vacuum inside
  dd4hep::Material be( description.material( "Be" ) );
  dd4hep::Cone     section1( 400.0, 10.0, 20.0, 100.0, 110.0 );
  dd4hep::Volume   section1_vol = add_vaccuum( section1, be, "section1", description );
  return section1_vol;
}

static dd4hep::Ref_t create_element( dd4hep::Detector& description, xml_h e, dd4hep::Ref_t /* sens */ ) {
  // Preparing the detector element based on the XML received
  xml_det_t          x_det( e );
  int                det_id   = x_det.id();
  std::string        det_name = x_det.nameStr();
  dd4hep::DetElement det( det_name, det_id );
  dd4hep::Volume     mother = description.pickMotherVolume( det );

  // The Beam pipe is n aembly of sections
  // Assembly env_vol(det_name);
  // env_vol.setVisAttributes(description, x_det.visStr());

  // Adding the first section
  auto section1 = create_section1( description );

  xml_dim_t            pos = x_det.position();
  dd4hep::Transform3D  tr( dd4hep::Position( pos.x(), pos.y(), pos.z() ) );
  dd4hep::PlacedVolume pv = mother.placeVolume( section1, tr );
  det.setPlacement( pv );
  return det;
}

DECLARE_DETELEMENT( DD4hep_BeamPipe_detector_v1_0, create_element )
