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
// Small root script to analyse the FT simulation output from DDG4
//
//==========================================================================
#include "DD4hep/DetElement.h"
#include "DD4hep/Detector.h"
#include "DDG4/Geant4Data.h"
#include "TBranch.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"
#include <iomanip>
#include <iostream>

void FT_ana_ddg4( const char* fname ) {
  static bool first = true;
  if ( first ) {
    gSystem->Load( "libDDG4IO.so" );
    gSystem->Load( "libDDG4Plugins.so" );
    dd4hep::Detector& d = Detector::getInstance();
    d.fromXML( "file:compact/components/debug/FT.xml" );
    first = false;
  }
  dd4hep::Detector&         desc    = Detector::getInstance();
  dd4hep::DetElement        ft_det  = desc.detector( "FT" );
  dd4hep::SensitiveDetector ft_sens = desc.sensitiveDetector( "FT" );
  Readout                   ft_ro   = ft_sens.readout();
  IDDescriptor              ft_spec = ft_sens.idSpec();

  TFile* f = TFile::Open( fname );
  if ( f->IsZombie() ) {
    cout << "Failed to open properly the input file: " << fname << endl;
    return;
  }
  TTree* t = (TTree*)f->Get( "EVENT" );
  t->Print();
  TBranch* b = t->GetBranch( "FTHits" );
  if ( b ) b->Print();
  vector<dd4hep::sim::Geant4Tracker::Hit*>* hits = new vector<dd4hep::sim::Geant4Tracker::Hit*>();
  b->SetAddress( &hits );
  std::map<unsigned long long, size_t>           hit_counts, layer_counts, station_counts;
  vector<pair<const BitFieldElement*, VolumeID>> flds;

  for ( int i = 0, n = b->GetEntries(); i < n; ++i ) {
    b->GetEntry( i );
    for ( auto* h : *hits ) { hit_counts[h->cellID]++; }
  }
  for ( const auto& e : hit_counts ) {
    ft_spec.decodeFields( e.first, flds );
    cout << "CellID: " << std::hex << std::setw( 8 ) << std::setfill( '0' ) << e.first << std::dec << " ";
    for ( const auto& f : flds ) {
      cout << f.first->name() << " : " << f.second << "  ";
      if ( f.first->name() == "layer" ) layer_counts[f.second]++;
      if ( f.first->name() == "station" ) station_counts[f.second]++;
    }
    cout << e.second << endl;
  }
  for ( const auto& e : station_counts ) cout << "Station-count: " << e.first << " : " << e.second << "  " << endl;
  for ( const auto& e : layer_counts ) cout << "Layer-count:   " << e.first << " : " << e.second << "  " << endl;
  gSystem->Exit( 0 );
}
