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

/* Fast zone memory allocator with deallocation support, for use as obstack
   or as general purpose allocator.  It does no compaction.  If the usage
   pattern is non-optimal it might waste some memory while running.  E.g.
   allocating many small things at once, and then deallocating only every
   second one, there is a high chance, that actually no memory is freed.  */
// $Id$

#include "kallocator.h"
#include <kdebug.h>

class KZoneAllocator::MemBlock
{
  public:
    MemBlock(size_t s) : size(s), ref(0), older(0), newer(0)
      { begin = new char[s]; }
    ~MemBlock() { delete [] begin; }
    bool is_in(void *ptr) const {return !(begin > (char *)ptr
    				          || (begin + size) <= (char *)ptr); }
    size_t size;
    unsigned int ref;
    char *begin;
    MemBlock *older;
    MemBlock *newer;
};

KZoneAllocator::KZoneAllocator(unsigned long _blockSize)
: currentBlock(0), blockSize(1), blockOffset(0), log2(0), num_blocks(0), 
  hashList(0), hashSize(0), hashDirty(true)
{
  while (blockSize < _blockSize)
    blockSize <<= 1, log2++;
  /* Make sure, that a block is allocated at the first time allocate()
     is called (even with a 0 size).  */
  blockOffset = blockSize + 1;
}

KZoneAllocator::~KZoneAllocator()
{
  unsigned int count = 0;
  if (hashList) {
    /* No need to maintain the different lists in hashList[] anymore.
       I.e. no need to use delBlock().  */
    for (unsigned int i = 0; i < hashSize; i++)
      delete hashList[i];
    delete [] hashList;
    hashList = 0;
  }
  MemBlock *next;
  for (; currentBlock; currentBlock = next) {
    next = currentBlock->older;
    delete currentBlock;
    count++;
  }
  if (count > 1)
    qDebug("zone still contained %d blocks", count);
}

void KZoneAllocator::insertHash(MemBlock *b)
{
  unsigned int adr = ((unsigned int)b->begin) & (~(blockSize - 1));
  unsigned int end = ((unsigned int)b->begin) + blockSize;
  while (adr < end) {
    unsigned int key = adr >> log2;
    key = key & (hashSize - 1);
    if (!hashList[key])
      hashList[key] = new QValueList<MemBlock *>;
    hashList[key]->append(b);
    adr += blockSize;
  }
}

void KZoneAllocator::addBlock(MemBlock *b)
{
  b->newer = 0;
  b->older = currentBlock;
  if (currentBlock)
    b->older->newer = b;
  currentBlock = b;
  num_blocks++;
  /* If we either have no hashList at all, or since it's last construction
     there are now many more blocks we reconstruct the list.  But don't
     make it larger than a certain maximum.  */
  if (hashList && ((num_blocks / 4) > hashSize && hashSize < 64*1024))
    hashDirty = true;
  /* Only insert this block into the hashlists, if we aren't going to
     reconstruct them anyway.  */
  if (hashList && !hashDirty)
    insertHash (b);
}

void KZoneAllocator::initHash()
{
  if (hashList) {
    for (unsigned int i = 0; i < hashSize; i++)
      delete hashList[i];
    delete [] hashList;
    hashList = 0;
  }
  hashSize = 1;
  while (hashSize < num_blocks)
    hashSize <<= 1;
  if (hashSize < 1024)
    hashSize = 1024;
  if (hashSize > 64*1024)
    hashSize = 64*1024;
  hashList = new QValueList<MemBlock *> *[hashSize];
  memset (hashList, 0, sizeof(QValueList<MemBlock*> *) * hashSize);
  hashDirty = false;
  for (MemBlock *b = currentBlock; b; b = b->older)
    insertHash(b);
}

void KZoneAllocator::delBlock(MemBlock *b)
{
  /* Update also the hashlists if we aren't going to reconstruct them
     soon.  */
  if (hashList && !hashDirty) {
    unsigned int adr = ((unsigned int)b->begin) & (~(blockSize - 1));
    unsigned int end = ((unsigned int)b->begin) + blockSize;
    while (adr < end) {
      unsigned int key = adr >> log2;
      key = key & (hashSize - 1);
      if (hashList[key]) {
	QValueList<MemBlock *> *list = hashList[key];
	QValueList<MemBlock *>::Iterator it = list->begin();
	QValueList<MemBlock *>::Iterator endit = list->end();
	for (; it != endit; ++it)
	  if (*it == b) {
	    list->remove(it);
	    break;
	  }
      }
      adr += blockSize;
    }
  }
  if (b->older)
    b->older->newer = b->newer;
  if (b->newer)
    b->newer->older = b->older;
  if (b == currentBlock) {
    currentBlock = 0;
    blockOffset = blockSize;
  }
  delete b;
  num_blocks--;
}

void *
KZoneAllocator::allocate(size_t _size)
{
   // Use the size of (void *) as alignment
   const size_t alignment = sizeof(void *) - 1;
   _size = (_size + alignment) & ~alignment;   

   if ((unsigned long) _size + blockOffset > blockSize)
   {
      if (_size > blockSize) {
	qDebug("KZoneAllocator: allocating more than %lu bytes", blockSize);
	return 0;
      }
      addBlock(new MemBlock(blockSize));
      blockOffset = 0;
      //qDebug ("Allocating block #%d (%x)\n", num_blocks, currentBlock->begin);
   }
   void *result = (void *)(currentBlock->begin+blockOffset);
   currentBlock->ref++;
   blockOffset += _size;
   return result;
}

void
KZoneAllocator::deallocate(void *ptr)
{
  if (hashDirty)
    initHash();

  unsigned int key = (((unsigned int)ptr) >> log2) & (hashSize - 1);
  QValueList<MemBlock *> *list = hashList[key];
  if (!list) {
    /* Can happen with certain usage pattern of intermixed free_since()
       and deallocate().  */
    //qDebug("Uhoh");
    return;
  }
  QValueList<MemBlock*>::ConstIterator it = list->begin();
  QValueList<MemBlock*>::ConstIterator endit = list->end();
  for (; it != endit; ++it) {
    MemBlock *cur = *it;
    if (cur->is_in(ptr)) {
      if (!--cur->ref) {
	if (cur != currentBlock)
	  delBlock (cur);
	else
	  blockOffset = 0;
      }
      return;
    }
  }
  /* Can happen with certain usage pattern of intermixed free_since()
     and deallocate().  */
  //qDebug("Uhoh2");
}

void
KZoneAllocator::free_since(void *ptr)
{
  /* If we have a hashList and it's not yet dirty, see, if we will dirty
     it by removing too many blocks.  This will make the below delBlock()s
     faster.  */
  if (hashList && !hashDirty)
    {
      const MemBlock *b;
      unsigned int removed = 0;
      for (b = currentBlock; b; b = b->older, removed++)
	if (b->is_in (ptr))
	  break;
      if (hashSize >= 4 * (num_blocks - removed))
        hashDirty = true;
    }
  while (currentBlock && !currentBlock->is_in(ptr)) {
    currentBlock = currentBlock->older;
    delBlock (currentBlock->newer);
  }
  blockOffset = ((char*)ptr) - currentBlock->begin;
}
