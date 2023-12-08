all: protestar protestar-model-learn example-api pyprotestar

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)
UNAME_P := $(shell uname -p)

PROTESTAR_ROOT_DIR = .
PROTESTAR_MAIN_DIR = src
PROTESTAR_CORE_DIR = src/core
PROTESTAR_COMMON_DIR = src/common
PROTESTAR_COMPRESSORS_DIR = src/compressors
PROTESTAR_PARSER_DIR = src/parsers
PROTESTAR_3RD_PARTY_DIR = src/3rd-party
PROTESTAR_APP_DIR = src/app
PROTESTAR_LIB_DIR = src/lib-cxx
PROTESTAR_EXAMPLE_DIR = src/example_api
PROTESTAR_MODEL_LEARN_DIR = src/coord_model_learn
PROTESTAR_EXAMPLES_DIR = src/examples
PROTESTAR_CXX_DIR = src/lib-cxx
PROTESTAR_LIBS_DIR = libs
MIMALLOC_INLUCDE_DIR = src/3rd-party/mimalloc/include
LIBS_DIR = . #/usr/local/lib
INCLUDE_DIR= . #/usr/local/include
PYPROTESTAR_DIR = pyprotestar
PYBIND11_LIB = $(PYPROTESTAR_DIR)/pybind11-2.8.1

INC_ZLIB=src/3rd-party/dependencies-zlib
INC_ZSTD=src/3rd-party/dependencies-zstd/lib
INC_MIMALLOC=src/3rd-party/mimalloc/include
INC_REFRESH_ARCHIVE=src/libs/refresh/archive
INC_REFRESH_COMPRESSION=src/libs/refresh/compression
INC_REFRESH_RC=src/libs/refresh/range_coder
INC_SAJSON=src/3rd-party/sajson
INC_VCLCLASS=src/3rd-party/vclclass

LIB_ZLIB=src/3rd-party/dependencies-zlib/libz.a
LIB_ZSTD=src/3rd-party/dependencies-zstd/lib/libzstd.a
LIB_PSA=$(OUT_BIN_DIR)/libpsa.a

OUT_BIN_DIR = bin
OUT_INCLUDE_DIR = include

D_OS =
D_ARCH = 

ifeq ($(UNAME_S),Darwin)
	D_OS=MACOS
	ifeq ($(UNAME_M),arm64)
		D_ARCH=ARM64
	else
		D_ARCH=X64
	endif
	AR_OPT=rcs
else
	D_OS=LINUX
	D_ARCH=X64
	ifeq ($(UNAME_M),arm64)
		D_ARCH=ARM64
	endif
	ifeq ($(UNAME_M),aarch64)
		D_ARCH=ARM64
	endif
	AR_OPT=rcs -o
endif

CPU_FLAGS =
STATIC_CFLAGS = 
STATIC_LFLAGS = 
PY_FLAGS =

ifeq ($(D_OS),MACOS)
	CC = g++-11

	ifeq ($(D_ARCH),ARM64)
		CPU_FLAGS = -march=armv8.4-a
	else
		CPU_FLAGS = -m64 -mavx
	endif
	STATIC_CFLAGS = -static-libgcc -static-libstdc++ -pthread
	STATIC_LFLAGS = -static-libgcc -static-libstdc++ -pthread	
	PY_FLAGS = -Wl,-undefined,dynamic_lookup -fPIC 
else
	CC 	= g++

	ifeq ($(D_ARCH),ARM64)
		CPU_FLAGS = -march=armv8-a
		STATIC_CFLAGS =
		STATIC_LFLAGS = -static-libgcc -static-libstdc++ -pthread	
	else
		CPU_FLAGS = -m64 -mavx
		STATIC_CFLAGS = -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
		STATIC_LFLAGS = -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
	endif
	PY_FLAGS = -fPIC
endif

AR 	= ar
CFLAGS	= -fPIC -Wall -O3 -fsigned-char $(CPU_FLAGS) $(STATIC_CFLAGS) -std=c++17 
CLINK	= -lm $(STATIC_LFLAGS) -O3 -std=c++17
PYPROTESTAR_CFLAGS = $(PY_FLAGS) -Wall -shared -std=c++17 -O3


MIMALLOC_OBJ=src/3rd-party/mimalloc/mimalloc.o

$(MIMALLOC_OBJ):
	$(CC) -DMI_MALLOC_OVERRIDE -O3 -DNDEBUG -fPIC -Wall -Wextra -Wno-unknown-pragmas -fvisibility=hidden -ftls-model=initial-exec -fno-builtin-malloc -c -I src/3rd-party/mimalloc/include src/3rd-party/mimalloc/src/static.c -o $(MIMALLOC_OBJ)


# default install location (binary placed in the /bin folder)
prefix      = /usr/local

# optional install location
exec_prefix = $(prefix)

APP_OBJS = \
$(PROTESTAR_APP_DIR)/main.o

LIB_OBJS = \
$(PROTESTAR_LIB_DIR)/lib-cxx.o

EXAMPLE_OBJS = \
$(PROTESTAR_EXAMPLE_DIR)/example_api.o

MODEL_APP_OBJS = \
$(PROTESTAR_MODEL_LEARN_DIR)/main.o \
$(PROTESTAR_MODEL_LEARN_DIR)/model_learn.o

PARSER_OBJS = \
$(PROTESTAR_PARSER_DIR)/cif-input.o \
$(PROTESTAR_PARSER_DIR)/cif-output.o \
$(PROTESTAR_PARSER_DIR)/conversion.o \
$(PROTESTAR_PARSER_DIR)/input-load.o \
$(PROTESTAR_PARSER_DIR)/json.o \
$(PROTESTAR_PARSER_DIR)/json-io.o \
$(PROTESTAR_PARSER_DIR)/json-base.o \
$(PROTESTAR_PARSER_DIR)/pdb-input.o \
$(PROTESTAR_PARSER_DIR)/pdb-output.o

LIB_PARSER_OBJS = \
$(PROTESTAR_PARSER_DIR)/cif-output.o \
$(PROTESTAR_PARSER_DIR)/conversion.o \
$(PROTESTAR_PARSER_DIR)/json.o \
$(PROTESTAR_PARSER_DIR)/json-base.o \
$(PROTESTAR_PARSER_DIR)/pdb-output.o

COMPRESSOR_OBJS = \
$(PROTESTAR_COMPRESSORS_DIR)/conf-compressor.o \
$(PROTESTAR_COMPRESSORS_DIR)/model_compress.o \
$(PROTESTAR_COMPRESSORS_DIR)/pae-compressor.o \
$(PROTESTAR_COMPRESSORS_DIR)/serializer.o \
$(PROTESTAR_COMPRESSORS_DIR)/struct-base.o \
$(PROTESTAR_COMPRESSORS_DIR)/struct-compressor.o \
$(PROTESTAR_COMPRESSORS_DIR)/struct-decompressor.o

LIB_COMPRESSOR_OBJS = \
$(PROTESTAR_COMPRESSORS_DIR)/conf-compressor.o \
$(PROTESTAR_COMPRESSORS_DIR)/model_compress.o \
$(PROTESTAR_COMPRESSORS_DIR)/pae-compressor.o \
$(PROTESTAR_COMPRESSORS_DIR)/serializer.o \
$(PROTESTAR_COMPRESSORS_DIR)/struct-base.o \
$(PROTESTAR_COMPRESSORS_DIR)/struct-decompressor.o

CORE_OBJS = \
$(PROTESTAR_CORE_DIR)/collection.o \
$(PROTESTAR_CORE_DIR)/psa_base.o \
$(PROTESTAR_CORE_DIR)/psa_compressor.o \
$(PROTESTAR_CORE_DIR)/psa_decompression_library.o \
$(PROTESTAR_CORE_DIR)/psa_decompressor.o

LIB_CORE_OBJS = \
$(PROTESTAR_CORE_DIR)/collection.o \
$(PROTESTAR_CORE_DIR)/psa_base.o \
$(PROTESTAR_CORE_DIR)/psa_decompression_library.o

UTILS_OBJS = \
$(PROTESTAR_CORE_DIR)/utils.o

COMMON_OBJS = \
$(PROTESTAR_COMMON_DIR)/aa_atoms.o \
$(PROTESTAR_COMMON_DIR)/atom_extractor.o \
$(PROTESTAR_COMMON_DIR)/model.o

$(LIB_ZLIB):
	cd src/3rd-party/dependencies-zlib; ./configure; make libz.a

$(LIB_ZSTD):
	cd src/3rd-party/dependencies-zstd/lib; make

$(APP_OBJS) $(LIB_OBJS) $(PARSER_OBJS) $(COMPRESSOR_OBJS) $(CORE_OBJS) $(COMMON_OBJS) $(UTILS_OBJS) $(MODEL_APP_OBJS) : %.o: %.cpp
	$(CC) $(CFLAGS) -I $(INC_ZLIB) -I $(INC_ZSTD) -I $(INC_MIMALLOC) -I $(INC_REFRESH_ARCHIVE) -I $(INC_REFRESH_COMPRESSION) -I $(INC_REFRESH_RC) -I $(INC_SAJSON) -I $(INC_VCLCLASS) -c $< -o $@


%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

protestar: $(MIMALLOC_OBJ) $(APP_OBJS) $(PARSER_OBJS) $(COMPRESSOR_OBJS) $(CORE_OBJS) $(COMMON_OBJS) $(UTILS_OBJS) $(LIB_ZLIB) $(LIB_ZSTD)
	-mkdir -p $(OUT_BIN_DIR)
	$(CC) $(CLINK) -o $(OUT_BIN_DIR)/$@ $^ $(LIB_ZLIB) $(LIB_ZSTD)

protestar-model-learn: $(MIMALLOC_OBJ) $(MODEL_APP_OBJS) $(COMMON_OBJS) $(PARSER_OBJS) $(UTILS_OBJS) $(LIB_ZLIB) 
	-mkdir -p $(OUT_BIN_DIR)
	$(CC) $(CLINK) -o $(OUT_BIN_DIR)/$@ $^ $(LIB_ZLIB)

$(LIB_PSA): $(LIB_OBJS) $(LIB_PARSER_OBJS) $(LIB_COMPRESSOR_OBJS) $(LIB_CORE_OBJS) $(COMMON_OBJS) $(UTILS_OBJS)
	-mkdir -p $(OUT_BIN_DIR)
	$(AR) $(AR_OPT) $(LIB_PSA) $(LIB_OBJS) $(LIB_PARSER_OBJS) $(LIB_COMPRESSOR_OBJS) $(LIB_CORE_OBJS) $(COMMON_OBJS) $(UTILS_OBJS)
	
example-api: $(EXAMPLE_OBJS) $(LIB_ZSTD) $(LIB_PSA)
	-mkdir -p $(OUT_BIN_DIR)
	$(CC) $(CLINK) -o $(OUT_BIN_DIR)/$@ $^ $(LIB_ZSTD) $(LIB_PSA)

.PHONY:pyprotestar
pyprotestar: $(PYPROTESTAR_DIR)/pyprotestar.cpp $(PROTESTAR_CXX_DIR)/lib-cxx.o \
	$(MIMALLOC_OBJ) $(LIB_OBJS) $(LIB_PARSER_OBJS) $(LIB_COMPRESSOR_OBJS) $(LIB_CORE_OBJS) $(COMMON_OBJS) $(UTILS_OBJS) $(LIB_ZSTD)
	$(CXX) $(PYPROTESTAR_CFLAGS)  \
	$(PYPROTESTAR_DIR)/pyprotestar.cpp \
	$(LIB_OBJS) $(LIB_PARSER_OBJS) $(LIB_COMPRESSOR_OBJS) $(LIB_CORE_OBJS) $(COMMON_OBJS) $(UTILS_OBJS) $(LIB_ZSTD) \
	-I $(PROTESTAR_MAIN_DIR) \
	-I $(PROTESTAR_APP_DIR) \
	-I $(PYBIND11_LIB)/include \
	-I `python3 -c "import sysconfig;print(sysconfig.get_paths()['include'])"` \
	-o 'pyprotestar/'$@`python3-config --extension-suffix`

clean:
	-rm $(PROTESTAR_APP_DIR)/*.o
	-rm $(PROTESTAR_LIB_DIR)/*.o
	-rm $(PROTESTAR_EXAMPLE_DIR)/*.o
	-rm $(PROTESTAR_PARSER_DIR)/*.o
	-rm $(PROTESTAR_COMMON_DIR)/*.o
	-rm $(PROTESTAR_CORE_DIR)/*.o
	-rm $(PROTESTAR_COMPRESSORS_DIR)/*.o
	-rm $(PROTESTAR_PARSER_DIR)/*.o
	-rm $(PROTESTAR_MODEL_LEARN_DIR)/*.o
	-rm $(MIMALLOC_OBJ)
	-rm $(OUT_BIN_DIR)/protestar
	-rm $(OUT_BIN_DIR)/example-api
	-rm $(OUT_BIN_DIR)/protestar-model-learn
	-rm $(OUT_BIN_DIR)/libpsa.a
	-rm -f $(PYPROTESTAR_DIR)/*.o
	-rm -f $(PYPROTESTAR_DIR)/*.so
	cd src/3rd-party/dependencies-zlib; make clean;
	cd src/3rd-party/dependencies-zstd/lib; make clean;
