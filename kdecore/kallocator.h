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
// KDE Memory Allocator
// $Id$

#ifndef KALLOCATOR_H
#define KALLOCATOR_H

#include <stdio.h>
#include <qlist.h>

/**
 * Memory allocator for large groups of small objects.
 * This should be used for large groups of objects that are created and
 * destroyed together. When used carefully for this purpose it is faster
 * and more memory efficient than malloc.
 * @author Waldo Bastian <bastian@kde.org>
 * @version $Id$
 */
class KZoneAllocator
{
public:
    /**
     * Create a ZoneAllocator 
     * @param _blockSize Size in bytes of the blocks requested from malloc. 
     */
    KZoneAllocator(long _blockSize = 128*1024);

    /**
     * Destruct the ZoneAllocator and free all memory allocated by it.
     */
    ~KZoneAllocator();

    /**
     * Allocate a memory block.
     * @param _size Size in bytes of the memory block. Memory is not alligned!
     */
    void* allocate(size_t _size);
                
protected:    
    long blockSize;
    QList<char> memoryBlocks;
    char *currentBlock;
    long blockOffset;
};

#endif
