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
