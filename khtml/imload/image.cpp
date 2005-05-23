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

#include "animprovider.h"
#include "image.h"
#include "imageloader.h"
#include "imagemanager.h"
#include "imageowner.h"
#include "frame.h"

namespace khtmlImLoad {

Image::Image(ImageOwner* _owner)
{
    owner     = _owner;
    loader    = 0;    
    frames    = 0;
    numFrames = 0;
    animProvider = 0;
    fullyDecoded = false;
    inError      = false;
    
    curWidth        = curHeight        = 0;
    origWidth       = origHeight       = 0;
}

Image::~Image()
{
    delete   animProvider;
    delete   loader;    
    delete[] frames;
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
        if (frames)
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
            
            //We don't care about the initial fed-in status.
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
        fullyDecoded = true;
    
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
    }
}

//Scale frame, taking into account that it may not be full-size
void Image::scaleFrame(int frame, int width, int height)
{
    if (frames[frame].origFrameWidth() == origWidth && frames[frame].origFrameHeight() == origHeight)
    {
        frames[frame].scale(width, height);
    }
    else
    {
        float xRatio = width/float(origWidth);
        float yRatio = height/float(origHeight);
        
        frames[frame].scale(qRound(frames[frame].origFrameWidth ()*xRatio),
                            qRound(frames[frame].origFrameHeight()*yRatio));
    }
}

void Image::scale(int width, int height)
{
    curWidth  = width;
    curHeight = height;
    
    if (!frames) // if there is nothing to scale yet, the above will get used at frame creation
        return;
    
    for (int frame = 0; frame < numFrames; frame++)
        scaleFrame(frame, width, height);
}

void Image::dontScale()
{
    if (!frames)
    {
        curWidth = curHeight = 0;
        return;
    }
    
    for (int frame = 0; frame < numFrames; frame++)
        frames[frame].dontScale();
}

void Image::paint(int dx, int dy, QPainter* p, int sx, int sy, int width, int height, int timeLimit)
{
    if (!frames)
    {
        //### CHECKME -- it this a sensible behavior for a fall back, when there is no image.
        p->fillRect(dx, dy, width, height, Qt::black);
        return;
    }
    

    if (animProvider)
        animProvider->paint(dx, dy, p, sx, sy, width, height);
    else
        //Without an nimation controller, we always paint frame 0
        frames[0].paint(dx, dy, p, sx, sy, width, height, timeLimit);
}

void Image::paintFrame(int frame, int dx, int dy, QPainter* p, int sx, int sy, int width, int height)
{
    frames[frame].paint(dx, dy, p, sx, sy, width, height);
}

void Image::installAnimProvider(AnimProvider* provider)
{
    animProvider = provider;
}

void Image::notifyImageInfo(int width, int height, int _numFrames)
{
    origWidth  = width;
    origHeight = height;
    
    numFrames  = _numFrames;
    //Do not create the frames array yet, since there isn't
    //a frame yet!
    
    owner->imageHasGeometry(this, width, height);
}

void Image::notifyFrameInfo(int frame, int fwidth, int fheight, const ImageFormat& format)
{
    if (!frames)
        frames = new Frame[numFrames];

    frames[frame].init(this, fwidth, fheight, format);
    if (curWidth)
        scaleFrame(frame, curWidth, curHeight);
}

void Image::notifyScanline(int frame, uchar version, uchar* data)
{
    frames[frame].notifyScanline(version, data);
}

void Image::notifyFinished(int frame)
{
    frames[frame].notifyFinished();
}

void Image::requestScanline(int frame, unsigned int lineNum, uchar* lineBuf)
{
    frames[frame].requestScanline(lineNum, lineBuf);
}

void Image::notifyChanged(Frame* frame, int startLine, int endLine)
{
    int frameID = frame - frames;

    //We permit progressive decoding of frame 0.
    //we pressume that if the format can do this, it will hold on this frame 
    //while things are still loading, hence we pass it on.    
    if (frameID == 0)
        owner->imageChange(this, QRect(0, startLine, frame->frameWidth(), endLine - startLine + 1));
}

void Image::switchFrame()
{
    //We're called by the animation controller to say 
    //thata new frame may have to be painted
    owner->imageChange(this, QRect(0, 0,
                                   size().width(), 
                                   size().height()));
}

QSize Image::originalSize() const
{
    return QSize(origWidth, origHeight);
}

QSize Image::size() const
{
    if (curWidth)
        return QSize(curWidth, curHeight);
    else
        return originalSize();
}

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
