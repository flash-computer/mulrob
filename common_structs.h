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
	PRC_WORD_SZ = 4,
	PRC_HWORD_SZ = 2,
	PRC_BYTE_SZ = 1
} PRS_WORD_SZ;

typedef enum
{
	PRC_STATUS_FALIURE = 0x0,
	PRC_STATUS_FREE = PRC_STATUS_FALIURE,	// Only defined in certain conditions, like if a loadstore unit is busy or free. Undefined otherwise.
	PRC_STATUS_SUCCESS = 0x1,
	PRC_STATUS_BUSY = 0x2
} PRS_STATUS;

typedef enum
{
	PRC_DESTTYPE_REG, PRC_DESTTYPE_MEM, PRC_DESTTYPE_FAULT
} PRS_DESTTYPE;

typedef enum
{
	PRC_RATENTRY_REG, PRC_REGENTRY_ROB
} PRS_RATENTRYTYPE;

/* ----------------------------------- STRUCTS ----------------------------------- */
/* ----------------------------------- ======= ----------------------------------- */
/* ----------------------------------- ======= ----------------------------------- */

typedef struct
{
	byte op;
	word s1;	// Source operand 1
	word s2;	// Source operand 2
}

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
	ROBDestEntry dest;
	PRS_WORD_SZ width;
	word	val;
	bool	processed;
	instruction i;
	word	instruction_addr;
} PRS_ROBEntry;

typedef struct
{
	ROBEntry buffer[PRC_ROBENTRIES];
	word rt;	// Retirement Pointer
	word ex;	// Head Execution Pointer
	word pf;	// Prefetch Pointer
} PRS_ROBuffer;	// Reorder Buffer

typedef struct
{
	bool engaged;	// Is the Unit busy with writing to memory
	index store_index;
	umax rem_time;
	PRS_OPreturn opr;
	PRS_ROBDestEntry dest;
} PRS_RetUnit;

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
	PRS_LoaderStorer loaders[PRC_LOADERS];
	PRS_LoaderStorer storer[PRC_STORERS];
} PRS_LSUnit;	// The Loadstore unit

typedef struct
{
	bool filled;
	bool synced;	// If the cache line is synced with the one above it
	word addr;
	byte line[1<<PRC_CACHELINEWIDTH];
	PRS_ACacheLine *next;	// Next in line to be replaced, using the LRU heuristic
	PRS_ACacheLine *prev;
} PRS_ACacheLine;

typedef struct
{
	PRS_ACacheLine *lines;	// Array of CacheLines
	index size;
	index first;	// Index of the next CacheLine to replace.
	index last;
} PRS_ACache;

typedef struct
{
	PRS_RATENTRYTYPE type;
	word ref;
} PRS_RATEntry;

typedef struct
{
	PRS_RATEntry entries[32];
} PRS_RATable;

typedef struct
{
	PRS_RATable rat;
	PRS_ACache *l0;
} PRS_RATCacheEntry;

typedef struct
{
	PRS_RATCacheEntry rats[PRC_BBENTRIES];
} PRS_RATCache;

typedef struct
{
	word addr;
	RATCacheEntry *rce;
} PRS_BBEntry;	// Branch Buffer Entry

typedef struct
{
	PRS_BBEntry bb[PRC_BBENTRIES];
} PRS_BBuffer;

typedef struct
{
	word regs[32];	// Registers 0-27 are valid registers. Registers 28-31 aren't physically there in hardware, I just feel 32 is a cleaner number

	PRS_RATable rat;
} PRS_PRFile;	// The Primary Register File

typedef struct
{
	byte *mem;
	byte width:10;	// Bitwidth of the memory chunk. The mem pointer should be valid for accessing all unsigned offsets fitting in this value
	bool wrap;	// If true, wrap the chunk after the bitwidth, otherwise fetch nulls;
} PRS_Chunk;

typedef struct
{
	chunk rd;
	chunk wr;
} PRS_RWChunk;

typedef struct
{
	mbchunk chunks[0x1000];
} PRS_PrimaryMemory;

typedef struct
{
	PRS_PrimaryMemory mem;
	PRS_ACache caches[PRC_CACHES];
}​ PRS_MemUnit;

/* ----------------------------------- CPU ----------------------------------- */
/* ----------------------------------- === ----------------------------------- */
/* ----------------------------------- === ----------------------------------- */

typedef struct
{
	PRS_RTUnit rtu;
	PRS_LSUnit lsu;

	PRS_PFUnit pfu;
	PRS_EXUnit exu;

	#ifdef PRM_MULROB
		PRS_ROBuffer robs[PRC_ROBS];
		index crob;
	#else
		PRS_ROBuffer rob;
	#endif

	PRS_PRFile prf;
	PRS_RATable rat;

	umax rtu_stall;
	umax lsu_stall;
	umax pfu_stall;
	umax exu_stall;

	PRS_MeUnit mem;
} PRS_cpu;

#ifdef PRM_MULROB
	#define PRM_CROB(c) ((c)->robs[(c)->crob])
#else
	#define PRM_CROB(c) ((c)->rob)
#endif
