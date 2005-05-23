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

#ifndef FRAME_H
#define FRAME_H

#include "imageformat.h"
#include "tilecache.h"
#include <qpixmap.h>

class QPainter;
class Image;
class Plane;
class ScaledPlane;

class Frame
{
    friend class FrameInternal;
private:
    //Interface to the image, which helps feed frames w/data.
    Image*       owner;
    unsigned int loaderScanline;
    uchar        lastVersion;

    //Constructs a pixmap for the given tile. If cacheOnly is true only cached pixmaps
    //will be returned. "expensive" returns true if any work other than looking things
    //up from cache was done
    const QPixmap& pixmap(unsigned int tileX, unsigned int tileY, bool cacheOnly,
                          bool& expensive);
    
    //Constructs an image for the given tile. After this call, the tile's .image will be set
    //if forceBasicPlane, the basic image will always be used, otherwise scaled will be preferred
    //if it exists    
    const QImage& image(bool forceBasicPlane, unsigned int tileX, unsigned int tileY,
                        bool* expensive = 0);
    
    //Incorporate the scanline into update range
    void requestUpdate(unsigned int line);
    
    //Sets the state as not having updates
    void noUpdates();
private:
    //Interface to the update coalescing handling (and related data)
    friend class Updater;
    bool         updatesPending;
    unsigned int updatesStartLine;
    unsigned int updatesEndLine;
    
    /**
     Called by the updater when the image should tell its owners about new changes
    */
    void notifyPerformUpdate();
private:
    //Original image info
    Plane      * basicPlane;
    ScaledPlane* scaledPlane;
    
    ImageFormat format;
        
    //Our double-buffer. Used by all of the library
    static QPixmap* doubleBuffer;
public:
    /**
     Returns the frame original width
    */
    int origFrameWidth() const;
    /**
     Returns the frame original height
    */
    int origFrameHeight() const;

    /**
     Return the frame scaled width
     */
    int frameWidth() const;
    
    /**
     Return the frame scaled height
     */
    int frameHeight() const;

    /**
     Tells the frame to display with the given width and height
     */
    void scale(int width, int height);
    
    /**
     Tells the frame to display with the width and height of the source
     */
    void dontScale();
    
    /**
     Paints a portion of the frame on the painter 'p' at dx and dy. 
     The source rectangle starts at sx, sy and has dimension width * height.
     The last parameter, if set, will act as a time limit. In this case, the routine 
     will interrupt after that many ms, and issue a deferred update. 
    */
    void paint(unsigned int dx, unsigned int dy, QPainter* p, unsigned int sx, unsigned int sy, 
               unsigned int width, unsigned int height, int timeLimit = 0);
    
    /**
     Called by the controlling image to provide a scanline. 
     */
    void notifyScanline(uchar version, uchar* line); 
    
    /**
     Called to say that the last image version is in fact the finest one, even if it was not marked
     with the proper version key
    */
    void notifyFinished();
    
    /** 
     Used by loaders to request a copy of the scanline thus far in the image
    */
    void requestScanline(unsigned int lineNum, uchar* lineBuf);
    
    Frame() {};
    
    /**
     Creates an empty frame, owned by an image. You must call this after ctor. This is messy, but 
     I need it to be able to use contiguous arrays of these
    */
    void init(Image* owner, int width, int height, const ImageFormat& format); 
    
    ~Frame();
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
