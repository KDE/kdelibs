/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "collector.h"
#include "object.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

namespace KJS {

  void pruneObject(Imp *imp);
    
  class CollectorBlock {
  public:
    CollectorBlock(int s);
    ~CollectorBlock();
    int size;
    int filled;
    void** mem;
    CollectorBlock *prev, *next;
  };

}; // namespace

using namespace KJS;

CollectorBlock::CollectorBlock(int s)
  : size(s),
    filled(0),
    prev(0L),
    next(0L)
{
  mem = new void*[size];
  memset(mem, 0, size * sizeof(void*));
}

CollectorBlock::~CollectorBlock()
{
  delete [] mem;
}

Collector* Collector::curr = 0L;
int Collector::count = 0;

Collector::Collector()
  : root(0L),
    filled(0)
{
  count++;
#ifdef KJS_DEBUG_MEM
  collecting = false;
#endif
}

Collector::~Collector()
{
  privateCollect();
#ifdef KJS_DEBUG_MEM
  assert(filled == 0);
#endif
  
  delete root;

  if (curr == this)
    curr = 0L;
  count--;
}

Collector* Collector::init()
{
  return (curr = new Collector());
}

void* Collector::allocate(size_t s)
{
  if (!curr)
    init();

  if (s == 0)
    return 0L;
  
  return curr->share(malloc(s));
}

void* Collector::share(void *m)
{
  assert(m);

  if (!root) {
    root = new CollectorBlock(BlockSize);
    currentBlock = root;
  }

  CollectorBlock *block = currentBlock;

  assert(block->filled <= block->size);
  if (block->filled >= block->size) {
#ifdef KJS_DEBUG_MEM
    printf("allocating new block of size %d\n", block->size);
#endif
    CollectorBlock *tmp = new CollectorBlock(BlockSize);
    block->next = tmp;
    block = currentBlock = tmp;
  }
  void **r = block->mem + block->filled;
  *r = m;
  filled++;
  block->filled++;

  return m;
}

void Collector::collect()
{
  if (!curr)
    return;

  curr->privateCollect();
}

void Collector::privateCollect()
{
#ifdef KJS_DEBUG_MEM
  printf("collecting %d objects total\n", Imp::filled);
  collecting = true;
#endif

  CollectorBlock *block = root;
  while (block) {
#ifdef KJS_DEBUG_MEM
    printf("cleaning block filled %d out of %d\n", block->filled, block->size);
#endif
    Imp **r = (Imp**)block->mem;
    for (int i = 0; i < block->filled; i++, r++)
      if (*r) {
	assert((*r)->refcount > 0);
	if (!(*r)->deref()) {
	  // our memory managment can't deal with copying complex objects from
	  // one frame to another yet. We'll therefore better prune the object.
#ifdef KJS_DEBUG_MEM
	  printf("Collector: leaving object %p (%s) alive for now. %d other refs\n",
		 (*r), (*r)->typeInfo()->name, (*r)->refcount);
#endif	  
	  if ((Collector*)(*r)->internal == this)
	      pruneObject(*r);
	  continue;
	}
	// emulate 'operator delete()'
	(*r)->~Imp();
	free(*r);
	*r = 0L;
	filled--;
      }
    block = block->next;
  }

  // delete the emtpy containers
  block = root;
  while (block) {
    CollectorBlock *next = block->next;
    delete block;
    block = next;
  }

  root = 0L;

#ifdef KJS_DEBUG_MEM
  collecting = false;
#endif
}

void KJS::pruneObject(Imp *imp)
{
  /* TODO: remove properties */
  imp->setPrototype(KJSO());
}
