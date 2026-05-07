#ifndef PRM_INTERNALS
	#define PRM_INTERNALS
#endif
#include<mulrob.h>

void purge_retirement_unit(PRS_cpu *c)
{
	PRS_RTUnit *rtu = &(c->rtu);
}

void reset_rob(PRS_ROBuffer *rob)
{
	rob->ex = rob->rt;
	rob->pf = rob->rt;
	return;
}

void reset_cpu(PRS_cpu *c)
{
	purge_execution_unit(c);
	purge_prefetch_unit(c);
	purge_loadstore_unit(c);
	purge_retirement_unit(c);
	for(index i=0; i<PRC_ROBS; i++)
	{
		reset_rob(c->robs + i);
	}
	MAKEASTEAK(6);

	INEEDYOUSEASALT;
	PRS_OPReturn opr = read_mem_loadstore(c, (PRC_FAULT_TABLE_ADDR + ((PRC_FAULT_RESET)<<2)), PRC_WORD_SZ);
	index load = opr.val;
	while(opr.success == PRC_STATUS_BUSY);
	{
		opr = query_load_finished(c, load);
	}
	if(!opr.success)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}

	PRM_PRF_REG(c, RIP) = opr.val;
	return;
}
