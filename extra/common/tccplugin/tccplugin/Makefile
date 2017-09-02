PAL3PATCH_HEADER_PATH := /c/src/PAL3patch/PAL3patch/include
PAL3PATCH_LIB_PATH := /c/src/PAL3patch/PAL3patch

LIBTCC_HEADER_PATH := /c/tcc/libtcc
LIBTCC_LIB_PATH := /c/tcc/libtcc


CC := gcc
DLLTOOL := dlltool
CCLD := $(CC)
SED := sed


CFLAGS := -O0 -Wall -Wsign-compare -Wpointer-arith -Werror -finput-charset=GB18030 -fexec-charset=UTF-8 -fwide-exec-charset=UTF-16LE -fms-extensions -mms-bitfields -fno-strict-aliasing -falign-functions -mfpmath=387 -I $(PAL3PATCH_HEADER_PATH) -I $(LIBTCC_HEADER_PATH)
CCLDFLAGS := -L $(PAL3PATCH_LIB_PATH) -L $(LIBTCC_LIB_PATH) -lPAL3patch -llibtcc

TARGET_DLL := tccplugin.dll

TARGET_DLL_CFILES := $(TARGET_DLL:.dll=.c)
TARGET_DLL_HFILES := $(TARGET_DLL:.dll=.h)
TARGET_DLL_COBJS := $(TARGET_DLL_CFILES:.c=.o)


.PHONY: all clean

all: $(TARGET_DLL)

clean:
	rm -rf $(TARGET_DLL) $(TARGET_DLL:.dll=.o) $(TARGET_DLL:.dll=.def) $(TARGET_DLL:.dll=.ord.def)

$(TARGET_DLL): $(TARGET_DLL_COBJS)
	$(DLLTOOL) -z $(TARGET_DLL:.dll=.ord.def) $(TARGET_DLL:.dll=.o)
	echo "LIBRARY $(TARGET_DLL)" > $(TARGET_DLL:.dll=.def)
	$(SED) -e '/^;/d' -e 's/ @ [0-9][0-9]*//' $(TARGET_DLL:.dll=.ord.def) >> $(TARGET_DLL:.dll=.def)	
	$(CCLD) -shared -o $(TARGET_DLL) $(TARGET_DLL_COBJS) $(CCLDFLAGS)

$(TARGET_DLL_COBJS): $(TARGET_DLL_HFILES)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<