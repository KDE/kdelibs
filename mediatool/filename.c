#include <string.h>
#include "mediatool.h"

void FileNameSet(MdCh_FNAM *fnChunk, char *filename)
{
  int	len;

  len=strlen(filename);
  if(len>LEN_FNAME)
    len=LEN_FNAME;
  memcpy(fnChunk->filename,filename,len);
  /* terminate string with 0 */
  (fnChunk->filename)[len]=0;

  EventCounterRaise(&(fnChunk->count),1);
}

int8 FileNameGet(MdCh_FNAM *fnChunk, char *filename)
{
  /* Repeat reading, until the event counter has settled
   * This is not a perfect solution, but it works. Theoretically this
   * function can never terminate, practically, it will do only one or
   * two cycles.
   */
  if (EventCounterRead(&(fnChunk->count) , 0 ) == 0)
    return 0;

  do    
    {
      strcpy(filename,fnChunk->filename);
    }
  while ( EventCounterRead(&(fnChunk->count) , 0) );

  return 1;
}
