# freetype
FREETYPE_INCLUDE_PATH := C:\freetype-2.9.1\include
FREETYPE_LIBRARY_PATH := C:\freetype-2.9.1\objs
FREETYPE_LIB := freetype291MT.lib

# dx90sdk
DX90SDK_LIBRARY_PATH := C:\DX90SDK\Lib
D3DX9_LIB := d3dx9.lib



TARGET_DLL := PAL3patch.dll
TARGET_DLL_SRCPATH := src
TARGET_DLL_INCPATH := include/PAL3patch



CC := gcc
CCLD := $(CC)
DLLTOOL := dlltool
STRIP := strip
SED := sed
CFLAGS := -O0 -Wall -Wsign-compare -Wpointer-arith -Werror -finput-charset=GB18030 -fexec-charset=UTF-8 -fwide-exec-charset=UTF-16LE -fms-extensions -mms-bitfields -fno-strict-aliasing -falign-functions -mfpmath=387 -isystem $(FREETYPE_INCLUDE_PATH) -I $(TARGET_DLL_INCPATH)

CL := cl
LINK := link
CLFLAGS := -nologo -Od -MT
LINKFLAGS := -nologo -dll -opt:ref -incremental:no -machine:I386 -libpath:"$(FREETYPE_LIBRARY_PATH)" -libpath:"$(DX90SDK_LIBRARY_PATH)"
LINKLIBS := $(FREETYPE_LIB) $(D3DX9_LIB) winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib



TARGET_DLL_HFILES := $(wildcard $(TARGET_DLL_INCPATH)/*.h)
TARGET_DLL_COMMONHEADER := $(TARGET_DLL_INCPATH)/common.h
TARGET_DLL_GCH := $(TARGET_DLL_COMMONHEADER).gch

TARGET_DLL_CLFILES := $(TARGET_DLL_SRCPATH)/asmentry.c $(TARGET_DLL_SRCPATH)/unpackerentry.c 
TARGET_DLL_CLOBJS := $(TARGET_DLL_CLFILES:.c=.obj)

TARGET_DLL_CFILES := $(filter-out $(TARGET_DLL_CLFILES), $(wildcard $(TARGET_DLL_SRCPATH)/*.c))
TARGET_DLL_CFILES_ABOUT := $(TARGET_DLL_SRCPATH)/about.c
TARGET_DLL_COBJS := $(TARGET_DLL_CFILES:.c=.o)

TARGET_DLL_OBJS := $(TARGET_DLL_COBJS) $(TARGET_DLL_CLOBJS)

TARGET_DLL_ROBJ := $(TARGET_DLL:.dll=.obj)
TARGET_DLL_TMPFILES := $(TARGET_DLL_ROBJ) $(TARGET_DLL:.dll=.exp) $(TARGET_DLL:.dll=.lib) $(TARGET_DLL:.dll=.def) $(TARGET_DLL:.dll=.r.def)



.PHONY: clean $(TARGET_DLL_CFILES_ABOUT)

all: $(TARGET_DLL)

clean:
	rm -f $(TARGET_DLL) $(TARGET_DLL_OBJS) $(TARGET_DLL_GCH) $(TARGET_DLL_TMPFILES)

$(TARGET_DLL_GCH): $(TARGET_DLL_HFILES)
	$(CC) $(CFLAGS) $(TARGET_DLL_COMMONHEADER)

$(TARGET_DLL_COBJS): $(TARGET_DLL_GCH)

$(TARGET_DLL_SRCPATH)/%.o: $(TARGET_DLL_SRCPATH)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET_DLL_SRCPATH)/%.obj: $(TARGET_DLL_SRCPATH)/%.c
	$(CL) $(CLFLAGS) -c -Fo"$@" $<

$(TARGET_DLL): $(TARGET_DLL_OBJS)
	$(CCLD) -o $(TARGET_DLL_ROBJ) -r -Wl,--unique=.drectve -nostdlib $(TARGET_DLL_COBJS) -lmingwex -lgcc
	$(DLLTOOL) -z $(TARGET_DLL:.dll=.r.def) $(TARGET_DLL_ROBJ)
	echo "LIBRARY $(TARGET_DLL)" > $(TARGET_DLL:.dll=.def)
	$(SED) -e '/^;/d' -e 's/ @ [0-9][0-9]*//' $(TARGET_DLL:.dll=.r.def) >> $(TARGET_DLL:.dll=.def)
	$(STRIP) --strip-unneeded --remove-section=.drectve $(TARGET_DLL_ROBJ)
	$(LINK) $(LINKFLAGS) $(LINKLIBS) /def:"$(TARGET_DLL:.dll=.def)" /out:"$(TARGET_DLL)" /implib:"$(TARGET_DLL:.dll=.lib)" $(TARGET_DLL_ROBJ) $(TARGET_DLL_CLOBJS)
