# PAL3patch headers
PAL3PATCH_INCLUDE_PATH := /c/src/PAL3patch/PAL3patch/include

# libtcc headers
LIBTCC_INCLUDE_PATH := /c/tcc/libtcc



CC := gcc
CCLD := $(CC)
STRIP := strip
DLLTOOL := dlltool
MSDEV := msdev
SED := sed


CFLAGS := -O0 -Wall -Wsign-compare -Wpointer-arith -Werror -finput-charset=GB18030 -fexec-charset=UTF-8 -fwide-exec-charset=UTF-16LE -fms-extensions -mms-bitfields -fno-strict-aliasing -falign-functions -mfpmath=387 -I $(PAL3PATCH_INCLUDE_PATH) -I $(LIBTCC_INCLUDE_PATH)
MSDEVFLAGS := -make all -rebuild

TARGET_DLL := tccplugin.dll

TARGET_DLL_CFILES := $(TARGET_DLL:.dll=.c)
TARGET_DLL_HFILES := $(TARGET_DLL:.dll=.h)
TARGET_DLL_COBJS := $(TARGET_DLL_CFILES:.c=.o)

TARGET_DLL_MSVC_DSP := $(TARGET_DLL:.dll=.dsp)
TARGET_DLL_MSVC_OBJ := $(TARGET_DLL:.dll=.obj)
TARGET_DLL_MSVC_TMPFILES := $(TARGET_DLL_MSVC_OBJ) $(TARGET_DLL:.dll=.exp) $(TARGET_DLL:.dll=.lib) $(TARGET_DLL:.dll=.plg) $(TARGET_DLL:.dll=.dsw) $(TARGET_DLL:.dll=.ncb) $(TARGET_DLL:.dll=.opt) $(TARGET_DLL:.dll=.def) $(TARGET_DLL:.dll=.ord.def)

.PHONY: all clean

all: $(TARGET_DLL)

clean:
	rm -rf $(TARGET_DLL) $(TARGET_DLL_COBJS) $(TARGET_DLL_MSVC_TMPFILES)

$(TARGET_DLL): $(TARGET_DLL_COBJS)
	$(CCLD) -o $(TARGET_DLL_MSVC_OBJ) -r -Wl,--unique=.drectve -nostdlib $(TARGET_DLL_COBJS) -lmingwex -lgcc
	$(DLLTOOL) -z $(TARGET_DLL:.dll=.ord.def) $(TARGET_DLL_MSVC_OBJ)
	echo "LIBRARY $(TARGET_DLL)" > $(TARGET_DLL:.dll=.def)
	$(SED) -e '/^;/d' -e 's/ @ [0-9][0-9]*//' $(TARGET_DLL:.dll=.ord.def) >> $(TARGET_DLL:.dll=.def)
	$(STRIP) --strip-unneeded --remove-section=.drectve $(TARGET_DLL_MSVC_OBJ)	
	$(MSDEV) $(TARGET_DLL_MSVC_DSP) $(MSDEVFLAGS)
	

$(TARGET_DLL_COBJS): $(TARGET_DLL_HFILES)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<