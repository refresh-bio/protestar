 #!/usr/bin/env python3
import sys
import pyprotestar
import textwrap
import subprocess
import os
import io
import json
import Bio
import Bio.PDB

print('Compress CIF, PDB, PAE, and CONF files into Protestar archive')
proc = subprocess.Popen(['../bin/protestar', 'add', '--type', 'ALL', '--indir-recursive', '../apsd-data/', '--out', 'archive.psa'])
proc.wait()


print('\nOpen the archive')
archive = pyprotestar.Archive()
if not archive.Open("archive.psa", True):
    print("Error: cannot open Protestar archive")
    sys.exit(1)

print(f'Get numbers of files of different types from the archive.\n'
    f'CIF:{archive.GetNoFilesCIF()}\n'
    f'PDB:{archive.GetNoFilesPDB()}\n'
    f'PAE:{archive.GetNoFilesPAE()}\n'
    f'CONF:{archive.GetNoFilesCONF()}\n')

print(f'\nList all files of different types')
print('CIF:')
for f in archive.ListFilesCIF(): print(f)
print('PDB:')
for f in archive.ListFilesPDB(): print(f)
print('PAE:')
for f in archive.ListFilesPAE(): print(f)
print('CONF:')
for f in archive.ListFilesCONF(): print(f)


print(f'\nGet contents of {archive.ListFilesCIF()[0]}.cif file and print coordinates of the first ten atoms')
content = archive.GetFileCIF(archive.ListFilesCIF()[0])
handle = io.StringIO(content)
cifparser = Bio.PDB.MMCIFParser(QUIET=False) 

structure = cifparser.get_structure("cif-sample", handle)
atoms = [atom for atom in structure.get_atoms()][0:10]
for atom in atoms:
    print(f'{atom.name}: {atom.coord}')


print(f'\nGet contents of {archive.ListFilesPDB()[0]}.pdb file and print coordinates of all Nitrogen atoms')
content = archive.GetFilePDB(archive.ListFilesPDB()[0])
handle = io.StringIO(content)
pdbparser = Bio.PDB.PDBParser(QUIET=False) 

structure = pdbparser.get_structure("pdb-sample", handle)
atoms = [atom for atom in structure.get_atoms() if atom.name == 'N']
for atom in atoms:
    print(f'{atom.name}: {atom.coord}')
   
print(f'Get contents of {archive.ListFilesPAE()[0]}.json and {archive.ListFilesCONF()[0]}.json files as Python JSONs')
pae = json.loads(archive.GetFilePAE(archive.ListFilesPAE()[0])) 
conf = json.loads(archive.GetFileCONF(archive.ListFilesCONF()[0])) 

os.remove('archive.psa')
