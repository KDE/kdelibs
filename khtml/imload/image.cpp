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

#include <qpainter.h>
#include <limits.h>

#include "image.h"
#include "imageloader.h"
#include "imageowner.h"
#include "pixmapplane.h"
#include "rawimageplane.h"
#include "scaledimageplane.h"

namespace khtmlImLoad {

Image::Image(ImageOwner* _owner)
{
    owner       = _owner;
    loader      = 0;
    loaderPlane = 0;
    original    = 0;
    loaderScanline = 0;
    
    fullyDecoded = false;
    inError      = false;
    
    width = height = 0;

    noUpdates();
}

Image::~Image()
{
    delete   loader;
    delete   original;
    assert(scaled.isEmpty());
}

void Image::requestUpdate(int line)
{
    updatesStartLine = qMin(line, updatesStartLine);
    updatesEndLine   = qMax(line, updatesEndLine);
    if (!updatesPending)
    {
        updatesPending = true;
        ImageManager::updater()->haveUpdates(this);
    }
}

void Image::noUpdates()
{
    updatesPending   = false;
    updatesStartLine = INT_MAX;
    updatesEndLine   = 0;
}

void Image::notifyPerformUpdate()
{
    owner->imageChange(this, QRect(0, updatesStartLine,
                                   width, updatesEndLine - updatesStartLine + 1));
    noUpdates();
}

void Image::loadError()
{
    inError = true;
    owner->imageError(this);
}



bool Image::processData(uchar* data, int length)
{
    if (inError)
        return false;

    //...if we don't have a loder
    if (!loader)
    {
        if (original)
        {
            //We could have already discarded it as we're all done. remind the caller about it
            return false;
        }
        else
        {
            //need to to do auto detection... so append all the data into a buffer
            int oldSize = bufferPreDetect.size();
            bufferPreDetect.resize(oldSize + length);
            memcpy(bufferPreDetect.data(), data, length);
            
            //Attempt to create a loader
            loader = ImageManager::loaderDatabase()->loaderFor(bufferPreDetect);
            
            //if can't, return...
            if (!loader)
            {
                //if there is more than 4K of data,
                //and we see no use for it, abort.
                if (bufferPreDetect.size() > 4096)
                {
                    loadError();
                    return false;
                }
                return true;
            }

            loader->setImage(this);
            
            //All the data is now in the buffer
            length = 0;
        }
    }
    
    int pos = 0, stat = 0;
    
    //If we got this far, we have the loader. 
    //just feed it any buffered data, and the new data. 
    if (!bufferPreDetect.isEmpty())
    {
        do
        {
            stat = loader->processData(reinterpret_cast<uchar*>(bufferPreDetect.data() + pos), 
                                           bufferPreDetect.size() - pos);
            if (stat == bufferPreDetect.size() - pos)
                break;

            pos += stat;                                           
        }        
        while (stat > 0);                                               
        bufferPreDetect.resize(0);
    }
    
    if (length) //if there is something we did not feed from the buffer already..
    {
        pos = 0;
        do 
        {
            stat = loader->processData(data + pos, length - pos);
        
            if (stat == length - pos)
                break;
        
            pos  += stat;
        }
        while (stat > 0);
    }
    
    //If we just finished decoding...
    if (stat == ImageLoader::Done)
    {
        fullyDecoded = true;
        owner->imageDone(this);
    }
    
    if (stat == ImageLoader::Error)
    {
        loadError();
        return false;
    }
    
    return true; //Need more stuff
}

void Image::processEOF()
{
    if (inError) //Input error already - nothing to do
        return; 

    //If no loader detected, and we're at EOF, it's an error
    if (!loader )
    {
        loadError();
        return;
    }
    
    //Otherwise, simply tell the loader, and check whether we decoded all right
    bool decodedOK = loader->processEOF() == ImageLoader::Done;
    
    //... and get rid of it
    delete loader;
    loader = 0;
    
    if (!decodedOK)
    {
        loadError();
    }
    else
    {
        fullyDecoded = true;
        owner->imageDone(this);
    }
}


void Image::notifyImageInfo(int _width, int _height)
{
    width  = _width;
    height = _height;
    
    owner->imageHasGeometry(this, width, height);
}

void Image::notifyAppendFrame(int fwidth, int fheight, const ImageFormat& format)
{
    //Create the new frame.
    QImage image = format.makeImage (fwidth, fheight);
    //IMPORTANT: we use image.width(), etc., below for security/paranoia
    //reasons -- so we e.g. end up with a size 0 image if QImage overflow
    //checks kick in, etc.
    //### probably notifyError in that case?
    RawImagePlane* iplane = new RawImagePlane(image.width(), image.height());
    iplane->image         = image;
    iplane->format        = format;
    PixmapPlane*   plane  = new PixmapPlane  (image.width(), image.height(), iplane);


    if (loaderPlane) //Had a previous plane
    {
        loaderPlane->nextFrame = plane;
        loaderPlane            = plane;
    }
    else
    {
        //Created the first one
        loaderPlane = original = plane;
    }

    //Go through the list of scaled sizes, and build frames for that.

    loaderScanline = 0;
}

void Image::notifyScanline(uchar version, uchar* data)
{
    RawImagePlane* plane = static_cast<RawImagePlane*>(loaderPlane->parent);
    if (loaderScanline >= plane->height)
        return;

    //Load the data in..
    if (plane->format.type != ImageFormat::Image_RGBA_32)
    {
        //Can just copy
        std::memcpy(plane->image.scanLine(loaderScanline), data,
            plane->format.depth() * plane->image.width());
    }
    else
    {
        //Premultiply. Note that this is assuming that any combination
        //Will not actually look at the pixel.
        QRgb* dst = reinterpret_cast<QRgb*>(plane->image.scanLine(loaderScanline));
        QRgb* src = reinterpret_cast<QRgb*>(data);
        for (int x = 0; x < plane->image.width(); ++x)
        {
            QRgb col = src[x];
            dst[x]  = (((qRed  (col)*qAlpha(col)) & 0xFF00)<<8) |
                       ((qGreen(col)*qAlpha(col)) & 0xFF00)     |
                       (((qBlue (col)*qAlpha(col))) >> 8)       |
                       (col & 0xFF000000);
        }
    }

    //Set the version.
    plane->versions[loaderScanline] = version;

    //calculate update region. Note that we ignore scaling when doing this, and just emit the
    //scaled version when delivering the event. It's easier this way, and we don't have to worry
    //about what happens to updates in case of change of scaling.
    //We only do this for the first frame --- other stuff will only
    //be full-frame switches from the animation controller
    if  (loaderPlane == original)
        requestUpdate(loaderScanline);

    loaderScanline++;
    if (loaderScanline == plane->height) //Next pass of progressive image
        loaderScanline = 0;
}

void Image::requestScanline(unsigned int lineNum, uchar* lineBuf)
{
    RawImagePlane* plane = static_cast<RawImagePlane*>(loaderPlane->parent);
    if (lineNum >= plane->height)
        return;

    std::memcpy(lineBuf, plane->image.scanLine(lineNum),
                plane->image.width() * plane->format.depth());
}

QSize Image::size() const
{
    return QSize(width, height);
}

bool Image::complete() const
{
    //### FIXME: this isn't quite right in case of animation
    //controller -- e.g. if animation is disabled, we only
    //care about frame 1.
    return fullyDecoded;
}

static QPair<int, int> trSize(QSize size)
{
    return qMakePair(size.width(), size.height());
}

PixmapPlane* Image::getSize(QSize size)
{
    if (size == this->size())
        return original;

    return scaled[trSize(size)];
}

void Image::derefSize(QSize size)
{
    if (size == this->size()) return;

    QPair<int, int> key = trSize(size);
    PixmapPlane* plane = scaled[key];
    --plane->refCount;
    if (plane->refCount == 0)
    {
        delete plane;
        scaled.remove(key);
    }
}

void Image::refSize(QSize size)
{
    if (size == this->size()) return;

    QPair<int, int> key = trSize(size);
    PixmapPlane* plane = scaled[key];
    if (plane)
    {
        ++plane->refCount;
    }
    else
    {
#ifdef __GNUC__
#warning "Security paranoia needed here"
#endif
        //Compute scaling ratios
        double wRatio = size.width()  / double(width);
        double hRatio = size.height() / double(height);
    
        //Go through and make scaled planes for each position
        PixmapPlane* first = 0, *prev = 0;

        //### might need unification with ScaledImagePlane's size handling
        for (PixmapPlane* cur = original; cur; cur = cur->nextFrame)
        {
            int newWidth  = qRound(cur->width  * wRatio);
            int newHeight = qRound(cur->height * hRatio);
            QImage image  = cur->parent->format.makeImage(newWidth, newHeight);
            ScaledImagePlane* splane = new ScaledImagePlane(
                    image.width(), image.height(),
                    static_cast<RawImagePlane*>(cur->parent));
            PixmapPlane*      plane  = new PixmapPlane(
                    image.width(), image.height(), splane);

            if (prev)
                prev->nextFrame = plane;
            else
                first           = plane;

            prev            = plane;
        }

        first->refCount = 1;
        scaled[key]     = first;
#ifdef __GNUC__
#warning "FIXME: Clone animation controllers"
#endif
    }
}

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
