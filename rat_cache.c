#ifndef PRM_INTERNALS
	#define PRM_INTERNALS
#endif
#include<mulrob.h>

void init_rat_cache(PRS_RATCache *rch)
{
	if(!rch)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
	for(index i=0; i<PRC_BBENTRIES; i++)
	{
		init_acache(&(rch->rats[i].l0), PRC_RATL0SIZE);
		// Next is the previous entry in the buffer because the requests propagate backwards in the buffer for l0
		rch->rats[i].l0.prev = rch->rats + ((i + 1)%PRC_BBENTRIES);
		rch->rats[i].l0.next = rch->rats + ((i + PRC_BBENTRIES - 1)%PRC_BBENTRIES);
	}
	return;
}

PRS_OPReturn query_current_l0(PRS_cpu *c, word addr, word val, PRS_WORD_SZ width)
{
	if(!c)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}

	PRS_OPReturn opr;
	do
	{
		opr = query_data_in_cache(, , );
	}
	while(opr.success == )
}

PRS_OPReturn write_to_l0_cache()
{

}

PRS_OPReturn save_rat_to_branch_buffer(PRS_cpu *c, PRS_BBuffer *bb)
{
	if(((bb->las + PRC_BBENTRIES - bb->cur)%PRC_BBENTRIES)<(PRC_BBENTRIES-1))
	{
		PRS_RATCacheEntry *rce = bb->bb[bb->las].rce;
		rce->rat = c->prf.rat;
		PRC_ACache *l0 = &(rat->l0);
		for(index i=0; i<PRC_RATL0SIZE; i++)
		{
			l0->lines[i].filled = false;
		}
		l0->first = 0;
		l0->last = 0;
		return (PRS_OPReturn){PRC_STATUS_SUCCESS, (bb->las)++, 1, 0};
	}
	return (PRS_OPReturn){PRC_STATUS_FALIURE, 0, 1, PRC_FAULT_RETRY};
}

