MSDEV := msdev

TARGET_EXE := PatchConfig.exe

OUTPUT_DIR := Debug_PAL3 Release_PAL3 Debug_PAL3A Release_PAL3A

TARGET_EXE_MSVC_DSP := $(TARGET_EXE:.exe=.dsp)
TARGET_EXE_MSVC_TMPFILES := $(TARGET_EXE:.exe=.plg) $(TARGET_EXE:.exe=.ncb) $(TARGET_EXE:.exe=.opt) $(TARGET_EXE:.exe=.aps) $(TARGET_EXE:.exe=.clw)


.PHONY: all clean pal3 pal3a

all: pal3 pal3a

pal3:
	$(MSDEV) $(TARGET_EXE_MSVC_DSP) -make "PatchConfig - Win32 Release PAL3" -rebuild

pal3a:
	$(MSDEV) $(TARGET_EXE_MSVC_DSP) -make "PatchConfig - Win32 Release PAL3A" -rebuild

clean:
	rm -rf $(TARGET_EXE_MSVC_TMPFILES) $(OUTPUT_DIR) PAL3patch.conf PAL3patch.in PAL3patch.out PAL3Apatch.conf PAL3Apatch.in PAL3Apatch.out config.ini save snap
