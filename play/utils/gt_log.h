/**
* ////////////////////////////////////////////////////////////////////////////
*  @file gt_log.h
*  GT-Tools
*
*  Created by Yngvi Bjornsson on Jan 2007.
*
* ////////////////////////////////////////////////////////////////////////////
*/
#ifndef INCL_GT_LOG_H
#define INCL_GT_LOG_H 

#include "gt_file.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_gt_log *GTLogHdl;  ///< Game-tree log-file handle.
        
GTLogHdl
/**
 Create a new game-tree-log handle.
        
 removed param[in] strFileName     Name of file.
 @param[in] strGameVersion  Tag to identify the game (may not contain white 
                            spaces).
 @param[in] p_datadescr    Pointer to the data-description structure.
        
 @return  A valid game-tree-log handle if successful, otherwise NULL.
*/
gtl_newHdl( const char *strGameVersion, const GTDataDescript *p_datadescr );


void 
/**
 Deletes a game-tree-log handle.
    
 @param[in,out] p_hGTL Pointer to game-tree log-file handle, handle will be 
                       set to NULL.
*/
gtl_deleteHdl( GTLogHdl *p_hGTL );


GTErrorNo
/**
 Create a new game-tree-log file. Subsequent nodes are looged to this file.
    
 @param[in] hGTL         Game-tree-log handle.
 @param[in] strFileName  Name of file.
 @param[in] strStartPos  String description of start position (e.g. FEN).
    
 @return  GT_ERR_NONE if successful, otherwise a descriptive errorcode.
*/
gtl_startTree(GTLogHdl hGTL, const char *strFileName, const char *strStartPos);


GTErrorNo
/**
 Enter a game-tree node.
    
 @param[in] hGTL      Game-tree-log handle.
 @param[in] moveLast  Move that led to current game position.

 @return  GT_ERR_NONE if successful, otherwise a descriptive errorcode.
*/
gtl_enterNode( GTLogHdl hGTL, GTMove moveLast );


GTErrorNo
/**
 Exit a game-tree node.
    
 @param[in]  hGTL      Game-tree-log handle.
 removed param[in]  moveLast  Move that led to current game position.
 @param[in] *record    Pointer to extended data record.

 @return  GT_ERR_NONE if successful, otherwise a descriptive errorcode.
*/
gtl_exitNode( GTLogHdl hGTL, const void *record );


GTErrorNo
/*
 Closes game-tree-file associated with log handle.
 Must now call gtl_startTree(), before writing any nodes.

 @param[in]  hGTL      Game-tree-log handle.
*/
gtl_stopTree( GTLogHdl hGTL );


#ifdef __cplusplus
}
#endif

#endif