CC := gcc
CFLAGS := -Wall -O0
SFLAGS := -Wall
LDFLAGS := -Wall -Wl,--disable-stdcall-fixup -Wl,--subsystem,windows

TARGET_EXE := launcher.exe

TARGET_DLL := PAL3patch.dll
TARGET_DLL_SRCPATH := $(TARGET_DLL:.dll=)

TARGET_DLL_HFILES := $(wildcard $(TARGET_DLL_SRCPATH)/*.h)

TARGET_DLL_CFILES := $(wildcard $(TARGET_DLL_SRCPATH)/*.c)
TARGET_DLL_COBJS := $(TARGET_DLL_CFILES:.c=.o)

TARGET_DLL_SFILES := $(wildcard $(TARGET_DLL_SRCPATH)/*.S)
TARGET_DLL_SOBJS := $(TARGET_DLL_SFILES:.S=.o)

TARGET_DLL_OBJS := $(TARGET_DLL_COBJS) $(TARGET_DLL_SOBJS)

.PHONY: clean

all: $(TARGET_EXE) $(TARGET_DLL)

clean:
	rm -f $(TARGET_EXE) $(TARGET_DLL) $(TARGET_DLL_OBJS)

$(TARGET_EXE):
	$(CC) $(CFLAGS) -Wl,--subsystem,windows -o $@ $(@:.exe=)/$(@:.exe=.c)
	strip $@

$(TARGET_DLL_OBJS): $(TARGET_DLL_HFILES)

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.S
	$(CC) $(SFLAGS) -c -o $@ $<

$(TARGET_DLL): $(TARGET_DLL_OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(TARGET_DLL_OBJS) $(TARGET_DLL_SRCPATH)/$(@:.dll=.def)
	strip $@
