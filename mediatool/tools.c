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
