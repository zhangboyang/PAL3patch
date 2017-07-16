CC := gcc
CFLAGS := -Wall -O0

TARGET_EXE := lshint.exe PAL3Aanalyse.exe PAL3Afixdump.exe PAL3Amakepe.exe PAL3Amkimport.exe
TARGET_DLL := PAL3Adump.dll PAL3Amemunpack.dll PAL3Atestvalloc.dll

.PHONY: clean

all: $(TARGET_EXE) $(TARGET_DLL)

clean:
	rm -f $(TARGET_EXE) $(TARGET_DLL)

.SECONDEXPANSION:

%.exe: $$*/$$*.c
	$(CC) $(CFLAGS) -o $@ $(@:.exe=)/$(@:.exe=.c)
	strip $@

%.dll: $$*/$$*.c $$*/$$*_asm.S $$*/$$*.def
	$(CC) $(CFLAGS) -shared -Wl,--subsystem,windows -o $@ $(@:.dll=)/$(@:.dll=.c) $(@:.dll=)/$(@:.dll=_asm.S) $(@:.dll=)/$(@:.dll=.def)
	strip $@
