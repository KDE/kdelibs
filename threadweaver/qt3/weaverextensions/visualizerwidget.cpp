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

#include <qwidget.h>
#include <qlayout.h>

#include <kled.h>
#include <klocale.h>

#include <weaver.h>

#include "visualizerwidget.h"

namespace ThreadWeaver {

    VisualizerWidget::VisualizerWidget( QWidget *parent,
                                        const char *name)
	: QWidget (parent, name)
    {
        mLayout = new QGridLayout ( this, 1, 1, 2, 2);
        if (parent == 0)
        {
            resize ( minimumSizeHint() );
            setCaption (i18n("ThreadWeaver Thread Status"));
            show();
        }
    }

    VisualizerWidget::~VisualizerWidget ()
    {
    }

    void VisualizerWidget::threadCreated (ThreadWeaver::Thread *thread)
    {
        add (thread);
    }

    void VisualizerWidget::add (ThreadWeaver::Thread *thread)
    {
        const int LEDsPerRow = 8;
        int row, col;
        KLed *led = new KLed (Qt::green, KLed::Off, KLed::Flat, KLed::Rectangular,
                              this);
        led->show();
        led->setFixedSize (16, 4);
        row = (mThreads.size()-1) / LEDsPerRow;
        col = (mThreads.size()-1) % LEDsPerRow;
        mLayout->addWidget (led, row, col);
        mThreads[thread] = led;
    }

    void VisualizerWidget::threadDestroyed (ThreadWeaver::Thread *thread)
    {
        mThreads.erase (thread);
    }

    void VisualizerWidget::threadBusy (ThreadWeaver::Thread *thread)
    {
        KLed *led = mThreads[thread];
        if (!led)
        {
            add (thread);
            led = mThreads[thread];
        }
        if (led) led->setState(KLed::On);
    }

    void VisualizerWidget::threadSuspended (ThreadWeaver::Thread *thread)
    {
        KLed *led = mThreads[thread];
        if (!led)
        {
            add (thread);
            led = mThreads[thread];
        }
        if (led) led->setState (KLed::Off);
    }

}

