CC := gcc
STRIP := strip

CFLAGS := -Wall -Werror -Os -finput-charset=cp936 -fexec-charset=cp936 -fms-extensions

TARGET_LAUNCHER_CFILES := launcher.c
TARGET_LAUNCHER := launcher.exe
TARGET_LAUNCHER_USEUNPACKED := launcher_useunpacked.exe
TARGET_EXE := $(TARGET_LAUNCHER) $(TARGET_LAUNCHER_USEUNPACKED)

.PHONY: clean

all: $(TARGET_EXE)

clean:
	rm -f $(TARGET_EXE)

$(TARGET_LAUNCHER): $(TARGET_LAUNCHER_CFILES)
	$(CC) $(CFLAGS) -Wl,--subsystem,windows -o $@ $(TARGET_LAUNCHER_CFILES)
	$(STRIP) $@

$(TARGET_LAUNCHER_USEUNPACKED): $(TARGET_LAUNCHER_CFILES)
	$(CC) $(CFLAGS) -DUSE_UNPACKED_EXE -Wl,--subsystem,windows -o $@ $(TARGET_LAUNCHER_CFILES)
	$(STRIP) $@
