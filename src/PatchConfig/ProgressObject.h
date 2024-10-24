#ifndef PAL3PATCHCONFIG_PROGRESSOBJECT
#define PAL3PATCHCONFIG_PROGRESSOBJECT

class ProgressObject {
public:
	virtual ~ProgressObject();
	virtual void set_maximum(unsigned value) = 0;
	virtual void set_progress(unsigned value) = 0;
	virtual bool is_cancelled() = 0;
};

class SubProgress;

class ProgressGroup : public ProgressObject {
	friend SubProgress;
private:
	unsigned sum_curv, sum_maxv;
	std::vector<SubProgress *> ch;
public:
	ProgressGroup();
	~ProgressGroup();
	SubProgress *sub();
};

class SubProgress : public ProgressObject {
	friend ProgressGroup;
private:
	ProgressGroup *grp;
	unsigned curv, maxv;
	SubProgress(ProgressGroup *group);
public:
	void set_maximum(unsigned value);
	void set_progress(unsigned value);
	bool is_cancelled();
};

template<typename T>
class ProgressBinder {
private:
	ProgressObject *po;
	T* v;
	unsigned f;
public:
	ProgressBinder(ProgressObject *progress, T *value, unsigned factor)
	{
		po = progress;
		v = value;
		f = factor;
	}
	~ProgressBinder()
	{
		update();
	}
	bool update()
	{
		if (v) po->set_progress(*v * f);
		return !po->is_cancelled();
	}
	void override(T value)
	{
		v = NULL;
		po->set_progress(value * f);
	}
};

#endif
