#ifndef PRM_CACHE_SIZES
	#define PRM_CACHE_SIZES(level) cache_sizes
#endif

void update_cache_access(PRS_cpu *c, index level, PRS_ACache *cch, PRS_ACacheLine *line)	// Update the Cache Line linked list for LRU replacement
{
	if(level >= PRC_CACHES || !c)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}

	if(level < PRC_CACHES)
	{
		PRS_ACache *cch = (c->mem.caches + level);
	}
	else if(cch == NULL)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}

	index linedex = line - cch->lines;
	if(line->prev)
	{
		line->prev->next = line->next;
	}
	if(line->next)
	{
		line->next->prev = line->prev;
		if(linedex == cch->first)
		{
			cch->first = cch->lines - line->next;
		}
	}
	if(cch->last != cch->first)
	{
		cch->lines[cch->last].next = line;
		line->prev = cch->lines + cch->last;
		cch->line = linedex;
	}
	return;
}

void sync_cache_line(PRS_cpu *c, index level, PRS_ACache *cch; PRS_ACacheLine line)
{
	if(level >= PRC_CACHES || !c)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}

	if(level < PRC_CACHES)
	{
		PRS_ACache *cch = (c->mem.caches + level);
	}
	else if(cch == NULL)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}

	PRS_ACacheLine *sync = NULL;
	for(index i=0; i<cch->size; i++)	// TOCONSIDER: Instead of iterating on the array, iterate on the linked list instead. It might be faster when the cache is not filled and there'd be no need to check sync->filled
	{
		sync = cch->lines + i;
		if(sync->addr & PRC_CACHELINEMASK == line.addr && sync->filled)
		{
			for(index j=0; j<(1<<PRC_CACHELINEWIDTH); j++)
			{
				sync->line[i] = line.line[i];
			}
			sync->synced = line.synced;	// Since a sync request always comes from a lower cache, the sync state will be reflected in this line. Having it be always be false and only calling sync_line when the line is indeed unsynced is a valid optimization as well. This is just a slightly more general way to do it.
			update_cache_access(c, level, NULL, sync);
			return;
		}
	}
	PRM_ERROR(PRC_E_UNEXPECTED);	// If a sync_cache_line request is being made. It should be made to a as large or higher level cache, which should always atleast contain all lines from it's lower level counterparts. For a single core processor like this one atleast.
}

// If level is less than PRC_CACHES. The cache is taken to be the corresponding cache from the Memory Unit
PRS_OPReturn replace_cache_line(PRS_cpu *c, index level, PRS_ACache *cch; PRS_ACacheLine line)
{
	if(level >= PRC_CACHES || !c)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}

	if(level < PRC_CACHES)
	{
		PRS_ACache *cch = (c->mem.caches + level);
	}
	else if(cch == NULL)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}

	if(cch->first >= cch->size)
	{
		PRM_ERROR(PRC_E_UNFITSTRUCTSTATE);
	}
	PRS_ACacheLine *rep = cch->lines[cch->first];
	index repdex = cch->first;

	if(level < PRC_CACHES && !(rep->synced))
	{
		if(level < PRC_CACHES - 1)
		{
			sync_cache_line(c, level+1, NULL, *rep);
		}
		else
		{
			//TODO: write the cache line to memory
			//prisc_memrd()
		}
	}

	rep->filled = true;
	rep->synced = true;	// Since a replacement always comes from a higher cache, this line in the cache is synced with it's elder at the time of this update
	rep->addr = line.addr;
	for(index i=0; i<(1<<PRC_CACHELINEWIDTH); i++)
	{
		rep->line[i] = line.line[i];
	}
	// Could just use update_cache_access, but it's faster this way
	if(rep->next)
	{
		rep->next->prev = NULL;
		cch->first = (rep->next - cch->lines);
	}
	if(cch->last != cch->first)
	{
		cch->lines[cch->last].next = rep;
		rep->prev = cch->lines + cch->last;
		cch->last = repdex;
	}
	return (PRS_OPReturn){PRC_STATUS_SUCCESS, rep->addr, 1, 0};
}

int init_acache(PRS_ACache *cch, index size)
{
	if(!cch)
	{
		return 1;
	}
	cch->size = 0;
	cch->lines = NULL;
	cch->first = cch->last = size;
	if(!size)
	{
		return 1;
	}
	cch->lines = (PRS_ACacheLine *)malloc(size * sizeof(PRS_ACacheLine));
	if(!(cch->lines))
	{
		return 1;
	}
	cch->size = size;
	for(index i=0; i<size; i++)
	{
		cch->lines[i].filled = false;
		cch->lines[i].synced = false;
		cch->lines[i].next = (i^(size-1))?(cch->lines + (i+1)):NULL;
		cch->lines[i].prev = (i)?(cch->lines + (i-1)):NULL;
	}
	cch->first = 0;
	cch->last = size-1;
	return 0;
}

int init_memory_unit(PRS_MemUnit *mem)
{
	if(!mem)
	{
		return 1;
	}
	if(!init_primary_mem(mem->mem))
	{
		return 1;
	}
	for(index i=0; i<PRC_CACHES; i++)
	{
		if(init_acache(mem->caches+i), cache_sizes(i))
		{
			return 1;
		}
	}
	return 0;
}

PRS_OPReturn prisc_memrd(cpu c, word address, PRS_ACacheLine *lineout)
{
	if(!cpu->mem)
	{
		PRM_ERROR(PRC_E_NOMEM);
	}
	word cc = (((address & WORD(0xFFF00000))>>20) & WORD(0xFFF));
	word off = (address & WORD(0x000FFFFF));
	chunk rd = cpu->mem.chunks[cc].rd;
	if(!(rd->mem))
	{
		return (PRS_ACacheLineReturn){(PRS_OPReturn){PRC_STATUS_FALIURE, 0, 1, PRC_F_UNMAPPED_MEM}, line};
	}
	word offmask = ((word)(1<<(rd.wrap))-1)
	word linemask = offmask & (~((word)(1<<PRC_CACHELINEWIDTH)-1));
	off &= linemask;

	lineout->addr = address & (~((word)(1<<PRC_CACHELINEWIDTH)-1));

	for(index i=0; i<(1<<PRC_CACHELINEWIDTH); i++)
	{
		lineout->line[i] = rd->mem[off + (i & offmask)];
	}
	return (PRS_ACacheLineReturn){(PRS_OPReturn){PRC_STATUS_SUCCESS, 0, 1, PRC_F_UNMAPPED_MEM}, line};
}
