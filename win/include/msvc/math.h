/*
   This file is part of the KDE libraries
   Copyright (C) 2005 Christian ehrlicher <ch.ehrlicher@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEWIN_MATH_H
#define KDEWIN_MATH_H

// some functions which aren't available with msvc
// float rintf( float x )
// double rint( double x )
// long double rintl( long double x )
// float nearbyintf(float x)
// double nearbyint(double x)
// long double nearbyintl(long double x)

// this is easy
__inline float rintf( float x )
{
    __asm {
        fld x
        frndint
    }
}

__inline double rint( double x )
{
    __asm {
        fld x
        frndint
    }
}

__inline long double rintl( long double x )
{
    __asm {
        fld x
        frndint
    }
}

// this is a little bit more complicated - don't raise an exception
// -> set fpu control word bit 5 so it won't generate one
__inline float nearbyintf(float x)
{
    unsigned int tmpMSW1;
    unsigned int tmpMSW2;
    __asm {
        // get current state
        fnstcw tmpMSW1
    }
    // set bit 5
    tmpMSW2 = tmpMSW1 | 0x00000020;
	__asm {
        // and load
        fldcw tmpMSW2
        // do the job
        fld x
        frndint
        // clear exception
        fclex
        // restore old state
        fldcw tmpMSW1
    }
}

__inline double nearbyint(double x)
{
    unsigned int tmpMSW1;
    unsigned int tmpMSW2;
    __asm {
        // get current state
        fnstcw tmpMSW1
    }
    // set bit 5
    tmpMSW2 = tmpMSW1 | 0x00000020;
	__asm {
        // and load
        fldcw tmpMSW2
        // do the job
        fld x
        frndint
        // clear exception
        fclex
        // restore old state
        fldcw tmpMSW1
    }
}

__inline long double nearbyintl(long double x)
{
    unsigned int tmpMSW1;
    unsigned int tmpMSW2;
    __asm {
        // get current state
        fnstcw tmpMSW1
    }
    // set bit 5
    tmpMSW2 = tmpMSW1 | 0x00000020;
	__asm {
        // and load
        fldcw tmpMSW2
        // do the job
        fld x
        frndint
        // clear exception
        fclex
        // restore old state
        fldcw tmpMSW1
    }
}

/* regular header from msvc includes */
#include <../include/math.h>

// convenience function to avoid useless casts from int to whatever
__inline long double log(int x)
{
	return logl((double)x);	
}

#endif /* KDEWIN_MATH_H */
