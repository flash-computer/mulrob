#ifndef PRC_CACHES
	#define PRC_CACHES 3
#endif

#ifndef PRC_ROBS
	#define PRC_ROBS 2
#endif

#ifndef PRC_BBENTRIES
	#define PRC_BBENTRIES 16
#endif

#ifndef PRC_INSTRUCTIONLOADERS
	#define PRC_INSTRUCTIONLOADERS 16
#endif

#ifndef PRC_LOADERS
	#define PRC_LOADERS 8
#endif

#ifndef PRC_STORERS
	#define PRC_STORERS 8
#endif

#ifndef PRC_ROBENTRIES
	#define PRC_ROBENTRIES 256
#endif

#ifndef PRC_CACHELINEWIDTH
	#define PRC_CACHELINEWIDTH 7
	#define PRC_CACHELINEMASK (~((word)(1<<PRC_CACHELINEWIDTH)-1))
	#define PRC_CACHELINESIZE ((word)(1<<PRC_CACHELINEWIDTH))
#endif

#ifndef PRC_BTBMASKWIDTH
	#define PRC_BTBMASKWIDTH 10
	#define PRC_BTBMASK (((word)(1<<(PRC_BTBMASKWIDTH + 2))) - 1)
	#define PRC_BTBENTRIES ((word)(1<<PRC_BTBMASKWIDTH))
#endif
