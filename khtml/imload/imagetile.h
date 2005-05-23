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

#ifndef IMAGE_TILE_H
#define IMAGE_TILE_H

#include "imageformat.h"
#include "pool.h"
#include "tile.h"
#include "tilestack.h"

#include <qimage.h>

class ImageTile: public Tile
{
private:
    QImage image;
    
    virtual void discard();
    
    ImageTile()
    {}
    
    ~ImageTile()
    {}
    
    
    bool reconstructable;
    bool dirty;
    
    friend class Pool<ImageTile>;
public:

    //Creates a new stripe with a given owner, and dimension
    static ImageTile* create(bool reconst, TileStack* owner, int width, int height, const ImageFormat& format);
    
    //Restores a stripe, from discarded info, reads it to the pool, etc.
    static ImageTile* restore(TileStack* owner, int width, int height, const ImageFormat& format);
    
    QImage& cachedImage()
    {
        return image;
    }
    
    void setDirty()
    {
        dirty = true;
    }
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
