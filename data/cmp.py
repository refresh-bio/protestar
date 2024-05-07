import os
import sys
import filecmp
import gzip

if len(sys.argv) != 3:
    print("Usage: cmp.py <check-dir> <ref-dir>")

check_dir = sys.argv[1]
ref_dir = sys.argv[2]

all_agree = True
print("Comparing files...")
for file in os.listdir(check_dir):
    if os.path.isfile(check_dir + "/" + file):

        print(file + " ", end='')

        ref_path = ref_dir + "/" + file
        if os.path.exists(ref_path):
            f1 = open(check_dir + "/" + file, 'rt')
            f2 = open(ref_path, 'rt')

            b1 = f1.read()
            b2 = f2.read()
            agree = (b1 == b2)
            all_agree &= agree
            print(agree)
        elif os.path.exists(ref_path + ".gz"):
            f1 = open(check_dir + "/" + file, 'rt')
            f2 = gzip.open(ref_path + ".gz", 'rt')

            b1 = f1.read()
            b2 = f2.read()
            agree = (b1 == b2)
            all_agree &= agree
            print(agree, "(gz)")
        else:
            all_agree = False
            print("Not found")

if all_agree:
    sys.exit(0)

sys.exit(-1)