$(MAKE) := make

.PHONY: all clean

all:
	$(MAKE) -C PAL3patch all
	$(MAKE) -C PAL3 all
	$(MAKE) -C PatchConfig all
	
clean:
	$(MAKE) -C PAL3patch clean
	$(MAKE) -C PAL3 clean
	$(MAKE) -C PatchConfig clean
