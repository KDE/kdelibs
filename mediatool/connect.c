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
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>      /* To generate unique connection ids */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "mediatool.h"
#include "chunk.h"

#define MAX_CONN 100


/* Some utility function prototypes */
int ConnGetNewRef();
void LogError(char *message);
/*void MdConnectFindSlot(MediaCon **mcon); */
void GetShmAdrByRef(int shm_id, char **shm_adr);


/* Array, where open connections are stored by the master */
static MediaCon	*Connections [MAX_CONN];




/******************************************************************************
 *
 * Function:	MdConnectInit()
 *
 * Task:	Initialize the Mediatool library. Must be called once
 *		by the master. This function clears the "open connections"
 *		array. After that it fills in connections, which are still
 *		open.
 *		There are 3 reasons, why open connections exist:
 *		1) You started playing, and the master crashed.
 *		   The connection will simply be entered in the "open
 *		   connections" array.
 *		2) You quit the master with saying "continue playing".
 *		   This is handled as before.
 *		3) You started playing, and the slave crashed.
 *		   The connection is removed, and not entered in the "oc"
 *		   array.
 *
 *		To find out, what happened, the shm_nattch field is examined.
 *		shmctl() is called, to retrieve this information.
 *		
 *		
 *		
 *		
 * in:		-
 * 
 * out:		-
 *
 *****************************************************************************/
void MdConnectInit()
{
  int i;
  static char		LibraryInitialized=0;

  if (LibraryInitialized)
      return;

  for (i=0; i<MAX_CONN; i++)
      Connections[i] = NULL;

  LibraryInitialized=1;
}


#ifdef FALSE
/******************************************************************************
 *
 * Function:	MdConnectFindSlot()
 *
 * Task:	Find the adress of a free slot in the connections table. The
 *		adress of the slot is entered in *mcon. NULL denominates an
 *		error (No free slots left).
 *
 * inout:      	**mcon		Adress of a Pointer to a media connection
 *
 *****************************************************************************/
void MdConnectFindSlot(MediaCon **mcon)
{
  int i;

  for (i=0; i<MAX_CONN; i++)
    {
      if ( Connections[i] == NULL )
	{
	  *mcon = Connections[i];
	  return;
	}
    }
  mcon = NULL;
}
#endif

/******************************************************************************
 *
 * Function:	MdConnect()
 *
 * Task:	Connect to an existing media connection by giving the id of
 *		the connection.
 *
 * in:		shm_talkid	Id of connection. The client program can
 *				find out this name by evaluating the
 *				"-media" option (See documentation).
 * 
 * out:		mcon		Pointer to a MediaCon-structure. Is filled out
 *				here. If the shm_talkid is unknown,
 *				adress 0 is returned.
 *
 *****************************************************************************/
void MdConnect(int shm_talkid, MediaCon *mcon)
{
  char		*tmpadr;
  MdCh_IHDR	*HeadChunk;

  mcon->shm_adr	= NULL;
  mcon->ref	= 0;

  MdConnectInit();
  GetShmAdrByRef(shm_talkid, &tmpadr);

  if ( tmpadr == NULL )
    return;

  HeadChunk = (MdCh_IHDR*)FindChunkData(tmpadr, "IHDR");
  if ( HeadChunk == NULL )
    return;

  /* Copy reference id */
  mcon->ref      = HeadChunk->ref;
  mcon->shm_adr  = tmpadr;
  mcon->talkid   = shm_talkid;
}



/******************************************************************************
 *
 * Function:	MdConnectNew()
 *
 * Task:	Create a new media connection.
 *
 * in:		mcon		Pointer to a MediaCon structure
 * 
 * out:		
 *
 *****************************************************************************/
void MdConnectNew(MediaCon *mcon)
{
  char	        *tmpadr, *StartAdr;
  int		ret, ref, newRefnum;
  MdCh_IHDR	HeadChunk;
  MdCh_KEYS	KeysChunk;
  MdCh_STAT	StatChunk;
  MdCh_FNAM	FnamChunk;
  char		mckey[]="/tmp/.MediaCon";
  char		pathkey[40];
  

  int		shm_talkid;
  key_t		shm_talkkey;
  int		talksize=1024;

  mcon->ref      = 0;
  mcon->shm_adr  = NULL;
  MdConnectInit();
  newRefnum =  ConnGetNewRef();

  strcpy(pathkey,mckey);
  sprintf(pathkey+strlen(pathkey), "%i", newRefnum);

  /* Try to open temp file safely. We can't just do a
   *    if (stat() < 0)
   *            fopen(path, "w")
   * because that is still raceable by flipping symlinks.
   *
   * ftok() is a stupid kludge anyway, and is likely to clutter
   * your disk with stupid temp files. A better approach may be
   * to do a SHM_INFO (get max shmid), loop over all shmids and
   * do a SHM_STAT to see whether we can attach to that segment,
   * if yes, attach to it and check for MDTO signature and
   * pathkey in HeadChunk.ipcfname.     --okir
   */
  ret = open(pathkey, O_WRONLY|O_CREAT|O_EXCL, 0600);
  if (ret >= 0)
     close(ret);
  else if (errno != EEXIST)
  {
     LogError("Could not create a shared talk key file.");
     return;
  }

  /* Now it is guaranteed, a file exists. Get the adress. */
  /* Get the talk key for that file. */
  shm_talkkey = ftok(pathkey, 123);
  if ( shm_talkkey == -1 )
    {
      LogError("Could not get talk key.\n");
      return;
    }

  shm_talkid = shmget(shm_talkkey, talksize, IPC_CREAT | 0x180 ); /* %110000000 */
  if ( shm_talkid == -1 ) {
    LogError("Could not get shm id.\n");
    return;
  }

  GetShmAdrByRef(shm_talkid, &tmpadr);
  StartAdr = tmpadr;

  /* Prepare the memory piece */
  /* 1) Write the Mediatool signature. */
  memcpy (tmpadr, "MDTO", 4);

  /* 2a) Prepare header chunk */
  ref            = shm_talkid; /* !!! ConnGetNewRef(); */

  HeadChunk.shm_size	= talksize;
  HeadChunk.ref		= ref;
  /* !!! Connection name must be filled out somewhere */
  memset(HeadChunk.name, 0, LEN_NAME+1);
  strcpy(HeadChunk.name, "(unnamed)" );
  HeadChunk.revision	= 1;
  HeadChunk.version     = 0;
  strcpy(HeadChunk.ipcfname,pathkey);
  /* 2b) Write header chunk */
  if (! WriteChunk(tmpadr, "IHDR", (char*)(&HeadChunk), sizeof(MdCh_IHDR) ))
    /* Something has terribly gone wrong! */
    return;

  /* 3a) Prepare keys chunk */
  KeysChunk.pause	      = 0;
  EventCounterReset(&(KeysChunk.forward));
  EventCounterReset(&(KeysChunk.backward));
  EventCounterReset(&(KeysChunk.prevtrack));
  EventCounterReset(&(KeysChunk.nexttrack));
  EventCounterReset(&(KeysChunk.exit));
  EventCounterReset(&(KeysChunk.eject));
  EventCounterReset(&(KeysChunk.play));
  EventCounterReset(&(KeysChunk.posnew));
  KeysChunk.pos_new =0;

  /* 3b) Write keys chunk */
  if (! WriteChunk(tmpadr, "KEYS", (char*)(&KeysChunk), sizeof(MdCh_KEYS) ))
    /* Something has terribly gone wrong! */
    return;

  /* 4a) Prepare status chunk */
  StatChunk.status	= \
  StatChunk.supp_keys	= \
  StatChunk.pos_current	= \
  StatChunk.pos_max	= 0;
  memset(StatChunk.songname, 0, LEN_NAME+1);
  /* 4b) Write status chunk */
  if(! WriteChunk(tmpadr, "STAT", (char*)(&StatChunk), sizeof(MdCh_STAT) ))
    /* Something has terribly gone wrong! */
    return;

  /* 4a) Prepare filename chunk */
  EventCounterReset(&(FnamChunk.count));
  FnamChunk.filename[0]	= 0;
  /* 4b) Write status chunk */
  if(! WriteChunk(tmpadr, "FNAM", (char*)(&FnamChunk), sizeof(MdCh_FNAM) ))
    /* Something has terribly gone wrong! */
    return;

  /* I don't need to write the end chunk explicitly. WriteChunk() automatically
   * appends the end chunk.
   */


/*  num=MediatypesInit(); */
/*  fprintf(stderr,"There are %i known media types.\n", num); */

  /* Only if everything went well, I will fill out the MediaCon structure */
  mcon->shm_adr  = tmpadr;
  mcon->talkid	 = shm_talkid;
}



/******************************************************************************
 *
 * Function:	MdDisconnect()
 *
 * Task:	
 *
 * in:		
 * 
 * out:		
 *
 *****************************************************************************/
void MdDisconnect(MediaCon *mcon)
{
  struct shmid_ds InfoBuf;  /* shmctl() wants one */
  char	*helpptr;
  MdCh_IHDR *hdr;

  helpptr = mcon->shm_adr;
  mcon->shm_adr  = NULL;

  /* Mark SHM for automatic deletion on last detach */
  /* OK. I re-read the manpage on Linux and SunOS. It seems,
     that the SHM Segement never gets destroyed directly,
     when there is some segment attached ... fine, this is
     what I expected.
     Buttttt! There is no hint, if one can connect to the
     SHM Segment after marking it for deletion. Well, there
     is no hint, either, that you cant. So I will do the IPC_RMID
     shmctl() only on Linux, where it works by chance. */

  hdr = FindChunkData(helpptr,"IHDR");
  unlink(hdr->ipcfname);

  /* Detach when last client goes doen */
  shmctl(mcon->talkid, IPC_RMID, &InfoBuf);

  if ( helpptr != NULL)
    shmdt(helpptr);
}




/******************************************************************************
 *
 * Function:	
 *
 * Task:	
 *
 * in:		
 * 
 * out:		
 *
 *****************************************************************************/
void MdConnectDelete(int ref)
{
  int i;

  for (i=0; i<MAX_CONN; i++)
    {
      /* !!! Well, it compiles. But it does not do any useful work !!! */
      if ( Connections[i] == NULL )
	Connections[i] = NULL;
    }
}

/******************************************************************************
 *
 * Function:	ConnGetNewRef()
 *
 * Task:	Calculates a new (unused) reference number.
 *
 * in:		-
 * 
 * out:		reference number
 *
 * Notes:	The reference number is not at all checked for existence right
 *		now. As it is not used anyway by now, this should be not be a
 *		problem at the current moment.
 *
 *****************************************************************************/
int ConnGetNewRef()
{
  static int ref = 0;
  time_t myToD;
  
  myToD = time(NULL);
  /* The trick with "ref" guarantees unique ids, even when 2 connections
   * are created to the "same" time.
   * !!! Yup, this function should be secured by a semaphore. :-(
   */
  ref++;
  return(myToD + ref);
}


/******************************************************************************
 *
 * Function:	GetShmAdrByRef()
 *
 * Task:	Get the shared memory segment, that corresponds to a
 *		given connection ID.
 *
 * in:		shm_talkid	The ID, as stated above
 * 
 * out:		shm_adr		Adress of a pointer to a character array.
 *				Is filled with the corresponding memory
 *				adress. Or NULL, in case of failure.
 *
 *****************************************************************************/
void GetShmAdrByRef(int shm_talkid, char **shm_adr)
{
  *shm_adr = shmat ( shm_talkid , NULL, 0 );
  if ( (*shm_adr) == (char*)-1 ) {
    *shm_adr=NULL;
    return;
  }
}


void LogError(char *message)
{
  fprintf(stderr,message);
}

