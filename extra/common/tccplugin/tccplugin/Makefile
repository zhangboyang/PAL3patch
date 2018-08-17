# PAL3patch
PAL3PATCH_INCLUDE_PATH := C:\src\PAL3patch\PAL3patch\include
PAL3PATCH_LIBRARY_PATH := C:\src\PAL3patch\PAL3patch

# PAL3Apatch
PAL3APATCH_INCLUDE_PATH := C:\src\PAL3patch\PAL3Apatch\include
PAL3APATCH_LIBRARY_PATH := C:\src\PAL3patch\PAL3Apatch

# libtcc
LIBTCC_INCLUDE_PATH := C:\tcc\libtcc
LIBTCC_LIBRARY_PATH := C:\tcc\libtcc


CC := gcc
CCLD := $(CC)
STRIP := strip
DLLTOOL := dlltool
SED := sed
MKDIR := mkdir

CFLAGS := -O0 -Wall -Wsign-compare -Wpointer-arith -Werror -finput-charset=GB18030 -fexec-charset=UTF-8 -fwide-exec-charset=UTF-16LE -fms-extensions -mms-bitfields -fno-strict-aliasing -falign-functions -mfpmath=387 -I $(LIBTCC_INCLUDE_PATH)
CFLAGS_PAL3 := -I $(PAL3PATCH_INCLUDE_PATH) -DBUILD_FOR_PAL3
CFLAGS_PAL3A := -I $(PAL3APATCH_INCLUDE_PATH) -DBUILD_FOR_PAL3A

CL := cl
LINK := link
LINKFLAGS := -nologo -dll -opt:ref -incremental:no -machine:I386 -libpath:"$(LIBTCC_LIBRARY_PATH)"
LINKLIBS := libtcc.lib msvcrt.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
LINKFLAGS_PAL3 := -libpath:"$(PAL3PATCH_LIBRARY_PATH)"
LINKLIBS_PAL3 := PAL3patch.lib
LINKFLAGS_PAL3A := -libpath:"$(PAL3APATCH_LIBRARY_PATH)"
LINKLIBS_PAL3A := PAL3Apatch.lib


TARGET_DLL := PAL3/tccplugin.dll PAL3A/tccplugin.dll
TARGET_DLL_REAL := tccplugin.dll

TARGET_DLL_CFILES := tccplugin.c
TARGET_DLL_HFILES := tccplugin.h
TARGET_DLL_COBJS := $(TARGET_DLL:.dll=.o)
TARGET_DLL_ROBJS := $(TARGET_DLL:.dll=.obj)

.PHONY: all clean

.SECONDARY: $(TARGET_DLL_ROBJS)

all: $(TARGET_DLL)

clean:
	rm -rf PAL3 PAL3A

%.obj: %.o
	$(CCLD) -o $@ -r -Wl,--unique=.drectve -nostdlib $< -lmingwex -lgcc
	$(DLLTOOL) -z $(@:.obj=.r.def) $@
	echo "LIBRARY $(TARGET_DLL_REAL)" > $(@:.obj=.def)
	$(SED) -e '/^;/d' -e 's/ @ [0-9][0-9]*//' $(@:.obj=.r.def) >> $(@:.obj=.def)
	$(STRIP) --strip-unneeded --remove-section=.drectve $@

PAL3/%.dll: PAL3/%.obj
	$(LINK) $(LINKFLAGS) $(LINKFLAGS_PAL3) $(LINKLIBS_PAL3) $(LINKLIBS) /def:"$(@:.dll=.def)" /out:"$@" /implib:"$(@:.dll=.lib)" $<

PAL3A/%.dll: PAL3A/%.obj
	$(LINK) $(LINKFLAGS) $(LINKFLAGS_PAL3A) $(LINKLIBS_PAL3A) $(LINKLIBS) /def:"$(@:.dll=.def)" /out:"$@" /implib:"$(@:.dll=.lib)" $<

$(TARGET_DLL_COBJS): $(TARGET_DLL_HFILES)

PAL3/%.o: %.c
	@- $(MKDIR) PAL3
	$(CC) $(CFLAGS) $(CFLAGS_PAL3) -c -o $@ $<

PAL3A/%.o: %.c
	@- $(MKDIR) PAL3A
	$(CC) $(CFLAGS) $(CFLAGS_PAL3A) -c -o $@ $<
