#ifndef PRM_INTERNALS
	#define PRM_INTERNALS
#endif
#include<mulrob.h>

bool ask_rat(PRS_cpu *c, index dest, index roben)
{
	if(!c || dest > RFG)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
	if(c->rat.entries[dest].type == PRC_RATENTRY_ROB && c->rat.entries[dest].ref == roben)
	{
		return true;
	}
}

void rat_update(PRS_cpu *c, index dest, index roben)
{
	if(!c || dest > RFG || roben >= PRC_ROBENTRIES)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
	c->rat.entries[dest].type = PRC_RATENTRY_ROB;
	c->rat.entries[dest].ref = roben;
	PRS_ROBEntry *en = PRM_CROB(c).buffer + roben;
	en->dest.type = PRC_DESTTYPE_REG;
	return;
}

void save_rat(PRS_cpu *c, PRS_BBuffer *bb, index bbdex)
{
	if(!c || !bb)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
}
