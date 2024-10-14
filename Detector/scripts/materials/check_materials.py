###############################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################
from collections import defaultdict
from typing import Any, List, Mapping
from pprint import pprint
from xml.etree import ElementTree as ET

import argparse
import os
import logging
import sys

class BaseMaterial:
    def __init__(self, definition):
        self.definition = definition 

    def name(self):
        return self.definition[self.type()]["name"]

    def _has_value_prop(self):
        if "value" in self.definition[self.type()].keys():
            return True
        return False

    def __str__(self):
        tmp = ', '.join( [ str(a) + "=" + str(b) for a, b in sorted(self.definition[self.type()].items())]) 
        mystr = f"{self.type()}({tmp})."
        props = self.definition['properties']
        tmpprops = []
        for (title, content) in props:
            tmpstr = ', '.join( [ str(a) + ":" + str(b) for a, b in sorted(content.items())])
            tmpprops.append(f"{title}({tmpstr})")
        mystr += ",".join(tmpprops)
        return mystr

    def __repr__(self):
        return self.definition.__repr__()

    def __eq__(self, other):
        if not isinstance(other, BaseMaterial):
            return NotImplemented
        return self.definition == other.definition

    def __hash__(self):
        return self.__str__().__hash__()

    def is_definition(self):
        ''' Return True if this node is a proper material definition '''
        return True

class Material(BaseMaterial):
    ''' Base class for a material, represented by:
    <material name="Vacuum">  
      <D type="density" unit="g/cm3" value="0.0000000001" />
      <fraction n="0.754" ref="N"/>
      <fraction n="0.234" ref="O"/>
      <fraction n="0.012" ref="Ar"/>
    </material>

    or 

    <material name="Water">
      <D type="density" unit="g/cm3" value="1.0"/>
      <composite ref="H" n="2"/>
      <composite ref="O" n="1"/>
    </material>q
    
    '''
    def __init__(self, definition):
        super().__init__(definition)

    def type(self):
        return "material"

    def is_definition(self):
        ''' Return True if this node is a proper material definition 
        We ignore such tags which correspond to a material being used
        <materials>
            <material name="yoke"   value="MBXW:YokeIron"/>
            <material name="coil"   value="MBXW:CoilCopper"/>
            <material name="spacer" value="MBXW:G10"/>
        </materials>
        '''
        if self._has_value_prop():
            return False
        return True


class Element(BaseMaterial):
    ''' Base class for a material, represented e.g. by:
     <element Z="96" formula="Cm" name="Cm" >
        <atom type="A" unit="g/mol" value="247.07" />
    </element> '''

    def __init__(self, definition):
        super().__init__(definition)

    def type(self):
        return "element"

class Isotope(BaseMaterial):
    ''' Base class for an isotope, represented e.g. by:
    <isotope N="1" Z="1" name="H1">
      <atom unit="g/mole" value="1.00782503081372"/>
    </isotope>

    '''
    def __init__(self, definition):
        super().__init__(definition)

    def type(self):
        return "isotope"

def list_compact_files(path: str) -> List[str]:
    ''' Find all XML files in a directory containing a DD4hep compact description '''
    xmlfiles = []
    for dirpath, dirs, files in os.walk(path):	 
        xmlfiles += [ os.path.join(dirpath, f) for f in files if f .endswith(".xml") or f.endswith(".gdml")]
    return xmlfiles
    
def find_materials_in_file(filename: str) ->  List:
    ''' Find all <material> or <element> tags in a XML file  '''
    tree = ET.parse(filename)
    root = tree.getroot()
    materials = []

    def _process_element(p):
        edata=dict()
        edata[p.tag] = {key: value[:] for key, value in p.items()}
        properties = []
        for child in p:
            properties.append((child.tag, {key: value[:] for key, value in child.items()}))
        edata["properties"] = properties
        if p.tag == "material":
            return Material(edata)
        elif p.tag == "element":
            return Element(edata)
        elif p.tag == "isotope":
            return Isotope(edata)
        else:
            raise Exception("Unknown tag")

    for p in root.findall('.//material'):
        d = _process_element(p)
        logging.debug(f"material {filename} -> {d.name()}")
        if d.is_definition():
            materials.append(d)
    for p in root.findall('.//element'):
        d = _process_element(p)
        logging.debug(f"element {filename} -> {d.name()}")
        materials.append(d)
    for p in root.findall('.//isotope'):
        d = _process_element(p)
        logging.debug(f"element {filename} -> {d.name()}")
        materials.append(d)
    return materials

def find_materials_in_path(path: str) -> Mapping[str, List]:
    ''' Find all <material> or <element> tags in a compact subdirectory and
    returns a map filename -> lag list '''
    allmaterials = {}
    xmlfiles = list_compact_files(path)
    for x in xmlfiles:
        try:
            logging.debug(f"Processing file {x}")
            m = find_materials_in_file(x)
            if len(m) > 0:
                allmaterials[x] = m
        except Exception as e:
            logging.error(f"Error processing {x}: {str(e)}")

    return allmaterials

def check_materials_prefixes(materials_map, prefix_sep=":"):
    ''' Check whether the elements in the file have prefixes '''
    prefix_map = dict()
    get_prefix = lambda n: n.split(prefix_sep)[0] if prefix_sep in n else None
    for file, entries in materials_map.items():
        prefix_counts = defaultdict(int)
        for e in entries:
            logging.debug(f"{file} -> {e.name()}")
            prefix = get_prefix(e.name())
            prefix_counts[prefix] += 1
        prefix_map[file] = prefix_counts
    return prefix_map

def check_materials_consistency(materials_map: Mapping[str, List]):
    ''' Check the materials defined in a materials_map
    as returned by find_materials_in_path '''
    elements = defaultdict(list)
    materials = defaultdict(list)
    isotopes = defaultdict(list)
    elements_definitions = dict()
    materials_definitions = dict()
    isotopes_definitions = dict()
    files_with_materials = set()

    for file, entries in materials_map.items():
        def_count = 0
        for e in entries:
            if type(e) is Material:
                if e.is_definition():                    
                    materials[e.name()].append(file)
                    materials_definitions[(e.name(),file)] = e
                    def_count += 1
            elif type(e) is Element:
                elements[e.name()].append(file)
                elements_definitions[(e.name(),file)] = e
                def_count += 1
            elif type(e) is Isotope:
                isotopes[e.name()].append(file)
                isotopes_definitions[(e.name(),file)] = e
                def_count += 1
            else:
                raise Exception(f"Unknown material type: {e}")
        if def_count > 0:
            files_with_materials.add(file)

    print("=== Files containing material definitions")
    pprint(files_with_materials)

    print("\n=== Checking whether material names are prefixed by <namespace>:")
    prefix_map = check_materials_prefixes(materials_map)
    for file, prefix_counts in prefix_map.items():
        print(f"{file}: {dict(prefix_counts)}")

    print("\n=== Duplicate isotopes definition")
    problematic_isotopes = {m: l for m,l in isotopes.items() if len(l) > 1}
    pprint(problematic_isotopes)

    print("\n=== Duplicate elements definition")
    problematic_elements = {m: l for m,l in elements.items() if len(l) > 1}
    pprint(problematic_elements)
    
    print("\n     Comparing duplicate elements representations:")
    diff_elements = []
    ident_elements = []
    def compare_pairs(name, filelist, map):
        res = [(a, b) for idx, a in enumerate(filelist) for b in filelist[idx + 1:]]
        for a,b in res:
            if map[(name, a)] == map[(name, b)]:
                ident_elements.append((name, a,b))
            else:
                diff_elements.append((name, a,b))


        
    pelist = list(problematic_elements.items())
    for pe in pelist:
        compare_pairs(*pe, elements_definitions)

    print("\n\tdifferent elements:")
    for t in diff_elements:
        print(f"\t{t}")
    print("\n\tidentical elements:")
    for t in ident_elements:
        print(f"\t{t}")
       
    # print("\n=== materials definition")
    # pprint(dict(materials))

    print("\n=== Duplicated materials definition")
    problematic_materials = {m: l for m,l in materials.items() if len(l) > 1}
    pprint(problematic_materials)

    print("\n     Comparing duplicate materials representations:")
    diff_materials = []
    ident_materials = []
    def compare_pairs(name, filelist, map):
        res = [(a, b) for idx, a in enumerate(filelist) for b in filelist[idx + 1:]]
        for a,b in res:
            if map[(name, a)] == map[(name, b)]:
                ident_materials.append((name, a,b))
            else:
                diff_materials.append((name, a,b))


        
    pelist = list(problematic_materials.items())
    for pe in pelist:
        compare_pairs(*pe, materials_definitions)

    print("\n\tdifferent materials:")
    if len(diff_materials) > 0:
        for t in diff_materials:
            print(f"\t{t}")
        print(f"\n\tto compare them run e.g. python {sys.argv[0]} compare {diff_materials[0][1]} {diff_materials[0][2]}")

    else:
        print("\tNone")

    print("\n\tidentical materials:")
    if len(ident_materials) > 0:
        for t in ident_materials:
            print(f"\t{t}")
    else:
        print("\tNone")


def check():
    parser = argparse.ArgumentParser(description='Check the materials in a compact description.')
    parser.add_argument('command', help='command to run')
    parser.add_argument('path', help='path to check')
    args = parser.parse_args()
    logging.debug(f"Processing path {args.path}")
    allmaterials = find_materials_in_path(args.path)
    logging.debug(f"Now checking the consistency of materials definition")
    check_materials_consistency(allmaterials)

def compare_files():
    parser = argparse.ArgumentParser(description='Check the materials in a compact description.')
    parser.add_argument('command', help='command to run')
    parser.add_argument('file1', help='first file to check')
    parser.add_argument('file2', help='second file to check')
    args = parser.parse_args()
    
    m1 = find_materials_in_file(args.file1)
    m2 = find_materials_in_file(args.file2)
    d1 = { m.name():m for m in m1}
    d2 = { m.name():m for m in m2}
    n1 = set([m.name() for m in m1])
    n2 = set([m.name() for m in m2])

    print(f"Definitions only in {args.file1}:")
    pprint(n1  - n2)
    print(f"\n\nDefinitions only in {args.file2}:")
    pprint(n2  - n1)
    print(f"\n\nDefinitions in common:")
    pprint(n2.intersection(n1))
    for n in n2.intersection(n1):
        same = d1[n]== d2[n]
        print(f"\n{n} identical:{same}")
        if not same:
            print(f"{args.file1}:{d1[n]}")
            print(f"{args.file2}:{d2[n]}")
    args = parser.parse_args()
   
if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    if len(sys.argv) < 2:
        print("Please specify command")
    else:
        command = sys.argv[1].lower()
        if command == "check":
            check()
        elif command == "compare":
            compare_files()
        else:
            print(f"Unknown command: {command}")
    