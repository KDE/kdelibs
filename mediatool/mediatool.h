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

#ifndef MEDIATOOL_H
#define MEDIATOOL_H "$Id$"

typedef signed char    int8;
typedef unsigned char  uint8;
typedef signed int     int32;
typedef unsigned int   uint32;
typedef unsigned short uint16;

#define LEN_NAME	 32
#define LEN_FNAME	256
#define MAX_PATTERNS	10


/*****************************************************************************
 * Now comes everything about the connections.
 * Creating, connecting and disconnecting.
 *****************************************************************************/
typedef struct
{
  int		ref;
  int		talkid;
  char		*shm_adr;
} MediaCon;


void MdConnect(int shm_talkid, MediaCon *mcon);
void MdConnectNew(MediaCon *mcon);
void MdDisconnect(MediaCon *mcon);
void MdConnectInit(void);


/*****************************************************************************
 * Now comes everything about the playlists.
 * Creating, adding a item and so on.
 *****************************************************************************/
typedef struct MdPlayItem
{
  char			*Item;
  struct MdPlayItem	*Prev; /* Should be of type MdPlayItem !! */
  struct MdPlayItem	*Next;
} MdPlayItem_t;

typedef struct
{
  int32				count;
  struct MdPlayItem		*Current;
  struct MdPlayItem		*First;
  struct MdPlayItem		*Last;
} MdPlaylist;

MdPlaylist* PlaylistNew(void);
struct MdPlayItem* PlaylistAdd( MdPlaylist* Playlist, const char *fileURL, int32 pos );
void PlaylistShow( MdPlaylist* Playlist );




/*****************************************************************************
 * Event counter definition:
 * A single event counter counts a single dedicated event. Events used in
 * the mediatool protocol are usually key presses. The masters increments
 * the "current" item each time an event occurs. Functions are available
 * for the slave to check how many key presses occured since his last
 * "sampling".
 *****************************************************************************/
typedef struct
{
  uint32       	current;	/* Must be unsigned (!!!) */
  uint32       	last;
} EventCounter;


void   EventCounterRaise(EventCounter *evc, uint32 count);
uint32 EventCounterRead(EventCounter *evc, uint32 max);
void   EventCounterReset(EventCounter *evc);



/***********************************************************************************
 * Now comes everything about the chunks.
 ***********************************************************************************/

/* Definition of the chunk structure */
typedef struct
{
    int32	DataLength;	/* Length of data field [maximum: (2^31)-1 ] 	*/
    char	Type[4];	/* Chunk type. Ascii readable                	*/
/*  char	Data[];   */	/* Chunk data. The contents is dependend on     */
				/* the chunk type. It is described below.	*/
} MdChunk;




/***********************************************************************************
 * Definition of the various chunks (Data portions!).
 * Only the data portion of the chunk is included in the structures.
 ***********************************************************************************/



/* CHUNK: Header */
/* The version number of the protocol is built by the two values
   revision and version: The protocol is of version revision.version.
   For example, if revision is 1 and version is 13, it is version 13
   if revision 1. Masters and slaves should use the same revision.
   The version may differ, as versions with the same revisions are
   compatible. They will only contain bug fixes or minor changes
   as new chunk types
 */
   
typedef struct
{
  int32		shm_size;	/* Size of shared memory segment.	*/
  int32		ref;		/* Reference id.			*/
  /* Connection name, e.g: "tracker" or "playmidi". Set by the master.  */
  char		name[LEN_NAME+1];
  int8		revision;	/* Protocol revision			*/
  int8		version;	/* Protocol version			*/
  char          ipcfname[40];   /* Remember IPC filename for unlink()   */
} MdCh_IHDR;



/* CHUNK: Play options */
/* !!! Not yet supported. Structure may change soon. */
typedef struct
{
  int32		repeats;	/* 0:forever, 1: 1 time, 2: 2 times, ..	*/
  int32		transpose;	/* Tanspose all notes by "tanspose" 	*
  				 * half notes up (poitive value) or	*
				 * down (negative values)		*/
  int8		NTSCtiming;	/* 0:PALtiming(50Hz), 1:NTSCtiming(60Hz)*/
  int8		samplesize;	/* 8 or 16 for today			*/
  int8		stereo;		/* 0: Mono, 100: Full stereo		*
  				 * Values in between are allowed.	*/
  int8		oversample;	/* Oversampling factor			*/
} MdCh_POPT;



/* CHUNK: Player information. */
typedef struct
{
  float		version;
  char		name[LEN_NAME+1];	/* Player name. Looks like	*/
					/* "tracker V4.32"		*/
} MdCh_PINF;



/* CHUNK: Key status ("Master status") */
typedef struct
{
  /* The now following EventCounters are higher level representations
   * of key presses. They tell, how often a key was pressed since last
   * reading the event counter.
   */
  EventCounter	forward;       	/* Event counter: Forward		*/
  EventCounter	backward;      	/* Event counter: Backward		*/
  EventCounter	prevtrack;     	/* Event counter: Previous track       	*/
  EventCounter	nexttrack;     	/* Event counter: Next Track		*/
  EventCounter	exit;		/* Event counter: Exit player  		*/
  EventCounter	eject;		/* Event counter: Eject media/playlist	*/
  EventCounter	play;		/* Event counter: Play			*/
  EventCounter	stop;		/* Event counter: Stop			*/
  /* The pause key is an add-on. It is not helpful to use an event
   * counter with the pause key. The key is simply pressed or unpressed.
   */
  int8    	pause;		/* Status: Pause key			*/
  uint8		sync_id;	/* This is for helping with KAudio->snyc() */
  EventCounter	posnew;		/* Event counter: Notification of pos_new change */
  uint32	pos_new;	/* Wished play position */
  uint32        pos_new_m;      /* Wished play position 1/1000 (milli) */
} MdCh_KEYS;



/* CHUNK: Player status ("Slave status") */
typedef struct
{
  uint32	status;		/* Status of client. (Bit array).      	*/
  int32		supp_keys;	/* Which keys are supported by the	*/
		       		/* player (Bit array).			*/
  uint32	pos_current;	/* Curr. play position (eg: event num) 	*/
  uint32	pos_max;	/* Max. play position (eg: num events)	*/
  char		songname[LEN_NAME+1];
  uint8		sync_id;	/* This is for helping with KAudio->snyc() */
  uint32        pos_current_m;  /* Curr. play position (milli)		*/
} MdCh_STAT;



/* CHUNK: Filename */
typedef struct
{
  EventCounter	count;	/* Synchronizing primitive for chunk	*/
  char		filename[LEN_FNAME+1];
} MdCh_FNAM;


/* CHUNK: Text message */
typedef struct
{
  int32		category;      	/* What category falls the text in?	*/
       				/* e.g: lyric, errormessage, other     	*/
  int32		info1;		/*					*/
  int32		info2;		/*					*/
  /* char      	text[SIZE]; */	/* Text message				*/
} MdCh_TEXT;


/*
 * The end chunk has no data portion. So there´s no point in defining an
 * corresponding (empty) END-Chunk. The only effect would be an C compiler
 * complaining about "empty" structure definitions and the like.
 *
 * typedef struct
 * {
 *
 * } MdCh_IEND;
 */


/* The chunk commands */
MdChunk* FindChunk(char *adress, char *ChunkName);
void*    FindChunkData(char *adress, char *ChunkName);
MdChunk* WriteChunk (char *adress, char *ChunkName, char *data, int32 length);


/***********************************************************************************
 * Now comes the filename handling.
 ***********************************************************************************/


void FileNameSet(MdCh_FNAM *fnChunk, char *filename);
int8 FileNameGet(MdCh_FNAM *fnChunk, char *filename);


int MediatypesInit(void);

typedef struct
{
  char *Name;
  char *Description;
  char *Patterns[MAX_PATTERNS];
  int8 num_patterns;
  int8 argc;
  char *argv[MAX_PATTERNS];
} MediaType;


#define MD_KEY_PREVTRACK	  1
#define MD_KEY_NEXTTRACK	  2
#define MD_KEY_FORWARD		  4
#define MD_KEY_BACKWARD		  8
#define MD_KEY_PAUSE		 16
#define MD_KEY_PLAY		 32
#define MD_KEY_EXIT		 64
#define MD_KEY_EJECT		128
#define MD_KEY_STOP		256

#define MD_STAT_INIT		  1
#define MD_STAT_READY		  2
#define MD_STAT_EXITED	 	  4
#define MD_STAT_BUSY		  8
#define MD_STAT_PLAYING		 16
#define MD_STAT_PAUSING		 32

/*
 * Special note about "MD_CAT_NAME". This is not to be confused with the file
 * name. Several media formats have a name stored internally. For example, a
 * soundtracker module always has an internal name. Within midi songs, some
 * people use the first "Sequence/Track" field as the song name, but this is
 * no standard (So a midi player cannot tell the "real" name of a song).
 */

#define MD_CAT_OTHER		 1 /* Any message, not fitting in any   */
				   /* other category			*/
#define MD_CAT_LYRIC		 2 /* Lyrics (Text of song)		*/
#define MD_CAT_ERROR		 3 /* Error channel. Use instead of	*/
				   /* writing to stderr.       		*/
#define MD_CAT_COPYRIGHT	 4 /* Copyright notice			*/
#define MD_CAT_NAME		 5 /* Name of media			*/
#define MD_CAT_INSTRUMENT	 6 /* Instrument name, use "info1"	*/
				   /* field for intrument number       	*/
#define MD_CAT_AUTHOR		 7 /* Name of original author		*/
			       	   /* (e.g.: Johann Sebastian Bach)	*/
#define MD_CAT_TRANSCRIBER	 8 /* Name of transciber		*/
				   /* (e.g.: Rob Hubbard)		*/
#define MD_CAT_FILETYPE		 9 /* e.g. "General Midi", "MT32" or	*/
				   /* "PSID single-file"		*/

#endif /* MEDIATOOL_H */
