###############################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb Collaboration           #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################
INPUT=../DD4hep/examples/DDUpgrade/data/load_LHCb.xml
INPUT=checkout/examples/DDUpgrade/data_FTv64/load_LHCb_FTv64.xml
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output VP.gdml     -volname -path _dd_Structure_LHCb_BeforeMagnetRegion_UT
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output UT.gdml     -volname -path _dd_Structure_LHCb_BeforeMagnetRegion_UT
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output VP.gdml     -volname -path _dd_Structure_LHCb_BeforeMagnetRegion_VP
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output Rich1.gdml  -volname -path _dd_Structure_LHCb_BeforeMagnetRegion_Rich1
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output Rich2.gdml  -volname -path _dd_Structure_LHCb_AfterMagnetRegion_Rich2
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output Ecal.gdml   -volname -path _dd_Structure_LHCb_DownstreamRegion_Ecal
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output Hcal.gdml   -volname -path _dd_Structure_LHCb_DownstreamRegion_Hcal
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output FT.gdml     -volname -path _dd_Structure_LHCb_AfterMagnetRegion_T_FT
geoPluginRun -input file:${INPUT} -ui -destroy -plugin DD4hep_ROOTGDMLExtract -output Muon.gdml   -volname -path _dd_Structure_LHCb_DownStreamRegion_Muon
