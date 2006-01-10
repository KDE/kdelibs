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

class QPainter;

namespace khtmlImLoad {

class PixmapPlane;

/**
 A base class for animation support. This should be implemented by decoders
 wishing to implement animation. When this is installed, paint events
 for the image are redirected here.
*/
class AnimProvider
{
protected:
    PixmapPlane* frame0;
    PixmapPlane* curFrame;
    bool         shouldSwitchFrame; //Set by AnimTimer

    void nextFrame(); //Helper that goes to next frame or wraps around
public:
    AnimProvider(PixmapPlane* plane):frame0(plane), curFrame(plane),
                                     shouldSwitchFrame(true)
    {}

    void switchFrame()
    { shouldSwitchFrame = true; }
    
    virtual ~AnimProvider() {}

    //Must be implemented to create animation provider for the given
    //plane describing the same animation
    virtual AnimProvider* clone(PixmapPlane* plane) = 0;

    //Must be implemented to paint the given region. Note that clipping to the
    //overall canvas will be performed already
    virtual void paint(int dx, int dy, QPainter* p, int sx, int sy, int width, int height) = 0;
};

}

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
