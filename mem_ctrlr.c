#ifndef PRM_CACHE_SIZES
	#define PRM_CACHE_SIZES(level) cache_sizes
#endif

void update_cache_access(PRS_cpu *c, index level, PRS_ACacheLine *line) // Update the Cache Line linked list for LRU replacement
{
	if(level >= PRC_CACHES || !c || !line)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
	PRS_ACache *cch = (c->mem.caches + level);
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

PRS_OPReturn replace_cache_line(PRS_cpu *c, index level, PRS_ACacheLine line)
{
	if(level >= PRC_CACHES || !c)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
	PRS_ACache *cch = (c->mem.caches + level);
	if(cch->first >= cch->size)
	{
		PRM_ERROR(PRC_E_UNFITSTRUCTSTATE);
	}
	PRS_ACacheLine *rep = cch->lines[cch->first];
	index repdex = cch->first;

	rep->filled = true;
	rep->synced = line.synced;
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
