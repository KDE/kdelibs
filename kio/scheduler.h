// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _kio_scheduler_h
#define _kio_scheduler_h

#include "kio/job.h"
#include <qtimer.h>

namespace KIO {

    class Slave;
    class SlaveList;

    class Scheduler : public QObject {
	Q_OBJECT

    public:
	static void doJob(SimpleJob *job)
		{ self()->_doJob(job); }
	static void cancelJob(SimpleJob *job)
		{ self()->_cancelJob(job); }
        static void jobFinished(KIO::SimpleJob *job, KIO::Slave *slave)
            	{ self()->_jobFinished(job, slave); }

        void debug_info();

    public slots:
        //void slotSlaveDied(KIO::Slave *slave);
	void slotSlaveStatus(pid_t pid, const QCString &protocol,
	                     const QString &host, bool connected);

    protected slots:
        void startStep();
        void slotCleanIdleSlaves();

    protected:
	Scheduler();

    private:
        class ProtocolInfo;
        class ProtocolInfoDict;

	Scheduler(const Scheduler&);

	static Scheduler *self();
	static Scheduler *instance;
	void _doJob(SimpleJob *job);
	void _cancelJob(SimpleJob *job);
        void _jobFinished(KIO::SimpleJob *job, KIO::Slave *slave);
        void _scheduleCleanup();

	QList<SimpleJob> joblist;
	QTimer mytimer;
	QTimer cleanupTimer;
	bool busy;
	/* (Stephan) Of course this isn't meant to be
	 * the final solution, the slaves should be handled
	 * within the pool. I do a one slave per app first
	 * though. Who feels brave enough - may do it
	 */
	/* (Waba) I feel lucky today. Let's see if we can make
	 * it a list of slaves. Still not finished of course.
	 */
	SlaveList *slaveList;
	SlaveList *idleSlaves;
	
	ProtocolInfoDict *protInfoDict;
    };
};

#endif
