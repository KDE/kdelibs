/* This file is part of the KDE libraries
    Copyright (C) 1997 Christian Esken (chris@magicon.prima.ruhr.de)

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
#include "tools.h"


/******************************************************************************
 *
 * Function:	mystrdup()
 *
 * Task:	Duplicate a string, using freshly allocated memory.
 *		See "man strdup" for more information
 *
 * in:		s	Adress of string.
 * 
 * out:		char*	Adress of new created/copied string.
 *
 * Comment:	This is a drop-in replacement function for strdup().
 *		As the mentioned function is not POSIX, this is necessary
 *		for portabilty.
 *
 *****************************************************************************/
char *mystrdup(char *s)
{
  char *tmp;

  tmp = malloc(strlen(s)+1);
  if (tmp)
    strcpy(tmp,s);
  return tmp;
}
