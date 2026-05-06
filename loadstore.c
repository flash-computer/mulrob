void init_loadstore_unit(PRS_LSUnit *lsu)
{
	if(!lsu)
	{
		PRM_ERROR(PRC_E_UNFITSTRUCTSTATE);
	}

	for(index i=0; i<PRC_LOADERS; i++)
	{
		lsu->loaders[i].busy = PRC_STATUS_FREE;
	}

	for(index i=0; i<PRC_STORERS; i++)
	{
		lsu->storers[i].busy = PRC_STATUS_FREE;
	}

	return;
}

PRS_OPReturn write_mem_loadstore(PRS_cpu *c, word addr, word val, PRS_WORD_SZ width)
{
	for(index i=0; i<PRC_STORERS; i++)
	{
		if(!(lsu->storers[i].busy))
		{
			PRS_LoaderStorer *ls = (lsu->storers + i);
			ls->status.cyl = 0;
			ls->busy = PRC_STATUS_BUSY;
			ls->addr = addr;
			ls->val = val;
			ls->width = width;
			// TODO
			return /*something*/ ;
		}
	}
	return (PRS_OPReturn){0, 1, false, PRC_FAULT_RETRY};
}

PRS_OPReturn read_mem_loadstore(PRS_cpu *c, word addr, word val, PRS_WORD_SZ width)
{
	for(index i=0; i<PRC_LOADERS; i++)
	{
		if(!lsu->storers[i].busy)
		{
			PRS_LoaderStorer *ls = (lsu->loaders + i);
			ls->status.cyl = 0;
			ls->busy = PRC_STATUS_BUSY;
			ls->addr = addr;
			ls->val = val;
			ls->width = width;
			// TODO
		}
	}
	return (PRS_OPReturn){0, 1, false, PRC_FAULT_RETRY};
}

PRS_OPReturn query_store_finished(PRS_cpu *c, index store_index)
{
	switch(lsu->storers[store_index].busy)
	{
		case PRC_STATUS_BUSY:
			return (PRS_OPReturn){PRS_STATUS_BUSY, 0, lsu->storers[store_index].status.cyl, 0};
		case PRC_STATUS_SUCCESS:
			return lsu->storers[store_index].status;
		default:
			return (PRS_OPReturn){PRS_STATUS_FREE, 0, 0, 0};	// Be warned that this is indistinguishable from a faliure state without checking the busy status on the caller side manually. Ofcourse, it is because it's just undefined in the hardware to make this query to a unit that you know is or was not busy
	}
}

void loadstore_unit(PRS_cpu *c)
{
	PRS_LSUnit *lsu = &(c->lsu);
	PRS_LoaderStorer *ls = NULL;
	PRS_OPReturn opr;
	for(index i=0; i<PRC_LOADERS; i++)
	{
		umax cost = 0;
		ls = lsu->loaders + i;
		if(ls->busy == PRC_STATUS_BUSY)
		{
			for(int j = 0; j<PRC_CACHES && ls->status.cyl >= cost; j++)
			{
				cost = 0;
				if(ls->status.cyl >= cost)
				{
					opr = query_data_in_cache(c->mem->caches + j, ls->addr, ls->width);
					if(!opr.success)
					{
						if(opr.fault == PRC_FAULT_CACHEMISS)
						{
							continue;
						}
						//TODO: Update ROB Entry with the fault if tied to one
						lsu->status.success = PRC_STATUS_FALIURE;
						lsu->status.fault = opr.fault;
						lsu->status.val = opr.val;
						lsu->busy = PRC_STATUS_SUCCESS;
						goto next_loader;
					}
					else
					{
						//TODO: Update ROB Entry if associated with one
						ls->busy = PRC_STATUS_SUCCESS;
						ls->status.success = PRC_STATUS_SUCESS;
						ls->status.val = opr.val;
						ls->status.fault = 0;
						//TODO: Propogate cache line downwards
						goto next_loader;
					}
				}
				cost += cache_fetch_costs(j);
			}
			// I know I'm not doing this 100% right considering only one way costs of memory accesses where the operations in memory and cache happen at the tail end of each access, but unless the exact timing of a write in the memory comes into play somehow (it shouldn't, but with multiple addresses pointing to the same physical memory location, it might. That is an edge case I've chosen to sacrifice for simplicity even if this emulator's memory architecture does support those configurations), it should be fine.
			cost += cache_fetch_costs(PRC_CACHES);	// Cost of memory access
			if(status.cyl >= cost)
			{
				//TODO
				//prisc_memrd()
				//if successful propogate cache lines downwards
				(ls->status.cyl)++;
				goto next_loader;
			}
			//TODO: Add code to advance state of the load by 1 cycle
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
			(ls->status.cyl)++;
		}
		next_loader:
	}
	for(index i=0; i<PRC_STORERS; i++)
	{
		if(lsu->storers[i].busy)
		{
			//TODO: Add code to advance state of the store by 1 cycle.
			// Similar to the loaders logic
		}
	}
	return;
}
