/* This file is part of the KDE libraries
    Copyright (C) 1997 Christian Esken (esken@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include <string.h>
#include "chunk.h"
#include "mediatool.h"

void WriteEndChunkHere(char *adress);

/******************************************************************************
 *
 * Function:	FindChunkData()
 *
 * Task:	Return the memory adress of the data portion of a chunk.
 *
 * in:		adress		Adress of memory piece to examine. Must be a
 *				valid (initialized) mediatool memory piece.
 *				Checks are done to ensure this.
 *		ChunkName	Chunk to search for. Must be given as a 4
 *				byte Ascii string.
 * 
 * out:		MdChunk*  	Memory adress of the data portion of thechunk.
 *				NULL, if chunk not exist.
 *
 *****************************************************************************/
void *FindChunkData(char *adress, char *ChunkName)
{
  MdChunk	*chunk;
  char		*TmpAdr;

  chunk = FindChunk(adress, ChunkName);
  if ( chunk == NULL)
    return (NULL);

  TmpAdr = (char*)chunk;
  TmpAdr += sizeof(MdChunk);
  chunk = (MdChunk*)TmpAdr;
  return(chunk);
}

/******************************************************************************
 *
 * Function:	FindChunk()
 *
 * Task:	Return the memory adress of a chunk.
 *
 * in:		adress		Adress of memory piece to examine. Must be a
 *				valid (initialized) mediatool memory piece.
 *				Checks are done to ensure this.
 *		ChunkName	Chunk to search for. Must be given as a 4
 *				byte Ascii string.
 * 
 * out:		MdChunk*  	Memory adress of chunk. NULL, if chunk does
 *				not exist.
 *
 *****************************************************************************/
MdChunk* FindChunk(char *adress, char *ChunkName)
{
  /* Pointer to a chunk */
  MdChunk	*chunk;

  if (strlen(ChunkName) != 4)
    return(NULL);

  if (strncmp(adress, "MDTO", 4) !=0 )
    return(NULL);

  /* Increment adress pointer by 4. The pointer now points to the first
   * chunk.
   */
  adress +=4;

  while (1)
    {
      chunk = (MdChunk*)adress;
      /* Examine the chunk. */
        if (strncmp(chunk->Type, ChunkName, 4) == 0 )
	  return(chunk);
	else
	  {
	    /* Is this the end chunk? */
	    if (strncmp(chunk->Type, "IEND", 4) == 0 )
	      {
		/* Yes: Not found. Please note, that NULL is returned, when
		 * the end chunk is found. Nevertheless, you can search for
		 * the end chunk. It is detected beforehand with the first
		 * strncmp(), and the adress gets returned correctly.
		 */
		return(NULL);
	      }
	    else
	      {
		/* No: Point to next chunk */
		adress += chunk->DataLength + sizeof(MdChunk);
	      }
	  }
    }

}

/******************************************************************************
 *
 * Function:	WriteChunk()
 *
 * Task:	Write a chunk into a mediatool memory piece. The chunk
 *		replaces the end chunk. A new end chunk is automatically
 *		written.
 * Comments:   	Chunks may be written multiple times. It is not checked, if chunks
 *		appear mutiple times.
 *
 * in:		adress		Adress of memory piece to write to. Must be a
 *				valid (initialized) mediatool memory piece.
 *				Checks are done to ensure this.
 *		ChunkName	Chunk to write. Must be given as a 4
 *				byte Ascii string.
 *		data		Adress of data piece to write.
 *		length		Length of data piece.
 * 
 * out:		MdChunk*       	Memory adress of chunk. NULL, if chunk could
 *				not be written (No memory left for chunk).
 *
 *****************************************************************************/
MdChunk* WriteChunk (char *adress, char *ChunkName, char *data, int32 length)
{
  MdChunk	*TmpChunk;
  MdCh_IHDR	*HeaderChunk;
  char		*workptr;
  char		*StartAdress, *EndAdress;
  int32		memsize;

  if (strlen(ChunkName) != 4)
    return(NULL);

  /*
   * Remember start adress. This is used to check for staying into the bounds of
   * the mediatool memory piece.
   */
  StartAdress = adress;

  if (strncmp(adress, "MDTO" , 4) !=0 )
    /* No, this is no mediatool memory piece. Reject to write chunk. */
    return(NULL);

  if (strcmp(ChunkName, "IHDR") != 0)
    {
      /* Check for existence of header chunk only, if it is not requested to write the
       * header chunk.
       */
      TmpChunk = FindChunk( StartAdress, "IHDR" );
      if ( TmpChunk == NULL )
	{
	  /*
	   * Chunk could not be written, because header chunk could not be found (or invalid
	   * start adress is given.
	   */
	  return(NULL);
	}
      else
	{
	  /* Read the overall memory size from the header chunk */
	  workptr = (char*)TmpChunk;
	  workptr += sizeof(MdChunk);
	  HeaderChunk = (MdCh_IHDR*)workptr;
	  memsize = HeaderChunk->shm_size;

	  /* Now go to the end of the chunk list */
	  TmpChunk = FindChunk( StartAdress, "IEND" );
	  if ( TmpChunk == NULL )
	    /*
	     * Chunk could not be written, because end chunk could not be found (or invalid
	     * start adress is given).
	     */
	    return(NULL);
	}
      

    }
  else
    {
      /* Write header chunk was requested.
       * I know where to put the header chunk. It is always put right behind the signature.
       */
      TmpChunk = (MdChunk*)(StartAdress+4);
      memsize = ((MdCh_IHDR*)data)->shm_size;
    }


  /*
   * Lets predict the end adress. Then look, if it is inside the bounds of
   * the memory piece [Yes, I don't like segmentation violations ;-) ].
   */
  EndAdress = 2*sizeof(MdChunk) + length + (char*)TmpChunk;
  if ( (EndAdress-StartAdress) > memsize )
    /* BANG: Chunk won't fit into memory piece. Return with error (NULL pointer) */
    return(NULL);

  TmpChunk->DataLength = length;
  memcpy( &(TmpChunk->Type ), ChunkName, 4);
  /* Calculate the adress, whre the chunk data will be written */
  workptr = (char*)TmpChunk + sizeof(MdChunk);


  memcpy(workptr ,data, length);

  /* Append the end chunk */
  workptr += length;
  WriteEndChunkHere(workptr);

  return( (MdChunk*)StartAdress);
}


/******************************************************************************
 *
 * Function:	WriteEndChunkHere()
 *
 * Task:	Writes the end chunk at the given adress. Every memory
 *		adress is accepted (Unlike WriteChunk() no checks are
 *		performed).
 *
 * in:		adress		Adress, where to write the end chunk to.
 * 
 * out:		-
 *
 *****************************************************************************/
void WriteEndChunkHere(char *adress)
{
  MdChunk	*TmpChunk;

  TmpChunk = (MdChunk*)adress;
  TmpChunk->DataLength = 0;
  memcpy(&(TmpChunk->Type), "IEND", 4);
}


