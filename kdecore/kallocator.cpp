/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Objects
// $Id$

#include "kallocator.h"

KZoneAllocator::KZoneAllocator(long _blockSize)
: blockSize(_blockSize), blockOffset(0)
{
    currentBlock = new char[_blockSize];
    memoryBlocks.append(currentBlock);
}

KZoneAllocator::~KZoneAllocator()
{
    while (!memoryBlocks.isEmpty())
    {
        char *oldBuffer = (char *) memoryBlocks.take(0);
        delete [] oldBuffer;
    }
}

void *
KZoneAllocator::allocate(size_t _size)
{
   if ((long) _size + blockOffset > blockSize)
   {
      currentBlock = new char[blockSize];
      memoryBlocks.append(currentBlock);
      blockOffset = 0;
      printf("Allocating block #%d\n", memoryBlocks.count());
   } 
   void *result = (void *)(currentBlock+blockOffset);
   blockOffset += _size;
   return result;
}

