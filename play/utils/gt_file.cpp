/**
 * ////////////////////////////////////////////////////////////////////////////
 *  @file gt_file.c
 *  GT-Tools
 *
 *  Created by Yngvi Bjornsson on Jan 2007.
 *	@todo Log to file FEN str for start pos.
 *
 * ////////////////////////////////////////////////////////////////////////////
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <ctype.h>
#include "gt_file.h"

#define MIN(a,b)  ( ((a)<(b)) ? (a) : (b) )
// Copies at most n characters to dst; dst must be able to hold at
// least n+1 characters because of the zero terminating character.
#define SAFE_STRCPY(dst, src, n) { strncpy((dst),(src),(n)); dst[n+1]='\0'; }


// Map node-ids to/from file locations.
#define GT_RECORD_NID_TO_LOC(H, nid) \
    ( (H)->headerOffset + (sizeof(GTNode)+(H)->dataDescr.recSize)*((nid)-1) ) 

#define GT_RECORD_LOC_TO_NID(H, loc) \
    ( ( ((loc)-(H)->headerOffset) / (sizeof(GTNode)+(H)->dataDescr.recSize)) + 1 ) 

#define GT_RECORD_LOC_TO_RECNO(H, loc) \
    ( ( ((loc)-(H)->headerOffset) / (sizeof(GTNode)+(H)->dataDescr.recSize) ) ) 


// Game-tree file handle structure, hidden to the outside.
struct st_gt_file {
   char            strGameVersion[GT_MAX_SZ_STR+1];
   FILE           *fp;           ///< File pointer to file reading/appending.
   int             isWritable;   ///< Is file opened for writing?.
   GTNodeId        nidNext;      ///< Next node-id to be assigned by append.
   long            headerOffset; ///< Offset (bytes) to first node record in file.
   GTDataDescript  dataDescr;    ///< Description of the data being logged.
};


// Returns 1 if str contains a white-space, otherwise 0.
int spaceInStr( const char *str ) 
{
    unsigned int i=0;
    for ( i=0; i<strlen(str) ; ++i ) {
        if ( isspace( str[i] ) ) return 1;
    }
    return 0;
}

GTFileHdl gtf_createFile( const char *strFileName, 
                          const char *strGameVersion, 
                          const char *strStartPos, 
                          const GTDataDescript *p_dataDescr )
{
    GTFileHdl     hGTF  = NULL;
    GTErrorNo     errNo = GT_ERR_NONE; 
    FILE         *fp    = NULL;
    unsigned int  i     = 0;
    
    assert( strFileName != NULL );
    assert( strGameVersion != NULL );
    assert( p_dataDescr != NULL );    

    // Make sure arguments are okay, e.g. none white-spaces in tag strings.
    if ( strFileName==NULL || strGameVersion==NULL || p_dataDescr==NULL ) { 
        errNo = GT_ERR_ILLEGAL_ARG;
    }
    else if ( spaceInStr( strGameVersion )  
         || ( p_dataDescr->numAliases > GT_MAX_NUM_ALIASES )
         || ( p_dataDescr->numFields  > GT_MAX_NUM_FIELDS ) ) {
        errNo = GT_ERR_ILLEGAL_ARG;
    }
    else {    
        for ( i=0; errNo==GT_ERR_NONE && i<p_dataDescr->numAliases; ++i ) {
            if ( spaceInStr( p_dataDescr->alias[i].strName ) ) 
                { errNo = GT_ERR_ILLEGAL_ARG; }
        }
        for ( i=0; errNo==GT_ERR_NONE && i<p_dataDescr->numFields; ++i ) {
            if ( spaceInStr( p_dataDescr->field[i].strName ) ) 
                { errNo = GT_ERR_ILLEGAL_ARG; }
        }
    }
    assert( errNo == GT_ERR_NONE );
    if ( errNo != GT_ERR_NONE ) { return NULL; }

    // Arguments seem to be okay, continue on to create file. 
    // We open first in write mode, to truncate if exist, then reopen in
    // append mode, to ensure that new writes are always appended irrespective
    // of fseeks.
    fp = fopen( strFileName, "wb" );
    if ( fp != NULL ) { fp = freopen( strFileName, "ab", fp ); }    
    if ( fp != NULL ) {
        hGTF = (GTFileHdl) malloc( sizeof(struct st_gt_file) );
        if ( hGTF != NULL ) {
            hGTF->fp         = fp;
            hGTF->isWritable = 1;
            hGTF->nidNext    = 0;
            SAFE_STRCPY( hGTF->strGameVersion, strGameVersion, GT_MAX_SZ_STR ); 
            memcpy( &(hGTF->dataDescr), p_dataDescr, sizeof(GTDataDescript) );         
            fprintf( hGTF->fp, "%s %s %u %u %u\n", 
                     hGTF->strGameVersion, 
                     hGTF->dataDescr.strDataVersion,
                     hGTF->dataDescr.recSize, 
                     hGTF->dataDescr.numAliases,
                     hGTF->dataDescr.numFields );
            for ( i=0; i < hGTF->dataDescr.numAliases; ++i ) {
                fprintf( hGTF->fp, "%s %d\n", 
                         hGTF->dataDescr.alias[i].strName,
                         hGTF->dataDescr.alias[i].value );
            }
            for ( i=0; i < hGTF->dataDescr.numFields; ++i ) {
                fprintf( hGTF->fp, "%s %u %u\n", 
                         hGTF->dataDescr.field[i].strName,
                         hGTF->dataDescr.field[i].offsetInRecord,
                         hGTF->dataDescr.field[i].sizeInBytes );
            }
            hGTF->nidNext = 1;
            hGTF->headerOffset = ftell( hGTF->fp );
        }
        else { fclose( hGTF->fp ); }
    }
    
    return hGTF;
}


GTFileHdl gtf_openFile( const char *strFileName )
{
    GTErrorNo  errNo   = GT_ERR_NONE;
    int        numRead = 0;
    GTFileHdl  hGTF    = NULL;
    FILE      *fp      = NULL;

    assert( strFileName != NULL );
    if ( strFileName == NULL ) { return NULL; }
    
    fp = fopen( strFileName, "rb" );
    if ( fp != NULL ) {
        hGTF = (GTFileHdl) malloc( sizeof(struct st_gt_file) );
        if ( hGTF != NULL ) {        
            char strFormatHeader[99], strFormatField[99], strFormatAliases[99];
            sprintf( strFormatHeader, "\%%%us %%%us %%u %%u %%u\n", 
                     (unsigned int)(sizeof(hGTF->strGameVersion)-1), 
                     (unsigned int)(sizeof(hGTF->dataDescr.strDataVersion)-1) );
            sprintf( strFormatField, "\%%%us %%u %%u\n", 
                     (unsigned int)(sizeof(hGTF->dataDescr.field[0].strName)-1) );
            sprintf( strFormatAliases, "\%%%us %%d\n", 
                     (unsigned int)(sizeof(hGTF->dataDescr.alias[0].strName)-1) );
            hGTF->fp = fp;
            hGTF->isWritable = 0;
            numRead = fscanf( hGTF->fp, strFormatHeader, 
                              hGTF->strGameVersion, 
                              hGTF->dataDescr.strDataVersion,
                              &hGTF->dataDescr.recSize, 
                              &hGTF->dataDescr.numAliases, 
                              &hGTF->dataDescr.numFields
                              );
            assert( numRead == 5 );
            if ( numRead == 5 ) {
                int i = 0;
                hGTF->dataDescr.numAliases = MIN( GT_MAX_NUM_ALIASES, hGTF->dataDescr.numAliases );
                for ( i=0; i < hGTF->dataDescr.numAliases && errNo == GT_ERR_NONE; ++i ) {
                    numRead = fscanf( hGTF->fp, strFormatAliases, 
                                      hGTF->dataDescr.alias[i].strName,
                                      &hGTF->dataDescr.alias[i].value );
                    assert( numRead == 2 );
                    if ( numRead != 2 ) { errNo = GT_ERR_READ_FILE; }
                }
                hGTF->dataDescr.numFields = MIN( GT_MAX_NUM_FIELDS, hGTF->dataDescr.numFields );
                for ( i=0; i < hGTF->dataDescr.numFields && errNo == GT_ERR_NONE; ++i ) {
                    numRead = fscanf( hGTF->fp, strFormatField, 
                                     hGTF->dataDescr.field[i].strName,
                                    &hGTF->dataDescr.field[i].offsetInRecord,
                                    &hGTF->dataDescr.field[i].sizeInBytes );
                    assert( numRead == 3 );
                    if ( numRead != 3 ) { errNo = GT_ERR_READ_FILE; }
                    // NOTE: todo, assert that not larger than record size.
                }
                hGTF->headerOffset = ftell( hGTF->fp );
                if ( hGTF->headerOffset == -1 ) { errNo = GT_ERR_READ_FILE; }                        
                fseek( hGTF->fp, 0, SEEK_END );  
                hGTF->nidNext = GT_RECORD_LOC_TO_NID( hGTF, ftell( hGTF->fp ) );                
            }
            else { errNo = GT_ERR_READ_FILE; }
        }
        
        assert( errNo == GT_ERR_NONE );
        if ( errNo != GT_ERR_NONE )  { 
            if ( fp   != NULL ) { fclose( fp ); }
            if ( hGTF != NULL ) { free( hGTF ); hGTF = NULL; }             
        }
    }
    
    return hGTF;
}


void gtf_closeFile( GTFileHdl *p_hGTF )
{
    assert( p_hGTF != NULL );
    
    if ( *p_hGTF != NULL ) {
          fclose( (*p_hGTF)->fp );
          free( *p_hGTF );
       *p_hGTF = NULL;
    }
}


const char* gtf_getGameVersion( GTFileHdl hGTF )
{
    assert( hGTF != NULL );
    if ( hGTF == NULL ) return NULL;
    
    return hGTF->strGameVersion;
}


const GTDataDescript* gtf_getDataDescription( GTFileHdl hGTF )
{
    assert( hGTF != NULL );
    if ( hGTF == NULL ) return NULL;
    
    return &(hGTF->dataDescr);
}


unsigned int gtf_getNumNodes( GTFileHdl hGTF )
{
    assert( hGTF != NULL );
    assert( hGTF->fp != NULL );
    if ( hGTF == NULL ) return 0;
    if ( hGTF->fp == NULL ) return 0;
    
    fseek( hGTF->fp, 0, SEEK_END );  
    return GT_RECORD_LOC_TO_RECNO( hGTF, ftell(hGTF->fp) );
}


GTNodeId gtf_getNodeIdRoot( GTFileHdl hGTF )
{
    GTNodeId nid = 0;
    
    assert( hGTF != NULL );
    assert( hGTF->fp != NULL );
    if ( hGTF == NULL ) return 0;
    if ( hGTF->fp == NULL ) return 0;

    fseek( hGTF->fp, 0, SEEK_END );  
    nid = GT_RECORD_LOC_TO_NID( hGTF, ftell( hGTF->fp ) ) - 1;

    return nid;
}


GTErrorNo gtf_readNode( GTFileHdl hGTF, GTNodeId nid, GTNode *p_node, 
                        void *record )
{
    GTErrorNo errNo = GT_ERR_NONE;
    
    assert( hGTF != NULL );
    if ( hGTF == NULL ) return GT_ERR_ILLEGAL_HDL;

    assert( nid > 0 && nid < hGTF->nidNext );
    if ( !(nid > 0 && nid < hGTF->nidNext) ) return GT_ERR_ILLEGAL_ARG;

    if ( fseek( hGTF->fp, GT_RECORD_NID_TO_LOC(hGTF, nid), SEEK_SET ) != 0 ) {
       errNo = GT_ERR_READ_FILE;
    }    
    else if ( fread( p_node, sizeof(GTNode), 1, hGTF->fp ) != 1 ) {
       errNo = GT_ERR_READ_FILE;
    }
    else if ( fread( record, hGTF->dataDescr.recSize, 1, hGTF->fp ) != 1 ) {
       errNo = GT_ERR_READ_FILE;
    }

    return errNo;
}


GTErrorNo gtf_appendNode( GTFileHdl hGTF, GTNode *p_node, const void *record )
{
    GTErrorNo errNo = GT_ERR_NONE;
    
    assert( hGTF != NULL );
    assert( hGTF->isWritable != 0 );
    assert( p_node != NULL );
    assert( p_node->nidNode == GT_TO_BE_ASSIGNED );
    
    if ( hGTF == NULL ) return GT_ERR_ILLEGAL_HDL;
    if ( hGTF->isWritable == 0 ) return GT_ERR_ILLEGAL_HDL;
    if ( p_node == NULL ) return GT_ERR_ILLEGAL_ARG;
    if ( p_node->nidNode != GT_TO_BE_ASSIGNED ) return GT_ERR_ILLEGAL_NID;

    p_node->nidNode = hGTF->nidNext;
    
    if ( fwrite( p_node, sizeof(GTNode), 1, hGTF->fp ) != 1 ) {
       errNo = GT_ERR_WRITE_FILE;
    }
    else if ( fwrite( record, hGTF->dataDescr.recSize, 1, hGTF->fp )  != 1 ) {
       errNo = GT_ERR_WRITE_FILE;
    }
    else {
       hGTF->nidNext++;
    }

    return errNo;
}


int gtf_getFieldIntValue( GTFileHdl hGTF, unsigned int idx, const void *record )
{
    assert( hGTF != NULL );
    assert( idx < hGTF->dataDescr.numFields );
    assert( record != NULL );
    
 	return (int) *( (const char*)record + hGTF->dataDescr.field[idx].offsetInRecord );
    
}


