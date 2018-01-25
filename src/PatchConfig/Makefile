MSDEV := msdev
MSDEVFLAGS := -make all -rebuild

TARGET_EXE := PatchConfig.exe

OUTPUT_DIR := Debug_PAL3 Release_PAL3 Debug_PAL3A Release_PAL3A

TARGET_EXE_MSVC_DSP := $(TARGET_EXE:.exe=.dsp)
TARGET_EXE_MSVC_TMPFILES := $(TARGET_EXE:.exe=.plg) $(TARGET_EXE:.exe=.ncb) $(TARGET_EXE:.exe=.opt) $(TARGET_EXE:.exe=.aps) $(TARGET_EXE:.exe=.clw) PAL3patch.conf PAL3Apatch.conf save snap


.PHONY: all clean

all:
	$(MSDEV) $(TARGET_EXE_MSVC_DSP) $(MSDEVFLAGS)
clean:
	rm -rf $(TARGET_EXE_MSVC_TMPFILES) $(OUTPUT_DIR)
