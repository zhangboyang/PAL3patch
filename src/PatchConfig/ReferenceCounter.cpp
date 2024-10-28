#include "stdafx.h"

ReferenceCounter::ReferenceCounter()
{
	ref = 0;
}

ReferenceCounter::~ReferenceCounter()
{
	assert(!ref);
}

void ReferenceCounter::inc()
{
	ref++;
}

void ReferenceCounter::dec()
{
	assert(ref);
	if (!--ref) delete this;
}
