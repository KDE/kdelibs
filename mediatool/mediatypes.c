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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_FNMATCH_H
#include <fnmatch.h>
#endif
#include "mediatool.h"
#include "tools.h"

#define MAX_MTYPES   20

enum { ScanForType, ReadPattern, DelayToReadName, ReadName, ReadArgv};

int8 num_mtypes;
int8 mediatypes_init=0;

MediaType mtypes[MAX_MTYPES];


/******************************************************************************
 *
 * Function:	mediatype_clear()
 *
 * Task:	Sets the values of a MediaType struct to default
 *		values (initializes one MediaType).
 *
 * in:		mtype	Pointer to the MediaType
 * 
 * out:		-
 *
 *****************************************************************************/
void MediatypeClear(MediaType *mtype)
{
  int i;

  mtype->num_patterns = 0;
  mtype->Name         = NULL;
  mtype->Description  = NULL;
  for (i=0; i<MAX_PATTERNS; i++)
    (mtype->Patterns)[i]=NULL;
  mtype->argc = 0;
  for (i=0; i<20; i++)
    (mtype->argv)[i]=NULL;
}

void MediatypesRead(FILE *fd)
{
  char	TempString[256];
  int	count;
  int8	Status;

  /*
   * Rewinding is not necessary by now, but starting to read
   * at the beginning is what I really want.
   */
  rewind(fd);

  Status = ScanForType;
  while(1)
    {
      count=fscanf(fd,"%s", TempString);
      if(count==EOF)
	break;
      if (strlen(TempString)>255)
        {
          fprintf(stderr, "mediatypes_read(): mediatypes rc file contains LONG strings. Exiting!\n");
          exit(1);
        }

      if (strcmp(TempString,"type") == 0)
	{
	  num_mtypes++;
	  Status = DelayToReadName;
	}

      switch (Status)
	{
	case ScanForType:
	  break;

	case DelayToReadName:
	  Status = ReadName;
	  break;

	case ReadName:
	  mtypes[num_mtypes].Name = mystrdup(TempString);
	  Status = ReadPattern;
	  break;

	case ReadPattern:
	  mtypes[num_mtypes].Patterns[mtypes[num_mtypes].num_patterns] = mystrdup(TempString);
	  Status = ReadArgv;
	  mtypes[num_mtypes].num_patterns ++;
	  break;

	case ReadArgv:
          mtypes[num_mtypes].argv[mtypes[num_mtypes].argc] = mystrdup(TempString);
	  mtypes[num_mtypes].argc ++;
	  break;

	default:
          Status = ScanForType;
	  break;
	}
      
      // fprintf(stderr, "mediatypes_read(): %s\n", TempString);
    }

}

/******************************************************************************
 *
 * Function:	mediatypes_init()
 *
 * Task:	Initializes the mediatype list. The mediatype
 *		contains all informations for selecting and
 *		starting a media slave (=player).
 *
 * in:		-
 * 
 * out:		int	Number of known media types
 *
 *****************************************************************************/
int MediatypesInit(void)
{
  int	i;
  FILE	*MtypeFile;
  char	*filename;
  char	*TempPtr;

  /* Do only ONE initialization */
  if ( mediatypes_init !=0 )
    return num_mtypes;

  mediatypes_init = 1;
  MtypeFile=NULL;

  num_mtypes=0;
  for (i=0; i<MAX_PATTERNS; i++)
   MediatypeClear(&(mtypes[i]));

  TempPtr=getenv("HOME");
  if (TempPtr != NULL)
    {
      filename=malloc(strlen(TempPtr)+20);
      strcpy(filename,TempPtr);
      /* append ".mediatypes" after "/home/username" */
      strcpy(filename+strlen(filename),"/.mediatypes");
      MtypeFile = fopen(filename,"r");
      free(filename);
    }
  if (MtypeFile==NULL)
    MtypeFile = fopen("/usr/local/lib/mediatypes","r");

  if (MtypeFile==NULL)
    /* No known media types */
    return 0;


  MediatypesRead(MtypeFile);
  fclose(MtypeFile);

  return num_mtypes;
}

/*
MediaType *MediaTypeIdentify()
{

}
*/
