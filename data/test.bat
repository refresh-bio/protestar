set EXE="../src/x64/Release/protestar.exe"
set PY=C:\Python38\python.exe

mkdir apsd-pdb-out
%EXE% add -v 3 --type pdb --indir ../apsd-data/pdb --out apsd-pdb.psa
%EXE% get -v 3 --in apsd-pdb.psa --outdir apsd-pdb-out/ --type pdb --all
%PY% cmp.py ./apsd-pdb-out ../apsd-data/pdb

mkdir apsd-cif-out
%EXE% add -v 3 --type cif --indir ../apsd-data/cif --out apsd-cif.psa
%EXE% get -v 3 --in apsd-cif.psa --outdir apsd-cif-out/ --type cif --all
%PY% cmp.py ./apsd-cif-out ../apsd-data/cif

mkdir pdb-out
%EXE% add -v 3 --type pdb --indir ./pdb --out pdb.psa
%EXE% get -v 3 --in pdb.psa --outdir pdb-out/ --type pdb --all
%PY% cmp.py ./pdb-out ./pdb

mkdir cif-out
%EXE% add -v 3 --type cif --indir ../data/cif --out cif.psa
%EXE% get -v 3 --in cif.psa --outdir cif-out/ --type cif --all
%PY% cmp.py ./cif-out ../data/cif/lossless


mkdir nmr-out
%EXE% add -v 3 --type pdb --indir ./nmr --out nmr.psa
%EXE% get -v 3 --in nmr.psa --outdir nmr-out/ --type pdb --all
%PY% cmp.py ./nmr-out ./nmr

mkdir nucleotide-out
%EXE% add -v 3 --type pdb --indir ./nucleotide --out nucleotide.psa
%EXE% get -v 3 --in nucleotide.psa --outdir nucleotide-out/ --type pdb --all
%PY% cmp.py ./nucleotide-out ./nucleotide


mkdir ligands-out
%EXE% add -v 3 --type pdb --indir ./ligands --out ligands.psa
%EXE% get -v 3 --in ligands.psa --outdir ligands-out/ --type pdb --all
%PY% cmp.py ./ligands-out ./ligands

mkdir ligands-minimal-out
%EXE% add -v 3 --type pdb --indir ./ligands --out ligands-minimal.psa --minimal
%EXE% get -v 3 --in ligands-minimal.psa --outdir ligands-minimal-out/ --type pdb --all 
%PY% cmp.py ./ligands-minimal-out ./ligands/minimal

mkdir ligands-lossy-out
%EXE% add -v 3 --type pdb --indir ./ligands --out ligands-lossy.psa --lossy --max-error-bb 100 --max-error-sc 100 
%EXE% get -v 3 --in ligands-lossy.psa --outdir ligands-lossy-out/ --type pdb --all 
%PY% cmp.py ./ligands-lossy-out ./ligands/lossy_100_100


mkdir ligands-out
%EXE% add -v 3 --type cif --indir ./ligands --out ligands.psa
%EXE% get -v 3 --in ligands.psa --outdir ligands-out/ --type cif --all
%PY% cmp.py ./ligands-out ./ligands

mkdir ligands-minimal-out
%EXE% add -v 3 --type cif --indir ./ligands --out ligands-minimal.psa --minimal
%EXE% get -v 3 --in ligands-minimal.psa --outdir ligands-minimal-out/ --type cif --all 
%PY% cmp.py ./ligands-minimal-out ./ligands/minimal

mkdir ligands-lossy-out
%EXE% add -v 3 --type cif --indir ./ligands --out ligands-lossy.psa --lossy --max-error-bb 100 --max-error-sc 100 
%EXE% get -v 3 --in ligands-lossy.psa --outdir ligands-lossy-out/ --type cif --all 
%PY% cmp.py ./ligands-lossy-out ./ligands/lossy_100_100