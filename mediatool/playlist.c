#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mediatool.h"
#include "tools.h"


/* Define the "empty" item. Item, Prev and Next are NULL. */
struct MdPlayItem MdEmptyItem = {NULL,NULL,NULL};




/******************************************************************************
 *
 * Function:	PlaylistNew()
 *
 * Task:	Create a new playlist
 *
 * in:		
 * 
 * out:		Pointer to the created Playlist or NULL, if it could not be
 *		created. Please note that all playlist functions may be called
 *		with a Playlist pointing to NULL. In this case they will just
 *		do nothing. This ensures, a crash will occur not in the
 *		library, where it is difficult to debug, but in the user
 *		program.
 *
 *****************************************************************************/
MdPlaylist* PlaylistNew(void)
{
  MdPlaylist	*tmpPlaylist;
  struct MdPlayItem	*tmpItem;

  tmpPlaylist = malloc(sizeof(MdPlaylist));
  /* failure */
  if ( !tmpPlaylist ) return NULL;
  tmpItem     = malloc(sizeof(struct MdPlayItem));
  /* failure */
  if ( !tmpItem )
    {
      free(tmpPlaylist);
      return NULL;
    }
  else
    /* Copy the "empty" template to our personal empty item */
    memcpy(tmpItem, &MdEmptyItem, sizeof(struct MdPlayItem));

  /* success, set up the list */
  tmpPlaylist->count	= 0;
  tmpPlaylist->Current	= NULL;
  tmpPlaylist->First	= NULL;
  tmpPlaylist->Last	= NULL;

  return tmpPlaylist;
}



/******************************************************************************
 *
 * Function:	PlaylistAdd()
 *
 * Task:	Adds an item into a playlist
 *
 * in:		Playlist	The playlist, where to insert the item.
 *		URL		Pointer to a filename (URL style).
 *		pos		Place to insert item (0 means first, -1 means
 *				last, everything else designates a wish to
 *				insert the item behind position pos).
 *
 * out:		Pointer to the item created, or NULL if it could not be created.
 *
 *****************************************************************************/
struct MdPlayItem* PlaylistAdd( MdPlaylist* Playlist, char *fileURL, int32 pos )
{
  struct MdPlayItem	*tmpItem, *newItem;
  int32			tmp_pos;

  /* Check for existance of playlist */
  if ( !Playlist ) return NULL;

  newItem = malloc(sizeof(struct MdPlayItem));
  /* failure */
  if ( !newItem ) return NULL;


  if ( Playlist->count == 0 )
    {
      /* Special case: Empty playlist */
      newItem->Next       = NULL;
      newItem->Prev       = NULL;    
      newItem->Item       = mystrdup(fileURL);
      Playlist->First = Playlist->Last = newItem;
    }
  else
    {

      /* Calculate a pointer, where to insert the item */
      if ( pos == -1 )
	tmpItem = Playlist->Last;
      else
	{
	  tmpItem = Playlist->First;
	  tmp_pos = pos;
	  /* Scan the list, till I find the item with number "pos" or till
	   * the end of the list is reached. But scan the list only, IF there
	   * actually is a list (So I first check for an empty list)
	   */
	  while (tmp_pos && tmpItem->Next !=NULL )
	    {
	      tmpItem=tmpItem->Next;
	      tmp_pos--;
	    }
	}

      /* tmpItem now holds a pointer for inserting the item. */
      /* Fill in the entrys, and update the pointers of the neighbour items */
      newItem->Next       = tmpItem->Next;
      newItem->Prev       = tmpItem;    
      newItem->Item       = mystrdup(fileURL);
      if (tmpItem->Next)
	/* Only, if not at end of the list. */
	tmpItem->Next->Prev = newItem;
      tmpItem->Next       = newItem;
    }


  (Playlist->count)++;
  /* Update the "Last" pointer. The "while" could be replaced by a "if",
   * but I am a bit paranoid. Here I am fixing things up, if somthing
   * goes wrong. ;-)
   */
  while ( Playlist->Last->Next )
    Playlist->Last = Playlist->Last->Next;
  /* Update the "First" pointer */
  while ( Playlist->First->Prev )
    Playlist->First = Playlist->First->Prev;
  

  return newItem;
}

void PlaylistShow( MdPlaylist* Playlist )
{
  struct MdPlayItem	*tmpItem;
  int32			num;

  /* Check for existance of playlist */
  if ( !Playlist )
    {
      fprintf(stderr, "Cannot show playlist. Pointer is NULL.\n");
      return;
    }

  fprintf(stderr, "Showing playlist %p with %i items.\n", Playlist, Playlist->count);
  fprintf(stderr, "Current is %p\n", Playlist->Current);

  tmpItem = Playlist->First;
  num     = 1;
  while ( tmpItem )
    {
      fprintf(stderr, "%i: %s", num, tmpItem->Item);
      if ( tmpItem == Playlist->Current )
	fprintf(stderr," (current)");
      fprintf(stderr,"\n");
      tmpItem = tmpItem->Next;
      num++;
    }
}
