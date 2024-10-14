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
"""

   dd4hep simulation example setup using the python configuration

   @author  M.Frank
   @version 1.0

"""

import os, sys, time, DDG4
from DDG4 import OutputLevel as Output
from GaudiKernel.SystemOfUnits import GeV, m


def run():
    kernel = DDG4.Kernel()
    project_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    kernel.loadGeometry(
        os.path.join(project_dir, "compact/components/debug/FT.xml"))
    kernel.setOutputLevel('Geant4Converter', Output.DEBUG)
    kernel.setOutputLevel('ConstructGeometry', Output.DEBUG)

    DDG4.importConstants(kernel.detectorDescription(), debug=False)
    geant4 = DDG4.Geant4(kernel, tracker='Geant4TrackerCombineAction')
    geant4.printDetectors()
    # Configure UI
    if len(sys.argv) > 1:
        geant4.setupCshUI(macro=sys.argv[1])
    else:
        geant4.setupCshUI()

    #geant4.setupUI()
    #import pdb
    #pdb.set_trace()

    # Configure field
    field = geant4.setupTrackingField(prt=True)
    # Configure Event actions
    prt = DDG4.EventAction(kernel, 'Geant4ParticlePrint/ParticlePrint')
    prt.OutputLevel = Output.DEBUG
    prt.OutputType = 3  # Print both: table and tree
    kernel.eventAction().adopt(prt)

    # Configure I/O
    evt_root = geant4.setupROOTOutput('RootOutput',
                                      'FT_' + time.strftime('%Y-%m-%d_%H-%M'))

    # Setup particle gun
    gun = geant4.setupGun(
        "Gun",
        particle='e+',
        energy=100 * GeV,
        multiplicity=1,
        isotrop=True,
        position='(0,0,-300*cm)',
        ThetaMin=0,
        ThetaMax=0.4,
        Standalone=True)
    setattr(gun, 'print', True)
    """
  gen = DDG4.GeneratorAction(kernel,"Geant4InputAction/InputO1");
  gen.Input = "Geant4EventReaderHepMC|../DD4hep/examples/DDG4/data/LHCb_MinBias_HepMC.txt"
  gen.MomentumScale = 1.0
  gen.Mask = 1
  geant4.buildInputStage([gen],output_level=Output.DEBUG)
  """

    #seq,action = geant4.setupTracker('CODEXb')
    seq, action = geant4.setupTracker('FT')
    action.OutputLevel = Output.DEBUG

    # And handle the simulation particles.
    part = DDG4.GeneratorAction(kernel,
                                "Geant4ParticleHandler/ParticleHandler")
    kernel.generatorAction().adopt(part)
    part.OutputLevel = Output.INFO
    part.enableUI()
    user = DDG4.Action(kernel,
                       "Geant4TCUserParticleHandler/UserParticleHandler")
    user.TrackingVolume_Zmax = 999999. * m  # Something big. All is a tracker
    user.TrackingVolume_Rmax = 999999. * m
    user.enableUI()
    part.adopt(user)

    # Now build the physics list:
    ##phys = kernel.physicsList()
    phys = geant4.setupPhysics('QGSP_BERT')
    ph = DDG4.PhysicsList(kernel, 'Geant4PhysicsList/Myphysics')
    ph.addParticleGroup('G4LeptonConstructor')
    ph.addParticleGroup('G4BaryonConstructor')
    ph.addParticleGroup('G4MesonConstructor')
    ph.addParticleGroup('G4BosonConstructor')
    ph.enableUI()
    phys.adopt(ph)
    phys.enableUI()
    phys.dump()
    # run
    kernel.configure()
    seq = kernel.detectorConstruction()
    act = seq.get("ConstructGeometry")
    act.GeoInfoPrintLevel = Output.ERROR
    print('++++   ', seq.name(), act.name(), act.GeoInfoPrintLevel)
    kernel.initialize()
    kernel.run()
    kernel.terminate()


if __name__ == "__main__":
    run()
