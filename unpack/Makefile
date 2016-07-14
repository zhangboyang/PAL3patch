CC := gcc
CFLAGS := -Wall -O2

TARGET_EXE := lshint.exe PAL3analyse.exe PAL3fixdump.exe PAL3makepe.exe PAL3mkimport.exe
TARGET_DLL := PAL3dump.dll PAL3memunpack.dll

.PHONY: clean

all: $(TARGET_EXE) $(TARGET_DLL)

clean:
	rm -f $(TARGET_EXE) $(TARGET_DLL)

$(TARGET_EXE):
	$(CC) $(CFLAGS) -o $@ $(@:.exe=)/$(@:.exe=.c)
	strip $@

$(TARGET_DLL):
	$(CC) $(CFLAGS) -shared -Wl,--subsystem,windows -o $@ $(@:.dll=)/$(@:.dll=.c) $(@:.dll=)/$(@:.dll=_asm.S) $(@:.dll=)/$(@:.dll=.def)
	strip $@
