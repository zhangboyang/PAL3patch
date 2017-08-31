# freetype headers
FREETYPE_INCLUDE_PATH := C:\freetype\include

CC := gcc
CCAS := $(CC)
CCLD := $(CC)
AR := ar
STRIP := strip
DLLTOOL := dlltool
MSDEV := msdev
SED := sed

TARGET_DLL := PAL3patch.dll
TARGET_DLL_SRCPATH := src
TARGET_DLL_INCPATH := include/PAL3patch

CFLAGS := -O0 -Wall -Wsign-compare -Wpointer-arith -Werror -finput-charset=cp936 -fexec-charset=cp936 -fms-extensions -mms-bitfields -fno-strict-aliasing -falign-functions -mfpmath=387 -isystem $(FREETYPE_INCLUDE_PATH) -I $(TARGET_DLL_INCPATH)
SFLAGS :=
CCLDFLAGS := -Wl,--disable-stdcall-fixup -Wl,--subsystem,windows -lgdi32 -lcomctl32 -lwinmm
CCLDFLAGS_MSVC :=
MSDEVFLAGS := -make all -rebuild

TARGET_DLL_HFILES := $(wildcard $(TARGET_DLL_INCPATH)/*.h)
TARGET_DLL_COMMONHEADER := $(TARGET_DLL_INCPATH)/common.h
TARGET_DLL_GCH := $(TARGET_DLL_COMMONHEADER).gch

TARGET_DLL_CFILES := $(wildcard $(TARGET_DLL_SRCPATH)/*.c)
TARGET_DLL_CFILES_ABOUT := $(TARGET_DLL_SRCPATH)/about.c
TARGET_DLL_COBJS := $(TARGET_DLL_CFILES:.c=.o)

TARGET_DLL_SFILES := $(wildcard $(TARGET_DLL_SRCPATH)/*.S)
TARGET_DLL_SFILES_DRECTVE := $(TARGET_DLL_SRCPATH)/drectve.S
TARGET_DLL_SOBJS := $(TARGET_DLL_SFILES:.S=.o)
TARGET_DLL_ENTRY_SOBJS := $(filter-out $(TARGET_DLL_SFILES_DRECTVE:.S=.o), $(TARGET_DLL_SOBJS))

TARGET_DLL_OBJS := $(TARGET_DLL_COBJS) $(TARGET_DLL_SOBJS)


TARGET_DLL_MSVC_DSP := $(TARGET_DLL:.dll=.dsp)
TARGET_DLL_MSVC_OBJ := $(TARGET_DLL:.dll=.obj)
TARGET_DLL_MSVC_TMPFILES := $(TARGET_DLL_MSVC_OBJ) $(TARGET_DLL:.dll=.exp) $(TARGET_DLL:.dll=.lib) $(TARGET_DLL:.dll=.plg) $(TARGET_DLL:.dll=.dsw) $(TARGET_DLL:.dll=.ncb) $(TARGET_DLL:.dll=.opt) $(TARGET_DLL:.dll=.def) $(TARGET_DLL:.dll=.ord.def)

MSVC_ENTRY_LIB := $(TARGET_DLL:.dll=_entry.lib)

.PHONY: clean $(TARGET_DLL_CFILES_ABOUT)

dll: $(TARGET_DLL)

all: $(TARGET_DLL) $(MSVC_ENTRY_LIB)

clean:
	rm -f $(TARGET_DLL) $(TARGET_DLL_OBJS) $(MSVC_ENTRY_LIB) $(TARGET_DLL_GCH) $(TARGET_DLL_MSVC_TMPFILES)

entrylib: $(MSVC_ENTRY_LIB)

$(TARGET_DLL_GCH): $(TARGET_DLL_HFILES)
	$(CC) $(CFLAGS) $(TARGET_DLL_COMMONHEADER)

$(TARGET_DLL_COBJS): $(TARGET_DLL_GCH)

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.S
	$(CCAS) $(SFLAGS) -c -o $@ $<

$(TARGET_DLL): $(TARGET_DLL_OBJS)
	$(CCLD) $(CCLDFLAGS_MSVC) -o $(TARGET_DLL_MSVC_OBJ) -r -Wl,--unique=.drectve -nostdlib $(TARGET_DLL_OBJS) -lmingwex -lgcc
	$(DLLTOOL) -z $(TARGET_DLL:.dll=.ord.def) $(TARGET_DLL_MSVC_OBJ)
	echo "LIBRARY $(TARGET_DLL)" > $(TARGET_DLL:.dll=.def)
	$(SED) -e '/^;/d' -e 's/ @ [0-9][0-9]*//' $(TARGET_DLL:.dll=.ord.def) >> $(TARGET_DLL:.dll=.def)
	$(STRIP) --strip-unneeded --remove-section=.drectve $(TARGET_DLL_MSVC_OBJ)
	$(MSDEV) $(TARGET_DLL_MSVC_DSP) $(MSDEVFLAGS)

$(MSVC_ENTRY_LIB): $(TARGET_DLL_ENTRY_SOBJS)
	$(AR) rcs $@ $(TARGET_DLL_ENTRY_SOBJS)
