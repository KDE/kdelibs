/*
    Large image displaying library.

    Copyright (C) 2004,2005 Maks Orlovich (maksim@kde.org)

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
#include "scaledimageplane.h"
#include "imagemanager.h"

namespace khtmlImLoad {

bool ScaledImagePlane::isUpToDate(unsigned int tileX, unsigned int tileY,
                            PixmapTile* tile)
{
    if (tile->pixmap.isNull()) return false;

    for (unsigned int line = 0; line < tileHeight(tileY); ++line)
    {
        if (tile->versions[line] < parent->versions[yScaleTable[line + tileY*Tile::TileSize]])
            return false;
    }

    return true;
}

template<typename T>
static void scaleLoop(QImage* dst, unsigned int* xScaleTable,
                      int line, const QImage& src, int srcLine, int tileX, int tileY)
{
    const T* srcPix = reinterpret_cast<const T*>(src.scanLine (srcLine));
    T*       dstPix = reinterpret_cast<T*>(dst->scanLine(line));
    
    xScaleTable += tileX * Tile::TileSize;
    for (int x = 0; x < (int)dst->width(); ++x)
    {
        *dstPix = srcPix[xScaleTable[x]];
        ++dstPix;
    }
}
//### is special version for TileSize worth it?

void ScaledImagePlane::ensureUpToDate(unsigned int tileX, unsigned int tileY,
                            PixmapTile* tile)
{
    ImageTile& imageTile = tiles.at(tileX, tileY);

    //Create the image if need be.
    if (imageTile.image.isNull())
    {
        imageTile.image = parent->format.makeImage(tileWidth (tileX),
                                                   tileHeight(tileY));
        std::memset(imageTile.versions, 0, Tile::TileSize);
    }
    else ImageManager::imageCache()->touchEntry(&imageTile);

    //Pull in updates to the image.
    for (unsigned int line = 0; line < tileHeight(tileY); ++line)
    {
        int origLine = yScaleTable[line + tileY*Tile::TileSize];
        if (imageTile.versions[line] < parent->versions[origLine])
        {
            if (parent->format.depth() == 1)
                scaleLoop<quint8>(&imageTile.image, xScaleTable, line, 
                                parent->image, origLine, tileX, tileY);
            else
                scaleLoop<quint32>(&imageTile.image, xScaleTable, line,
                                parent->image, origLine, tileX, tileY);
        }
    }

    //Now, push stuff into the pixmap.
    updatePixmap(tile, imageTile.image, tileX, tileY, 0, 0, imageTile.versions);
}

ScaledImagePlane::~ScaledImagePlane()
{
    delete[] xScaleTable;
    delete[] yScaleTable;
}


}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
