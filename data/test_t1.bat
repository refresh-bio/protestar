set EXE="../src/x64/Release/protestar.exe"
set PY=python.exe

mkdir apsd-pdb-out
%EXE% add -v 3 --type pdb --indir ../apsd-data/pdb --out apsd-pdb.psa -t 1
%EXE% get -v 3 --in apsd-pdb.psa --outdir apsd-pdb-out/ --type pdb --all  -t 1
%PY% cmp.py ./apsd-pdb-out ../apsd-data/pdb 

mkdir apsd-cif-out
%EXE% add -v 3 --type cif --indir ../apsd-data/cif --out apsd-cif.psa  -t 1
%EXE% get -v 3 --in apsd-cif.psa --outdir apsd-cif-out/ --type cif --all  -t 1
%PY% cmp.py ./apsd-cif-out ../apsd-data/cif

mkdir pdb-out
%EXE% add -v 3 --type pdb --indir ./pdb --out pdb.psa  -t 1
%EXE% get -v 3 --in pdb.psa --outdir pdb-out/ --type pdb --all  -t 1
%PY% cmp.py ./pdb-out ./pdb

mkdir cif-out
%EXE% add -v 3 --type cif --indir ../data/cif --out cif.psa  -t 1
%EXE% get -v 3 --in cif.psa --outdir cif-out/ --type cif --all  -t 1
%PY% cmp.py ./cif-out ../data/cif

mkdir ligands-out
%EXE% add -v 3 --type pdb --indir ./ligands --out ligands.psa  -t 1
%EXE% get -v 3 --in ligands.psa --outdir ligands-out/ --type pdb --all  -t 1
%PY% cmp.py ./ligands-out ./ligands

mkdir nmr-out
%EXE% add -v 3 --type pdb --indir ./nmr --out nmr.psa  -t 1
%EXE% get -v 3 --in nmr.psa --outdir nmr-out/ --type pdb --all  -t 1
%PY% cmp.py ./nmr-out ./nmr

mkdir nucleotide-out
%EXE% add -v 3 --type pdb --indir ./nucleotide --out nucleotide.psa  -t 1
%EXE% get -v 3 --in nucleotide.psa --outdir nucleotide-out/ --type pdb --all  -t 1
%PY% cmp.py ./nucleotide-out ./nucleotide