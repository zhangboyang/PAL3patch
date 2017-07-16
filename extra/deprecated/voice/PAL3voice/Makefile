MSDEV := msdev
MSDEVFLAGS := -make all

TARGET_DLL := PAL3voice.dll

OUTPUT_DIR := Debug Release

TARGET_DLL_MSVC_DSP := $(TARGET_DLL:.dll=.dsp)
TARGET_DLL_MSVC_TMPFILES := $(TARGET_DLL:.dll=.plg) $(TARGET_DLL:.dll=.ncb) $(TARGET_DLL:.dll=.opt)


.PHONY: all clean

all:
	$(MSDEV) $(TARGET_DLL_MSVC_DSP) $(MSDEVFLAGS)
clean:
	rm -rf $(TARGET_DLL_MSVC_TMPFILES) $(OUTPUT_DIR)
