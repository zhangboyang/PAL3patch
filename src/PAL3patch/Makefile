# use msvc linker to link d3dx9.lib
# MUST run 'make clean' after modifying this option
LINK_D3DX9 := 1


CC := gcc
CCAS := $(CC)
CCLD := $(CC)
AR := ar
STRIP := strip

CFLAGS := -O0 -Wall -Wsign-compare -Wpointer-arith -Werror -finput-charset=cp936 -fexec-charset=cp936 -fms-extensions -fno-strict-aliasing
SFLAGS :=
CCLDFLAGS := -Wl,--disable-stdcall-fixup -Wl,--subsystem,windows -lgdi32 -lcomctl32 -lwinmm
CCLDFLAGS_MSVC :=

MSDEV := msdev
MSDEVFLAGS := -make all -rebuild

ifeq ($(LINK_D3DX9), 0)
	CFLAGS += -DDYNLINK_D3DX9_AT_RUNTIME
endif

TARGET_DLL := PAL3patch.dll
TARGET_DLL_SRCPATH := src

TARGET_DLL_HFILES := $(wildcard $(TARGET_DLL_SRCPATH)/*.h)
TARGET_DLL_COMMONHEADER := $(TARGET_DLL_SRCPATH)/common.h
TARGET_DLL_GCH := $(TARGET_DLL_COMMONHEADER).gch

TARGET_DLL_CFILES := $(wildcard $(TARGET_DLL_SRCPATH)/*.c)
TARGET_DLL_CFILES_ABOUT := $(TARGET_DLL_SRCPATH)/about.c
TARGET_DLL_COBJS := $(TARGET_DLL_CFILES:.c=.o)

TARGET_DLL_SFILES := $(wildcard $(TARGET_DLL_SRCPATH)/*.S)
TARGET_DLL_SOBJS := $(TARGET_DLL_SFILES:.S=.o)

TARGET_DLL_OBJS := $(TARGET_DLL_COBJS) $(TARGET_DLL_SOBJS)


TARGET_DLL_MSVC_DSP := $(TARGET_DLL:.dll=.dsp)
TARGET_DLL_MSVC_OBJ := $(TARGET_DLL:.dll=.obj)
TARGET_DLL_MSVC_SYMBOLS := $(TARGET_DLL_SRCPATH)/msvc_symbols.txt
TARGET_DLL_MSVC_TMPFILES := $(TARGET_DLL_MSVC_OBJ) $(TARGET_DLL:.dll=.exp) $(TARGET_DLL:.dll=.lib) $(TARGET_DLL:.dll=.plg) $(TARGET_DLL:.dll=.dsw) $(TARGET_DLL:.dll=.ncb) $(TARGET_DLL:.dll=.opt)

MSVC_ASM_LIB := $(TARGET_DLL:.dll=_asm.lib)

.PHONY: clean $(TARGET_DLL_CFILES_ABOUT)

dll: $(TARGET_DLL)

all: $(TARGET_DLL) $(MSVC_ASM_LIB)

clean:
	rm -f $(TARGET_DLL) $(TARGET_DLL_OBJS) $(MSVC_ASM_LIB) $(TARGET_DLL_GCH) $(TARGET_DLL_MSVC_TMPFILES)

msvcasm: $(MSVC_ASM_LIB)

$(TARGET_DLL_GCH): $(TARGET_DLL_HFILES)
	$(CC) $(CFLAGS) $(TARGET_DLL_COMMONHEADER)

$(TARGET_DLL_COBJS): $(TARGET_DLL_GCH)

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.S
	$(CCAS) $(SFLAGS) -c -o $@ $<

$(TARGET_DLL): $(TARGET_DLL_OBJS)
ifeq ($(LINK_D3DX9), 0)
	$(CCLD) -shared -o $@ $(TARGET_DLL_OBJS) $(TARGET_DLL_SRCPATH)/$(@:.dll=.def) $(CCLDFLAGS)
	$(STRIP) $@
else
	$(CCLD) $(CCLDFLAGS_MSVC) -o $(TARGET_DLL_MSVC_OBJ) -r -Wl,--retain-symbols-file $(TARGET_DLL_MSVC_SYMBOLS) -nostdlib $(TARGET_DLL_OBJS) -lmingwex -lgcc
	$(MSDEV) $(TARGET_DLL_MSVC_DSP) $(MSDEVFLAGS)
endif

$(MSVC_ASM_LIB): $(TARGET_DLL_SOBJS)
	$(AR) rcs $(MSVC_ASM_LIB) $(TARGET_DLL_SOBJS)
