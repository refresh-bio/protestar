import os
import sys
import math
import functools

datasets = ['mouse']

def compare(sx, sy):
# ATOM   8486  CB  SER A1113      -7.422   0.612   8.990  1.00 93.16           C  
    x_atom = sx[12:16].strip()
    y_atom = sy[12:16].strip()
    x_id = sx[22:26]
    y_id = sy[22:26]
    
    if x_id + '-' + x_atom + '.' < y_id + '-' + y_atom + '.' :
        return -1
    if x_id + '-' + x_atom + '.' > y_id + '-' + y_atom + '.' :
        return 1
    return 0

def get_atom_desc(s):
    return s[12:16].strip() + "*" + s[17:20].strip() + "*" + s[21] + "*" + s[22:26].strip()

def process_ds(ds):
    ref_files = os.listdir(ds + '/orig')

    for tool in os.listdir(ds):
        if tool == 'orig':
            continue

        f_out = open(ds + "-" + tool + ".csv", "w")

        for fn in ref_files:
            print(tool, fn)
            in_ref = open(ds + '/orig/' + fn, "r")
            lines = in_ref.readlines()
            in_ref.close()

            ref_atoms = []
            
            for s in lines:
                if s[:4] == 'ATOM' or s[2:6] == 'ATOM':
                    ref_atoms.append(s.rstrip())

            in_lossy = open(ds + '/' + tool + '/' + fn, 'r')
            lines = in_lossy.readlines()
            in_lossy.close()
            
            lossy_atoms = []
            for s in lines:
                if s[:4] == 'ATOM' or s[2:6] == 'ATOM':
                    lossy_atoms.append(s.rstrip())
        
            if len(ref_atoms) != len(lossy_atoms):
                print("Wrong number of atoms!")
                continue
                
            ref_atoms.sort(key=functools.cmp_to_key(compare))
            lossy_atoms.sort(key=functools.cmp_to_key(compare))
            
#            f_out.write(fn + "\n")
            
            for i in range(len(ref_atoms)):
                x_ref_X = eval(ref_atoms[i][30:38])
                x_ref_Y = eval(ref_atoms[i][38:46])
                x_ref_Z = eval(ref_atoms[i][46:54])

                x_lossy_X = eval(lossy_atoms[i][30:38])
                x_lossy_Y = eval(lossy_atoms[i][38:46])
                x_lossy_Z = eval(lossy_atoms[i][46:54])
                
#                ATOM   8486  CB  SER A1113      -7.422   0.612   8.990  1.00 93.16           C  

#                if ref_atoms[i][11:26] != lossy_atoms[i][11:26]:
                if get_atom_desc(ref_atoms[i]) != get_atom_desc(lossy_atoms[i]):
                    print(ds + "-" + tool + "-" + fn + "  Wrong order of atoms", '*' + ref_atoms[i][11:26] + '*', '*' + lossy_atoms[i][11:26] + '*')
        
                diff = math.sqrt((x_ref_X - x_lossy_X) ** 2 + (x_ref_Y - x_lossy_Y) ** 2 + (x_ref_Z - x_lossy_Z) ** 2)
        
                f_out.write(fn+','+ref_atoms[i][12:16].strip() + ',' + str(diff) + "\n")
                
                if diff > 10:
                    print(tool, fn, i, diff)
            
        f_out.close()

for ds in datasets:
    process_ds(ds)
