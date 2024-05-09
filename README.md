# Protein Structures Archiver


[![GitHub Actions CI](../../workflows/GitHub%20Actions%20CI/badge.svg)](../../actions/workflows/main.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) 

![x86-64](https://img.shields.io/static/v1?label=%E2%80%8B&message=x86-64&color=yellow&logo=PCGamingWiki&logoColor=white)
![Apple M1](https://img.shields.io/static/v1?label=%E2%80%8B&message=Apple%20M1&color=yellow&logo=Apple&logoColor=white)
![Windows](https://img.shields.io/badge/%E2%80%8B-Windows-00A98F?logo=windows)
![Linux](https://img.shields.io/static/v1?label=%E2%80%8B&message=Linux&color=00A98F&logo=linux&logoColor=white)
![macOS](https://img.shields.io/badge/%E2%80%8B-macOS-00A98F?logo=apple)

Protein Structures Archiver (ProteStAr) is a tool designed to compress collections of files describing protein structures.
In the current version it supports PDB, mmCIF, PAE (prediction aligned errors in JSON format), confidence (in JSON format).

The tool offers high compression ratios (more than 4 times better than gzip) and fast random access queries.
For example the whole [ESM Atlas](https://github.com/facebookresearch/esm/tree/main/scripts/atlas) v0 database with ~600M protein structures of raw size 67.7 TB (15.5 TB when gzipped) can be stored in 3.78 TB.
Moreover, when we turn on one of lossy modes this drops to 1.59 TB.
The compression of the whole dataset took about 30 hours using 16 thread on workstation equipped with AMD TR 3995WX CPU.

## Quick start
```bash
git clone --recurse-submodules https://github.com/refresh-bio/protestar
cd protestar && make -j

# Compress a collection of 10 PDB files in a single directory --- lossless mode
./bin/protestar add --type pdb --indir apsd-data/pdb/ --out test_pdb.psa

# Compress a collection of 10 CIF files in a single directory --- lossless mode
./bin/protestar add --type cif --indir apsd-data/cif/ --out test_cif.psa

# Compress a collection of 10 PAE files in a single directory --- lossless mode
./bin/protestar add --type pae --indir apsd-data/pae/ --out test_pae.psa

# Compress a collection of all files in a single directory (traversed recursively) --- lossless mode
./bin/protestar add --type ALL --indir-recursive apsd-data/ --out test_all.psa

# Compress a collection of 10 PDB files in a single directory --- lossy and minimal mode 
./bin/protestar add --type pdb --indir apsd-data/pdb/ --out test_pdb_10_100.psa --minimal --lossy --max-error-bb 10 --max-error-sc 100

# Extend a collection of previously compressed data by adding 10 PAE files in lossy mode (level 2)
./bin/protestar add --type pae --indir apsd-data/pae/ --in test_pdb_10_100.psa --out test_mixed.psa --lossy --pae-lossy-level 2

# List contents of the archive (all file types)
./bin/protestar list --in test_mixed.psa --type ALL

# Extract a single file from the archive to the current directory
./bin/protestar get --in test_mixed.psa --outdir ./ --type ALL --file AF-A0A075B6I1-F1-model_v4

# Extract all PAE files from the archive
./bin/protestar get --in test_mixed.psa --outdir ./ --type pae --all

# Show some info about the archive
./bin/protestar info --in test_mixed.psa
```

## Instalation and configuration
ProteStAr should be downloaded from [https://github.com/refresh-bio/protestar](https://github.com/refresh-bio/protestart) and compiled. The supported OS are:
* Windows: Visual Studio 2022 solution provided,
* Linux: make project (G++ 9.0 or newer required).

Support for MacOS and well as ARM-based CPUs will be added soon.

## Version history
* 1.1.0 (8 May 2024)
  * Support of ANISOU, SIGATM, and SIGUIJ sections in PDB files.
  * Some fixes in the PDB and CIF output formatting.
  * Support of MacOS (M1 and x64 architectures).
* 1.0.0 (8 December 2023)
  * *pyprotestar* Python package added,
  * fixed incorrect alignment of ATOM column in some PDB files. 
* 0.7 (20 July 2023)
  * first public release.

## Usage
`protestar <command> <options>`

Command:
* `add` &ndash; add files to archive
* `get` &ndash; get files from archive
* `list` &ndash; list archive contents
* `info` - show some statistics of the archive

### Creating new archive or appending to existing archive
` protestar add <options>`

Options:
* `--type <string>` &ndash; file type: cif, pdb, pae, conf, ALL
* `--in <string>` &ndash; name of input archive (if you want to extend an existing archive)
* `--out <string>` &ndash; name of output archive
* `--infile <string>` &ndash; file name to add
* `--indir <string>` &ndash; directory with files to add
* `--indir-recursive <string>` &ndash; directory (recursive) with files to add
* `--inlist <string>` &ndash; name of file with paths to files to add
* `--intar <string>` &ndash; name of tar file with files to add
* `-t|--threads <int>` &ndash; no. of threads
* `--fast` &ndash; slightly faster compression but slightly worse ratio (only for CIF|PDB files)
* `--minimal` &ndash; minimal mode (only most important fields from CIF|PDB files are stored)
* `--lossy` &ndash; turn-on lossy compression (only for CIF|PDB|PAE files)
* `--max-error-bb` &ndash; max error (in mA [0, 500]) of backbone atom coordinates (only for CIF|PDB files)
* `--max-error-sc` &ndash; max error (in mA [0, 500]) of side-chain atom coordinates (only for CIF|PDB files)
* `--pae-lossy-level` &ndash; lossy level from range [0, 4] (only for PAE files)
* `--single-bf` &ndash; enable single B-factor value (only for CIF|PDB files)
* `-v|--verbose <int>` &ndash; verbosity level

### Extracting selected or all files
`protestar get <options>`

Options:
* `--type <string>` &ndash; file type: cif, pdb, pae, conf, ALL
* `--in <string>` &ndash; name of input archive
* `--outdir <string>` &ndash; output directory
* `--file <string>` &ndash; file name to get
* `--list <string>` &ndash; name of file with file names to get
* `--all` &ndash; get all files
* `-t|--threads <int>` &ndash; no. of threads
* `-v|--verbose <int>` &ndash; verbosity level

### Listing archive contents
`protestar list <options>`

Options:
* `--in <string>` &ndash; name of input archive
* `--type <string>` &ndash; file type: cif, pdb, pae, conf, ALL
* `--show-file-info` &ndash; show some information about file types

### Showing some info about archive
`protestar info <options>`

Options:
* `--in <string>` &ndash; name of input archive

## ProteStAr decompression library
ProteStAr files can be accessed also with C++ library. 
Python library will be available soon.

### C++ library
THe C++ API is provided in `src/lib-cxx/protestar-api.h` file. 
You can also take a look at `src/example_api` to see the API in use.

### Python package
ProteStAr archives can be accessed through *pyprotestar* Python package. The package has to be compiled separately:
```
make -j pyprotestar
```
As a result, a library named like `pyprotestar.cpython-38-x86_64-linux-gnu.so` will be created in the `pyprotestar` directory. To make the package visible in Python, go to this directory and extend the `PYTHONPATH` environment variable with the following commands:
```
cd pyprotestar
source set_path.sh
```
After that, *pyprotestar* package can be imported in a Python script:
```
import pyprotestar
```
In the current directory one can find an example script named [pyprotestar_test.py](./pyprotestar/pyprotestar_test.py) which compresses a set of CIF, PDB, PAE, and CONF files using a regular ProteStAr binary (it assumes it was previously built with `make -j` command and is available in `../bin/` subdirectory) and then accesses the resulting archive using *pyprotestar*. A single file of each type is extracted from the archive and given as an input to the appropriate parser. In particular, [Bio.PDB package](https://biopython.org/docs/1.75/api/Bio.PDB.html) from Biopython is used for CIF/PDB, while PAE and CONF files are parsed using regular JSON library. Therefore, Biopython has to be installed prior running the script:
```
pip install biopython
python3 pyprotestar_test.py
```

## Test data
The data in apsd-data were selected from [AlphaFold Protein Structure Database](https://alphafold.ebi.ac.uk/) to allow quick experiments of the tool.

## Datasets used in experiments
* The full datasets used in the experiments were taken from [AlphaFold Protein Structure Database](https://alphafold.ebi.ac.uk/download) and [ESM Atlas](https://github.com/facebookresearch/esm/tree/main/scripts/atlas).
* The subset of ESM Atlas used in the experiments can be downloaded from [ESM subset](https://polslpl-my.sharepoint.com/:u:/g/personal/sdeorowicz_polsl_pl/EZlvCxYITEhNuXJeorf5vggBQlwCuBiEu6vzoUmEutAtoA?e=fYI6an) (1.6 GB file).

## Known issues and limitations
* After decompression of CIF files, the formating of tables may be a bit different than the original one. The contents is, however, the same.

## Citing

Deorowicz, S., Gudyś, A. (2023) Efficient protein structure archiving using ProteStAr, biorXiv preprint, [https://doi.org/10.1101/2023.07.20.549913](https://doi.org/10.1101/2023.07.20.549913).


