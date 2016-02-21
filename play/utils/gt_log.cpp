/**
 * ////////////////////////////////////////////////////////////////////////////
 *  @file gt_log.c
 *  GT-Tools
 *
 *  Created by Yngvi Bjornsson on Jan 2007.
 *
 * ////////////////////////////////////////////////////////////////////////////
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gt_log.h"

#define INITIAL_STACK_SIZE  500

// Copies at most n characters to dst; dst must be able to hold at
// least n+1 characters because of the zero terminating character.
#define SAFE_STRCPY(dst, src, n) { strncpy((dst),(src),(n)); dst[n+1]='\0'; }

struct st_gt_log {
  char            strGameVersion[GT_MAX_SZ_STR+1];
  GTDataDescript  dataDescr;
  GTFileHdl       hGTF;
  unsigned int    ply;
  unsigned int    size_stack;
  GTNode         *stack;
};


GTLogHdl gtl_newHdl( const char *strGameVersion, 
                     const GTDataDescript *p_dataDescr )
{
  GTLogHdl hGTL = NULL;

  assert( strGameVersion != NULL );
  assert( p_dataDescr != NULL );    
    
  hGTL = (GTLogHdl) malloc( sizeof(struct st_gt_log) );  
  if ( hGTL != NULL ) {
      SAFE_STRCPY( hGTL->strGameVersion, strGameVersion, GT_MAX_SZ_STR ); 
      memcpy( &hGTL->dataDescr, p_dataDescr, sizeof(GTDataDescript) );
      hGTL->hGTF        = NULL;
      hGTL->ply         = 0;
      hGTL->size_stack  = INITIAL_STACK_SIZE;
      hGTL->stack  = (GTNode*) malloc( hGTL->size_stack * sizeof(GTNode) );
      if ( hGTL->stack != NULL ) {
         hGTL->stack[0].nidNode        = 0;
         hGTL->stack[0].nidLastChild   = 0;
         hGTL->stack[0].nidPrevSibling = 0;
      }
      else {
         free( hGTL );
         hGTL = NULL;
      }      
  }  
  return hGTL;
}


void gtl_deleteHdl( GTLogHdl *p_hGTL )
{
    assert(  p_hGTL != NULL );
    
    if ( *p_hGTL != NULL ) {
        GTLogHdl hGTL = *p_hGTL;
        if ( hGTL->hGTF != NULL ) { gtf_closeFile( &(hGTL->hGTF) ); }
        free( hGTL->stack );
        *p_hGTL = NULL;
    }
}


GTErrorNo gtl_startTree( GTLogHdl hGTL, 
                         const char *strFileName, const char *strStartPos )
{  
  assert( hGTL != NULL && hGTL->stack != NULL );
  assert( hGTL->stack != NULL );
  assert( strFileName != NULL ); 
  assert( strStartPos != NULL );

  if ( hGTL == NULL ) return GT_ERR_ILLEGAL_HDL;

  if ( hGTL->hGTF != NULL ) { gtf_closeFile( &(hGTL->hGTF) ); }
  hGTL->hGTF = gtf_createFile(strFileName, hGTL->strGameVersion, 
                              strStartPos, &hGTL->dataDescr);
    
  hGTL->ply                     = 0;
  hGTL->stack[0].nidNode        = GT_NID_NONE;
  hGTL->stack[0].nidLastChild   = GT_NID_NONE;
  hGTL->stack[0].nidPrevSibling = GT_NID_NONE;
  
  return ( hGTL->hGTF != NULL ) ? GT_ERR_NONE : GT_ERR_OPEN_FILE;
}


GTErrorNo gtl_stopTree( GTLogHdl hGTL )
{
    assert( hGTL != NULL );
    if ( hGTL == NULL ) return GT_ERR_ILLEGAL_HDL;

    if ( hGTL->hGTF != NULL ) { gtf_closeFile( &(hGTL->hGTF) ); }
    hGTL->hGTF = NULL;
    
    return GT_ERR_NONE;
}


GTErrorNo gtl_enterNode( GTLogHdl hGTL, GTMove moveLast )
{
  assert( hGTL != NULL );
  if ( hGTL == NULL ) return GT_ERR_ILLEGAL_HDL;
    
  hGTL->ply++;
  assert( hGTL->ply < hGTL->size_stack );

  if (  hGTL->ply < hGTL->size_stack ) {
      hGTL->stack[hGTL->ply].nidNode        = GT_NID_NONE;
      hGTL->stack[hGTL->ply].nidLastChild   = GT_NID_NONE;
      hGTL->stack[hGTL->ply].nidPrevSibling = GT_NID_NONE;
      hGTL->stack[hGTL->ply].move           = moveLast;
  }
  return GT_ERR_NONE;
} 


GTErrorNo gtl_exitNode( GTLogHdl hGTL, const void *record )
{
  GTErrorNo error = GT_ERR_NONE;
    
  assert( hGTL != NULL );
  if ( hGTL == NULL ) return GT_ERR_ILLEGAL_HDL;
    
  hGTL->stack[hGTL->ply].nidNode        = GT_TO_BE_ASSIGNED;
  hGTL->stack[hGTL->ply].nidPrevSibling = hGTL->stack[hGTL->ply-1].nidLastChild;
  error = gtf_appendNode( hGTL->hGTF, &(hGTL->stack[hGTL->ply]), record );
  hGTL->stack[hGTL->ply-1].nidLastChild = hGTL->stack[hGTL->ply].nidNode;
  hGTL->ply--;
  
  return error;
}
