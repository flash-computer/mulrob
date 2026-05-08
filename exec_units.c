#ifndef PRM_INTERNALS
	#define PRM_INTERNALS
#endif
#include<mulrob.h>

const static PRS_OPReturn (*exec_units[])(PRS_cpu *, instruction) = {};

void nop(word src1, word src22)
{
	return;
}

void undefined_instruction(word src1, word src2)
{
	PRM_ERROR(PRC_E_UNDEFINEDINSTRUCTION);
}

PRS_OPReturn decode_src_operand(byte oper, word aug)
{
	// Number of entries to be made in the rob for this decoding to work
	index robcost = 0;
	if(oper & 0x80)
	{
		get_rat_ref(c, );
		switch(oper & 0x60)
		{
		}
	}
	else
	{
		switch(oper & 0x60)
		{
		}
	}
}

// The decoding unit advances the prefetch pointer in the ROB, by the way, not the prefetch unit
void decoding_unit(PRS_cpu *c, )
{
	if(!c)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
	word oppset = 1;
	PRS_OPReturn opr = decode_src_operand
}
