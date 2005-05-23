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

#include "imagetile.h"
#include "imageloader.h"
#include "imagemanager.h"

namespace khtmlImLoad {

template<>
ImageTile* Pool<ImageTile>::poolHead = 0;

ImageTile* ImageTile::create(bool reconst, TileStack* owner, int w, int h, const ImageFormat& format)
{
    //Create and register tile
    ImageTile* tile = Pool<ImageTile>::create();
    tile->owner           = owner;
    tile->reconstructable = reconst;
    tile->dirty           = false;
    
    //Give the proper geometry, set palette if needed be
    tile->image = QImage(w, h, format.depth()*8);
    tile->image.setAlphaBuffer(format.hasAlpha());
    
    if (format.type == ImageFormat::Image_Palette_8)
    {
        tile->image.setNumColors(format.palette.size());
        for (int c = 0; c < format.palette.size(); c++)
            tile->image.setColor(c, format.palette[c]);
    }
    
    return tile;
}

ImageTile* ImageTile::restore(TileStack* owner, int width, int height, const ImageFormat& format)
{
    //Free some room
    ImageManager::imageCache()->acquireSpot();

    //Create a new tile
    ImageTile* tile = create(false, owner, width, height, format);
    
    //register it with owner, cache
    owner->image = tile;    
    ImageManager::imageCache()->addEntry(owner->image);
    
    //Copy in data.
    //### FIXME: Handle unavailability.
    owner->swappedInfo.swapIn(tile->cachedImage().bits());
    
    return tile;
}

void ImageTile::discard()
{
    if (!reconstructable) //we don't swap out things we can rebuild anyway
    {
        if (!owner->swappedInfo.exists()) 
        {
            int size = image.width() * image.height() * (image.depth() / 8);
            //### FIXME: Handle unavailability somehow
        
            owner->swappedInfo = ImageManager::vmMan()->alloc(size);
            dirty = true; //must write out the first time
        }
        
        if (dirty)
            owner->swappedInfo.swapOut(image.bits());
    }

    //Set the image to be null
    image.reset();
    
    //mark as discarded
    owner->image = 0;
    
    //if reconstructable, clear progress
    if (reconstructable)
    {
        for (unsigned char pos = 0; pos < Tile::TileSize; ++pos)
            owner->imageProgress[pos] = 0;
    }
    
    //Return to pool.
    Pool<ImageTile>::release(this);
}

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
