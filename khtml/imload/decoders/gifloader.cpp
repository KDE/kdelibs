/*
    Large image load library -- GIF decoder

    Copyright (C) 2004 Maksim Orlovich <maksim@kde.org>
    Based almost fully on animated gif playback code,
         (C) 2004 Daniel Duley (Mosfet) <dan.duley@verizon.net>
    
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

#include "animprovider.h"

#include "gifloader.h"
#include "imageloader.h"
#include "imagemanager.h"
#include "pixmapplane.h"

#include <QByteArray>
#include <QPainter>
#include <QVector>

#include <stdlib.h>

#include <config.h> //For endian
extern "C" {
#include <gif_lib.h>
}
namespace khtmlImLoad {

static int INTERLACED_OFFSET[] = { 0, 4, 2, 1 };
static int INTERLACED_JUMP  [] = { 8, 8, 4, 2 };


enum GIFConstants
{
    //Graphics control extension has most of animation info
    GCE_Code                = 0xF9,
    GCE_Size                = 4,
    //Fields of the above
    GCE_Flags               = 0,
    GCE_Delay               = 1,
    GCE_TransColor          = 3,
    //Contents of mask
    GCE_DisposalMask        = 0x1C,
    GCE_DisposalUnspecified = 0x00,
    GCE_DisposalLeave       = 0x04,
    GCE_DisposalBG          = 0x08,
    GCE_DisposalRestore     = 0x0C,
    GCE_TransColorMask      = 0x01
};

struct GIFFrameInfo
{
    bool         trans;
    QColor       bg;
    QRect        geom;
    unsigned int delay;
    char         mode;
    //###
};

/**
 An anim provider for the animated GIFs. We keep a backing store for
 the screen.
*/
class GIFAnimProvider: public AnimProvider
{
protected:
    QVector<GIFFrameInfo> frameInfo;
    int                   frame;
    QPixmap               backing;
    bool                  firstTime;
    QColor                bgColor;
public:
    GIFAnimProvider(PixmapPlane* plane, QVector<GIFFrameInfo> _frames, QColor bg):
        AnimProvider(plane), bgColor(bg)
    {
        frameInfo = _frames;
        frame     = 0;
        backing   = QPixmap(plane->width, plane->height);
        firstTime = true;
    }
    
    virtual void paint(int dx, int dy, QPainter* p, int sx, int sy, int width, int height)
    {
        if (!width || !height)
            return; //Nothing to draw.
            
        if (!shouldSwitchFrame)
        {
            p->drawPixmap(dx, dy, backing, sx, sy, width, height);
        }
        else
        {
            shouldSwitchFrame = false;

            //Prepare backing store.
            if (firstTime)
            {
                //First time: merely fill with background
                backing.fill(bgColor);
                firstTime = false;
            }
            else
            {
                //Perform action required by the previous frame.
                //### FIXME: test how Unspecified behaves in
                //Mozilla, IE
                if (frameInfo[frame].mode == GCE_DisposalBG)
                    backing.fill(bgColor);

                //Leave requires no work, of course, and for restore,
                //We merely do not damage the backing store.

                ++frame;
                if (frame >= frameInfo.size())
                    frame = 0;

                nextFrame();
            }


            //Paint the frame on the backing store -- unless we're
            //supposed to leave it untouched
            if (0 && frameInfo[frame].mode != GCE_DisposalRestore)
            {
            }
            else
            {
                //Special case. We draw directly to the painter.
                //Figure out how much of the frame we're supposed to paint.
                QRect portion(sx, sy, width, height);
                portion &= frameInfo[frame].geom;
                dx += portion.x() - frameInfo[frame].geom.x();
                dy += portion.y() - frameInfo[frame].geom.y();
                curFrame->paint(dx, dy, p, portion.x(), portion.y(),
                                portion.width(), portion.height());
            }

            ImageManager::animTimer()->nextFrameIn(this, frameInfo[frame].delay);
        }
    }

    virtual AnimProvider* clone(PixmapPlane*)
    {
        return 0; //### FIXME
    }
};


class GIFLoader: public ImageLoader
{
    QByteArray buffer;
    int        bufferReadPos;
public:
    GIFLoader()
    {
        bufferReadPos = 0;
    }
    
    ~GIFLoader()
    {
    }
    
    virtual int processData(uchar* data, int length)
    {
        //Collect data in the buffer
        int pos = buffer.size();
        buffer.resize(buffer.size() + length);
        memcpy(buffer.data() + pos, data, length);        
        return length;
    }
    
    static int gifReaderBridge(GifFileType* gifInfo, GifByteType* data, int limit)
    {
        GIFLoader* me = static_cast<GIFLoader*>(gifInfo->UserData);
        
        int remBytes = me->buffer.size() - me->bufferReadPos;
        int toRet    = qMin(remBytes, limit);
        
        memcpy(data, me->buffer.data() + me->bufferReadPos, toRet);
        me->bufferReadPos += toRet;
        
        return toRet;
    }
    
    
    static unsigned int decode16Bit(char* signedLoc)
    {
        unsigned char* loc = reinterpret_cast<unsigned char*>(signedLoc);
    
        //GIFs are little-endian
        return loc[0] | (((unsigned int)loc[1]) << 8);
    }
    
    static void palettedToRGB(uchar* out, uchar* in, ImageFormat& format, int w)
    {
        int outPos = 0;
        for (int x = 0; x < w; ++x)
        {
            QRgb color = format.palette[in[x]];
            
            out[outPos]   = qBlue (color);
            out[outPos+1] = qGreen(color);
            out[outPos+2] = qRed  (color);
            out[outPos+3] = qAlpha(color);

            outPos += 4;
        }
    }    
    
    virtual int processEOF()
    {
        //Feed the buffered data to libUnGif
        GifFileType* file = DGifOpen(this, gifReaderBridge);
        
        if (!file)
            return Error;
        
        if (DGifSlurp(file) == GIF_ERROR)
        {
            DGifCloseFile(file);
            return Error;
        }

        //We use canvas size only for animations
        if (file->ImageCount > 1)
            notifyImageInfo(file->SWidth, file->SHeight);
        
        QVector<GIFFrameInfo> frameProps;
        
        ColorMapObject* globalColorMap = file->Image.ColorMap;
        if (!globalColorMap)
            globalColorMap = file->SColorMap;
            
        QColor bg(globalColorMap->Colors[file->SBackGroundColor].Red,
                  globalColorMap->Colors[file->SBackGroundColor].Green,
                  globalColorMap->Colors[file->SBackGroundColor].Blue);
                  
        bool prevWasBG = false;//true;
        
        //Extract out all the frames
        for (int frame = 0; frame < file->ImageCount; ++frame)
        {
            int w = file->SavedImages[frame].ImageDesc.Width;
            int h = file->SavedImages[frame].ImageDesc.Height;

            //For non-animated images, use the frame size for dimension
            if (frame == 0 && file->ImageCount == 1)
                notifyImageInfo(w, h);
        
            //Extract colormap, geometry, so that we can create the frame        
            ColorMapObject* colorMap = file->SavedImages[frame].ImageDesc.ColorMap;
            if (!colorMap)  colorMap = file->Image.ColorMap;
            if (!colorMap)  colorMap = file->SColorMap;
            
            GIFFrameInfo frameInf;
            int          trans = -1;
            frameInf.delay = 100;
            frameInf.mode  = GCE_DisposalUnspecified;
            
            //Go through the extension blocks to see whether there is a color key            
            for (int ext = 0; ext < file->SavedImages[frame].ExtensionBlockCount; ++ext)
            {                
                qDebug("ext:%d, fun:%x", ext, file->SavedImages[frame].ExtensionBlocks[ext].Function);
                if ((file->SavedImages[frame].ExtensionBlocks[ext].Function  == GCE_Code) && 
                    (file->SavedImages[frame].ExtensionBlocks[ext].ByteCount >= GCE_Size))
                {
                    if (file->SavedImages[frame].ExtensionBlocks[ext].Bytes[GCE_Flags] & GCE_TransColorMask)
                        trans = ((unsigned char)file->SavedImages[frame].ExtensionBlocks[ext].Bytes[GCE_TransColor]);
                       
                    frameInf.mode  = file->SavedImages[frame].ExtensionBlocks[ext].Bytes[GCE_Flags] & GCE_DisposalMask;
                    frameInf.delay = decode16Bit(&file->SavedImages[frame].ExtensionBlocks[ext].Bytes[GCE_Delay]) * 10;
                    
                    qDebug("Specified delay:%dms, disposal mode:%d", frameInf.delay, frameInf.mode);
                    if (frameInf.delay < 100)
                        frameInf.delay = 100;
                }
            }
            
            //Note: trans color for the first frame uses the background color,
            //otherwise it uses proper transparency, except if the previous frame 
            //used BG fill.
            ImageFormat format;
            if (trans != -1 && !prevWasBG)
                format.type = ImageFormat::Image_RGBA_32;
            else
                format.type = ImageFormat::Image_Palette_8;
                
            //Read in colors.
            for (int c = 0; c < colorMap->ColorCount; ++c)
                format.palette.append(qRgba(colorMap->Colors[c].Red,
                                            colorMap->Colors[c].Green,
                                            colorMap->Colors[c].Blue, 255));
                                            
            //Pad with black as a precaution
            for (int c = colorMap->ColorCount; c < 256; ++c)
                format.palette.append(qRgba(0, 0, 0, 255));
                
            //Put in the colorkey color 
            if (trans != -1)
            {
                if (prevWasBG)
                {
                    format.palette[trans] = qRgba(colorMap->Colors[file->SBackGroundColor].Red,
                                                  colorMap->Colors[file->SBackGroundColor].Green,
                                                  colorMap->Colors[file->SBackGroundColor].Blue, 255);
                }
                else
                {
                    format.palette[trans] = qRgba(0, 0, 0, 0);
                }
            }
            
            
                        
            prevWasBG = (frameInf.mode == GCE_DisposalBG);
                        
            //Now we can declare frame format
            notifyAppendFrame(w, h, format);
            
            frameInf.bg   = bg;
            frameInf.geom = QRect(file->SavedImages[frame].ImageDesc.Left,
                                  file->SavedImages[frame].ImageDesc.Top, 
                                  w, h);

            frameInf.trans  = format.hasAlpha();
                                                                    
            frameProps.append(frameInf); 
            
            qDebug("frame:%d:%d,%d:%dx%d, trans:%d", frame, frameInf.geom.x(), frameInf.geom.y(), w, h, trans);
                                   
            uchar* buf;
            if (format.hasAlpha())
                buf = new uchar[w*4];
            else
                buf = new uchar[w];
                
            if (file->SavedImages[frame].ImageDesc.Interlace)
            {
                //Interlaced. Considering we don't do progressive loading of gif's, a useless annoyance...
                //Fortunately, it's just a shuffle. Unfortunately, the framework sort of sucks at those.
                int index = 0; //index of line in memory
                for (int pass = 0; pass < 4; ++pass)
                {                    
                    int nextNewLine = INTERLACED_OFFSET[pass];
                    
                    for (int line = 0; line < h; ++line)
                    {
                        if (line == nextNewLine)
                        {
                            uchar* toFeed = (uchar*) file->SavedImages[frame].RasterBits + w*index;
                            if (format.hasAlpha())
                            {
                                palettedToRGB(buf, toFeed, format, w);
                                toFeed = buf;
                            }
                            
                            notifyScanline(pass + 1, toFeed);
                            ++index;
                            nextNewLine += INTERLACED_JUMP[pass];
                        }
                        else
                        {
                            //Get scanline, feed it back in.
                            requestScanline(line, buf);
                            notifyScanline (pass + 1, buf);
                        }
                    }
                }
            }
            else
            {
                for (int line = 0; line < h; ++line)
                {
                    uchar* toFeed = (uchar*) file->SavedImages[frame].RasterBits + w*line;
                    if (format.hasAlpha())
                    {
                        palettedToRGB(buf, toFeed, format, w);
                        toFeed = buf;
                    }
                    
                    notifyScanline(1, toFeed);
                }
            }
                            
            delete[] buf;
        }
        
        if (file->ImageCount > 1)
        { //need animation provider
            PixmapPlane* frame0  = requestFrame0();
            frame0->animProvider = new GIFAnimProvider(frame0, frameProps, bg);
        }
         
        return Done;
    }
};


ImageLoaderProvider::Type GIFLoaderProvider::type()
{    
    return Efficient;
}

    
ImageLoader* GIFLoaderProvider::loaderFor(const QByteArray& prefix)
{
    uchar* data = (uchar*)prefix.data();
    if (prefix.size() < 6) return 0;

    if (data[0] == 'G'  &&
        data[1] == 'I'  &&
        data[2] == 'F'  &&
        data[3] == '8'  &&
      ((data[4] == '7') || (data[4] == '9')) && 
        data[5] == 'a')
         return new GIFLoader;

    return 0;
}

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
