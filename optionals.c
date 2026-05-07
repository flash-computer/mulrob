#ifndef PRM_INTERNALS
	#define PRM_INTERNALS
#endif
#include<mulrob.h>

// Assumes a static cost for each cache level. Obviously this is not true, especially for primary memory, where there are many other factors, significantly DRAM refresh cycles, involed. But it's fair enough
const static umax fetch_costs_by_cache_level[PRC_CACHES + 1] = {4, 18, 56, 368};

umax cache_fetch_costs(index level)
{
	if(level <= PRC_CACHES)
	{
		return fetch_costs_by_cache_level[level];
	}
	PRM_ERROR(PRC_E_UNEXPECTED);
}

const static index cache_sizes_by_level[PRC_CACHES] = {1<<7, 1<<12, 1<<17};

index cache_sizes(index level)
{
	if(level < PRC_CACHES)
	{
		return cache_sizes_by_level[level];
	}
	PRM_ERROR(PRC_E_UNEXPECTED);
}
