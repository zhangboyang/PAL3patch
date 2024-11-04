#ifndef PAL3PATCHCONFIG_PROGRESSOBJECT
#define PAL3PATCHCONFIG_PROGRESSOBJECT

class ProgressObject {
public:
	virtual ~ProgressObject();
	virtual void reset(unsigned maxvalue) = 0;
	virtual void progress(unsigned value) = 0;
	virtual bool cancelled() = 0;
};

template<typename T>
class ProgressBinder {
private:
	ProgressObject *po;
	T* v;
	unsigned f;
public:
	ProgressBinder(ProgressObject *progress, T *value, T maxvalue, unsigned factor)
	{
		po = progress;
		v = value;
		f = factor;
		po->reset(maxvalue * f);
	}
	~ProgressBinder()
	{
		update();
	}
	bool update()
	{
		if (v) po->progress(*v * f);
		return !po->cancelled();
	}
	void override(T value)
	{
		v = NULL;
		po->progress(value * f);
	}
};

#endif
