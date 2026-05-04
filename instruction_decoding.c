#include<stdio.h>
#include<stdlib.h>
#include<bool.h>
#include<stdint.h>
#include<limits.h>

typedef unsigned char uch;
typedef uint32_t u32;
typedef uintmax_t umx;
typedef size_t szt;

#ifndef N_ROB
	#define N_ROB 1
#endif

#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define R15 15
#define R16 16
#define R17 17
#define R18 18
#define R19 19
#define R20 20
#define R21 21
#define R22 22
#define R23 23

#define R24 24
#define RIP R24	// Instruction Pointer

#define R25 25
#define RSP R25	// Stack Pointer

#define R26 26
#define RFP R26	// Frame Pointer

#define R27 27
#define RFG R27	// Flags Register

#define PRO_IM 28	// Immediate Proxy (Absolute for Destination)
#define PRO_AB 39	// Absolute Proxy (Indirect for Destination)
#define PRO_AF 30	// Absolute Address Formulation
#define PRO_FF 31	// Flag Formulation (Operand bytes skipped for destination (still counted in instruction length for simplicity))

PRS_OPReturn PreDecode(PRS_cpu *c, word inst)
{
	if(!c)
	{
		PRM_ERROR(PRC_E_UNEXPECTED);
	}
	for(szt i=0; i<N_ROB; i++)
	{
		if(((c->pf_ptr[i] - c->mem_offset) < mem_size) && (c->pf_ptr[i] >= c->mem_offset))
		{
			if(c->pf_ptr[i] & 0x3) // Instructions should be byte aligned
			{
				exit(2);
			}
			// TODO
			// if(c->mem[c->pf_ptr[i] - c->mem_offset] == BRANCH OPCODE) callback_prefetch_unit(i, c->pf_ptr[i]);
			c->pf_ptr[i] += InstructionLength(c, c->pf_ptr[i]); // Next Instruction
			continue;
		}
		exit(1);
	}
}

bool IsOperandProxy(cpu *c, u32 ptr)
{
	uch op = c->mem[ptr - c->mem_offset];
	return ((!(op & 0xF0)) || ((op & 0x1C) == 1C));
}

bool InstructionLength(cpu *c, u32 ptr)
{
	if(ptr & 0x3)
	{
		exit(2);
	}
	return (1 + IsOperandProxy(c, ptr+1) + IsOperandProxy(c, ptr+2) + IsOperandProxy(c, ptr+3))<<2;
}

// TODO: Replace these TODOs about bounds checking and the associated bounds checked access with a MACRO that queues issues a load in the ROB instead
// TODO: Change the post fetch register ops to prefetch width ops, as per the new ISA design
u32 DecodeSrcOperand(cpu *c, u32 ptr)
{
	if((ptr & 0x3) % 3)
	{
		uch op = c->mem[ptr - c->mem_offset];
		if(op & 0xF0)
		{
			uch imm = (op & 0x1F);
			switch(op & 0x60)
			{
				case 0x00:
					return imm;
				case 0x20:
					// TODO: Bounds checking
					return ((u32 *)(c->mem))[(c->prf[RSP] - c->mem_offset) - (imm<<2)];
				case 0x40:
					// TODO: Bounds checking
					return ((u32 *)(c->mem))[(c->prf[RFP] - c->mem_offset) + (imm<<2)];
				case 0x60:
					return (c->prf[RFG] & (1<<imm))>>imm;
				default:
					exit(0xF);
			}
		}
		else
		{
			uch ofs = (ptr & 0x1) ? 3 : ((IsOperandProxy(c, ptr-1)<<2) + 2);
			u32 val = 0;
			if((op & 0x1C) == 0x1C) // Is Proxy (3 input and gate)
			{
				// Would a Jump table be faster for such short switch statements?
				switch(op & 0x3)
				{
					case 0x0:
						// TODO: Just write a fucking wrapper function for memory access with Bounds Checking, for fuck's sake
						val = ((u32 *)(c->mem))[(ptr + ofs) - c->mem_offset];
						break;
					case 0x1:
						val = ((u32 *)(c->mem))[(((u32 *)(c->mem))[(ptr + ofs) - c->mem_offset]) - c->mem_offset];
						break;
					case 0x2:
						// TODO: Address Formulation
						break;
					case 0x3:
						// TODO: Flag Formulation
						break;
					default:
						exit(0xF);
				}
			}
			else
			{
				val = c->prf[op & 0x1F];
			}
			switch(op & 0x60)
			{
				case 0x00:
					break;
				case 0x20:
					val = ~val;
					break;
				case 0x40:
					// TODO: Only valid when running on an architecture with 2s complement integers
					val = -val;
					break;
				case 0x60:
					val = //ReverseBitstring(val);
				default:
					break;
			}
			return val;
		}
	}
	exit(2);
}
