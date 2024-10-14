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
// Small root script to analyse the VP simulation output from DDG4
//
//==========================================================================
#include "DD4hep/DetElement.h"
#include "DD4hep/Detector.h"
#include "DDG4/Geant4Data.h"
#include "TBranch.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TSystem.h"
#include "TTree.h"
#include <iomanip>
#include <iostream>

void VP_ana_ddg4( const char* fname, const char* hist_file ) {
  static bool first = true;
  if ( first ) {
    // gSystem->Load( "libDDG4IO.so" );
    gSystem->Load( "libDDG4Plugins.so" );
    dd4hep::Detector& d = dd4hep::Detector::getInstance();
    d.fromXML( "file:$DETECTOR_PROJECT_ROOT/compact/components/debug/VP.xml" );
    first = false;
  }
  dd4hep::Detector&         desc    = dd4hep::Detector::getInstance();
  dd4hep::DetElement        ft_det  = desc.detector( "VP" );
  dd4hep::SensitiveDetector ft_sens = desc.sensitiveDetector( "VP" );
  dd4hep::Readout           ft_ro   = ft_sens.readout();
  dd4hep::IDDescriptor      ft_spec = ft_sens.idSpec();

  TFile* f = TFile::Open( fname );
  if ( f->IsZombie() ) {
    cout << "Failed to open properly the input file: " << fname << endl;
    return;
  }
  TTree* t = (TTree*)f->Get( "EVENT" );
  t->Print();
  TBranch* b = t->GetBranch( "VPHits" );
  if ( b ) b->Print();
  vector<dd4hep::sim::Geant4Tracker::Hit*>* hits = new vector<dd4hep::sim::Geant4Tracker::Hit*>();
  b->SetAddress( &hits );
  std::map<unsigned long long, size_t>                           hit_counts, ladder_counts, module_counts, side_counts;
  std::map<unsigned long long, size_t>                           ladder_nums, module_nums, side_nums;
  std::map<unsigned long long, TH2D*>                            module_xy_hists;
  vector<pair<const dd4hep::BitFieldElement*, dd4hep::VolumeID>> flds;
  TH1D hist( "Hits_z", "Hit distribution in z", 2300, -350., 800. );
  for ( int i = 0, n = b->GetEntries(); i < n; ++i ) {
    b->GetEntry( i );
    for ( auto* h : *hits ) {
      hit_counts[h->cellID]++;
      const auto& pos = h->position;
      hist.Fill( pos.Z() );
      ft_spec.decodeFields( h->cellID, flds );
      int mod_num = -1;
      for ( const auto& f : flds ) {
        if ( f.first->name() == "module" ) {
          mod_num = f.second;
          break;
        }
      }
      auto ihist = module_xy_hists.find( mod_num );
      if ( ihist == module_xy_hists.end() ) {
        char name[32], title[128];
        ::snprintf( name, sizeof( name ), "Mod_%02d_XY", mod_num );
        ::snprintf( title, sizeof( title ), "Module %02d: XY", mod_num );
        TH2D* h = new TH2D( name, title, 100, -50., 50., 100, -50., 50. );
        ihist   = module_xy_hists.insert( make_pair( mod_num, h ) ).first;
      }
      ihist->second->Fill( pos.X(), pos.Y(), 1. );
    }
  }
  for ( const auto& e : hit_counts ) {
    ft_spec.decodeFields( e.first, flds );
    cout << "CellID: " << std::hex << std::setw( 8 ) << std::setfill( '0' ) << e.first << std::dec << " ";
    for ( const auto& f : flds ) {
      cout << f.first->name() << " : " << f.second << "  ";
      if ( f.first->name() == "ladder" ) {
        ladder_nums[f.second]++;
        ladder_counts[f.second] += e.second;
      }
      if ( f.first->name() == "module" ) {
        module_nums[f.second]++;
        module_counts[f.second] += e.second;
      }
      if ( f.first->name() == "side" ) {
        side_nums[f.second]++;
        side_counts[f.second] += e.second;
      }
    }
    cout << e.second << endl;
  }
  for ( const auto& e : module_counts ) cout << "Module-count:  " << e.first << " : " << e.second << "  " << endl;
  for ( const auto& e : ladder_counts ) cout << "Layer-count:   " << e.first << " : " << e.second << "  " << endl;
  for ( const auto& e : side_counts ) cout << "Side-count:    " << e.first << " : " << e.second << "  " << endl;

  for ( const auto& e : module_nums ) cout << "Module-num:  " << e.first << " : " << e.second << "  " << endl;
  for ( const auto& e : ladder_nums ) cout << "Layer-num:   " << e.first << " : " << e.second << "  " << endl;
  for ( const auto& e : side_nums ) cout << "Side-num:    " << e.first << " : " << e.second << "  " << endl;
  TPaveStats* p = (TPaveStats*)hist.GetListOfFunctions()->FindObject( "stats" );
  hist.GetListOfFunctions()->Remove( p );
  hist.SetStats( 0 );
  hist.DrawCopy();
  char   hist_file_def[128];
  time_t tim = ::time( 0 );
  ::strftime( hist_file_def, sizeof( hist_file_def ), "VP_histos_%Y-%m-%d_%H_%M_%S.root", ::localtime( &tim ) );
  TFile* hf = TFile::Open( std::strcmp( hist_file, "" ) ? hist_file : hist_file_def, "RECREATE" );
  hist.Write();
  for ( auto& h : module_xy_hists ) h.second->Write();
  hf->Close();
  // gSystem->Exit(0);
}
