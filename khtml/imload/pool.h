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

#ifndef POOL_H
#define POOL_H

namespace khtmlImLoad {

template<typename T>
class Pool
{
private:
    static T* poolHead;
    
public:
    static T* create()    
    {
        if (poolHead)
        {
            T* toRet = poolHead;
            poolHead = static_cast<T*>(poolHead->cacheNext); //we abuse cacheNext fields here to safe memory
            return toRet;
        }
        else
            return new T;
    }
    
    static void release(T* entry)
    {
        //### TODO: Limit!
        entry->cacheNext = poolHead;
        poolHead        = entry;
    }
};

}

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
