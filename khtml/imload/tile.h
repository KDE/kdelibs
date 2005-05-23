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

#ifndef TILE_H
#define TILE_H

class TileStack;

/** 
 The base class for all cacheable tiles.
 This provides the interface to the cache functions
*/
class Tile
{
public:
    enum {TileSize = 64}; 
    //Note:this can be safely reduced, but not increased --- ScaledPlane
    //relies for byte offsets in a single row in a tile fitting into bytes
protected:
    friend class TileCache;
    
    //Interface to the cache LRU chains
    Tile* cacheNext;
    Tile* cachePrev;
    
    void unlink()
    {
        cacheNext->cachePrev = cachePrev;
        cachePrev->cacheNext = cacheNext;
        cacheNext = 0;
        cachePrev = 0;
    }
    
    void linkBefore(Tile* node)
    {
        this->cacheNext = node;
        this->cachePrev = node->cachePrev;
        
        node->cachePrev = this;
        this->cachePrev->cacheNext = this;
    }
    
    /**
     Called when the stripe is dropped from the cache, to release the managed resources.
     Note that the cache never calls delete on entries, as they are supposed to recycle the nodes as needed from discard
     */
    virtual void discard() {} 
    
    //If 0, we're unlocked, otherwise locked by N callees
    int cacheLocked;    
        
protected:
    Tile(): cacheNext(0), cachePrev(0), cacheLocked(0)
    {}
    
    /** 
     We must link each tile to its owner so that it can be notified of the discards 
    */
    TileStack* owner;
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
