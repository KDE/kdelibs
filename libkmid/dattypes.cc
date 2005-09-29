/**************************************************************************
 
    dattypes.cc  - Some always useful definitions and functions
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
#include "dattypes.h"
#include <stdio.h>

ushort readShort(FILE *fh)
{
  uchar c1;
  uchar c2;

  fread(&c1,1,1,fh);
  fread(&c2,1,1,fh);
  return (c1<<8)|c2;
}

ulong readLong(FILE *fh)
{
  uchar c1;
  uchar c2;
  uchar c3;
  uchar c4;
  ulong l;

  fread(&c1,1,1,fh);
  fread(&c2,1,1,fh);
  fread(&c3,1,1,fh);
  fread(&c4,1,1,fh);
  l=((c1<<24)|(c2<<16)|(c3<<8)|c4);
  return l;
}

#ifdef DEBUG

void printfdebug(const char *format, int a, int b, int c)
{
  char *s=(char *)format;
  int i=0;
  while (*s!=0)
  {
    if (*s=='%') i++;
    s++;
  } 
  switch (i)
  {
    case (1) : fprintf(stderr,format,a); break;
    case (2) : fprintf(stderr,format,a,b); break;
    case (3) : fprintf(stderr,format,a,b,c); break;
    default : fprintf(stderr,format); break;
  }

}

void printfdebug(const char *format, int a, long b)
{
  fprintf(stderr,format,a,b);
}

void printfdebug(const char *format, double a, double b, double c)
{
  char *s=(char *)format;
  int i=0;
  while (*s!=0)
  {
    if (*s=='%') i++;
    s++;
  } 
  switch (i)
  {
    case (1) : fprintf(stderr,format,a); break;
    case (2) : fprintf(stderr,format,a,b); break;
    case (3) : fprintf(stderr,format,a,b,c); break;
    default : fprintf(stderr,format); break;
  }

}
#else

void printfdebug(const char *, int , int , int )
{
}
void printfdebug(const char *, int , long )
{
}
void printfdebug(const char *, double , double , double )
{
}
#endif
