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

#ifndef ANIM_PROVIDER_H
#define ANIM_PROVIDER_H

#include "image.h"

namespace khtmlImLoad {

/**
 A base class for animation support. Decoders should inherit off this, and implement the pure virtual. The AnimTimer should be used to 
 schedule new frames.
 
 Note: it's suggested that you schedule the next frame only after 
 the previous one has been -drawn- to avoid animating things 
 that are not on the screen
*/
class AnimProvider
{
protected:
    Image* image;

    AnimProvider(Image* _image)
    {
        image = _image;
        image->installAnimProvider(this);
    }   
    
        
    /**
     Implementations of this should switch 
     the frame, prepare next one for painting
    */
    virtual void doSwitchFrame() = 0;
    
    /**
     Asks the image to render the corresponding frame 
    */
    void paintFrame(int frame, int dx, int dy, QPainter* p, int sx, int sy, int width, int height)
    {
        image->paintFrame(frame, dx, dy, p, sx, sy, width, height);
    }
public:
    virtual ~AnimProvider();

    /**
     Requests painting.
     ### timeLimit?
    */
    virtual void paint(int dx, int dy, QPainter* p, int sx, int sy, int width, int height) = 0;


    //Normally called by AnimTimer
    virtual void switchFrame()
    {
        //Prepare the implementation
        doSwitchFrame();
        
        //Schedule
        image->switchFrame();
    }

};

}

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
