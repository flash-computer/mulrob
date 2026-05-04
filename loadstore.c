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
			return /*something*/ ; //
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
