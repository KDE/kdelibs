/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2002 Michael Matz (matz@kde.org)
              
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

#include <qvaluelist.h>

class KZoneAllocatorPrivate;


/**
 * Memory allocator for large groups of small objects.
 * This should be used for large groups of objects that are created and
 * destroyed together. When used carefully for this purpose it is faster
 * and more memory efficient than malloc.  Additionally to a usual obstack
 * like allocator you can also free the objects individually.  Because it
 * does no compaction it still is faster then malloc()/free().  Depending
 * on the exact usage pattern that might come at the expense of some
 * memory though.
 * @author Waldo Bastian <bastian@kde.org>, Michael Matz <matz@kde.org>
 * @version $Id$
 */
class KZoneAllocator
{
public:
    /**
     * Creates a KZoneAllocator object.
     * @param _blockSize Size in bytes of the blocks requested from malloc.
     */
    KZoneAllocator(unsigned long _blockSize = 8*1024);

    /**
     * Destructs the ZoneAllocator and free all memory allocated by it.
     */
    ~KZoneAllocator();

    /**
     * Allocates a memory block.
     * @param _size Size in bytes of the memory block. Memory is aligned to
     * the size of a pointer.
     */
    void* allocate(size_t _size);

    /**
     * Gives back a block returned by @ref allocate() to the zone
     * allocator, and possibly deallocates the block holding it (when it's
     * empty).  The first @ref deallocate() after many @ref allocate() calls
     * (or the first at all) builds an internal data structure for speeding
     * up deallocation.  The consistency of that structure is maintained
     * from then on (by @ref allocate() and @ref deallocate()) unless many
     * more objects are allocated without any intervening deallocation, in
     * which case it's thrown away and rebuilt at the next @ref deallocate().
     *
     * The effect of this is, that such initial @ref deallocate() calls take
     * more time then the normal calls, and that after this list is built, i.e.
     * generally if @ref deallocate() is used at all, also allocate() is a
     * little bit slower.  This means, that if you want to squeeze out the last
     * bit performance you would want to use KZoneAllocator as an obstack, i.e.
     * just use the functions @ref allocate() and @ref free_since().  All the
     * remaining memory is returned to the system if the zone allocator
     * is destroyed.
     * @param ptr Pointer as returned by @ref allocate().
     */
    void deallocate(void *ptr);

    /**
     * Deallocate many objects at once.
     * @ref free_since() deallocates all objects allocated after @p ptr, 
     * @em including @p ptr itself.
     *
     * The intended use is something along the lines of:
     * <pre>
     * KZoneAllocator alloc(8192);
     * void *remember_me = alloc.allocate(0);
     * for (int i = 0; i < 1000; i++)
     *   do_something_with (alloc.allocate(12));
     * alloc.free_since (remember_me);
     * </pre>
     * Note, that we don't need to remember all the pointers to the 12-byte
     * objects for freeing them.  The @ref free_since() does deallocate them
     * all at once.
     * @param ptr Pointer as returned by @ref allocate().  It acts like
     * a kind of mark of a certain position in the stack of all objects,
     * off which you can throw away everything above that mark.
     */
    void free_since(void *ptr);

protected:
    class MemBlock;
    typedef QValueList<MemBlock *> MemList;
    void addBlock(MemBlock *b);
    void delBlock(MemBlock *b);
    void insertHash(MemBlock *b);
    void initHash();
    MemBlock *currentBlock;
    unsigned long blockSize;
    unsigned long blockOffset;
    unsigned int log2;
    unsigned int num_blocks;
    MemList **hashList;
    unsigned int hashSize;
    bool hashDirty;
private:
    KZoneAllocatorPrivate *d;
};

#endif
