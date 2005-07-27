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

#include <assert.h>

#include <qtimer.h>
#include "image.h"
#include "updater.h"

namespace khtmlImLoad {

/**
 We keep 10 separate tables for each of ten 100ms portions of a second.
 Each entry is scheduled to be delivered 5 slots ahead
*/

Updater::Updater()
{
    timePortion = 0;

    QTimer* updatePusher = new QTimer(this);
    connect(updatePusher, SIGNAL(timeout()),
            this,         SLOT  (pushUpdates()));

    updatePusher->start(100);                        
}

void Updater::haveUpdates(Image* frame)
{
    assert(frame);
    int schedulePortion = (timePortion + 1) % 10;
    frames[schedulePortion].append(frame);
}

void Updater::pushUpdates()
{
    timePortion++;
    if (timePortion >= 10)
        timePortion = 0;
        
    //Notify all images for the given slice.
    QVector<Image*>::const_iterator iter;
    for (iter = frames[timePortion].constBegin();
        iter != frames[timePortion].constEnd()  ; ++iter)
    {        
        (*iter)->notifyPerformUpdate();
    }
    
    //Dump the contents of the table, everything delivered
    frames[timePortion].clear();
}

}

#include "updater.moc"
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
