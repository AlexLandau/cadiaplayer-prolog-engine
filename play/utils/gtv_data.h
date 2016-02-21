//
// Data record for GTV.
//
#ifndef INCL_GTV_DATA
#define INCL_GTV_DATA

#include <stddef.h>

//#include "gtv.h"

typedef unsigned int Move_t;

typedef struct st_data {
  int    alpha;
  int    beta;
  int    depth;
  int    best;
  int    type;
} Data_t;

static int         GTVnumOptfields = 5;
static GTF_Field_t GTVOptfields[]  = 
                           { { "alpha", offsetof(Data_t,alpha) },
	 						 { "beta" , offsetof(Data_t,beta) },
							 { "depth", offsetof(Data_t,depth) },
							 { "best" , offsetof(Data_t,best)},
							 { "type" , offsetof(Data_t,type) } };

#define gtvFlagPV         1
#define gtvFlagCut        2
#define gtvFlagAll        4
#define gtvFlagResearch   8
#define gtvFlagQRoot     16
#define gtvFlagQNode     32
#define gtvFlagNull      64
#define gtvFlagNullVer	128
#define gtvFlagIID      256

#define toGTVFlag(t)     ( ((t)==NodePV) ? gtvFlagPV : ( ((t)==NodeCut) ? gtvFlagCut : gtvFlagAll ) )

extern "C" GTV_t gtv;

#endif