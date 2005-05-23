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
#ifndef VM_MAN_H
#define VM_MAN_H

#include <kvmallocator.h>
#include <qmap.h>

class VMManager
{
public:
    struct VMBlock
    {
        KVMAllocator::Block* block;
        KVMAllocator*        allocator;
        
        VMBlock()
        {
            block     = 0;
            allocator = 0;     
            length    = 0;       
        }
        
        bool exists()
        {
            return block;
        }
        
        void swapIn(void* dst)
        {
            allocator->copyBlock(dst, block);
        }
        
        void swapOut(void* src)
        {
            allocator->copyBlock(block, src);
        }
    private:
        unsigned int length;
        friend class VMManager;    
    };

    void free(VMBlock& block);
    
    VMBlock alloc(unsigned int size);  
private:    
    VMBlock doAlloc(KVMAllocator* alloc, unsigned int size);

    // The list of allocators and the amount of data allocated in each
    QMap<KVMAllocator*, unsigned int> allocStats;
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
