/*
    Large image displaying library.

    Copyright (C) 2004 Maks Orlovich (maksim@kde.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "vmmanager.h"

namespace khtmlImLoad {

const int AllocatorLimit = 0x3FFFFFFF; /* limit files to about a gig */

VMManager::VMBlock VMManager::doAlloc(KVMAllocator* alloc, unsigned int size)
{
    VMBlock toRet;
    
    allocStats[alloc] += size;
    toRet.allocator = alloc;
    toRet.length    = size;
    toRet.block     = alloc->allocate(size);
    return toRet;  
}

VMManager::VMBlock VMManager::alloc(unsigned int size)
{
    //Scan through the list of allocators, and see which one can fit this w/o going over limit
    for (QMap<KVMAllocator*, unsigned int>::iterator i =  allocStats.begin();
                                                     i != allocStats.end()  ; ++i)
        if (i.data() + size < AllocatorLimit)
            return doAlloc(i.key(), size);
    
    //Need a new one.
    return doAlloc(new KVMAllocator(), size);
}

void VMManager::free(VMBlock& block)
{
    allocStats[block.allocator] -= block.length;
    block.allocator->free(block.block);
    block.block     = 0;
    block.allocator = 0;
    block.length    = 0;
}

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
