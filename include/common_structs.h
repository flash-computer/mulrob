typedef uint32_t word;
typedef uint16_t hword;
typedef uint8_t byte;

typedef uintmax_t umax;

typedef size_t index;

/* ----------------------------------- ENUMS ----------------------------------- */
/* ----------------------------------- ===== ----------------------------------- */
/* ----------------------------------- ===== ----------------------------------- */

typedef enum
{
	PRC_E_ALLCLEAR = 0,
	PRC_E_UNEXPECTED = 0x1,
	PRC_E_UNFITSTRUCTSTATE = 0x2,
	PRC_E_UNDEFINEDINSTRUCTION = 0x3
} PRS_ERRORS;

typedef enum
{
	// Faults
	PRC_FAULT_MEMACCESS = 0,
	PRC_FAULT_UNALIGNEDACCESS = 1,
	// Reset vector
	PRC_FAULT_RESET = (1<<PRC_FAULT_TABLE_WIDTH) - 1,
	// Not really faults
	PRC_FAULT_RETRY = (1<<PRC_FAULT_TABLE_WIDTH) + 1,
	PRC_FAULT_CACHEMISS = (1<<PRC_FAULT_TABLE_WIDTH) + 2
} PRS_FAULTS;

typedef enum
{
	PRC_WORD_SZ = 4,
	PRC_HWORD_SZ = 2,
	PRC_BYTE_SZ = 1
} PRS_WORD_SZ;

typedef enum
{
	PRC_MEMACCESS_READ = 0,
	PRC_MEMACCESS_WRITE = 1
} PRS_MEMACCESS_TYPE;

typedef enum
{
	PRC_STATUS_FALIURE = 0x0,
	PRC_STATUS_FREE = PRC_STATUS_FALIURE,	// Only defined in certain conditions, like if a loadstore unit is busy or free. Undefined otherwise.
	PRC_STATUS_SUCCESS = 0x1,
	PRC_STATUS_BUSY = 0x2,
	// For use by the decode unit
	PRC_DECODE_REG  = 0x0,
	PRC_DECODE_ADDRESS = 0x1
} PRS_STATUS;

typedef enum
{
	PRC_DESTTYPE_REG, PRC_DESTTYPE_MEM, PRC_DESTTYPE_FAULT, PRC_DESTTYPE_NOP
} PRS_DESTTYPE;

typedef enum
{
	PRC_RATENTRY_REG, PRC_RATENTRY_ROB
} PRS_RATENTRYTYPE;

/* ----------------------------------- STRUCTS ----------------------------------- */
/* ----------------------------------- ======= ----------------------------------- */
/* ----------------------------------- ======= ----------------------------------- */

typedef struct
{
	byte op;
	word s1;	// Source operand 1
	word s2;	// Source operand 2
} instruction;

typedef struct
{
	word i;
	word ops[3];
} PRS_RAWinstruction;

typedef struct
{
	PRS_STATUS success;	// Was operation successful
	word val; 	// Value of operation
	umax cyl;	// Cycle cost of operation
	word fault;	// If operation was unsuccessful, this is the fault that was generated
} PRS_OPReturn;

typedef struct
{
	word type;
	word iden;
} PRS_ROBDestEntry;

typedef struct
{
	PRS_ROBDestEntry dest;
	PRS_WORD_SZ width;
	word	val;
	bool	processed;
	instruction i;
	word	instruction_addr;
} PRS_ROBEntry;

typedef struct
{
	PRS_ROBEntry buffer[PRC_ROBENTRIES];
	word rt;	// Retirement Pointer
	word ex;	// Head Execution Pointer
	word pf;	// Prefetch Pointer
} PRS_ROBuffer;	// Reorder Buffer

typedef struct
{
	bool engaged;	// Is the Unit busy with writing to memory
	index store_index;
	umax rem_time;
	PRS_OPReturn opr;
	PRS_ROBDestEntry dest;
} PRS_RTUnit;

typedef struct
{
	PRS_OPReturn status;
	PRS_STATUS busy;	// Only reflects the business of the unit. Only when this variable is PRC_STATUS_SUCCESS, does the `status` variable reflect the status of the load.
	word addr;
	word val;
	PRS_WORD_SZ width;
} PRS_LoaderStorer;

typedef struct
{
	PRS_LoaderStorer iloaders[PRC_INSTRUCTIONLOADERS]; // Special loaders to aid the prefetcher in instruction fetching to it's internal instruction buffer specifically. Usually would load from the i-cache but for this (so far), there is so separation between data and instruction cache
	PRS_LoaderStorer loaders[PRC_LOADERS];
	PRS_LoaderStorer storers[PRC_STORERS];
} PRS_LSUnit;	// The Loadstore unit

typedef struct PRS_ACacheLine
{
	bool filled;
	bool synced;	// If the cache line is synced with the one above it
	word addr;
	byte line[1<<PRC_CACHELINEWIDTH];
	struct PRS_ACacheLine *next;	// Next in line to be replaced, using the LRU heuristic
	struct PRS_ACacheLine *prev;
} PRS_ACacheLine;

typedef struct PRS_ACache
{
	PRS_ACacheLine *lines;	// Array of CacheLines
	index size;
	index first;	// Index of the next CacheLine to replace.
	index last;
	struct PRS_ACache *next;
	struct PRS_ACache *prev;
} PRS_ACache;

typedef struct
{
	PRS_RATENTRYTYPE type;
	index ref;
} PRS_RATEntry;

typedef struct
{
	PRS_RATEntry entries[32];
} PRS_RATable;

typedef struct
{
	PRS_RATable rat;
	PRS_ACache l0;
} PRS_RATCacheEntry;

typedef struct
{
	PRS_RATCacheEntry rats[PRC_BBENTRIES];
} PRS_RATCache;

typedef struct
{
	word addr;
	bool predicted_taken;
	PRS_RATCacheEntry *rce;
} PRS_BBEntry;	// Branch Buffer Entry

typedef struct
{
	PRS_BBEntry bb[PRC_BBENTRIES];
	index cur;	// The currently tracked branch
	index las;	// The most recent registered branch in the buffer
} PRS_BBuffer;

typedef struct
{
	word regs[32];	// Registers 0-27 are valid registers. Registers 28-31 aren't physically there in hardware, I just feel 32 is a cleaner number

	PRS_RATable rat;
} PRS_PRFile;	// The Primary Register File

typedef struct
{
	byte *mem;
	byte width;	// Bitwidth of the memory chunk. The mem pointer should be valid for accessing all unsigned offsets fitting in this value
	bool wrap;	// If true, wrap the chunk after the bitwidth, otherwise fetch nulls;
} PRS_Chunk;

typedef struct
{
	PRS_Chunk rd;
	PRS_Chunk wr;
} PRS_RWChunk;

// TODO: Enshrine these widths in defs
typedef struct
{
	PRS_RWChunk chunks[0x1000];
} PRS_PrimaryMemory;

typedef struct
{
	PRS_PrimaryMemory mem;
	PRS_ACache caches[PRC_CACHES];
} PRS_MemUnit;

// We're just going to use a 2 bit saturating counter here
typedef struct
{
	word addr;
	word history:2;
} PRS_BTBufferEntry;

typedef struct
{
	PRS_BTBufferEntry btb[PRC_BTBENTRIES];	// The Branch Target Buffer
	#ifdef PRM_MULROB
		PRS_BBuffer bbs[PRC_ROBS];	// The Branch Queue Associated with each ROB
	#endif

} PRS_PFUnit;

typedef struct
{
	//TODO: Fill the execution unit struct with it's thigamajigs and reservation stations
} PRS_EXUnit;

/* ----------------------------------- CPU ----------------------------------- */
/* ----------------------------------- === ----------------------------------- */
/* ----------------------------------- === ----------------------------------- */

typedef struct
{
	PRS_RTUnit rtu;
	PRS_LSUnit lsu;

	PRS_PFUnit pfu;
	PRS_EXUnit exu;

	PRS_ROBuffer robs[PRC_ROBS];
	index crob;

	PRS_PRFile prf;
	PRS_RATable rat;

	umax rtu_stall;
	umax lsu_stall;
	umax pfu_stall;
	umax exu_stall;

	PRS_MemUnit mem;

	// c.cyl, I need you c.cyl
	umax cyl;
} PRS_cpu;
