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

#ifndef TILE_STACK_H
#define TILE_STACK_H

#include "vmmanager.h"
#include "tile.h"

namespace khtmlImLoad {

class ImageTile;
class PixmapTile;

/**
 A TileStack packs together all components related to a portion of an image --- 
 the pixmap tile, the image tile, and info on discarding
*/
struct TileStack
{
    VMManager::VMBlock swappedInfo;
    ImageTile*     image;
    PixmapTile*    pixmap;   
    
    unsigned char imageProgress [Tile::TileSize];
    unsigned char pixmapProgress[Tile::TileSize];
    
    bool isDiscarded()
    {
        return (swappedInfo.exists() && !image);
    }
    
    TileStack()
    {
        image  = 0;
        pixmap = 0;
        for (unsigned int c = 0; c < Tile::TileSize; ++c)
        {
            imageProgress [c] = 0;
            pixmapProgress[c] = 0;
        }
    }    
};

}

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
