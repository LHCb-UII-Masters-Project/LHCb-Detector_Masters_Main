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
//
void plot_module( TFile* file ) {
  // TFile *file = TFile::Open("VP_histos_2019-01-31_09_50_01.root");
  TH2D* mod0    = (TH2D*)file->Get( "Mod_00_XY" );
  TH2D* mod1    = (TH2D*)file->Get( "Mod_01_XY" );
  mod0          = (TH2D*)file->Get( "Hits_z" );
  TPaveStats* p = (TPaveStats*)mod0->GetListOfFunctions()->FindObject( "stats" );
  mod0->GetListOfFunctions()->Remove( p );
  mod0->SetStats( 0 );
  mod0->SetTitle( "" );
  // mod0->Add(mod1, 1.0);
  mod0->DrawCopy( "Colz" );
}
