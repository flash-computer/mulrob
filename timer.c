#ifndef PRM_INTERNALS
	#define PRM_INTERNALS
#endif
#include<mulrob.h>

// Broadcast the retirement of an instruction
// TODO: Add logic for Broadcasts which the RAT cache will use in the multiple ROB architecture
void broadcast_rtu_retirement(PRS_cpu *c, index roben)
{
}

void retirement_unit(PRS_cpu *c)
{
	if(!c)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
	PRS_RTUnit *rtu = &(c->rtu);
	PRS_ROBEntry re = PRM_ROB_RT(PRM_CROB(c));
	PRS_ROBDestEntry dest;
	PRS_OPReturn opr;

	if(rtu->engaged)
	{
		rtu->opr = query_store_finished(c, rtu->store_index);
		if(rtu->opr.success != PRC_STATUS_BUSY)
		{
			goto finish_rtu_op;
		}
		return;
	}

	try_retire:
	dest = re.dest;

	if(PRM_ISBRANCH(re.i.op))
	{
		if(re.dest.iden == re.val)
		{
			PRM_REF_REG(c, RIP) = re.val; // This one is fine without the macro, since this write always is the full word width
			(PRM_CROB(c).rt)++;
			return;
		}
		else
		{
			goto branch_mispredicted_unrecovered;
		}
	}

	switch(dest.type)
	{
		case PRC_DESTTYPE_FAULT: // If the destination indicates a fault
			goto fault_routine;
		case PRC_DESTTYPE_NOP:
			break;
		case PRC_DESTTYPE_MEM:	// If the destination is a memory address
			opr = write_mem_loadstore(c, dest.iden, re.val, re.width);
			if(!opr.success)
			{
				if(opr.fault == PRC_FAULT_RETRY) // Not a Fault
				{
					return;
				}
				goto fault_routine;
			}
			if(opr.success == PRC_STATUS_BUSY)
			{
				rtu->engaged = true;
				rtu->store_index = opr.val;
				return;
			}
			break;
		case PRC_DESTTYPE_REG:
			// ask_rat returns true if the entry pointed at by dest.iden in the RAT points to the same entry in ROB as PRM_CROB(c).rt
			if(ask_rat(c, dest.iden, PRM_CROB(c).rt))
			{
				PRM_WRITE_REG(c, dest.iden, re.val, re.width);
			}
			break;
		default:
			PRM_ERROR(PRC_E_UNEXPECTED);
	}
	broadcast_rtu_retirement(c, PRM_CROB(c).rt);
	(PRM_CROB(c).rt)++;
	return;

	finish_rtu_op:
	{
		rtu->engaged = false;
		opr = rtu->opr;
		if(opr.success == PRC_STATUS_SUCCESS)
		{
			(PRM_CROB(c).rt)++;
			return;
		}
		if(opr.fault == PRC_FAULT_RETRY) // Not a fault, just an indication to retry
		{
			goto try_retire;
		}
		dest.type = PRC_DESTTYPE_FAULT;
		dest.iden = opr.fault;
		goto fault_routine;
	}

	fault_routine:
	{
		purge_execution_unit(c);
		purge_prefetch_unit(c);
		purge_loadstore_unit(c);
		for(index i=0; i<PRC_ROBS; i++)
		{
			reset_rob(c->robs + i);
		}
		MAKEASTEAK(6);

		rtu->engaged = false;
		rtu->store_index = 0;
		// Push the Return address on the stack before making the jump to the fault routine (The hardware one, not this C label)
		while(1)
		{
			INEEDYOUSEASALT;
			if(!rtu->engaged)
			{
				opr = write_mem_loadstore(c, PRM_PRF_REG(c, RSP), PRM_PRF_REG(c, RIP), PRC_WORD_SZ);
				if(!opr.success)
				{
					reset_cpu(c);
					return;
				}
				if(opr.success == PRC_STATUS_SUCCESS)
				{
					rtu->engaged = true;
					rtu->store_index = opr.val;
				}
			}
			else
			{
				opr = query_store_finished(c, rtu->store_index);
				if(!opr.success)
				{
					reset_cpu(c);
					return;
				}
				if(opr.success == PRC_STATUS_SUCCESS)
				{
					break;
				}
			}
			if((!(c->lsu_stall)))
			{
				loadstore_unit(c);
			}
			else
			{
				c->lsu_stall -= (c->lsu_stall)? 1 : 0;
			}
		}

		PRM_PRF_REG(c, RSP) += PRC_WORD_SZ;

		// TODO: Replace with proper read
		/* opr = read_mem_free(WORD(0xFFFFFFFF), PRC_WORD_SZ);

		if(!opr.success)
		{
			reset_cpu(c);
			return;
		}
		*/

		PRM_PRF_REG(c, RIP) = PRC_FAULT_TABLE_ADDR + dest.iden;
		INEEDYOUSEASALT;

		return;
	}

	branch_mispredicted_unrecovered:
	{
		purge_execution_unit(c);
		purge_prefetch_unit(c);
		purge_loadstore_unit(c);
		for(index i=0; i<PRC_ROBS; i++)
		{
			reset_rob(c->robs + i);
		}
		INEEDYOUSEASALT;

		PRM_PRF_REG(c, RIP) = re.val;
		INEEDYOUSEASALT;

		return;
	}
}

void timer_unit(PRS_cpu *c)
{
	if(!c)
	{
		PRM_ERROR(PRC_E_UNFITSTRUCTSTATE);
	}

	if(PRM_ROB_RT(PRM_CROB(c)).processed && (!(c->rtu_stall)))
	{
		retirement_unit(c);
	}
	else
	{
		c->rtu_stall -= (c->rtu_stall)? 1 : 0;
	}

	if(PRM_ROBCMP(PRM_CROB(c).rt, PRM_CROB(c).pf) < (PRC_ROBENTRIES - 1) && (!(c->pfu_stall)))
	{
		prefetch_unit(c);
	}
	else
	{
		c->pfu_stall -= (c->pfu_stall)? 1 : 0;
	}

	if(PRM_ROBCMP(PRM_CROB(c).ex, PRM_CROB(c).pf) > 0 && (!(c->rtu_stall)))
	{
		execution_unit(c);
	}
	else
	{
		c->exu_stall -= (c->exu_stall)? 1 : 0;
	}

	if((!(c->lsu_stall)))
	{
		loadstore_unit(c);
	}
	else
	{
		c->lsu_stall -= (c->lsu_stall)? 1 : 0;
	}
	INEEDYOUSEASALT;
	return;
}
