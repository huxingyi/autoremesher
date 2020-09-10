CC=gcc
CFLAGS=-O3 -Wall -pthread
LDFLAGS=-pthread
BLOSC_LIB= $(wildcard ../blosc/*.c)

# The list of executables
# Generated PNG (intermediate) files
SOURCES := $(wildcard *.c)
EXECUTABLES := $(patsubst %.c, %.exe, $(SOURCES))

# Support for internal LZ4 and LZ4HC
LZ4_DIR = ../internal-complibs/lz4-1.7.5
CFLAGS += -DHAVE_LZ4 -I$(LZ4_DIR)
BLOSC_LIB += $(wildcard $(LZ4_DIR)/*.c)

# Support for external LZ4 and LZ4HC
#LDFLAGS += -DHAVE_LZ4 -llz4

# Support for internal Snappy
#SNAPPY_DIR = ../internal-complibs/snappy-1.1.1
#CFLAGS += -DHAVE_SNAPPY -I$(SNAPPY_DIR)
#BLOSC_LIB += $(wildcard $(SNAPPY_DIR)/*.cc)

# Support for external Snappy
LDFLAGS += -DHAVE_SNAPPY -lsnappy

# Support for external Zlib
LDFLAGS += -DHAVE_ZLIB -lz

# Support for internal Zlib
#ZLIB_DIR = ../internal-complibs/zlib-1.2.8
#CFLAGS += -DHAVE_ZLIB -I$(ZLIB_DIR)
#BLOSC_LIB += $(wildcard $(ZLIB_DIR)/*.c)


# Support for internal Zstd
ZSTD_DIR = ../internal-complibs/zstd-1.1.4
CFLAGS += -DHAVE_ZSTD -I$(ZSTD_DIR) -I$(ZSTD_DIR)/common
BLOSC_LIB += $(wildcard $(ZSTD_DIR)/*/*.c)


.PHONY: all
all: $(EXECUTABLES)

test: $(EXECUTABLES)
	sh test_all.sh

%.exe: %.c $(BLOSC_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) "$<" $(BLOSC_LIB) -o "$@"

clean:
	rm -rf $(EXECUTABLES)
