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

#ifndef IMAGE_H
#define IMAGE_H

#include <QByteArray> 
#include <QSize>
#include <QTimer> 
#include "imageformat.h"

class QPainter;
class Image;
class ImageOwner;
class ImageLoader;

/**
 An image represents a static picture or an animation. Think of it as a replacement for QMovie that will try
 hard to be efficient
*/
class Image
{
public:
    /**
     Creates an image with a given owner; the owner will be notified about the repaint event, the image geometry,
     and so on; and also if the image can not be decoded. The image must be fed data through the processData() 
     call; and it will take care of the rest automatically.
    */
    Image(ImageOwner* owner);
    
    /**
     Provides new data for decoding. The method will return false if there is no longer any use to feeding it more data 
     (i.e. if the image is complete, or broken, etc.); however, it is safe to do so.
    */
    bool processData(uchar* data, int length);
    
    /**
     Notifies the image that the data source is exhausted, in case it cares. This should be called at the 
     end of the data stream in order for non-incremental decoders to work
    */
    void processEOF();
    
    /**
     Cleans up
    */
    ~Image();
    
    /**
     Tells the image to display with the given width and height
     */
    void scale(int width, int height);
    
    /**
     Tells the image to display with the width and height of the source
     */
    void dontScale();
    
    /**
     Paints a portion of the current frame on the painter 'p' at dx and dy. 
     The source rectangle starts at sx, sy and has dimension width * height
     
     If timeLimit is passed, the painting code will interrupt itself after that many ms, and use
     the update notification to trigget a further repaint.
    */
    void paint(int dx, int dy, QPainter* p, int sx, int sy, int width, int height, int timeLimit = 0); 
    
    /**
     Returns the original size
    */
    QSize originalSize() const;
    
    /**
     Returns the current size
    */
    QSize size() const;   
private:
    //Interface to the loader.
    friend class ImageLoader;
    
    /**
     Called from the loader to notify of new geometry, the number of frames, frame rate, etc.
     Note that frame delays < 100 ms will be rounded up to 100. The loader must also call notifyFrameInfo 
     for each frame to specify the frame size and format (since they don't have to match)
     
     ### frameDelay?
    */
    void notifyImageInfo(int width, int height, int frames); //const ImageFormat& format, int frames, int frameDelay);
    
    /**
     Called to notify of format of a frame
    */
    void notifyFrameInfo(int frame, int fwidth, int fheight, const ImageFormat& format);
    
    /**
     Called from the loader to feed a new scanline to the given frame (in consecutive order in each frame), through 
     various progressive versions
     */
    void notifyScanline(int frame, uchar version, uchar* data);
    
    /**
     Called from loader to state that the last version feed was in fact last
     */
    void notifyFinished(int frame);
    
    /**
     Called from loader to request the current contents of the line in the basic plane
     */
    void requestScanline(int frame, unsigned int lineNum, uchar* lineBuf);

private:
    //Interface to the frames
    friend class Frame;
    
    /**
     Called from the frame to tell about a changed region notification
     */
    void notifyChanged(Frame* frame, int startLine, int endLine); 
private:
    //Interface to the animation provider
    friend class AnimProvider;
    
    void installAnimProvider(AnimProvider* provider);
    
    /**
     Asks the given frame to be painted
    */
    void paintFrame(int frame, int dx, int dy, QPainter* p, int sx, int sy, int width, int height);
    
    /**
     Called to notify that a new frame is to be displayed.
    */
    void switchFrame();
    
private:
    ImageOwner * owner;
    ImageLoader* loader;
    AnimProvider* animProvider;
    
    QByteArray bufferPreDetect;

    Frame* frames;    
    int numFrames;
    bool fullyDecoded;
    bool inError;

    //Original size of the image
    int origWidth, origHeight;
    
    //Scaled size of the image, or 0 if to use original
    int curWidth, curHeight;
    
    //A little helper to set the error condition.
    void loadError();
    
    //Helper for resizing the frames, since they may not be the size of the 
    //image itself
    void scaleFrame(int frame, int width, int height);
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
