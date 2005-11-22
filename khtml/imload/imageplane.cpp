/*
    Large image displaying library.

    Copyright (C) 2005 Maks Orlovich (maksim@kde.org)

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
#include "imageplane.h"
#include "imagemanager.h"
#include <cstring>

#include <QPainter>

namespace khtmlImLoad {

bool ImagePlane::checkUpToDate(const unsigned char* versions, PixmapTile* tile)
{
    //If the tile is discarded, it's clearly not up-to-date ;-)
    if (tile->pixmap.isNull())
        return false;

    //Now, just check the version array. It's enough to memcmp,
    //since we init all to zero, and assume the pixmap plains don't
    //magically figure out the image before the loader.
    return std::memcmp(versions, tile->versions, Tile::TileSize) == 0;
}

void ImagePlane::setupTile(unsigned int tileX, unsigned int tileY, PixmapTile* tile)
{
    tile->pixmap = QPixmap(tileWidth (tileX), tileHeight(tileY));
    ImageManager::pixmapCache()->addEntry(tile);
}

void ImagePlane::updatePixmap(PixmapTile* tile, const QImage& image,
                   unsigned int tileX, unsigned int tileY,
                   unsigned int offX,  unsigned int offY,
                   unsigned char* versions)
{
    //Determine the range which needs pushing.
    unsigned int first = 0xFFFF, last;
    if (tile->pixmap.isNull())
    {
        //### this can be wasteful if we do conversion
        setupTile(tileX, tileY, tile);
        first = 0;
        last  = tileHeight(tileY) - 1;
    }
    else
    {
        ImageManager::pixmapCache()->touchEntry(tile);
    
        for (unsigned int line = 0; line < tileHeight(tileY); ++line)
        {
            if (versions[line] > tile->versions[line])
            {
                last = line;
                if (first == 0xFFFF)
                    first = line;
                //Will fix it.
                tile->versions[line] = versions[line];
            }
        }
    }

    //Special case, hopefully-fast path: if we just wants to push
    //the whole image to the whole tile, do convertFromImage directly.
    if (offX == 0 && offY == 0 && first == 0  && last == image.height() - 1 &&
        tile->pixmap.width()  == image.width() &&
        tile->pixmap.height() == image.height())
    {
       tile->pixmap = QPixmap::fromImage(image);
    }
    else
    {
        if (image.hasAlphaChannel())
        {
            //### When supported, use the src op.
            QRect imagePortion(offX, offY,
                               tile->pixmap.width(),
                               tile->pixmap.height());

            QImage portion = image.copy(imagePortion);
            tile->pixmap.fill(Qt::transparent);
            QPainter p(&tile->pixmap);
            p.drawImage(0, 0, portion);
        }
        else
        {
            QRect imagePortion(offX, offY + first,
                               tile->pixmap.width(), last - first + 1);

            QImage portion = image.copy(imagePortion);
            //Push the image portion update
            QPainter p(&tile->pixmap);
            p.drawImage(QPoint(0, first), portion);
        }
    }
}

ImagePlane::~ImagePlane()
{}

}
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
