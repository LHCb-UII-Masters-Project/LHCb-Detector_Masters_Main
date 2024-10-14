#!/usr/bin/env python
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

import ROOT
import dd4hep
import DDRec
from dd4hep import units

description = dd4hep.Detector.getInstance()
description.fromXML('compact/components/debug/FT.xml')
material_manager = DDRec.MaterialManager(description.world().volume())

materials = material_manager.materialsBetween(
    ROOT.Vector3D(0 * units.m, 0 * units.m, 0 * units.m),
    ROOT.Vector3D(5 * units.m, 5 * units.m, 10 * units.m),
)
total_length = 0
total_rad_length = 0
total_int_length = 0
for material, length in materials:
    total_length += length
    total_rad_length += length / material.radLength()
    total_int_length += length / material.intLength()
    print('Found', length * units.mm, 'mm of', material.name(),
          '(rad length =', material.radLength(), ')')

averaged_material = material_manager.createAveragedMaterial(materials)
print(total_length, total_rad_length, total_int_length)
print(total_length / averaged_material.radiationLength(), 'X0')
print(total_length / averaged_material.interactionLength(), 'lambda')
