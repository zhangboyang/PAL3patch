#ifndef PAL3PATCHCONFIG_REFERENCECOUNTER
#define PAL3PATCHCONFIG_REFERENCECOUNTER

class ReferenceCounter {
private:
	size_t ref;
public:
	ReferenceCounter();
	virtual ~ReferenceCounter();
	void inc();
	void dec();
};

#endif
