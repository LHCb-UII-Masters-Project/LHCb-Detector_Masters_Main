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
"""
Code to make the link map in ReadoutMap.xml for the FT
"""
from collections import OrderedDict
import cppyy
import GaudiPython
FTChannelID = cppyy.gbl.LHCb.Detector.FTChannelID
FTSourceID = cppyy.gbl.LHCb.Detector.FTSourceID


#Input file and lines
def readPCI():
    fileAddress = "../pcie40s.csv"
    inFile = open(fileAddress)
    if not inFile:
        print("ERROR: Could not open the fibers DB in " + fileAddress)
        exit()
    lines = inFile.readlines()
    columnNames = [l.replace('\n', '') for l in lines[0].split(",")]
    allCards = [[k.replace('\n', '') for k in l.split(',')] for l in lines[1:]
                if (l.find('SATEL') != -1 or l.find('SCTEL') != -1)]
    print("INFO: Found", len(allCards), "activated cards in FT")
    return [{
        'name': l[0],
        'place': l[1],
        'nLinksPart0': int(l[3]),
        'SourceIDPart0': int(l[4]),
        'nLinksPart1': int(l[5]),
        'SourceIDPart1': int(l[6]) if l[6] != '' else 0,
        'LinksBank0': [FTChannelID.kInvalidChannelID()] * int(l[3]),
        'LinksBank1': [FTChannelID.kInvalidChannelID()] * int(l[5]),
    } for l in allCards]


tell40s = readPCI()


def readFibers():
    fileAddress = '../fibers.csv'
    inFile = open(fileAddress)
    if not inFile:
        print("ERROR: Could not open the fibers DB in " + fileAddress)
        exit()
    lines = inFile.readlines()
    columnNames = [l.replace('\n', '') for l in lines[0].split(",")]
    allCards = [[k.replace('\n', '') for k in l.split(',')] for l in lines[1:]]
    nLinks = 24

    #Treat the lines
    def treat(l):
        station = int(l[2][l[2].find('T') + 1:l[2].find('L')])
        layer = int(l[2][l[2].find('L') + 1:l[2].find('Q')])
        quarter = int(l[2][l[2].find('Q') + 1:l[2].find('M')])
        module = int(l[2][l[2].find('M') + 1:l[2].find('D')])
        iSipm = int(l[2][l[2].find('D') + 1:])
        #SourceID
        dataLink = [0, 1, 2][int(l[3][l[3].find('_') + 2:]) - 1]
        port = int(l[1]) >= nLinks
        sourceID = FTSourceID(
            FTChannelID.StationID(station), FTChannelID.LayerID(layer),
            FTChannelID.QuarterID(quarter), dataLink, port)
        #ChannelID
        siPMID = iSipm % 4
        matID = int(iSipm / 4)
        if sourceID.isReversedInDecoding():
            siPMID = 3 - (iSipm % 4)
            matID = 3 - int(iSipm / 4)
        chan = FTChannelID(
            FTChannelID.StationID(station), FTChannelID.LayerID(layer),
            FTChannelID.QuarterID(quarter), FTChannelID.ModuleID(module),
            FTChannelID.MatID(matID), siPMID, 0)

        return {
            'name': l[0],
            'iCard': int(l[0].replace("SATEL", "").replace("SCTEL", "")),
            'SourceID': sourceID,
            'place': l[3],
            'iLink': int(l[1]),
            'ID': chan.channelID(),
        }

    allTells = [
        treat(l) for l in allCards
        if (l[0].find('SATEL') != -1 or l[0].find('SCTEL') != -1)
    ]
    print("INFO: Found", len(allTells), "activated links in FT")
    return allTells


links = readFibers()
#Make sure we got everyone
for station in [1, 2, 3]:
    nmods = 6 if station == 3 else 5
    for layer in range(4):
        for quarter in range(4):
            for module in range(nmods):
                for mat in range(4):
                    for sipm in range(4):
                        link = FTChannelID(
                            FTChannelID.StationID(station),
                            FTChannelID.LayerID(layer),
                            FTChannelID.QuarterID(quarter),
                            FTChannelID.ModuleID(module),
                            FTChannelID.MatID(mat), sipm, 0)
                        found = False
                        for otherLink in links:
                            if otherLink['ID'] == link.channelID():
                                found = True
                                break
                        if not found:
                            print("ERROR: Link", link.toString(), "not found")

#Check the consistency and fill the banks
for tell40 in tell40s:
    for link in links:
        if link['name'] == tell40['name']:
            if link['place'] != tell40['place']:
                print("ERROR! For card " + tell40['name'] + " in " +
                      tell40['place'] +
                      ", there is a link in a different place: " +
                      link['place'])
                exit()
            if link['SourceID'] == tell40['SourceIDPart0']:
                tell40['LinksBank0'][link['iLink']] = link['ID']
            elif link['SourceID'] == tell40['SourceIDPart1']:
                tell40['LinksBank1'][link['iLink'] -
                                     tell40['nLinksPart0']] = link['ID']
            else:
                print("ERROR! For card " + bank['name'] + " in " +
                      bank['place'] + ", the source ID " +
                      str(link['SourceID']) +
                      " is not contained in the possible TELL source IDs, " +
                      str(bank['SourceIDPart0']) + " and " +
                      str(bank['SourceIDPart1']))
                exit()
    #Check that for T1 and T2, each bank only contains one module
    #    if tell40['SourceIDPart0'].station() != 3:

print(
    "INFO: Consistency between fibers.csv, pcie40s.csv, and source ID encoding has been checked."
)

#Create the ordered banks
banks = [(t['SourceIDPart0'], t['LinksBank0'], t['name'], t['place'],
          t['name'].find('SATEL') != -1) for t in tell40s]
banks += [(t['SourceIDPart1'], t['LinksBank1'], t['name'], t['place'],
           t['name'].find('SATEL') != -1) for t in tell40s
          if t['nLinksPart1'] != 0]
orderedBanks = sorted(banks)
nBanks = len(orderedBanks)
#Check the ordering of the links
strongOrdering = True  #order between banks
weakOrdering = True  #order between TELL40s
for i in range(nBanks):
    for j in range(i + 1, nBanks):
        bankA = orderedBanks[i]
        bankB = orderedBanks[j]
        if bankA[4] != bankB[4]:
            continue
        for linkA in bankA[1]:
            if linkA == FTChannelID.kInvalidChannelID():
                continue
            linksBelow = ""
            for linkB in bankB[1]:
                if linkB == FTChannelID.kInvalidChannelID():
                    continue
                if linkA == linkB:
                    print("WTF", linkA, linkB)
                if linkA > linkB:
                    linksBelow += str(FTChannelID(linkB).toString()) + " "
            if linksBelow != "":
                print("ERROR: Link", FTChannelID(linkA).toString(), "in bank",
                      i, "(", bankA[3],
                      ") has the following links below it in bank", j, "(",
                      bankB[3], ")", ":", linksBelow)
                strongOrdering = False
                if bankA[2] != bankB[2]:
                    weakOrdering = False

print("INFO: Checked the strong ordering:", strongOrdering)
print("INFO: Checked the weak ordering:", weakOrdering)

# Now do the printing
print("INFO: Bank vector")
lineToPrint = ""
for i in range(len(orderedBanks)):
    if (i % 10 == 0):
        print(lineToPrint)
        lineToPrint = ""
    lineToPrint += str(orderedBanks[i][0]) + " "
print(lineToPrint)

print("INFO: Link map")
for i in range(len(orderedBanks)):
    lineToPrint = ""
    for l in orderedBanks[i][1]:
        lineToPrint += str(l) + " "
    print(lineToPrint)
