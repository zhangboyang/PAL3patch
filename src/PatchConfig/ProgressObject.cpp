#include "stdafx.h"

ProgressObject::~ProgressObject()
{
}

ProgressGroup::ProgressGroup() : sum_curv(0), sum_maxv(0)
{
}
ProgressGroup::~ProgressGroup()
{
	std::vector<SubProgress *>::iterator it;
	for (it = ch.begin(); it != ch.end(); it++) {
		delete *it;
	}
}
SubProgress *ProgressGroup::sub()
{
	SubProgress *ret = new SubProgress(this);
	ch.push_back(ret);
	return ret;
}
void ProgressGroup::reset()
{
	std::vector<SubProgress *>::iterator it;
	for (it = ch.begin(); it != ch.end(); it++) {
		delete *it;
	}
	ch.clear();
	sum_curv = 0;
	sum_maxv = 0;
	set_progress(0);
	set_maximum(0);
}

SubProgress::SubProgress(ProgressGroup *group) : grp(group), curv(0), maxv(0)
{
}
void SubProgress::set_maximum(unsigned value)
{
	grp->sum_maxv = grp->sum_maxv - maxv + value;
	maxv = value;
	grp->set_maximum(grp->sum_maxv);
}
void SubProgress::set_progress(unsigned value)
{
	grp->sum_curv = grp->sum_curv - curv + value;
	curv = value;
	grp->set_progress(grp->sum_curv);
}
bool SubProgress::is_cancelled()
{
	return grp->is_cancelled();
}
