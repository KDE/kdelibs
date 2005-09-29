/*  dattypes.h  - Some useful definitions and functions 
    This file is part of LibKMid 0.9.5
    Copyright (C) 1997,98,99,2000  Antonio Larrosa Jimenez
    LibKMid's homepage : http://www.arrakis.es/~rlarrosa/libkmid.html                                         
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/ 
#ifndef _DATTYPES_H
#define _DATTYPES_H

#include <stdio.h>
#include <sys/types.h>

#undef uchar
#undef ushort
#undef ulong

/**
 * Unsigned char
 */
typedef unsigned char uchar;

/**
 * Unsigned short
 */
typedef unsigned short ushort;

/**
 * Unsigned long
 */
typedef unsigned long ulong;

ushort readShort(FILE *fh);
ulong  readLong (FILE *fh);

void printfdebug(const char *s,int a=0,int b=0, int c=0);
void printfdebug(const char *s,int a,long b);
void printfdebug(const char *s,double a,double b=0, double c=0);

#endif
