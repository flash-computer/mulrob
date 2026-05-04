// TODO: Actually writing this logic
#ifdef PRM_MULROB

	void prefetch_unit(PRS_cpu *c) // If the project is being compiled for testing the multiple ROB logic
	{
		PRS_PFUnit pfu = &(c->pfu);
	}

#else

	void prefetch_unit(PRS_cpu *c) // If the project is being compiled with the simple single ROB logic
	{
	}

#endif
