###############################################################################
# (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################
import os
import sys

from Gaudi.Configuration import *
import GaudiPython as GP
from GaudiConf import IOHelper
from Configurables import LHCbApp, DDDBConf, CondDB
from Configurables import TransportSvc
from Configurables import ApplicationMgr

import optparse

parser = optparse.OptionParser()
(opts, args) = parser.parse_args()

# Phase 1: Configuration
# ==============================================================================
#LHCbApp().OutputLevel = DEBUG
LHCbApp().DDDBtag = "dddb-20200508"
LHCbApp().CondDBtag = "sim-20200515-vc-md100"
LHCbApp().Simulation = True
CondDB().Upgrade = True

# Phase 2: GaudiPython setup
# ==============================================================================
from GaudiPython.Bindings import gbl, AppMgr, Helper
appMgr = GP.AppMgr()
appMgr.initialize()

eventSvc = appMgr.evtsvc()
appMgr.ExtSvc += ['TransportSvc']
transportSvc = appMgr.service('TransportSvc', 'ITransportSvc')
detSvc = appMgr.detSvc()
appMgr.start()

# Phase 3: Now dumpping information from the DetDesc geometry
# ==============================================================================
muon = detSvc[
    '/dd/Structure/LHCb/DownstreamRegion/Muon']  # the Detector Element
geom = muon.geometry()  # The GeometryInfo object
lvol = geom.lvolume()  # The actual logical volume


def processLVolume(lvol):
    print("LVOL: %s" % lvol.name())
    for pvol in lvol.pvolumes():
        print("PVOL: %s" % pvol.name())
        processLVolume(pvol.lvolume())


processLVolume(lvol)
