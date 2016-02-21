/**
 * ////////////////////////////////////////////////////////////////////////////
 *  @file gt_file.h
 *  GT-Tools
 *
 *  Created by Yngvi Bjornsson on Jan 2007.
 *
 * ////////////////////////////////////////////////////////////////////////////
 */
#ifndef INCL_GT_FILE_H
#define INCL_GT_FILE_H 

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
    
#define GT_MAX_NUM_ALIASES    20
#define GT_MAX_NUM_FIELDS     20
#define GT_MAX_SZ_STR         20 

#define GT_NID_NONE           0
#define GT_TO_BE_ASSIGNED     0

#define GT_ERR_NONE           0
#define GT_ERR_ILLEGAL_HDL    1
#define GT_ERR_ILLEGAL_ARG    2
#define GT_ERR_ILLEGAL_NID    3
#define GT_ERR_WRITE_FILE     4
#define GT_ERR_READ_FILE      5
#define GT_ERR_OPEN_FILE      6

typedef int          GTErrorNo; ///< Error code.

typedef unsigned int GTNodeId;  ///< Node identifier.

typedef unsigned int GTMove;    ///< Move representation in game-tree. 

/// GTNode description structure.
typedef struct {
    GTNodeId nidNode;        ///< Nodeid of current node.
    GTNodeId nidLastChild;   ///< Nodeid of the last child of the current node.
    GTNodeId nidPrevSibling; ///< Nodeid of current node's previous sibling node.
    GTMove   move;
} GTNode; 

/// Data description structure.
typedef struct {
    char     strDataVersion[GT_MAX_SZ_STR+1];
    unsigned int recSize;

    unsigned int numAliases;
    struct {
        char  strName[GT_MAX_SZ_STR+1];
        int   value;
    } alias[GT_MAX_NUM_ALIASES]; ///< Alias information.

    unsigned int numFields;
    struct {
        char          strName[GT_MAX_SZ_STR+1];
        unsigned int  offsetInRecord;
        unsigned int  sizeInBytes;           
    } field[GT_MAX_NUM_FIELDS]; ///< Field information.
   
} GTDataDescript;  


typedef struct st_gt_file *GTFileHdl;  ///< Game-tree file handle.


GTFileHdl
/**
 Create a new game-tree file.
 
 @param[in] strFileName    Name of file.
 @param[in] strGameVersion Tag to identify the game (may not contain white 
                           spaces).
 @param[in] strStartPos    String description of start position (e.g. FEN). 
 @param[in] p_datadescr    Pointer to the data-description structure.
        
 @return  A valid game-tree file handle if successful, otherwise NULL.
*/
gtf_createFile( const char *strFileName, const char *strGameVersion, 
                const char *strStartPos, const GTDataDescript *p_datadescr );


GTFileHdl
/**
 Open an exisiting game-tree file.
        
 @param[in] strFileName  Name of the file to open.
 
 @return  A valid game-tree file handle if successful, otherwise NULL.
*/
gtf_openFile( const char *strFileName );


void 
/**
 Closes an open game-tree file and deletes the handle.
    
 @param[in,out] p_hGTF Pointer to game-tree handle, handle will be set to NULL.
*/
gtf_closeFile( GTFileHdl *p_hGTF );


const char
/**
 Get the version number of the game.
 
 @param[in] hGTF  Game-tree file handle.
*/
*gtf_getGameVersion( GTFileHdl hGTF );


const GTDataDescript*
/**
 Get data description.

 @param[in] hGTF  Game-tree file handle.
*/
gtf_getDataDescription( GTFileHdl hGTF );


GTNodeId
/**
 Get the number of nodes in the file. 

 @param[in] hGTF  Game-tree file handle.
 
 @return  Number of nodes in the file.
 */
gtf_getNumNodes( GTFileHdl hGTF );


GTNodeId
/**
 Get the node-id of the root node of the tree (start point for navigating).
        
 @param[in] hGTF  Game-tree file handle.
 
 @return  Node id of root node (id will be GT_NID_NONE, if tree is empty).
*/
gtf_getNodeIdRoot( GTFileHdl hGTF );


GTErrorNo
/**
 Get the node identifier of the root of the game tree.

 @param[in]   hGTF   Game-tree file handle.
 @param[in]   nid    Node-id of record to read.
 @param[out] *p_node Pointer to a node-data record.
 @param[out] *record Pointer to an extended data record. Must be at least 
                     \p recSize long (see GTDataDescript).

 @return  GT_ERR_NONE if successful, otherwise a descriptive errorcode.
*/
gtf_readNode( GTFileHdl hGTF, GTNodeId nid, GTNode *p_node, void *record );


GTErrorNo
/**
 Append node to the game-tree file, automatically assigning the node a node-id.
 Works only with handles returned by gtf_createFile().
 
 @param[in]      hGTF    Game-tree file handle.
 @param[in,out] *p_node  Pointer to a completed node-data record. All node-id's
                         in the record must be set to existing node id's (i.e. 
                         of nodes already appeneded to the file), except for
                         the current node id (\p nidNode) which must be set to
                         GT_NID_AUTOASSIGN. Upon returning, the \p .nid will
                         hold the node id for the node just appended.
  @param[in]    *record  Pointer to extended data record.

  @return  GT_ERR_NONE if successful, otherwise a descriptive errorcode.
*/
gtf_appendNode( GTFileHdl hGTF, GTNode *p_node, const void *record );


int
/**
Get value of a field within extended data record.
 
 @param[in]   hGTF   Game-tree file handle.
 @param[in]   idx    Index of field in data-description field stucture.
 @param[out] *record Pointer to an extended data record.
 
 @return      Value of requested integer field.
 */
gtf_getFieldIntValue( GTFileHdl hGTF, unsigned int idx, const void *record );


#ifdef __cplusplus
}
#endif
    
#endif
