/* -*- C++ -*-

   This file declares the Thread Visualizer.

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

#ifndef VISUALIZERWIDGET_H
#define VISUALIZERWIDGET_H

#include <qwidget.h>
#include <qmap.h>

class KLed;
class QGridLayout;

namespace ThreadWeaver {

    class Thread;

    class VisualizerWidget : public QWidget
    {
        Q_OBJECT
    public:
        VisualizerWidget (QWidget *parent = 0, const char* name = 0);
        ~VisualizerWidget();
    public:
        /* Create similar interface to WeaverExtension (which is a QObject
           for other reasons). */
        void threadCreated (Thread *);
	void threadDestroyed (Thread *);
	void threadBusy (Thread *);
	void threadSuspended (Thread *);
    protected:
        /** Add a thread led. */
        void add (Thread*);
	QMap <Thread*, KLed*> mThreads;
        QGridLayout *mLayout;
    };
}

#endif // VISUALIZERWIDGET_H
