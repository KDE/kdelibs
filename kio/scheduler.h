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
#include "kio/jobclasses.h"
#include <qtimer.h>

#include <dcopobject.h>

namespace KIO {

    class Slave;
    class SlaveList;
    struct AuthKey;

    class Scheduler : public QObject, virtual public DCOPObject {
	Q_OBJECT

    public: // InfoDict needs Info, so we can't declare it private
	class ProtocolInfo;

	~Scheduler();

	static void doJob(SimpleJob *job)
		{ self()->_doJob(job); }
	static void scheduleJob(SimpleJob *job)
		{ self()->_scheduleJob(job); }
	static void cancelJob(SimpleJob *job)
		{ self()->_cancelJob(job); }
        static void jobFinished(KIO::SimpleJob *job, KIO::Slave *slave)
            	{ self()->_jobFinished(job, slave); }

        static void putSlaveOnHold(KIO::SimpleJob *job, const KURL &url)
        	{ self()->_putSlaveOnHold(job, url); }

        static void removeSlaveOnHold()
        	{ self()->_removeSlaveOnHold(); }

        void debug_info();

        virtual bool process(const QCString &fun, const QByteArray &data,
		             QCString& replyType, QByteArray &replyData);

        virtual QCStringList functions();

    public slots:
        void slotSlaveDied(KIO::Slave *slave);
	void slotSlaveStatus(pid_t pid, const QCString &protocol,
	                     const QString &host, bool connected);

    protected slots:
        void startStep();
        void slotCleanIdleSlaves();
        void slotAuthorizationKey( const QCString&, const QCString&, bool keep );

    protected:
        bool startJobScheduled(ProtocolInfo *protInfo);
        bool startJobDirect();
	Scheduler();

    private:
        class ProtocolInfoDict;

	Scheduler(const Scheduler&);

	static Scheduler *self();
	static Scheduler *instance;
	void _doJob(SimpleJob *job);
	void _scheduleJob(SimpleJob *job);
	void _cancelJob(SimpleJob *job);
        void _jobFinished(KIO::SimpleJob *job, KIO::Slave *slave);
        void _scheduleCleanup();
        void _putSlaveOnHold(KIO::SimpleJob *job, const KURL &url);
        void _removeSlaveOnHold();
        Slave *findIdleSlave(ProtocolInfo *protInfo, SimpleJob *job);
        Slave *createSlave(ProtocolInfo *protInfo, SimpleJob *job);

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

	Slave *slaveOnHold;
	KURL urlOnHold;


    typedef QList<AuthKey> AuthKeyList;
    AuthKeyList cachedAuthKeys;

    QList<SimpleJob> newJobs;

    /**
     * Checks whether the password daemon kdesud is
     * running or if it can be started if it is not.
     *
     * @return true if password daemon is/can be started successfully.
     */
    bool pingCacheDaemon() const;

    /**
     * Increments the reference count for application using the
     * give authorization key.
     *
     * The reference counting is used by @ref delCachedAuthentication
     * to determine when it is safe to delete the key from the cache.
     *
     * A call to this function will fail, i.e. return false, if there
     * is no entry for the given @p groupname value and/or the cache
     * deamon, @p kdesud, cannot be contacted.
     *
     * @return true if the registration succeeds.
     */
    bool regCachedAuthKey( const QCString&, const QCString& );

    /**
     * Deletes any cached keys for the given list.
     *
     * @param list list of cached authentication key to be deleted.
     */
    void delCachedAuthKeys( const AuthKeyList& list );

};

};
#endif
