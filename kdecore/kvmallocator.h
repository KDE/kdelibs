/*
    This file is part of the KDE libraries

    Copyright (C) 2000 Waldo Bastian (bastian@kde.org)
              
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
// KDE Virtual Memory Allocator

#ifndef KVMALLOCATOR_H
#define KVMALLOCATOR_H

#include <sys/types.h>

class KVMAllocatorPrivate;

/**
 * KVMAllocator is a virtual memory allocator.
 * Memory is allocated block-wise in a tmp file.
 * 
 * @author Waldo Bastian <bastian@kde.org>
 * @version $Id$
 */
class KVMAllocator
{
public:
    struct Block;

    /**
     * Create a KVMAllocator 
     */
    KVMAllocator();

    /**
     * Destruct the KVMAllocator and release all memory.
     */
    ~KVMAllocator();

    /**
     * Allocate a virtual memory block.
     * @param _size Size in bytes of the memory block.
     * @return the allocated memory block
     */
    Block *allocate(size_t _size);
    
    /**
     * Free a virtual memory block.
     * @param block the block to free
     */
    void free(Block *block);
    
    /**
     * Copy @p length bytes from @p _offset in the virtual memory block 
     * @p src to normal memory at address *dest.
     * @param dest the destination of the data
     * @param src the source block
     * @param _offset the offset in the source block
     * @param length the length of the data to copy
     */
    void copy(void *dest, Block *src, int _offset = 0, size_t length = 0);
     
    /**
     * Copy @p length bytes from normal memory at address @p src to 
     * @p _offset in the virtual memory block @p dest.
     * @param dest the block to copy the data to
     * @param src the source location of the data
     * @param _offset the offset in the destination block
     * @param length the length of the data to copy
     */
    void copy(Block *dest, void *src, int _offset = 0, size_t length = 0);  

    /**
     * Map a virtual memory block in memory
     * @param block the block to map
     */
    void *map(Block *block);
    
    /**
     * Unmap a virtual memory block
     * @param block the block to unmap
     */
    void unmap(Block *block);
    
private:
    KVMAllocatorPrivate *d;
};

#endif
