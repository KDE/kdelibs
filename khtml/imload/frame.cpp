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

#include "frame.h"
#include "image.h"
#include "imageloader.h"
#include "imagetile.h"
#include "imagemanager.h"
#include "pixmaptile.h"
#include "tilestack.h"
#include "plane.h"
#include "scaledplane.h"

#include <limits.h>

#include <qdatetime.h>
#include <qpainter.h>

QPixmap* Frame::doubleBuffer = 0;

int Frame::origFrameWidth() const
{
    return basicPlane->width; 
}

int Frame::origFrameHeight() const 
{
    return basicPlane->height; 
}

int Frame::frameWidth() const
{
    return scaledPlane ? scaledPlane->width : basicPlane->width; 
}

int Frame::frameHeight() const
{
    return scaledPlane ? scaledPlane->height: basicPlane->height; 
}

void Frame::requestUpdate(unsigned int line)
{
    updatesStartLine = QMIN(line, updatesStartLine);
    updatesEndLine   = QMAX(line, updatesEndLine);
    if (!updatesPending)
    {
        updatesPending = true;
        ImageManager::updater()->haveUpdates(this);
    }    
}

void Frame::noUpdates()
{
    updatesPending   = false;    
    updatesStartLine = -1;
    updatesEndLine   = 0;
}

void Frame::paint(unsigned int dx, unsigned int dy, QPainter* p, unsigned int sx, unsigned int sy, 
                  int sWidth, int sHeight, int timeLimit)
{
    Plane* usePlane = scaledPlane ? scaledPlane : basicPlane;
    
    QTime paintTimer;
    
    bool interrupted = false;
    bool expensive   = false;
    unsigned int interY;

    unsigned int elapsed = 0;
    
    
    QPainter* usePainter = p;
    unsigned int useDX      = dx;
    unsigned int useDY      = dy;
    
    //Do some basic clipping, discarding invalid requests and adjusting sizes of others
    if (sy >= usePlane->height)
        return;
        
    if (sx >= usePlane->width)
        return;
        
    unsigned int ey = sy + sHeight - 1;
    if (ey > usePlane->height - 1) 
        ey = usePlane->height - 1;
        
    unsigned int ex = sx + sWidth - 1;
    if (ex > usePlane->width - 1)
        ex = usePlane->width - 1;
    
    sHeight = ey - sy + 1;     
    sWidth  = ex - sx + 1;
    
    if (sWidth > Tile::TileSize || sHeight > Tile::TileSize)
    {
        if (!doubleBuffer)
            doubleBuffer = new QPixmap(sWidth, sHeight);
        else if ((doubleBuffer->width() < sWidth) || (doubleBuffer->height() < sHeight))
            doubleBuffer->resize(QMAX(sWidth,  doubleBuffer->width()), 
                                 QMAX(sHeight, doubleBuffer->height()));
                                 
        usePainter = new QPainter(doubleBuffer);
        usePainter->fillRect(0, 0, sWidth, sHeight, Qt::black);
        useDX = 0;
        useDY = 0;
    }
    
    //Calculate the range of tiles to paint, in both directions
    unsigned int startTileY = sy / Tile::TileSize;
    unsigned int endTileY   = ey / Tile::TileSize;
    
    unsigned int startTileX = sx / Tile::TileSize;
    unsigned int endTileX   = ex / Tile::TileSize;
    
    //Walk through all the rows
    unsigned int paintY = useDY;
    for (unsigned int tileY = startTileY; tileY <= endTileY; ++tileY)
    {
        //see how much we have to paint -- end points are different
        unsigned int startY = 0;
        unsigned int endY   = Tile::TileSize - 1;
        
        if (tileY == startTileY)
            startY = sy % Tile::TileSize;
            
        if (tileY == endTileY)
            endY   = ey % Tile::TileSize;
        
        unsigned int paintHeight = endY - startY + 1;
        
        //Now through some columns
        unsigned int paintX = useDX;
        for (unsigned int tileX = startTileX; tileX <= endTileX; ++tileX)
        {
            //calculate the horizontal size. Some redundancy here, 
            //since these are the same for all rows, but I'd rather
            //avoid heap allocation or alloca..
            unsigned int startX = 0;
            unsigned int endX   = Tile::TileSize - 1;
            
            if (tileX == startTileX)
                startX = sx % Tile::TileSize;
                
            if (tileX == endTileX)
                endX   = ex % Tile::TileSize;
                
            int paintWidth = endX - startX + 1;
            
            //Draw the thing... If we've timed out, we'll only
            //draw the cached fragments
            if (timeLimit)
                paintTimer.start();

            QPixmap pmap = pixmap(tileX, tileY, interrupted, expensive);

            if (timeLimit && expensive)
                elapsed += paintTimer.elapsed();
            
            if (!pmap.isNull())
            {
                //We have to be extra careful if we have an alpha channel and we're double-buffering
                if (format.hasAlpha() && usePainter != p)
                {
                    usePainter->end();
                    copyBlt(doubleBuffer, paintX, paintY, &pmap, startX, startY, paintWidth, paintHeight);
                    usePainter->begin(doubleBuffer);
                }
                else
                    usePainter->drawPixmap(paintX, paintY, pmap, startX, startY, paintWidth, paintHeight);
            }
            else
                usePainter->fillRect(paintX, paintY, paintWidth, paintHeight, Qt::black);
                
            paintX += paintWidth;
            
            if (timeLimit && !interrupted && elapsed >= timeLimit)
            {
                interrupted = true;
                
                interY = (paintY - useDY) + sy;
                if (tileX == endTileX)
                    interY += paintHeight; //This tile row done
            }
        }
        
        paintY += paintHeight;        
    }
    
    if (usePainter != p)
    {
        usePainter->end();
        delete usePainter;
        p->drawPixmap(dx, dy, *doubleBuffer, 0, 0, sWidth, sHeight); 
    }
    
    if (interrupted)
    {
        if (!scaledPlane)
        {
            //We can just merge in the lines directly into the update region
            requestUpdate(interY);
            requestUpdate(ey);
        }
        else
        {
            //This is a bit more complicate. We need to scan the rescaled table to get back to the original lines
            //(which will then be converted back to scaled ones). We have to do it this way due to the possibility of intervening
            //Rescales
            for (unsigned int lineY = interY; lineY <= ey; ++lineY)                
                requestUpdate(scaledPlane->yScaleTable[lineY].tile * Tile::TileSize + scaledPlane->yScaleTable[lineY].offset);
        }
    }
}

void Frame::requestScanline(unsigned int lineNum, uchar* lineBuf)
{
    /**
     This may seem like a good spot to do acquire locks for the row (if the offset is 0),
     but it's not needed, as the decoder is likely going to call notifyScanline() immediately anyway, and that 
     will lock the scanline, with everything still in the image cache 
    */
    unsigned int tileY   = lineNum / Tile::TileSize;
    unsigned int tileOff = lineNum % Tile::TileSize;

    //Go through all the tiles in the line, copy out images..
    for (unsigned int tileX = 0; tileX < (basicPlane->tilesWidth - 1); ++tileX)
    {
        const QImage& im = image(true, tileX, tileY);
        memcpy(lineBuf, im.scanLine(tileOff), Tile::TileSize * format.depth());
        lineBuf += Tile::TileSize * format.depth();
    }
    
    const QImage& im = image(true, basicPlane->tilesWidth - 1, tileY);
    memcpy(lineBuf, im.scanLine(tileOff), basicPlane->tileWidth(basicPlane->tilesWidth - 1) * format.depth());
}

class FrameInternal
{
public:
    template<typename T>
    static void scaleLoop(Frame* frame, T* destStart, unsigned int dx, unsigned int tw,
                        ScaledPlane* scaledPlane, unsigned int origTileY,unsigned int sy)
    {
        unsigned int lastOrigTileX = -1;
        
        T* src = 0;
        
        for (unsigned int x = 0; x < tw; ++x)
        {
            unsigned int pixelX = dx + x;
            if (scaledPlane->xScaleTable[pixelX].tile != lastOrigTileX)
            {
                lastOrigTileX = scaledPlane->xScaleTable[pixelX].tile;
                src = (T*)frame->image(true, lastOrigTileX, origTileY).scanLine(sy);
            }
            
            *destStart = src[scaledPlane->xScaleTable[pixelX].offset];
            ++destStart;
        }
        
    }
};

QImage& Frame::image(bool forceBasicPlane, unsigned int tileX, unsigned int tileY, bool* expensive)
{
    if (expensive) *expensive = false;

    Plane* usePlane = (scaledPlane && !forceBasicPlane) ? scaledPlane : basicPlane;
    
    TileStack& ts = usePlane->tile(tileX, tileY);
    
    //Size of the current tile
    int tw = usePlane->tileWidth(tileX);
    int th = usePlane->tileHeight(tileY);

    if (!ts.image)
    {
        if (expensive) *expensive = true;
        
        //The image is missing...
        if (ts.isDiscarded())
        {
            //If it's swapped out, we can just bring it back in
            ImageTile::restore(&ts, tw, th, format);
        }
        else
        {
            /**
                Just create a new entry w/o any information.
                Either the loader or the scaling code will fill it in.
                If it's scaled, we mark it as reconstructable, so we recalculate it if need be and do not swap out
            */            
            ImageManager::imageCache()->acquireSpot();
            ts.image = ImageTile::create(usePlane == scaledPlane, &ts, tw, th, format);
            ImageManager::imageCache()->addEntry(ts.image);
        }
    }
    else
        ImageManager::imageCache()->touchEntry(ts.image);
 
    /* 
      If we're scaling, we may have to pull data in from the original stripe to perform the on-demand
      resizing/incremental scaling.        
    */
    if (usePlane == scaledPlane)
    {
        ImageManager::imageCache()->lockEntry(ts.image);
        
        //Check the versions, pull in as need be. Not at that this relies on all image tiles in the basic plane 
        //having the same version.         
        for (unsigned int y = 0; y < th; ++y)
        {
            unsigned int pixelY = y + Tile::TileSize * tileY;
            
            unsigned int origTileY   = scaledPlane->yScaleTable[pixelY].tile;
            unsigned int origOffsetY = scaledPlane->yScaleTable[pixelY].offset;
            
            //### TODO???: optimization of copying the last row if possible 
            
            TileStack& refTS = basicPlane->tile(0, origTileY);
            
            //Check the version..
            if (ts.imageProgress[y] < refTS.imageProgress[origOffsetY])
            {
                if (expensive) *expensive = true;
                
                //Need to pull stuff in..
                ts.imageProgress[y] = refTS.imageProgress[origOffsetY];
                
                switch (format.depth())
                {
                    case 4:
                        FrameInternal::scaleLoop<Q_UINT32>(this, (Q_UINT32*)ts.image->cachedImage().scanLine(y), 
                                                           Tile::TileSize * tileX, tw, 
                                                           scaledPlane, origTileY, origOffsetY);                    
                        break;                        
                    case 1:
                        FrameInternal::scaleLoop<unsigned char>(this, ts.image->cachedImage().scanLine(y), 
                                                                Tile::TileSize * tileX, tw, 
                                                                scaledPlane, origTileY, origOffsetY);
                }
            }
        }
        
        ImageManager::imageCache()->unlockEntry(ts.image);
    }
        
    return ts.image->cachedImage();
}


const QPixmap& Frame::pixmap(unsigned int tileX, unsigned int tileY, bool cacheOnly, bool& expensive)
{
    expensive = false;

    Plane* usePlane = scaledPlane ? scaledPlane : basicPlane;

    TileStack& ts = usePlane->tile(tileX, tileY);
    
    //Size of the current tile
    int tw = usePlane->tileWidth(tileX);
    int th = usePlane->tileHeight(tileY);
    
    if (!ts.pixmap)
    { //No pixmap... need to create it
        if (cacheOnly)
            return ImageManager::emptyPixmap();
    
        //Construct a pixmap from the image
        ImageManager::pixmapCache()->acquireSpot();    
        ts.pixmap = PixmapTile::create(&ts, tw, th);
        ImageManager::pixmapCache()->addEntry(ts.pixmap);
        
        //Clear the version info 
        for (unsigned int line = 0; line < Tile::TileSize; line++)
            ts.pixmapProgress[line] = 0;
    }
    else
        ImageManager::pixmapCache()->touchEntry(ts.pixmap);
                    
    /*
      If the pixmap is only partial, we may have to update
      it from the image. First, grab the latest image, though,
      so we know if to update!
    */
    QImage im = QImage();
    if (!cacheOnly)
        im = image(false, tileX, tileY, &expensive);
    
    //Scan the version list, to see whether we need to update
    bool update = false;
    for (unsigned int line = 0; line < th; line++)
    {
        if (ts.pixmapProgress[line] < ts.imageProgress[line])
        {
            update = true;
            break;
        }
    }

    //Push updates if we didn't run out of painting time,
    //which would make us prefer stale ones
    if (update && !cacheOnly)
    {
        //Scan through each line, and pull in changes if need be
        for (int line = 0; line < th; line++)
        {
            int start = line;
            while (line < th && ts.pixmapProgress[line] < ts.imageProgress[line])
                ++line;
            //Line now points one past the line we need to process
            
            line = line - 1; //Make it point to the last line we want
            if (start <= line) 
            {
                QImage  lineImage = im.copy(0, start, tw, line - start + 1);
                if (lineImage.hasAlphaBuffer()) {
                    //It must be in a 32-bit format... Scan it, to check whether
                    //Is really does have alpha.
                    bool real = false;
                    QRgb* bits = (QRgb*)lineImage.bits();
                    for (QRgb* pos = bits; pos < bits + (line - start + 1)*tw; ++pos)
                    {
                        if (qAlpha(*pos) != 255) {
                            real = true;
                            break;
                        }
                    }

                    if (!real)
                        lineImage.setAlphaBuffer(false);
                }
                QPixmap linePixmap(lineImage);
                
                copyBlt(&ts.pixmap->cachedPixmap(), 0, start, &linePixmap);

                for (int c = start; c <= line; ++c)
                    ts.pixmapProgress[c] = ts.imageProgress[c];
            }
            else line = start; //moving on.
        }
    }

    expensive = expensive | update;
    
    return ts.pixmap->cachedPixmap();
}

void Frame::dontScale()
{
    delete scaledPlane;
    scaledPlane = 0;
}

void Frame::scale(int newScaleWidth, int newScaleHeight)
{
    //The minimum is 1x1.
    if (newScaleWidth  < 1) newScaleWidth  = 1;
    if (newScaleHeight < 1) newScaleHeight = 1;
    
    //Check whether we're asked to not scale at all.
    if ((newScaleWidth == basicPlane->width) && (newScaleHeight == basicPlane->height))
    {
        dontScale();
        return;
    }

    //if no change, also return.
    //### there is some opportunity for reuse of data here,
    //if only scaled in one direction
    if (scaledPlane && scaledPlane->width == newScaleWidth && scaledPlane->height == newScaleHeight)
        return;
        
    delete scaledPlane; //get rid of the old plane..
                
    //just create a new scaled plane...
    scaledPlane = new ScaledPlane(newScaleWidth,     newScaleHeight,
                                  basicPlane->width, basicPlane->height);
}

Frame::~Frame()
{
    delete basicPlane;
    delete scaledPlane;
}

void Frame::init(Image* _owner, int width, int height, const ImageFormat& _format)
{
    owner  = _owner;
    format = _format;
    
    //Create the basic plane, no scaled plane
    basicPlane  = new Plane(width, height);
    scaledPlane = 0;

    //next input line to read
    loaderScanline = 0;    
    lastVersion    = 0;
        
    //No updates yet.
    noUpdates();
}

void Frame::notifyScanline(uchar version, uchar* line)
{
    assert(version > 0);
    
    unsigned int lastTileY = (loaderScanline - 1) / Tile::TileSize;

    if (version != lastVersion)
    {
        assert(!((lastVersion == 0) && (version != 1)));
        lastVersion    = version;
        loaderScanline = 0;
        lastTileY      = basicPlane->tilesHeight - 1;
    }
    
    
    unsigned int tileY = loaderScanline / Tile::TileSize;
    
    //Lock new row, unlock previous row if need be
    if (lastTileY != tileY)
    {
        for (unsigned int tileX = 0; tileX < basicPlane->tilesWidth; ++tileX)
        {
            //unlock previously locked stuff, except at the very first row we write, which was never locked;
            if (!(version == 1 && tileY == 0))
                ImageManager::imageCache()->unlockEntry(basicPlane->tile(tileX, lastTileY).image);
                
            image(true, tileX, tileY); //make sure the tile gets created..
            ImageManager::imageCache()->lockEntry  (basicPlane->tile(tileX, tileY).image);
        }
    }
    
    //Also lock the row for images w/just one row of tiles .
    //Those never trigger above as then lastTileY is always tileY 
    if (basicPlane->tilesHeight == 1 && loaderScanline == 0 && version == 1)
    {
        for (unsigned int tileX = 0; tileX < basicPlane->tilesWidth; ++tileX)
        {
            image(true, tileX, 0);
            ImageManager::imageCache()->lockEntry(basicPlane->tile(tileX, 0).image);
        }
    }
    
    //Load the data in..
    for (unsigned int tileX = 0; tileX < basicPlane->tilesWidth; ++tileX)
    {
        TileStack& ts = basicPlane->tile(tileX, tileY);
                
        QImage& img = image(true, tileX, tileY);
        
        ts.imageProgress[loaderScanline % Tile::TileSize] = version;
            
        //copy in data
        memcpy(img.scanLine(loaderScanline % Tile::TileSize), line + format.depth() * Tile::TileSize * tileX, 
                            format.depth() * basicPlane->tileWidth(tileX));
                            
        //Mark as dirty..
        ts.image->setDirty();
    }
    
    //calculate update region. Note that we ignore scaling when doing this, and just emit the 
    //scaled version when delivering the event. It's easier this way, and we don't have to worry
    //about what happens to updates in case of change of scaling.    
    requestUpdate(loaderScanline);
    
    loaderScanline++; 
};

void Frame::notifyFinished()
{
    //Unlock the final row 
    unsigned int finalRow = basicPlane->tilesHeight - 1;
    for (unsigned int tileX = 0; tileX < basicPlane->tilesWidth; ++tileX)
        ImageManager::imageCache()->unlockEntry(basicPlane->tile(tileX, finalRow).image);
}

void Frame::notifyPerformUpdate()
{    
    //Calculate a scaled version of update range, if needed.
    unsigned int effStart = updatesStartLine;
    unsigned int effEnd   = updatesEndLine;
    
    if (scaledPlane && scaledPlane->height != basicPlane->height) 
    //If vertically scaled (horizontal scaling doesn't matter here)
    {
        //Setup an empty region, and expand by those lines which depended
        //on the original update region
        effStart = -1;
        effEnd   = 0;
        
        for (unsigned int line = 0; line < scaledPlane->height; line++)
        {
            unsigned int origY = scaledPlane->yScaleTable[line].tile * Tile::TileSize + scaledPlane->yScaleTable[line].offset;
            if ((origY >= updatesStartLine) && (origY <= updatesEndLine))
            {
                effStart = QMIN(effStart, line);
                effEnd   = QMAX(effEnd,   line);
            }
        }
    }
    
    noUpdates();
    
    if (effEnd != -1)
        owner->notifyChanged(this, effStart, effEnd);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
