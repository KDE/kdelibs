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

#ifndef WEAVERVISUALIZER_H
#define WEAVERVISUALIZER_H

#include <weaverextensions.h>

#include <qobject.h>

class QWidget;

namespace ThreadWeaver {

    class VisualizerWidget;

    /** A WeaverThreadVisualizer may be attached to a Weaver to gain debug
       information about thread execution.
       wParent is a widget which is supposed to contain the widget. If zero, a
       toplevel widget is created.
    */
    class WeaverThreadVisualizer : public WeaverExtension
    {
	Q_OBJECT
    public:
        WeaverThreadVisualizer( QObject *parent = 0, const char *name = 0,
                                QWidget *wParent = 0);
	~WeaverThreadVisualizer();

        QWidget *widget();
	/* Overload public interface. */
	void threadCreated (Thread *);
	void threadDestroyed (Thread *);
	void threadBusy (Thread *);
	void threadSuspended (Thread *);

    protected:
        VisualizerWidget *mVis;
    };

}

#endif // WEAVERVISUALIZER_H
