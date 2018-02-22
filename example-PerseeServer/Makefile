include CommonDefs.mak

BIN_DIR = Bin

INC_DIRS = \
	../Common

SRC_FILES = src/*.cpp ../../src/persee/*.cpp

USED_LIBS += OpenNI2 pthread z

EXE_NAME = PerseeServer

CFLAGS += -Wall -std=c++11


ifndef OPENNI2_INCLUDE
    $(error OPENNI2_INCLUDE is not defined. Please define it or 'source' the OpenNIDevEnvironment file from the installation)
else ifndef OPENNI2_REDIST
    $(error OPENNI2_REDIST is not defined. Please define it or 'source' the OpenNIDevEnvironment file from the installation)
endif

INC_DIRS += $(OPENNI2_INCLUDE)

include CommonCppMakefile

.PHONY: copy-redist
copy-redist:
	cp -R $(OPENNI2_REDIST)/* $(OUT_DIR)

$(OUTPUT_FILE): copy-redist
