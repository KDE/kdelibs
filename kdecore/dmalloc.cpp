#ifdef WITH_DMALLOC 

/*
 * File that facilitates C++ program debugging.
 *
 * Copyright 1999 by Gray Watson
 *
 * This file is part of the dmalloc package.
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of Gray Watson not be used in advertising
 * or publicity pertaining to distribution of the document or software
 * without specific, written prior permission.
 *
 * Gray Watson makes no representations about the suitability of the
 * software described herein for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The author may be contacted via http://www.dmalloc.com/
 *
 * $Id$
 */

/*
 * This file is used to effectively redirect new to the more familiar
 * malloc and delete to the more familiar free so they can be debugged
 * with the debug malloc library..  They also give the known error
 * behavior, too.
 *
 * Compile and link this in with the C++ program you want to debug.
 *
 * NOTE: I am not a C++ hacker so feedback in the form of other hints
 * and ideas for C++ users would be much appreciated.
 */
 
extern "C" {
#include <stdlib.h>
#include <stdio.h>

#define DMALLOC_DISABLE

#include "dmalloc.h"
#include "return.h"
}

/*
 * An overload function for the C++ new.
 */
void *
operator new(size_t size, const char *file, const int line)
{
    return _malloc_leap(file, line, size);
}

void *
operator new(size_t size)
{
    char	*file;
    GET_RET_ADDR(file);
    return _malloc_leap(file, 0, size);
}

/*
 * An overload function for the C++ new[].
 */
void *
operator new[](size_t size, const char *file, const int line)
{
    if (!size) size++;
    
    return _malloc_leap(file, line, size);
}

/*
 * An overload function for the C++ new[].
 */
void *
operator new[](size_t size)
{
    if (!size) size++;

    char	*file;
    GET_RET_ADDR(file);
    return _malloc_leap(file, 0, size);
}

/*
 * An overload function for the C++ delete.
 */
void
operator delete(void *pnt)
{
    if (!pnt)
	return;
    
    char	*file;
    GET_RET_ADDR(file);
    _free_leap(file, 0, pnt);
}

/*
 * An overload function for the C++ delete[].  Thanks to Jens Krinke
 * <j.krinke@gmx.de>
 */
void
operator delete[](void *pnt)
{
    if (!pnt)
	return;

    char	*file;
    GET_RET_ADDR(file);
    _free_leap(file, 0, pnt);
}

#endif

