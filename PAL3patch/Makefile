$(MAKE) := make

.PHONY: all clean

all:
	$(MAKE) -C PAL3patch all
	$(MAKE) -C PatchConfig all
	$(MAKE) -C PAL3voice all

clean:
	$(MAKE) -C PAL3patch clean
	$(MAKE) -C PatchConfig clean
	$(MAKE) -C PAL3voice clean
