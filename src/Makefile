$(MAKE) := make

.PHONY: all clean pal3 pal3a

all: pal3 pal3a

pal3:
	$(MAKE) -C PAL3patch all
	$(MAKE) -C PAL3 all
	$(MAKE) -C PatchConfig pal3

pal3a:
	$(MAKE) -C PAL3Apatch all
	$(MAKE) -C PAL3A all
	$(MAKE) -C PatchConfig pal3a

clean:
	$(MAKE) -C PAL3patch clean
	$(MAKE) -C PAL3 clean
	$(MAKE) -C PAL3Apatch clean
	$(MAKE) -C PAL3A clean
	$(MAKE) -C PatchConfig clean
