CC := gcc
CFLAGS := -Wall -Werror -Os -finput-charset=cp936 -fexec-charset=cp936
SFLAGS := -Wall -Werror
LDFLAGS := -Wall -Werror -Wl,--disable-stdcall-fixup -Wl,--subsystem,windows -lcomctl32 -lwinmm -lgdi32

TARGET_LAUNCHER := launcher.exe
TARGET_LAUNCHER_USEUNPACKED := launcher_useunpacked.exe
TARGET_EXE := $(TARGET_LAUNCHER) $(TARGET_LAUNCHER_USEUNPACKED)

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

$(TARGET_LAUNCHER):
	$(CC) $(CFLAGS) -Wl,--subsystem,windows -o $@ launcher/launcher.c
	strip $@

$(TARGET_LAUNCHER_USEUNPACKED):
	$(CC) $(CFLAGS) -DUSE_UNPACKED_EXE -Wl,--subsystem,windows -o $@ launcher/launcher.c
	strip $@

$(TARGET_DLL_OBJS): $(TARGET_DLL_HFILES)

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.S
	$(CC) $(SFLAGS) -c -o $@ $<

$(TARGET_DLL): $(TARGET_DLL_OBJS)
	$(CC) -shared -o $@ $(TARGET_DLL_OBJS) $(TARGET_DLL_SRCPATH)/$(@:.dll=.def) $(LDFLAGS)
	strip $@
