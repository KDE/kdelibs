/* -*- C++ -*-

   This file implements the Thread Visualizer.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id$
*/

#include <qframe.h>
#include <qlayout.h>
#include <kled.h>


#include <weaver.h>
#include "weavervisualizer.h"
#include "visualizerwidget.h"

namespace ThreadWeaver {

    WeaverThreadVisualizer::WeaverThreadVisualizer( QObject *parent,
                                                    const char *name,
                                                    QWidget *wParent)
	: WeaverExtension (parent, name),
          mVis (new VisualizerWidget (wParent))
    {
    }

    WeaverThreadVisualizer::~WeaverThreadVisualizer ()
    {
    }

    QWidget *WeaverThreadVisualizer::widget ()
    {
        return mVis;
    }

    void WeaverThreadVisualizer::threadCreated (ThreadWeaver::Thread *thread)
    {
        mVis->threadCreated (thread);
    }

    void WeaverThreadVisualizer::threadDestroyed (ThreadWeaver::Thread *thread)
    {
        mVis->threadDestroyed (thread);
    }

    void WeaverThreadVisualizer::threadBusy (ThreadWeaver::Thread *thread)
    {
         mVis->threadBusy (thread);
    }

    void WeaverThreadVisualizer::threadSuspended (ThreadWeaver::Thread *thread)
    {
        mVis->threadSuspended (thread);
    }

}

