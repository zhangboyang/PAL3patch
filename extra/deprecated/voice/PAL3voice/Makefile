PAL3PATCH_INCLUDE_PATH := /c/src/PAL3patch/PAL3patch/include


CC := gcc
CFLAGS := -O0 -Wall -Wsign-compare -Wpointer-arith -Werror -finput-charset=GB18030 -fexec-charset=UTF-8 -fwide-exec-charset=UTF-16LE -fms-extensions -fno-strict-aliasing -falign-functions -mfpmath=387 -I $(PAL3PATCH_INCLUDE_PATH)


MSDEV := msdev
MSDEVFLAGS := -make all -rebuild

TARGET_DLL := PAL3voice.dll

TARGET_DLL_GCC_CFILE := VoiceAPIAdapter.c
TARGET_DLL_GCC_COBJ := $(TARGET_DLL_GCC_CFILE:.c=.obj)

OUTPUT_DIR := Debug Release

TARGET_DLL_MSVC_DSP := $(TARGET_DLL:.dll=.dsp)
TARGET_DLL_MSVC_TMPFILES := $(TARGET_DLL:.dll=.plg) $(TARGET_DLL:.dll=.ncb) $(TARGET_DLL:.dll=.opt)


.PHONY: all clean

all:
	$(CC) $(CFLAGS) -c -o $(TARGET_DLL_GCC_COBJ) $(TARGET_DLL_GCC_CFILE)
	$(MSDEV) $(TARGET_DLL_MSVC_DSP) $(MSDEVFLAGS)

clean:
	rm -rf $(TARGET_DLL_GCC_COBJ) $(TARGET_DLL_MSVC_TMPFILES) $(OUTPUT_DIR)
