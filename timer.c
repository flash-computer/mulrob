#define SEASALT (c->cyl)
#define INEEDYOUSEASALT {SEASALT++;}

// Rememeber to do a bounds check before using these
#define PRM_REF_REG(c, reg) ((c)->prf.regs[(reg)])
#define PRM_WRITE_REG(c, reg, val, width) {if(width < PRC_WORD_SZ){word writeregmacro_mask = ((1<<(width<<3))-1); PRM_REF_REG((c), (reg)) = ((PRM_REF_REG((c), (reg)) & (~writeregmacro_mask)) | ((val) & (writeregmacro_mask)));}else{PRM_REF_REG((c), (reg)) = (val);}}

// TODO: Add logic for Broadcasts which the RAT cache will use in the multiple ROB architecture
void retirement_unit(PRS_cpu *c)
{
	PRS_RetUnit *rtu = &(c->rtu);
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
		#ifdef PRM_MULROB
			for(index i=0; i<PRC_ROBS; i++)
			{
				reset_rob(c->robs + i);
			}
		#else
			reset_rob(&(c->rob));
		#endif
		INEEDYOUSEASALT;

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
				if(opr.success = PRC_STATUS_SUCESS)
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
		opr = read_mem_free(WORD(0xFFFFFFFF), PRC_WORD_SZ);
		SEASALT += opr.time;
		if(!opr.success)
		{
			reset_cpu(c);
			return;
		}

		PRM_PRF_REG(c, RIP) = PRM_WRAP_WORD(PRC_FAULT_TABLE_ADDR + dest.iden);
		INEEDYOUSEASALT;

		return;
	}

	branch_mispredicted_unrecovered:
	{
		purge_execution_unit(c);
		purge_prefetch_unit(c);
		purge_loadstore_unit(c);
		#ifdef PRM_MULROB
			for(index i=0; i<PRC_ROBS; i++)
			{
				reset_rob(c->robs + i);
			}
		#else
			reset_rob(&(c->rob));
		#endif
		INEEDYOUSEASALT;

		PRM_PRF_REG(c, RIP) = re.val;
		INEEDYOUSEASALT;

		return;
	}
}

void loadstore_unit(PRS_cpu *c)
{
	PRS_LSUnit *lsu = &(c->lsu);
	for(index i=0; i<PRC_LOADERS; i++)
	{
		if(lsu->loaders[i].busy)
		{
			//TODO: Add code to advance state of the load by 1 cycle
			//Define cost thresholds for L1, L2, L3 and memory fetches
			/*If status.cyl exceeds the threshold for L1, check if data exists in L1.
			If it does, set the appropriate conditions and mark the loader business
			as PRC_STATUS_SUCCESS. If it does not, continue for further caches in a
			similar manner,	checking the sum of thresholds for all previous caches
			against status.cyl . If it reaches the memory, then you have to also check
			the validity of the memory, and then propagate the data up the caches.
			If the whole operation fails due to memory errors, or invalid addresses,
			set the status as such and mark the loader business as PRC_STATUS_SUCCESS
			regardless of the faliure or success of the operation, which would be
			reflected in status.*/
			(lsu->loaders[i].status.cyl)++;
		}
	}
	for(index i=0; i<PRC_STORERS; i++)
	{
		if(lsu->storers[i].busy)
		{
			//TODO: Add code to advance state of the load by 1 cycle.
			// Similar to the loaders logic
			(lsu->loaders[i].status.cyl)++;
		}
	}
	return;
}

void timer_unit(PRS_cpu *c)
{
	if(!(c->cpu_init_check == true))
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
