/* This file is part of the KDE libraries
   Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                      Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "scheduler.h"

#include "sessiondata.h"
#include "slaveconfig.h"
#include "authinfo.h"
#include "slave.h"
#include "connection.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>
#include <assert.h>
#include <kdesu/client.h>

#include <QtCore/QHash>
#include <QtGui/QWidget>
#include <QtDBus/QtDBus>

// Slaves may be idle for MAX_SLAVE_IDLE time before they are being returned
// to the system wide slave pool. (3 minutes)
#define MAX_SLAVE_IDLE (3*60)

using namespace KIO;

template class QHash<QString, KIO::Scheduler::ProtocolInfo*>;

typedef QMap<Slave*, Scheduler::JobList *> CoSlaveMap;

class KIO::SlaveList : public QList<Slave *>
{
};

//
// There are two kinds of protocol:
// (1) The protocol of the url
// (2) The actual protocol that the io-slave uses.
//
// These two often match, but not necessarily. Most notably, they don't
// match when doing ftp via a proxy.
// In that case (1) is ftp, but (2) is http.
//
// JobData::protocol stores (2) while Job::url().protocol() returns (1).
// The ProtocolInfoDict is indexed with (2).
//
// We schedule slaves based on (2) but tell the slave about (1) via
// Slave::setProtocol().

class KIO::Scheduler::JobData
{
public:
    JobData() : checkOnHold(false) { }

public:
    QString protocol;
    QString proxy;
    bool checkOnHold;
};

class ExtraJobData: public QHash<KIO::SimpleJob*, KIO::Scheduler::JobData>
{
public:
    ExtraJobData() {}
    ~ExtraJobData() {}
};

class KIO::Scheduler::ProtocolInfo
{
public:
    ProtocolInfo() : maxSlaves(1), skipCount(0)
    {
    }

    QList<SimpleJob *> joblist;
    SlaveList activeSlaves;
    int maxSlaves;
    int skipCount;
    QString protocol;
};

class ProtocolInfoDict : public QHash<QString, KIO::Scheduler::ProtocolInfo*>
{
  public:
    ProtocolInfoDict() { }

    KIO::Scheduler::ProtocolInfo *get( const QString &protocol);
};

KIO::Scheduler::ProtocolInfo *
ProtocolInfoDict::get(const QString &protocol)
{
    Scheduler::ProtocolInfo *info = value(protocol, 0);
    if (!info)
    {
        info = new Scheduler::ProtocolInfo;
        info->protocol = protocol;
        info->maxSlaves = KProtocolInfo::maxSlaves( protocol );

        insert(protocol, info);
    }
    return info;
}

class KIO::SchedulerPrivate
{
public:
    SchedulerPrivate() :
        busy( false ),
        slaveList( new SlaveList ),
        idleSlaves( new SlaveList ),
        coIdleSlaves( new SlaveList ),
        protInfoDict( new ProtocolInfoDict ),
        slaveOnHold( 0 ),
        extraJobData( new ExtraJobData ),
        slaveConfig( SlaveConfig::self() ),
        sessionData( new SessionData ),
        checkOnHold( true ) // !! Always check with KLauncher for the first request
    {
        slaveTimer.setObjectName( "Scheduler::slaveTimer" );
        slaveTimer.setSingleShot( true );
        coSlaveTimer.setObjectName( "Scheduler::coSlaveTimer" );
        coSlaveTimer.setSingleShot( true );
        cleanupTimer.setObjectName( "Scheduler::cleanupTimer" );
        cleanupTimer.setSingleShot( true );
    }
    ~SchedulerPrivate()
    {
        qDeleteAll( *protInfoDict );
        delete protInfoDict; protInfoDict = 0;
        delete idleSlaves; idleSlaves = 0;
        delete coIdleSlaves; coIdleSlaves = 0;
        qDeleteAll( *slaveList );
        delete slaveList; slaveList = 0;
        delete extraJobData; extraJobData = 0;
    }
    QTimer slaveTimer;
    QTimer coSlaveTimer;
    QTimer cleanupTimer;
    bool busy;

    SlaveList *slaveList;
    SlaveList *idleSlaves;
    SlaveList *coIdleSlaves;

    ProtocolInfoDict *protInfoDict;
    Slave *slaveOnHold;
    KUrl urlOnHold;
    Scheduler::JobList newJobs;

    CoSlaveMap coSlaves;
    ExtraJobData *extraJobData;
    SlaveConfig *slaveConfig;
    SessionData *sessionData;
    bool checkOnHold;
    QMap<QObject *,WId> m_windowList;
};

Scheduler::Scheduler()
    : QObject(), d(new SchedulerPrivate())
{
    setObjectName( "scheduler" );

    const QString dbusPath = "/KIO/Scheduler";
    const QString dbusInterface = "org.kde.KIO.Scheduler";
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject( "/KIO/Scheduler", this, QDBusConnection::ExportScriptableSlots |
                                                 QDBusConnection::ExportScriptableSignals );
    dbus.connect(QString(), dbusPath, dbusInterface, "reparseSlaveConfiguration",
                 this, SLOT(slotReparseSlaveConfiguration(QString)));

    connect(&(d->slaveTimer), SIGNAL(timeout()), SLOT(startStep()));
    connect(&(d->coSlaveTimer), SIGNAL(timeout()), SLOT(slotScheduleCoSlave()));
    connect(&(d->cleanupTimer), SIGNAL(timeout()), SLOT(slotCleanIdleSlaves()));
}

Scheduler::~Scheduler()
{
    delete d;
}

void Scheduler::doJob(SimpleJob *job)
{
    self()->_doJob(job);
}

void Scheduler::scheduleJob(SimpleJob *job)
{
    self()->_scheduleJob(job);
}

void Scheduler::cancelJob(SimpleJob *job)
{
    self()->_cancelJob(job);
}

void Scheduler::jobFinished(KIO::SimpleJob *job, KIO::Slave *slave)
{
    self()->_jobFinished(job, slave);
}

void Scheduler::putSlaveOnHold(KIO::SimpleJob *job, const KUrl &url)
{
    self()->_putSlaveOnHold(job, url);
}

void Scheduler::removeSlaveOnHold()
{
    self()->_removeSlaveOnHold();
}

void Scheduler::publishSlaveOnHold()
{
    self()->_publishSlaveOnHold();
}

KIO::Slave *Scheduler::getConnectedSlave(const KUrl &url,
        const KIO::MetaData &config )
{
    return self()->_getConnectedSlave(url, config);
}

bool Scheduler::assignJobToSlave(KIO::Slave *slave, KIO::SimpleJob *job)
{
    return self()->_assignJobToSlave(slave, job);
}

bool Scheduler::disconnectSlave(KIO::Slave *slave)
{
    return self()->_disconnectSlave(slave);
}

void Scheduler::registerWindow(QWidget *wid)
{
    self()->_registerWindow(wid);
}

void Scheduler::unregisterWindow(QObject *wid)
{
    self()->slotUnregisterWindow(wid);
}

bool Scheduler::connect( const char *signal, const QObject *receiver,
                         const char *member)
{
    return QObject::connect(self(), signal, receiver, member);
}

bool Scheduler::connect( const QObject* sender, const char* signal,
                         const QObject* receiver, const char* member )
{
    return QObject::connect(sender, signal, receiver, member);
}

bool Scheduler::disconnect( const QObject* sender, const char* signal,
                            const QObject* receiver, const char* member )
{
    return QObject::disconnect(sender, signal, receiver, member);
}

bool Scheduler::connect( const QObject *sender, const char *signal,
                         const char *member )
{
    return QObject::connect(sender, signal, member);
}

void Scheduler::checkSlaveOnHold(bool b)
{
    self()->_checkSlaveOnHold(b);
}

void Scheduler::emitReparseSlaveConfiguration()
{
    self()->reparseSlaveConfiguration( QString() );
}

void
Scheduler::debug_info()
{
}

void Scheduler::slotReparseSlaveConfiguration(const QString &proto)
{
    kDebug( 7006 ) << "reparseSlaveConfiguration( " << proto << " )" << endl;
    KProtocolManager::reparseConfiguration();
    d->slaveConfig->reset();
    d->sessionData->reset();
    NetRC::self()->reload();

    foreach( Slave *slave, *(d->slaveList) )
    {
        if ( slave->slaveProtocol() == proto || proto.isEmpty() )
        {
            slave->send( CMD_REPARSECONFIGURATION );
            slave->resetHost();
        }
    }
}

void Scheduler::_doJob(SimpleJob *job) {
    JobData jobData;
    jobData.protocol = KProtocolManager::slaveProtocol(job->url(), jobData.proxy);
//    kDebug(7006) << "Scheduler::_doJob protocol=" << jobData->protocol << endl;
    if (job->command() == CMD_GET)
    {
       jobData.checkOnHold = d->checkOnHold;
       d->checkOnHold = false;
    }
    d->extraJobData->insert(job, jobData);
    d->newJobs.append(job);
    d->slaveTimer.start(0);
#ifndef NDEBUG
    if (d->newJobs.count() > 150)
	kDebug() << "WARNING - KIO::Scheduler got more than 150 jobs! This shows a misuse in your app (yes, a job is a QObject)." << endl;
#endif
}

void Scheduler::_scheduleJob(SimpleJob *job) {
    d->newJobs.removeAll(job);
    JobData jobData = d->extraJobData->value(job);

    QString protocol = jobData.protocol;
//    kDebug(7006) << "Scheduler::_scheduleJob protocol=" << protocol << endl;
    ProtocolInfo *protInfo = d->protInfoDict->get(protocol);
    protInfo->joblist.append(job);

    d->slaveTimer.start(0);
}

void Scheduler::_cancelJob(SimpleJob *job) {
//    kDebug(7006) << "Scheduler: canceling job " << job << endl;
    Slave *slave = job->slave();
    if ( !slave  )
    {
        // was not yet running (don't call this on a finished job!)
        JobData jobData = d->extraJobData->value(job);
        d->newJobs.removeAll(job);
        ProtocolInfo *protInfo = d->protInfoDict->get(jobData.protocol);
        protInfo->joblist.removeAll(job);

        // Search all slaves to see if job is in the queue of a coSlave
        foreach( Slave* coSlave, *(d->slaveList) )
        {
           JobList *list = d->coSlaves.value(slave);
           if (list && list->removeAll(job)) {
               // Job was found and removed.
               // Fall through to kill the slave as well!
               slave = coSlave;
           }
        }
        if (!slave)
        {
           d->extraJobData->remove(job);
           return; // Job was not yet running and not in a coSlave queue.
        }
    }
    kDebug(7006) << "Scheduler: killing slave " << slave->slave_pid() << endl;
    slave->kill();
    _jobFinished( job, slave );
    slotSlaveDied( slave);
}

void Scheduler::startStep()
{
    while(d->newJobs.count())
    {
       (void) startJobDirect();
    }

    QHashIterator<QString, KIO::Scheduler::ProtocolInfo*> it(*d->protInfoDict);
    while(it.hasNext()) {
       it.next();
       if (startJobScheduled(it.value())) return;
    }
}

void Scheduler::setupSlave(KIO::Slave *slave, const KUrl &url, const QString &protocol, const QString &proxy , bool newSlave, const KIO::MetaData *config)
{
    QString host = url.host();
    int port = url.port();
    if ( port == -1 ) // no port is -1 in QUrl, but in kde3 we used 0 and the kioslaves assume that.
        port = 0;
    QString user = url.user();
    QString passwd = url.pass();

    if ((newSlave) ||
        (slave->host() != host) ||
        (slave->port() != port) ||
        (slave->user() != user) ||
        (slave->passwd() != passwd))
    {
        d->slaveConfig = SlaveConfig::self();

        MetaData configData = d->slaveConfig->configData(protocol, host);
        d->sessionData->configDataFor( configData, protocol, host );

        configData["UseProxy"] = proxy;

        QString autoLogin = configData["EnableAutoLogin"].toLower();
        if ( autoLogin == "true" )
        {
            NetRC::AutoLogin l;
            l.login = user;
            bool usern = (protocol == "ftp");
            if ( NetRC::self()->lookup( url, l, usern) )
            {
                configData["autoLoginUser"] = l.login;
                configData["autoLoginPass"] = l.password;
                if ( usern )
                {
                    QString macdef;
                    QMap<QString, QStringList>::ConstIterator it = l.macdef.begin();
                    for ( ; it != l.macdef.end(); ++it )
                        macdef += it.key() + '\\' + it.value().join( "\\" ) + '\n';
                    configData["autoLoginMacro"] = macdef;
                }
            }
        }
        if (config)
           configData += *config;
        slave->setConfig(configData);
        slave->setProtocol(url.protocol());
        slave->setHost(host, port, user, passwd);
    }
}

bool Scheduler::startJobScheduled(ProtocolInfo *protInfo)
{
    if (protInfo->joblist.isEmpty())
       return false;

//       kDebug(7006) << "Scheduling job" << endl;
    debug_info();
    bool newSlave = false;

    SimpleJob *job = 0;
    Slave *slave = 0;

    if (protInfo->skipCount > 2)
    {
       bool dummy;
       // Prevent starvation. We skip the first entry in the queue at most
       // 2 times in a row. The
       protInfo->skipCount = 0;
       job = protInfo->joblist.at(0);
       slave = findIdleSlave(protInfo, job, dummy );
    }
    else
    {
       bool exact=false;
       SimpleJob *firstJob = 0;
       Slave *firstSlave = 0;
       for(int i = 0; (i < protInfo->joblist.count()) && (i < 10); i++)
       {
          job = protInfo->joblist.at(i);
          slave = findIdleSlave(protInfo, job, exact);
          if (!firstSlave)
          {
             firstJob = job;
             firstSlave = slave;
          }
          if (!slave) break;
          if (exact) break;
       }

       if (!exact)
       {
         slave = firstSlave;
         job = firstJob;
       }
       if (job == firstJob)
         protInfo->skipCount = 0;
       else
         protInfo->skipCount++;
    }

    if (!slave)
    {
       if ( protInfo->maxSlaves > static_cast<int>(protInfo->activeSlaves.count()) )
       {
          newSlave = true;
          slave = createSlave(protInfo, job, job->url());
          if (!slave)
             d->slaveTimer.start(0);
       }
    }

    if (!slave)
    {
//          kDebug(7006) << "No slaves available" << endl;
//          kDebug(7006) << " -- active: " << protInfo->activeSlaves.count() << endl;
       return false;
    }

    protInfo->activeSlaves.append(slave);
    d->idleSlaves->removeAll(slave);
    protInfo->joblist.removeAll(job);
//       kDebug(7006) << "scheduler: job started " << job << endl;


    JobData jobData = d->extraJobData->value(job);
    setupSlave(slave, job->url(), jobData.protocol, jobData.proxy, newSlave);
    job->start(slave);

    d->slaveTimer.start(0);
    return true;
}

bool Scheduler::startJobDirect()
{
    debug_info();
    SimpleJob *job = d->newJobs.takeFirst();
    JobData jobData = d->extraJobData->value(job);

    QString protocol = jobData.protocol;
    ProtocolInfo *protInfo = d->protInfoDict->get(protocol);

    bool newSlave = false;
    bool dummy;

    // Look for matching slave
    Slave *slave = findIdleSlave(protInfo, job, dummy);

    if (!slave)
    {
       newSlave = true;
       slave = createSlave(protInfo, job, job->url());
    }

    if (!slave)
       return false;

    d->idleSlaves->removeAll(slave);
//       kDebug(7006) << "scheduler: job started " << job << endl;

    setupSlave(slave, job->url(), protocol, jobData.proxy, newSlave);
    job->start(slave);
    return true;
}

static Slave *searchIdleList(SlaveList *idleSlaves, const KUrl &url, const QString &protocol, bool &exact)
{
    QString host = url.host();
    int port = url.port();
    if ( port == -1 ) // no port is -1 in QUrl, but in kde3 we used 0 and the kioslaves assume that.
        port = 0;
    QString user = url.user();
    exact = true;

    foreach( Slave *slave, *idleSlaves )
    {
       if ((protocol == slave->slaveProtocol()) &&
           (host == slave->host()) &&
           (port == slave->port()) &&
           (user == slave->user()))
           return slave;
    }

    exact = false;

    // Look for slightly matching slave
    foreach( Slave *slave, *idleSlaves )
    {
       if (protocol == slave->slaveProtocol())
          return slave;
    }
    return 0;
}

Slave *Scheduler::findIdleSlave(ProtocolInfo *, SimpleJob *job, bool &exact)
{
    Slave *slave = 0;
    JobData jobData = d->extraJobData->value(job);

    if (jobData.checkOnHold)
    {
       slave = Slave::holdSlave(jobData.protocol, job->url());
       if (slave)
          return slave;
    }
    if (d->slaveOnHold)
    {
       // Make sure that the job wants to do a GET or a POST, and with no offset
       bool bCanReuse = (job->command() == CMD_GET);
       KIO::TransferJob * tJob = qobject_cast<KIO::TransferJob *>(job);
       if ( tJob )
       {
          bCanReuse = (job->command() == CMD_GET || job->command() == CMD_SPECIAL);
          if ( bCanReuse )
          {
            KIO::MetaData outgoing = tJob->outgoingMetaData();
            QString resume = (!outgoing.contains("resume")) ? QString() : outgoing["resume"];
            kDebug(7006) << "Resume metadata is '" << resume << "'" << endl;
            bCanReuse = (resume.isEmpty() || resume == "0");
          }
       }
//       kDebug(7006) << "bCanReuse = " << bCanReuse << endl;
       if (bCanReuse)
       {
          if (job->url() == d->urlOnHold)
          {
             kDebug(7006) << "HOLD: Reusing held slave for " << d->urlOnHold.prettyUrl() << endl;
             slave = d->slaveOnHold;
          }
          else
          {
             kDebug(7006) << "HOLD: Discarding held slave (" << d->urlOnHold.prettyUrl() << ")" << endl;
             d->slaveOnHold->kill();
          }
          d->slaveOnHold = 0;
          d->urlOnHold = KUrl();
       }
       if (slave)
          return slave;
    }

    return searchIdleList(d->idleSlaves, job->url(), jobData.protocol, exact);
}

Slave *Scheduler::createSlave(ProtocolInfo *protInfo, SimpleJob *job, const KUrl &url)
{
   int error;
   QString errortext;
   Slave *slave = Slave::createSlave(protInfo->protocol, url, error, errortext);
   if (slave)
   {
      d->slaveList->append(slave);
      d->idleSlaves->append(slave);
      connect(slave, SIGNAL(slaveDied(KIO::Slave *)),
                SLOT(slotSlaveDied(KIO::Slave *)));
      connect(slave, SIGNAL(slaveStatus(pid_t,const QByteArray&,const QString &, bool)),
                SLOT(slotSlaveStatus(pid_t,const QByteArray&, const QString &, bool)));
   }
   else
   {
      kError() <<": couldn't create slave : " << errortext << endl;
      if (job)
      {
         protInfo->joblist.removeAll(job);
         d->extraJobData->remove(job);
         job->slotError( error, errortext );
      }
   }
   return slave;
}

void Scheduler::slotSlaveStatus(pid_t, const QByteArray&, const QString &, bool)
{
}

void Scheduler::_jobFinished(SimpleJob *job, Slave *slave)
{
    JobData jobData = d->extraJobData->take(job);

    ProtocolInfo *protInfo = d->protInfoDict->get(jobData.protocol);
    slave->disconnect(job);
    protInfo->activeSlaves.removeAll(slave);
    if (slave->isAlive())
    {
       JobList *list = d->coSlaves.value(slave);
       if (list)
       {
          assert(slave->isConnected());
          assert(!d->coIdleSlaves->contains(slave));
          d->coIdleSlaves->append(slave);
          if (!list->isEmpty())
             d->coSlaveTimer.start(0);
          return;
       }
       else
       {
          assert(!slave->isConnected());
          d->idleSlaves->append(slave);
          slave->setIdle();
          _scheduleCleanup();
//          slave->send( CMD_SLAVE_STATUS );
       }
    }
    if (protInfo->joblist.count())
    {
       d->slaveTimer.start(0);
    }
}

void Scheduler::slotSlaveDied(KIO::Slave *slave)
{
    assert(!slave->isAlive());
    ProtocolInfo *protInfo = d->protInfoDict->get(slave->slaveProtocol());
    protInfo->activeSlaves.removeAll(slave);
    if (slave == d->slaveOnHold)
    {
       d->slaveOnHold = 0;
       d->urlOnHold = KUrl();
    }
    d->idleSlaves->removeAll(slave);
    JobList *list = d->coSlaves.value(slave);
    if (list)
    {
       // coSlave dies, kill jobs waiting in queue
       disconnectSlave(slave);
    }

    if (!d->slaveList->removeAll(slave))
        kDebug(7006) << "Scheduler: BUG!! Slave " << slave << "/" << slave->slave_pid() << " died, but is NOT in slaveList!!!\n" << endl;
    else
        slave->deref(); // Delete slave
}

void Scheduler::slotCleanIdleSlaves()
{
    SlaveList::iterator it = d->idleSlaves->begin();
    for( ; it != d->idleSlaves->end(); )
    {
        Slave *slave = *it;
        if (slave->idleTime() >= MAX_SLAVE_IDLE)
        {
           // kDebug(7006) << "Removing idle slave: " << slave->slaveProtocol() << " " << slave->host() << endl;
           Slave *removeSlave = slave;
           it = d->idleSlaves->erase( it );
           d->slaveList->removeAll( removeSlave );
           removeSlave->connection()->close();
           removeSlave->deref();
        }
        else
        {
            ++it;
        }
    }
    _scheduleCleanup();
}

void Scheduler::_scheduleCleanup()
{
    if (d->idleSlaves->count())
    {
        if (!d->cleanupTimer.isActive())
            d->cleanupTimer.start( MAX_SLAVE_IDLE*1000 );
    }
}

void Scheduler::_putSlaveOnHold(KIO::SimpleJob *job, const KUrl &url)
{
    Slave *slave = job->slave();
    slave->disconnect(job);

    if (d->slaveOnHold)
    {
        d->slaveOnHold->kill();
    }
    d->slaveOnHold = slave;
    d->urlOnHold = url;
    d->slaveOnHold->suspend();
}

void Scheduler::_publishSlaveOnHold()
{
    if (!d->slaveOnHold)
       return;

    d->slaveOnHold->hold(d->urlOnHold);
}

void Scheduler::_removeSlaveOnHold()
{
    if (d->slaveOnHold)
    {
        d->slaveOnHold->kill();
    }
    d->slaveOnHold = 0;
    d->urlOnHold = KUrl();
}

Slave *
Scheduler::_getConnectedSlave(const KUrl &url, const KIO::MetaData &config )
{
    QString proxy;
    QString protocol = KProtocolManager::slaveProtocol(url, proxy);
    bool dummy;
    Slave *slave = searchIdleList(d->idleSlaves, url, protocol, dummy);
    if (!slave)
    {
       ProtocolInfo *protInfo = d->protInfoDict->get(protocol);
       slave = createSlave(protInfo, 0, url);
    }
    if (!slave)
       return 0; // Error
    d->idleSlaves->removeAll(slave);

    setupSlave(slave, url, protocol, proxy, true, &config);

    slave->send( CMD_CONNECT );
    connect(slave, SIGNAL(connected()),
                SLOT(slotSlaveConnected()));
    connect(slave, SIGNAL(error(int, const QString &)),
                SLOT(slotSlaveError(int, const QString &)));

    d->coSlaves.insert(slave, new JobList);
//    kDebug(7006) << "_getConnectedSlave( " << slave << ")" << endl;
    return slave;
}

void
Scheduler::slotScheduleCoSlave()
{
    d->slaveConfig = SlaveConfig::self();
    SlaveList::iterator it = d->coIdleSlaves->begin();
    for( ; it != d->coIdleSlaves->end(); )
    {
        Slave* slave = *it;
        JobList *list = d->coSlaves.value(slave);
        assert(list);
        if (list && !list->isEmpty())
        {
           SimpleJob *job = list->takeFirst();
           it = d->coIdleSlaves->erase( it );
//           kDebug(7006) << "scheduler: job started " << job << endl;

           KUrl url =job->url();
           QString host = url.host();
           int port = url.port();
           if ( port == -1 ) // no port is -1 in QUrl, but in kde3 we used 0 and the kioslaves assume that.
               port = 0;

           if (slave->host() == "<reset>")
           {
              QString user = url.user();
              QString passwd = url.pass();

              MetaData configData = d->slaveConfig->configData(url.protocol(), url.host());
              slave->setConfig(configData);
              slave->setProtocol(url.protocol());
              slave->setHost(host, port, user, passwd);
           }

           assert(slave->protocol() == url.protocol());
           assert(slave->host() == host);
           assert(slave->port() == port);
           job->start(slave);
        } else {
            ++it;
        }
    }
}

void
Scheduler::slotSlaveConnected()
{
    Slave *slave = (Slave *)sender();
//    kDebug(7006) << "slotSlaveConnected( " << slave << ")" << endl;
    slave->setConnected(true);
    disconnect(slave, SIGNAL(connected()),
               this, SLOT(slotSlaveConnected()));
    emit slaveConnected(slave);
    assert(!d->coIdleSlaves->contains(slave));
    d->coIdleSlaves->append(slave);
    d->coSlaveTimer.start(0);
}

void
Scheduler::slotSlaveError(int errorNr, const QString &errorMsg)
{
    Slave *slave = (Slave *)sender();
    if (!slave->isConnected() || d->coIdleSlaves->contains(slave))
    {
       // Only forward to application if slave is idle or still connecting.
       emit slaveError(slave, errorNr, errorMsg);
    }
}

bool
Scheduler::_assignJobToSlave(KIO::Slave *slave, SimpleJob *job)
{
//    kDebug(7006) << "_assignJobToSlave( " << job << ", " << slave << ")" << endl;
    QString dummy;
    if ((slave->slaveProtocol() != KProtocolManager::slaveProtocol( job->url(), dummy ))
        ||
        (!d->newJobs.removeAll(job)))
    {
        kDebug(7006) << "_assignJobToSlave(): ERROR, nonmatching or unknown job." << endl;
        job->kill();
        return false;
    }

    JobList *list = d->coSlaves.value(slave);
    assert(list);
    if (!list)
    {
        kDebug(7006) << "_assignJobToSlave(): ERROR, unknown slave." << endl;
        job->kill();
        return false;
    }

    assert(!list->contains(job));
    list->append(job);
    d->coSlaveTimer.start(0); // Start job on timer event

    return true;
}

bool
Scheduler::_disconnectSlave(KIO::Slave *slave)
{
//    kDebug(7006) << "_disconnectSlave( " << slave << ")" << endl;
    CoSlaveMap::iterator coSlaveIt = d->coSlaves.find( slave );
    assert( coSlaveIt != d->coSlaves.end() );
    JobList *list = *coSlaveIt;
    d->coSlaves.erase( coSlaveIt );
    assert(list);
    if (!list)
       return false;
    // Kill jobs still in queue.
    while(!list->isEmpty())
    {
       Job *job = list->takeFirst();
       job->kill();
    }
    delete list;
    d->coIdleSlaves->removeAll(slave);
    assert(!d->coIdleSlaves->contains(slave));
    disconnect(slave, SIGNAL(connected()),
               this, SLOT(slotSlaveConnected()));
    disconnect(slave, SIGNAL(error(int, const QString &)),
               this, SLOT(slotSlaveError(int, const QString &)));
    if (slave->isAlive())
    {
       d->idleSlaves->append(slave);
       slave->send( CMD_DISCONNECT );
       slave->setIdle();
       slave->setConnected(false);
       _scheduleCleanup();
    }
    return true;
}

void
Scheduler::_checkSlaveOnHold(bool b)
{
    d->checkOnHold = b;
}

void
Scheduler::_registerWindow(QWidget *wid)
{
   if (!wid)
      return;

   QObject *obj = static_cast<QObject *>(wid);
   if (!d->m_windowList.contains(obj))
   {
      // We must store the window Id because by the time
      // the destroyed signal is emitted we can no longer
      // access QWidget::winId() (already destructed)
      WId windowId = wid->winId();
      d->m_windowList.insert(obj, windowId);
      connect(wid, SIGNAL(destroyed(QObject *)),
              this, SLOT(slotUnregisterWindow(QObject*)));
      QDBusInterface("org.kde.kded", "/kded", "org.kde.kded").
          call(QDBus::NoBlock, "registerWindowId", qlonglong(windowId));
   }
}

void
Scheduler::slotUnregisterWindow(QObject *obj)
{
   if (!obj)
      return;

   QMap<QObject *, WId>::Iterator it = d->m_windowList.find(obj);
   if (it == d->m_windowList.end())
      return;
   WId windowId = it.value();
   disconnect( it.key(), SIGNAL(destroyed(QObject *)),
              this, SLOT(slotUnregisterWindow(QObject*)));
   d->m_windowList.erase( it );
   QDBusInterface("org.kde.kded", "/kded", "org.kde.kded").
       call(QDBus::NoBlock, "unregisterWindowId", qlonglong(windowId));
}

Scheduler* Scheduler::self()
{
    K_GLOBAL_STATIC(Scheduler, instance)
    return instance;
}

#include "scheduler.moc"
