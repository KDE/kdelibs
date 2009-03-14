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
#include "job_p.h"

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

#ifndef KDE_USE_FINAL // already defined in job.cpp
static inline Slave *jobSlave(SimpleJob *job)
{
    return SimpleJobPrivate::get(job)->m_slave;
}
#endif

static inline int jobCommand(SimpleJob *job)
{
    return SimpleJobPrivate::get(job)->m_command;
}

static inline void startJob(SimpleJob *job, Slave *slave)
{
    SimpleJobPrivate::get(job)->start(slave);
}

typedef QList<Slave *> SlaveList;
typedef QMap<Slave *, QList<SimpleJob *> * > CoSlaveMap;

class KIO::SchedulerPrivate
{
public:
    class JobData;
    class ProtocolInfo;
    class ProtocolInfoDict : public QHash<QString, ProtocolInfo*>
    {
    public:
        ProtocolInfoDict() { }

        ProtocolInfo *get(const QString &protocol);
    };

    typedef QHash<KIO::SimpleJob*, JobData> ExtraJobData;
    typedef QList<SimpleJob *> JobList;

    SchedulerPrivate() :
        q(new Scheduler),
        busy( false ),
        slaveOnHold( 0 ),
        slaveConfig( SlaveConfig::self() ),
        sessionData( new SessionData ),
        checkOnHold( true ) // !! Always check with KLauncher for the first request
    {
        slaveTimer.setObjectName( "Scheduler::slaveTimer" );
        slaveTimer.setSingleShot( true );
        q->connect(&slaveTimer, SIGNAL(timeout()), SLOT(startStep()));
        coSlaveTimer.setObjectName( "Scheduler::coSlaveTimer" );
        coSlaveTimer.setSingleShot( true );
        q->connect(&coSlaveTimer, SIGNAL(timeout()), SLOT(slotScheduleCoSlave()));
        cleanupTimer.setObjectName( "Scheduler::cleanupTimer" );
        cleanupTimer.setSingleShot( true );
        q->connect(&cleanupTimer, SIGNAL(timeout()), SLOT(slotCleanIdleSlaves()));
    }
    ~SchedulerPrivate()
    {
        delete q; q = 0;
        qDeleteAll( protInfoDict );
        delete sessionData;
    }
    Scheduler *q;

    QTimer slaveTimer;
    QTimer coSlaveTimer;
    QTimer cleanupTimer;
    bool busy;

    ProtocolInfoDict protInfoDict;
    Slave *slaveOnHold;
    KUrl urlOnHold;
    JobList newJobs;

    CoSlaveMap coSlaves;
    ExtraJobData extraJobData;
    SlaveConfig *slaveConfig;
    SessionData *sessionData;
    bool checkOnHold;
    QMap<QObject *,WId> m_windowList;

    void doJob(SimpleJob *job);
    void scheduleJob(SimpleJob *job);
    void cancelJob(SimpleJob *job);
    void jobFinished(KIO::SimpleJob *job, KIO::Slave *slave);
    void scheduleCleanup();
    void putSlaveOnHold(KIO::SimpleJob *job, const KUrl &url);
    void removeSlaveOnHold();
    Slave *getConnectedSlave(const KUrl &url, const KIO::MetaData &metaData );
    bool assignJobToSlave(KIO::Slave *slave, KIO::SimpleJob *job);
    bool disconnectSlave(KIO::Slave *slave);
    void checkSlaveOnHold(bool b);
    void publishSlaveOnHold();
    void registerWindow(QWidget *wid);

    Slave *findIdleSlave(ProtocolInfo *protInfo, SimpleJob *job, bool &exact);
    Slave *createSlave(ProtocolInfo *protInfo, SimpleJob *job, const KUrl &url);

    void debug_info();

    void setupSlave(KIO::Slave *slave, const KUrl &url, const QString &protocol, const QString &proxy , bool newSlave, const KIO::MetaData *config=0);
    bool startJobScheduled(ProtocolInfo *protInfo);
    bool startJobDirect();

    void slotSlaveDied(KIO::Slave *slave);
    void slotSlaveStatus(pid_t pid, const QByteArray &protocol,
                         const QString &host, bool connected);

    void slotReparseSlaveConfiguration(const QString &);

    void startStep();
    void slotCleanIdleSlaves();
    void slotSlaveConnected();
    void slotSlaveError(int error, const QString &errorMsg);
    void slotScheduleCoSlave();
    void slotUnregisterWindow(QObject *);
};

class KIO::SchedulerPrivate::ProtocolInfo
{
public:
    ProtocolInfo() : maxSlaves(1), skipCount(0)
    {
    }

    ~ProtocolInfo()
    {
        qDeleteAll(allSlaves());
    }

    // bad performance, but will change this later
    SlaveList allSlaves() const
    {
        SlaveList ret(activeSlaves);
        ret.append(idleSlaves);
        ret.append(coSlaves.keys());
        ret.append(coIdleSlaves);
        return ret;
    }

    QList<SimpleJob *> joblist;
    SlaveList activeSlaves;
    SlaveList idleSlaves;
    CoSlaveMap coSlaves;
    SlaveList coIdleSlaves;
    int maxSlaves;
    int skipCount;
    QString protocol;
};

KIO::SchedulerPrivate::ProtocolInfo *
KIO::SchedulerPrivate::ProtocolInfoDict::get(const QString &protocol)
{
    ProtocolInfo *info = value(protocol, 0);
    if (!info)
    {
        info = new ProtocolInfo;
        info->protocol = protocol;
        info->maxSlaves = KProtocolInfo::maxSlaves( protocol );

        insert(protocol, info);
    }
    return info;
}

K_GLOBAL_STATIC(SchedulerPrivate, schedulerPrivate)
Scheduler* Scheduler::self()
{
    return schedulerPrivate->q;
}


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

class KIO::SchedulerPrivate::JobData
{
public:
    JobData() : checkOnHold(false) { }

public:
    QString protocol;
    QString proxy;
    bool checkOnHold;
};


Scheduler::Scheduler()
    : QObject(), d(0)
{
    setObjectName( "scheduler" );

    const QString dbusPath = "/KIO/Scheduler";
    const QString dbusInterface = "org.kde.KIO.Scheduler";
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject( "/KIO/Scheduler", this, QDBusConnection::ExportScriptableSlots |
                                                 QDBusConnection::ExportScriptableSignals );
    dbus.connect(QString(), dbusPath, dbusInterface, "reparseSlaveConfiguration",
                 this, SLOT(slotReparseSlaveConfiguration(QString)));
}

Scheduler::~Scheduler()
{
}

void Scheduler::doJob(SimpleJob *job)
{
    schedulerPrivate->doJob(job);
}

void Scheduler::scheduleJob(SimpleJob *job)
{
    schedulerPrivate->scheduleJob(job);
}

void Scheduler::cancelJob(SimpleJob *job)
{
    schedulerPrivate->cancelJob(job);
}

void Scheduler::jobFinished(KIO::SimpleJob *job, KIO::Slave *slave)
{
    schedulerPrivate->jobFinished(job, slave);
}

void Scheduler::putSlaveOnHold(KIO::SimpleJob *job, const KUrl &url)
{
    schedulerPrivate->putSlaveOnHold(job, url);
}

void Scheduler::removeSlaveOnHold()
{
    schedulerPrivate->removeSlaveOnHold();
}

void Scheduler::publishSlaveOnHold()
{
    schedulerPrivate->publishSlaveOnHold();
}

KIO::Slave *Scheduler::getConnectedSlave(const KUrl &url,
        const KIO::MetaData &config )
{
    return schedulerPrivate->getConnectedSlave(url, config);
}

bool Scheduler::assignJobToSlave(KIO::Slave *slave, KIO::SimpleJob *job)
{
    return schedulerPrivate->assignJobToSlave(slave, job);
}

bool Scheduler::disconnectSlave(KIO::Slave *slave)
{
    return schedulerPrivate->disconnectSlave(slave);
}

void Scheduler::registerWindow(QWidget *wid)
{
    schedulerPrivate->registerWindow(wid);
}

void Scheduler::unregisterWindow(QObject *wid)
{
    schedulerPrivate->slotUnregisterWindow(wid);
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
    schedulerPrivate->checkSlaveOnHold(b);
}

void Scheduler::emitReparseSlaveConfiguration()
{
    self()->reparseSlaveConfiguration( QString() );
}

void
SchedulerPrivate::debug_info()
{
}

void SchedulerPrivate::slotReparseSlaveConfiguration(const QString &proto)
{
    kDebug( 7006 ) << "reparseSlaveConfiguration( " << proto << " )";
    KProtocolManager::reparseConfiguration();
    slaveConfig->reset();
    sessionData->reset();
    NetRC::self()->reload();

    ProtocolInfoDict::ConstIterator it = proto.isEmpty() ? protInfoDict.constBegin() :
                                                           protInfoDict.find(proto);
    // not found?
    if (it == protInfoDict.constEnd()) {
        return;
    }
    ProtocolInfoDict::ConstIterator endIt = proto.isEmpty() ? protInfoDict.constEnd() :
                                                              it + 1;
    for (; it != endIt; ++it) {
        foreach(Slave *slave, (*it)->allSlaves()) {
            slave->send(CMD_REPARSECONFIGURATION);
            slave->resetHost();
        }
    }
}

void SchedulerPrivate::doJob(SimpleJob *job) {
    JobData jobData;
    jobData.protocol = KProtocolManager::slaveProtocol(job->url(), jobData.proxy);
//    kDebug(7006) << "protocol=" << jobData->protocol;
    if (jobCommand(job) == CMD_GET)
    {
       jobData.checkOnHold = checkOnHold;
       checkOnHold = false;
    }
    extraJobData.insert(job, jobData);
    newJobs.append(job);
    slaveTimer.start(0);
#ifndef NDEBUG
    if (newJobs.count() > 150)
        kDebug() << "WARNING - KIO::Scheduler got more than 150 jobs! This shows a misuse in your app (yes, a job is a QObject).";
#endif
}

void SchedulerPrivate::scheduleJob(SimpleJob *job) {
    newJobs.removeOne(job);
    const JobData& jobData = extraJobData.value(job);

    QString protocol = jobData.protocol;
//    kDebug(7006) << "protocol=" << protocol;
    ProtocolInfo *protInfo = protInfoDict.get(protocol);
    protInfo->joblist.append(job);

    slaveTimer.start(0);
}

void SchedulerPrivate::cancelJob(SimpleJob *job) {
    //    kDebug(7006) << "Scheduler: canceling job " << job;
    Slave *slave = jobSlave(job);
    if ( !slave  )
    {
        // was not yet running (don't call this on a finished job!)
        JobData jobData = extraJobData.value(job);
        newJobs.removeAll(job);
        ProtocolInfo *protInfo = protInfoDict.get(jobData.protocol);
        protInfo->joblist.removeAll(job);

        // Search all slaves to see if job is in the queue of a coSlave
        foreach(Slave* coSlave, protInfo->allSlaves())
        {
           JobList *list = protInfo->coSlaves.value(coSlave);
           if (list && list->removeAll(job)) {
               // Job was found and removed.
               // Fall through to kill the slave as well!
               slave = coSlave;
               break;
           }
        }
        if (!slave)
        {
           extraJobData.remove(job);
           return; // Job was not yet running and not in a coSlave queue.
        }
    }
    //kDebug(7006) << "Scheduler: killing slave " << slave->slave_pid();
    slave->kill();
    jobFinished( job, slave );
    slotSlaveDied(slave);
}

void SchedulerPrivate::startStep()
{
    while (newJobs.count()) {
       (void) startJobDirect();
    }

    QHashIterator<QString, ProtocolInfo*> it(protInfoDict);
    while(it.hasNext()) {
       it.next();
       if (startJobScheduled(it.value())) return;
    }
}

void SchedulerPrivate::setupSlave(KIO::Slave *slave, const KUrl &url, const QString &protocol, const QString &proxy , bool newSlave, const KIO::MetaData *config)
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
        slaveConfig = SlaveConfig::self();

        MetaData configData = slaveConfig->configData(protocol, host);
        sessionData->configDataFor( configData, protocol, host );

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
                    QMap<QString, QStringList>::ConstIterator it = l.macdef.constBegin();
                    for ( ; it != l.macdef.constEnd(); ++it )
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

bool SchedulerPrivate::startJobScheduled(ProtocolInfo *protInfo)
{
    if (protInfo->joblist.isEmpty())
       return false;

//       kDebug(7006) << "Scheduling job";
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
             slaveTimer.start(0);
       }
    }

    if (!slave)
    {
//          kDebug(7006) << "No slaves available";
//          kDebug(7006) << " -- active: " << protInfo->activeSlaves.count();
       return false;
    }

    protInfo->activeSlaves.append(slave);
    protInfo->idleSlaves.removeAll(slave);
    protInfo->joblist.removeOne(job);
//        kDebug(7006) << "scheduler: job started " << job;


    SchedulerPrivate::JobData jobData = extraJobData.value(job);
    setupSlave(slave, job->url(), jobData.protocol, jobData.proxy, newSlave);
    startJob(job, slave);

    slaveTimer.start(0);
    return true;
}

bool SchedulerPrivate::startJobDirect()
{
    debug_info();
    SimpleJob *job = newJobs.takeFirst();
    SchedulerPrivate::JobData jobData = extraJobData.value(job);

    QString protocol = jobData.protocol;
    ProtocolInfo *protInfo = protInfoDict.get(protocol);

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

    protInfo->activeSlaves.append(slave);
    protInfo->idleSlaves.removeAll(slave);
//       kDebug(7006) << "scheduler: job started " << job;

    setupSlave(slave, job->url(), protocol, jobData.proxy, newSlave);
    startJob(job, slave);
    return true;
}

static Slave *searchIdleList(SlaveList &idleSlaves, const KUrl &url, const QString &protocol, bool &exact)
{
    QString host = url.host();
    int port = url.port();
    if ( port == -1 ) // no port is -1 in QUrl, but in kde3 we used 0 and the kioslaves assume that.
        port = 0;
    QString user = url.user();
    exact = true;

    foreach( Slave *slave, idleSlaves )
    {
       if ((protocol == slave->slaveProtocol()) &&
           (host == slave->host()) &&
           (port == slave->port()) &&
           (user == slave->user()))
           return slave;
    }

    exact = false;

    // pick the first slave from the protocol's idle list: at least the protocol matches.
    if (!idleSlaves.isEmpty()) {
        Q_ASSERT(idleSlaves.first()->protocol() == protocol);
        return idleSlaves.first();
    }
    return 0;
}

Slave *SchedulerPrivate::findIdleSlave(ProtocolInfo *protInfo, SimpleJob *job, bool &exact)
{
    Slave *slave = 0;
    JobData jobData = extraJobData.value(job);

    if (jobData.checkOnHold)
    {
       slave = Slave::holdSlave(jobData.protocol, job->url());
       if (slave)
          return slave;
    }
    if (slaveOnHold)
    {
       // Make sure that the job wants to do a GET or a POST, and with no offset
       bool bCanReuse = (jobCommand(job) == CMD_GET);
       KIO::TransferJob * tJob = qobject_cast<KIO::TransferJob *>(job);
       if ( tJob )
       {
          bCanReuse = (jobCommand(job) == CMD_GET || jobCommand(job) == CMD_SPECIAL);
          if ( bCanReuse )
          {
            KIO::MetaData outgoing = tJob->outgoingMetaData();
            QString resume = (!outgoing.contains("resume")) ? QString() : outgoing["resume"];
            kDebug(7006) << "Resume metadata is" << resume;
            bCanReuse = (resume.isEmpty() || resume == "0");
          }
       }
//       kDebug(7006) << "bCanReuse = " << bCanReuse;
       if (bCanReuse)
       {
          if (job->url() == urlOnHold)
          {
             kDebug(7006) << "HOLD: Reusing held slave for" << urlOnHold;
             slave = slaveOnHold;
          }
          else
          {
             kDebug(7006) << "HOLD: Discarding held slave (" << urlOnHold << ")";
             slaveOnHold->kill();
          }
          slaveOnHold = 0;
          urlOnHold = KUrl();
       }
       if (slave)
          return slave;
    }

    return searchIdleList(protInfo->idleSlaves, job->url(), jobData.protocol, exact);
}

Slave *SchedulerPrivate::createSlave(ProtocolInfo *protInfo, SimpleJob *job, const KUrl &url)
{
   int error;
   QString errortext;
   Slave *slave = Slave::createSlave(protInfo->protocol, url, error, errortext);
   if (slave)
   {
      protInfo->idleSlaves.append(slave);
      q->connect(slave, SIGNAL(slaveDied(KIO::Slave *)),
                 SLOT(slotSlaveDied(KIO::Slave *)));
      q->connect(slave, SIGNAL(slaveStatus(pid_t,const QByteArray&,const QString &, bool)),
                 SLOT(slotSlaveStatus(pid_t,const QByteArray&, const QString &, bool)));
   }
   else
   {
      kError() << "couldn't create slave:" << errortext;
      if (job)
      {
         protInfo->joblist.removeAll(job);
         extraJobData.remove(job);
         job->slotError( error, errortext );
      }
   }
   return slave;
}

void SchedulerPrivate::slotSlaveStatus(pid_t, const QByteArray&, const QString &, bool)
{
}

void SchedulerPrivate::jobFinished(SimpleJob *job, Slave *slave)
{
    JobData jobData = extraJobData.take(job);

    ProtocolInfo *protInfo = protInfoDict.get(jobData.protocol);
    slave->disconnect(job);
    protInfo->activeSlaves.removeAll(slave);
    if (slave->isAlive())
    {
       JobList *list = protInfo->coSlaves.value(slave);
       if (list)
       {
          assert(slave->isConnected());
          assert(!protInfo->coIdleSlaves.contains(slave));
          protInfo->coIdleSlaves.append(slave);
          if (!list->isEmpty())
             coSlaveTimer.start(0);
          return;
       }
       else
       {
          assert(!slave->isConnected());
          protInfo->idleSlaves.append(slave);
          slave->setIdle();
          scheduleCleanup();
//          slave->send( CMD_SLAVE_STATUS );
       }
    }
    if (protInfo->joblist.count())
    {
       slaveTimer.start(0);
    }
}

void SchedulerPrivate::slotSlaveDied(KIO::Slave *slave)
{
    assert(!slave->isAlive());
    ProtocolInfo *protInfo = protInfoDict.get(slave->slaveProtocol());
    protInfo->activeSlaves.removeAll(slave);
    if (slave == slaveOnHold)
    {
       slaveOnHold = 0;
       urlOnHold = KUrl();
    }
    protInfo->idleSlaves.removeAll(slave);

    disconnectSlave(slave);

    slave->deref(); // Delete slave
}

void SchedulerPrivate::slotCleanIdleSlaves()
{
    foreach (ProtocolInfo *protInfo, protInfoDict) {
        SlaveList::iterator it = protInfo->idleSlaves.begin();
        for( ; it != protInfo->idleSlaves.end(); )
        {
            Slave *slave = *it;
            if (slave->idleTime() >= MAX_SLAVE_IDLE)
            {
                // kDebug(7006) << "Removing idle slave: " << slave->slaveProtocol() << " " << slave->host();
                Slave *removeSlave = slave;
                it = protInfo->idleSlaves.erase( it );
                removeSlave->connection()->close();
                removeSlave->deref();
            }
            else
            {
                ++it;
            }
        }
    }
    scheduleCleanup();
}

void SchedulerPrivate::scheduleCleanup()
{
    foreach (ProtocolInfo *protInfo, protInfoDict) {
        if (protInfo->idleSlaves.count() && !cleanupTimer.isActive()) {
            cleanupTimer.start(MAX_SLAVE_IDLE * 1000);
            break;
        }
    }
}

void SchedulerPrivate::putSlaveOnHold(KIO::SimpleJob *job, const KUrl &url)
{
    Slave *slave = jobSlave(job);
    slave->disconnect(job);

    if (slaveOnHold)
    {
        slaveOnHold->kill();
    }
    slaveOnHold = slave;
    urlOnHold = url;
    slaveOnHold->suspend();
}

void SchedulerPrivate::publishSlaveOnHold()
{
    if (!slaveOnHold)
       return;

    slaveOnHold->hold(urlOnHold);
}

void SchedulerPrivate::removeSlaveOnHold()
{
    if (slaveOnHold)
    {
        slaveOnHold->kill();
    }
    slaveOnHold = 0;
    urlOnHold = KUrl();
}

Slave *
SchedulerPrivate::getConnectedSlave(const KUrl &url, const KIO::MetaData &config )
{
    QString proxy;
    QString protocol = KProtocolManager::slaveProtocol(url, proxy);
    ProtocolInfo *protInfo = protInfoDict.get(protocol);
    bool dummy;
    Slave *slave = searchIdleList(protInfo->idleSlaves, url, protocol, dummy);
    if (!slave)  {
       slave = createSlave(protInfo, 0, url);
    }
    if (!slave) {
       return 0; // Error
    }
    protInfo->idleSlaves.removeAll(slave);

    setupSlave(slave, url, protocol, proxy, true, &config);

    slave->send( CMD_CONNECT );
    q->connect(slave, SIGNAL(connected()),
               SLOT(slotSlaveConnected()));
    q->connect(slave, SIGNAL(error(int, const QString &)),
               SLOT(slotSlaveError(int, const QString &)));

    protInfo->coSlaves.insert(slave, new JobList);
//    kDebug(7006) << "_getConnectedSlave( " << slave << ")";
    return slave;
}

void
SchedulerPrivate::slotScheduleCoSlave()
{
    slaveConfig = SlaveConfig::self();
    foreach (ProtocolInfo *protInfo, protInfoDict) {
        SlaveList::iterator it = protInfo->coIdleSlaves.begin();
        for( ; it != protInfo->coIdleSlaves.end(); )
        {
            Slave* slave = *it;
            JobList *list = protInfo->coSlaves.value(slave);
            assert(list);
            if (list && !list->isEmpty())
            {
                SimpleJob *job = list->takeFirst();
                it = protInfo->coIdleSlaves.erase( it );
                //           kDebug(7006) << "scheduler: job started " << job;

                KUrl url =job->url();
                QString host = url.host();
                int port = url.port();
                if ( port == -1 ) // no port is -1 in QUrl, but in kde3 we used 0 and the kioslaves assume that.
                    port = 0;

                if (slave->host() == "<reset>")
                {
                    QString user = url.user();
                    QString passwd = url.pass();

                    MetaData configData = slaveConfig->configData(url.protocol(), url.host());
                    slave->setConfig(configData);
                    slave->setProtocol(url.protocol());
                    slave->setHost(host, port, user, passwd);
                }

                assert(slave->protocol() == url.protocol());
                assert(slave->host() == host);
                assert(slave->port() == port);
                startJob(job, slave);
            } else {
                ++it;
            }
        }
    }
}

void
SchedulerPrivate::slotSlaveConnected()
{
    Slave *slave = static_cast<Slave *>(q->sender());
//    kDebug(7006) << "slotSlaveConnected( " << slave << ")";
    slave->setConnected(true);
    q->disconnect(slave, SIGNAL(connected()),
                  q, SLOT(slotSlaveConnected()));
    emit q->slaveConnected(slave);
    ProtocolInfo *protInfo = protInfoDict.get(slave->protocol());
    assert(!protInfo->coIdleSlaves.contains(slave));
    protInfo->coIdleSlaves.append(slave);
    coSlaveTimer.start(0);
}

void
SchedulerPrivate::slotSlaveError(int errorNr, const QString &errorMsg)
{
    Slave *slave = static_cast<Slave *>(q->sender());
    ProtocolInfo *protInfo = protInfoDict.get(slave->protocol());
    if (!slave->isConnected() || protInfo->coIdleSlaves.contains(slave))
    {
       // Only forward to application if slave is idle or still connecting.
       emit q->slaveError(slave, errorNr, errorMsg);
    }
}

bool
SchedulerPrivate::assignJobToSlave(KIO::Slave *slave, SimpleJob *job)
{
//    kDebug(7006) << "_assignJobToSlave( " << job << ", " << slave << ")";
    QString dummy;
    if ((slave->slaveProtocol() != KProtocolManager::slaveProtocol( job->url(), dummy ))
        ||
        (!newJobs.removeAll(job)))
    {
        kDebug(7006) << "_assignJobToSlave(): ERROR, nonmatching or unknown job.";
        job->kill();
        return false;
    }

    ProtocolInfo *protInfo = protInfoDict.get(slave->protocol());
    JobList *list = protInfo->coSlaves.value(slave);
    assert(list);
    if (!list)
    {
        kDebug(7006) << "_assignJobToSlave(): ERROR, unknown slave.";
        job->kill();
        return false;
    }

    assert(!list->contains(job));
    list->append(job);
    coSlaveTimer.start(0); // Start job on timer event

    return true;
}

bool
SchedulerPrivate::disconnectSlave(KIO::Slave *slave)
{
//    kDebug(7006) << "_disconnectSlave( " << slave << ")";
    ProtocolInfo *protInfo = protInfoDict.get(slave->protocol());   //### or slaveProtocol?
    CoSlaveMap::iterator coSlaveIt = protInfo->coSlaves.find( slave );
    if ( coSlaveIt != protInfo->coSlaves.end() ) {
        JobList *list = *coSlaveIt;
        protInfo->coSlaves.erase( coSlaveIt );
        if (list)
        {
            // Kill jobs still in queue.
            while(!list->isEmpty())
            {
               Job *job = list->takeFirst();
               job->kill();
            }
            delete list;
        }
    }
    protInfo->coIdleSlaves.removeAll(slave);
    assert(!protInfo->coIdleSlaves.contains(slave));
    QObject::disconnect(slave, SIGNAL(connected()),
                        q, SLOT(slotSlaveConnected()));
    QObject::disconnect(slave, SIGNAL(error(int, const QString &)),
                        q, SLOT(slotSlaveError(int, const QString &)));
    if (slave->isAlive())
    {
       protInfo->idleSlaves.append(slave);
       slave->send( CMD_DISCONNECT );
       slave->setIdle();
       slave->setConnected(false);
       scheduleCleanup();
    }
    return true;
}

void
SchedulerPrivate::checkSlaveOnHold(bool b)
{
    checkOnHold = b;
}

void
SchedulerPrivate::registerWindow(QWidget *wid)
{
   if (!wid)
      return;

   QWidget* window = wid->window();

   QObject *obj = static_cast<QObject *>(window);
   if (!m_windowList.contains(obj))
   {
      // We must store the window Id because by the time
      // the destroyed signal is emitted we can no longer
      // access QWidget::winId() (already destructed)
      WId windowId = window->winId();
      m_windowList.insert(obj, windowId);
      q->connect(window, SIGNAL(destroyed(QObject *)),
                 SLOT(slotUnregisterWindow(QObject*)));
      QDBusInterface("org.kde.kded", "/kded", "org.kde.kded").
          call(QDBus::NoBlock, "registerWindowId", qlonglong(windowId));
   }
}

void
SchedulerPrivate::slotUnregisterWindow(QObject *obj)
{
   if (!obj)
      return;

   QMap<QObject *, WId>::Iterator it = m_windowList.find(obj);
   if (it == m_windowList.end())
      return;
   WId windowId = it.value();
   q->disconnect(it.key(), SIGNAL(destroyed(QObject *)),
                 q, SLOT(slotUnregisterWindow(QObject*)));
   m_windowList.erase( it );
   QDBusInterface("org.kde.kded", "/kded", "org.kde.kded").
       call(QDBus::NoBlock, "unregisterWindowId", qlonglong(windowId));
}

#include "scheduler.moc"
