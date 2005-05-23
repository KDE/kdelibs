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

#ifndef TILE_CACHE_H
#define TILE_CACHE_H

#include <assert.h>

#include "tile.h"

/** 
 An LRU-replacement cache for tiles.
 ### TODO: Consider using a size policy that favors smaller images
*/
class TileCache
{
private:
    int sizeLimit;
    int size;
    
    /**
     We keep tiles in a double-linked list, with the most recent one being at the rear
     */
    Tile* front;
    Tile* rear;
    
    /** 
     There is also a second list, of "locked" entries, which are not manually discarded
     */
     Tile* lockedFront;
     Tile* lockedRear;
public:

    TileCache(int _sizeLimit):sizeLimit(_sizeLimit), size(0)
    {
        front = new Tile;
        rear  = new Tile;
        
        front->cacheNext = rear;
        rear ->cachePrev = front;
        
        lockedFront = new Tile;
        lockedRear  = new Tile;
        
        lockedFront->cacheNext = lockedRear;
        lockedRear ->cachePrev = lockedFront;
    }
    
    /**
     Prepares space for an entry to be inserted, discarding the last one. This does not attempt to pool 
     directly, as there may be multiple item types in the cache.
     
     We split this from addEntry for MM purposes, since the tile class will likely recycle the node.
     Thus, the proper use is:
     
     acquireSpot()
     allocate a new node
     addEntry()
     */
    void acquireSpot()
    {
        //If not full yet, nothing to do.
        if (size < sizeLimit)
            return;
            
        //Discard the front entry.
        Tile* frontOne = front->cacheNext;
        removeEntry(frontOne);
    }
    
    /**
     Add an entry to the cache. There must be a spot reserved with acquireSpot
     */
    void addEntry(Tile* tile)
    {
        //Must have room
        assert (size < sizeLimit);
        
        //Link in before the end sentinel, i.e. at the very last spot, increment usage count
        tile->cacheLocked = 0;
        tile->linkBefore(rear);
        size++;
    }
    
    
    /** 
     Add a "locked" entry to the cache. It will be owned by the cache, but not 
     discarded, nor would it count as resource usage. touchEntry, removeEntry, etc., will 
     work fine on it; it can be converted to a regular allocation using unlockEntry
    */
    void addLockedEntry(Tile* tile)
    {
        tile->cacheLocked = 1;
        tile->linkBefore(lockedRear);
    }
    
    /**
     Unlock an entry, making it discardable. (unless someone else locked it, too)
    */
    void unlockEntry(Tile* tile)
    {
        assert(tile->cacheLocked);
        
        --tile->cacheLocked;
        if (tile->cacheLocked == 0)
        {        
            tile->unlink();        
            acquireSpot();
            addEntry(tile);
        }
    }
    
    /**
     Locks an entry, making it impossible to discard
     */
    void lockEntry(Tile* tile)
    {
        ++tile->cacheLocked;
        if (tile->cacheLocked == 1) //first lock 
        {            
            size--;
            tile->unlink();        
            tile->linkBefore(lockedRear);
        }
    }
    
    /**
     "Touches" the entry, making it the most recent 
     (i.e. moves the entry to the rear of the chain)
     */
    void touchEntry(Tile* tile)
    {
        if (tile->cacheLocked)
            return;
            
        tile->unlink();
        tile->linkBefore(rear);
    }
    
    /**
     Removes the entry from the cache, discards it.
     */
    void removeEntry(Tile* tile)
    {
        tile->unlink();
        tile->discard();
        
        if (!tile->cacheLocked)
            size--;
    }
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
