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

#include <QTimer>
#include <QVector>

#include "animtimer.h"
#include "animprovider.h"

namespace khtmlImLoad {

AnimTimer::AnimTimer()
{
    lastTime = QTime::currentTime();
    

    QTimer* animTicks = new QTimer(this);
    connect(animTicks, SIGNAL(timeout()),
            this,      SLOT  (tick()));

    animTicks->start(10);
}

void AnimTimer::nextFrameIn(AnimProvider* provider, int ms)
{
    if (pending.contains(provider))
        return;
        
    pending[provider] = ms;
}

void AnimTimer::tick()
{
    QTime newTime = QTime::currentTime();
    int   change  = lastTime.msecsTo(newTime);
    lastTime      = newTime;
    if (change < 1) change = 1; //Just in case someone changes the clock or something
    
    QVector<AnimProvider*> toHandle;
    
    for (QMap<AnimProvider*, int>::iterator iter = pending.begin(); 
         iter != pending.end(); ++iter)
    {
        iter.data() -= change;
        if (iter.data() <= 0)
            toHandle.append(iter.key());
    }
        
    //Notify all images for the given slice.
    QVector<AnimProvider*>::const_iterator iter;
    for (iter  = toHandle.constBegin();
         iter != toHandle.constEnd()  ; ++iter)
    {
        pending.remove(*iter);
        (*iter)->switchFrame();
    }
}

void AnimTimer::destroyed(AnimProvider* provider)
{
    pending.remove(provider);
}

}

#include "animtimer.moc"
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
