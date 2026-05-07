#ifndef PRM_STDLIB
	#include<stdio.h>
	#include<stdlib.h>
	#include<stdbool.h>
	#include<stdint.h>
	#include<limits.h>

	#define PRM_STDLIB
#endif

#ifndef PRM_CONFIGURABLES_DEFINED
	#include"configurable.h"
#endif

#ifndef PRM_ESSESTIALS_DEFINED
	#define SEASALT (c->cyl)
	#define INEEDYOUSEASALT {SEASALT++;}
	#define MAKEASTEAK(adv) {SEASALT += (adv);}

	// Rememeber to do a bounds check before using these
	#define PRM_REF_REG(c, reg) ((c)->prf.regs[(reg)])
	#define PRM_WRITE_REG(c, reg, val, width) {if(width < PRC_WORD_SZ){word writeregmacro_mask = ((1<<(width<<3))-1); PRM_REF_REG((c), (reg)) = ((PRM_REF_REG((c), (reg)) & (~writeregmacro_mask)) | ((val) & (writeregmacro_mask)));}else{PRM_REF_REG((c), (reg)) = (val);}}

	// You can switch this with any other callback if you so want
	#ifndef PRM_ERROR
		#define PRM_ERROR(error) {fprintf(stderr, "\x1b[31;1mFATAL ERROR: \x1b[0mcode: %d at (%s,%d)", (error), __FILE__, __LINE__); exit(1);};
	#endif

	#define PRC_FAULT_TABLE_WIDTH 6
	#define PRC_FAULT_TABLE_ADDR (~(((word)(1<<(PRC_FAULT_TABLE_WIDTH + 2))) - 1))

	#define PRM_ESSENTIALS_DEFINED

	#define PRM_CROB(c) ((c)->robs[(c)->crob])

	#define PRM_ROB_RT(rob) (rob.buffer[rob.rt])

	// TODO: Placeholder. Fix.
	#define PRM_ISBRANCH(op) (op)

	#define PRM_PRF_REG(c, reg) ((c)->prf.regs[(reg)])

	#define PRM_ROBCMP(bf, af) ((af + PRC_ROBENTRIES - bf)%PRC_ROBENTRIES)
#endif

#ifndef PRM_TYPES
	#include"common_structs.h"
	#define PRM_TYPES
#endif

// The current design will only work for 2 RoBs at most though
#ifndef PRM_MULROB
	#define PRM_ROBS 1
#else
	#ifndef PRM_ROBS
		#define PRM_ROBS 2
	#endif
#endif

#if defined(PRM_INTERNALS) && !defined(PRM_INTERNALS_DEFINED)
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

	#define PR28 28
	#define PRIM PR28	// Immediate Proxy

	#define PR29 29
	#define PRAB PR29	// Absolute Proxy

	#define PR30 30
	#define PRAF PR30	// Address Formulation Proxy

	#define PR31 31
	#define PRFF PR31	// Flag Formulation Proxy

	#define PRM_INTERNALS_DEFINED
#endif

/* ----------------------------------- PROTOTYPES ----------------------------------- */
/* ----------------------------------- ========== ----------------------------------- */
/* ----------------------------------- ========== ----------------------------------- */

// Default implementations are provided, but not expected to necessarily be used for these
#ifndef PRM_ESSENTIAL_OPTIONAL_PROTOTYPES
	umax cache_fetch_costs(index level); // Should return a valid value for PRC_CACHES + 1(for primary memory) levels

	index cache_sizes(index level); // Should return a valid value for PRC_CACHES levels

	umax rob_periodicity(index priority); // How many cycles should elapse between fetching into a particular ROB, based on it's priority

	#define PRM_ESSENTIAL_OPTIONAL_PROTOTYPES
#endif

// Only the default implementations are necessarily supported for these
#ifndef PRM_ESSENTIAL_PROTOTYPES
	void init_loadstore_unit(PRS_LSUnit *lsu);
	PRS_OPReturn write_mem_loadstore(PRS_cpu *c, word addr, word val, PRS_WORD_SZ width);
	PRS_OPReturn read_mem_loadstore(PRS_cpu *c, word addr, PRS_WORD_SZ width);
	PRS_OPReturn query_store_finished(PRS_cpu *c, index idx);
	PRS_OPReturn query_load_finished(PRS_cpu *c, index idx);

	void update_cache_access(PRS_cpu *c, index level, PRS_ACache *cch, PRS_ACacheLine *line);
	PRS_OPReturn query_data_in_cache(PRS_ACache *cch, word addr, PRS_WORD_SZ width);
	int init_acache(PRS_ACache *cch, index size);
	int init_memory_unit(PRS_MemUnit *mem);
	// TODO: check if the usage in mem_ctrlr.c is correct
	int init_primary_mem(PRS_PrimaryMemory *mem);
	PRS_OPReturn prisc_memrd(PRS_cpu *c, word address, PRS_ACacheLine *lineout);

	bool ask_rat(PRS_cpu *c, index dest, index roben);
	void rat_update(PRS_cpu *c, index dest, index roben);
	void save_rat(PRS_cpu *c, PRS_BBuffer *bb, index bbdex);
	void broadcast_rtu_retirement(PRS_cpu *c, index roben);

	void prefetch_unit(PRS_cpu *c);
	void retirement_unit(PRS_cpu *c);
	void loadstore_unit(PRS_cpu *c);
	void execution_unit(PRS_cpu *c);
	void timer_unit(PRS_cpu *c);

	void purge_retirement_unit(PRS_cpu *c);
	void purge_execution_unit(PRS_cpu *c);
	void purge_loadstore_unit(PRS_cpu *c);
	void purge_prefetch_unit(PRS_cpu *c);
	void reset_rob(PRS_ROBuffer *rob);

	void reset_cpu(PRS_cpu *cpu);

	#define PRM_ESSENTIAL_PROTOTYPES
#endif
