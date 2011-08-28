/* -*- C++ -*-

This file declares the classes for the GUI Jobs example.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Trolltech, Norway. $

$Id: Jobs.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef Jobs_H
#define Jobs_H

#include <QtGui/QWidget>

#include <Job.h>
#include <ThreadWeaver.h>

#include "ui_JobsBase.h"
#include "WeaverObserverTest.h"

namespace ThreadWeaver {

    class JobCollection;

    class DummyJob : public Job
    {
    public:
        DummyJob (QObject* parent = 0);
    protected:
        void run ();
    };

    class Jobs : public QWidget
    {
        Q_OBJECT
    public:
        enum State {
            Initial = 0,
            Stopped = Initial,
            Disable,
            Stopping = Disable,
            ShuttingDown = Disable,
            Started
        };
        Jobs (QWidget *parent = 0);
        virtual ~Jobs();
    protected Q_SLOTS:
        void slotStart();
        void slotStop();
        void slotQuit();
        void slotLogStateChanged (int);
    protected:
        void setState ( State s );
        State state();
        JobCollection* m_jobs;
        bool m_quit;
        State m_state;
        Ui::JobsBase ui;
        WeaverObserverTest *m_log;
    protected Q_SLOTS:
        void slotJobDone (ThreadWeaver::Job*);
        void slotStopped ();
    };

}

#endif // defined Jobs_H
