#include "scheduler.h"
#include "slave.h"
#include <qlist.h>
#include <qdict.h>
#include <kdebug.h>
#include <assert.h>

//
// Slaves may be idle for MAX_SLAVE_IDLE time before they are being returned
// to the system wide slave pool.
#define MAX_SLAVE_IDLE 30

using namespace KIO;

Scheduler *Scheduler::instance = 0;

class KIO::Scheduler::ProtocolInfo
{
  public: 
     ProtocolInfo() : activeSlaves(0), idleSlaves(0), maxSlaves(5) { }
    
     int activeSlaves;
     int idleSlaves;
     int maxSlaves;
};

class KIO::Scheduler::ProtocolInfoDict : public QDict<KIO::Scheduler::ProtocolInfo>
{
  public:
    ProtocolInfoDict() { }

    KIO::Scheduler::ProtocolInfo *get( const QString &key);
};

KIO::Scheduler::ProtocolInfo *
KIO::Scheduler::ProtocolInfoDict::get(const QString &key)
{
  ProtocolInfo *info = find(key);
  if (!info)
  {
     info = new ProtocolInfo;
     insert(key, info);     
  }
  return info;
}

class KIO::SlaveList: public QList<Slave>
{
   public:
      SlaveList() { }
};

Scheduler::Scheduler()
    : QObject(0, "scheduler"), 
    mytimer(this, "Scheduler::mytimer"),
    cleanupTimer(this, "Scheduler::cleanupTimer")
{
    protInfoDict = new ProtocolInfoDict;
    slaveList = new SlaveList;
    idleSlaves = new SlaveList;
    connect(&mytimer, SIGNAL(timeout()),
	    SLOT(startStep()));
    connect(&cleanupTimer, SIGNAL(timeout()),
	    SLOT(slotCleanIdleSlaves()));
    joblist.setAutoDelete(false);
    busy = false;
}

void
Scheduler::debug_info()
{
    kDebugInfo(7006, "Scheduler Info");
    kDebugInfo(7006, "==========");
    kDebugInfo(7006, "Total Slaves: %d", slaveList->count());
    Slave *slave = slaveList->first();
    for(; slave; slave = slaveList->next())
    {
        ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
        kdDebug(7006) << " Slave: " << slave->protocol() << " " << slave->host() << slave->port() << endl;
        kdDebug(7006) << " -- activeSlaves: " << protInfo->activeSlaves << endl;
    }
    kDebugInfo(7006, "Idle Slaves: %d", idleSlaves->count());
    slave = idleSlaves->first();
    for(; slave; slave = idleSlaves->next())
    {
        kdDebug(7006) << " IdleSlave: " << slave->protocol() << " " << slave->host() << slave->port() << endl;

    }
    kDebugInfo(7006, "Jobs in Queue: %d", joblist.count());
    SimpleJob *job = joblist.first();
    for(; job; job = joblist.next())
    {
        kdDebug(7006) << " Job: " << job->url().url() << endl;
    }
}

void Scheduler::_doJob(SimpleJob *job) {
    joblist.append(job);
    kDebugInfo(7006, "Scheduler has now %d jobs %p", joblist.count(), job);
    mytimer.start(0, true);
}
	
void Scheduler::_cancelJob(SimpleJob *job) {
    kDebugInfo(7006, "Scheduler: canceling job %p", job);
    if ( job->slave() ) // was running
    {
        ProtocolInfo *protInfo = protInfoDict->get(job->slave()->protocol());
        protInfo->activeSlaves--;
        job->slave()->kill();
        _jobFinished( job, job->slave() );
    } else { // was not yet running (don't call this on a finished job!)
        joblist.remove(job);
    }
}

void Scheduler::startStep()
{
    while (joblist.count())
    {
       kDebugInfo(7006, "Scheduling job");
       SimpleJob *job = joblist.at(0);
       QString protocol = job->url().protocol();
       ProtocolInfo *protInfo = protInfoDict->get(protocol);
       QString host = job->url().host();
       int port = job->url().port();
       QString user = job->url().user();
       QString passwd = job->url().pass();

       bool newSlave = false;

       // Look for matching slave
       Slave *slave = idleSlaves->first();
       for(; slave; slave = idleSlaves->next())
       {
           if ((protocol == slave->protocol()) &&
               (host == slave->host()) &&
               (port == slave->port()) &&
               (user == slave->user()))
              break;
       }

       if (!slave)
       {
          if (protInfo->activeSlaves < protInfo->maxSlaves)
          {
             int error;
             QString errortext;
             slave = Slave::createSlave(job->url(), error, errortext);
             if (slave)
             {
                protInfo->idleSlaves++;
                newSlave = true;
                slaveList->append(slave);
                idleSlaves->append(slave);
                connect(slave, SIGNAL(slaveDied(KIO::Slave *)),
			SLOT(slotSlaveDied(KIO::Slave *)));
                connect(slave, SIGNAL(slaveStatus(pid_t,const QCString &,const QString &, bool)),
                        SLOT(slotSlaveStatus(pid_t,const QCString &, const QString &, bool)));
             }
             else
             {
                kDebugFatal(!slave, "couldn't create slave %d %s",
				    error, debugString(errortext));
             }
          }
       }

       if (!slave)
       {
          // Look for slightly matching slave
          slave = idleSlaves->first();
          for(; slave; slave = idleSlaves->next())
          {
             if (protocol == slave->protocol())
                break;
          }
       }

       if (!slave)
       {
          kDebugInfo(7006, "No slaves available");
          debug_info();
	  return;
       }

       protInfo->idleSlaves--;
       protInfo->activeSlaves++;
       idleSlaves->removeRef(slave);
       joblist.removeRef(job);
       kDebugInfo(7006, "scheduler: job started %p", job);
       if ((newSlave) ||
           (slave->host() != host) ||
           (slave->port() != port) ||
           (slave->user() != user) ||
           (slave->passwd() != passwd))
       {
           slave->setHost(host, port, user, passwd);
       }
       job->start(slave);
    }
}

void Scheduler::slotSlaveStatus(pid_t pid, const QCString &protocol, const QString &host, bool connected) {
    kDebugInfo(7006, "slave status");
    Slave *slave = (Slave*)sender();
    kDebugInfo(7006, "Slave = %p (PID = %d) protocol = %s host = %s %s", 
	slave, pid, protocol.data(), host.ascii() ? host.ascii() : "[None]",
	connected ? "Connected" : "Not connected");
}

void Scheduler::_jobFinished(SimpleJob *job, Slave *slave)
{
    ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
    slave->disconnect(job);
    protInfo->activeSlaves--;
    if (slave->isAlive())
    {
       idleSlaves->append(slave);
       protInfo->idleSlaves++;
       slave->setIdle();
       _scheduleCleanup();
       if (joblist.count())
       {
           kDebugInfo(7006, "Scheduler has now %d jobs", joblist.count());
           mytimer.start(0, true);
       }
       slave->connection()->send( CMD_SLAVE_STATUS );
    }
    else
    {
       slaveList->removeRef(slave);
       delete slave;
    }
}

void Scheduler::slotSlaveDied(KIO::Slave *slave)
{
    idleSlaves->removeRef(slave);
    slaveList->removeRef(slave);
    delete slave;
}

void Scheduler::slotCleanIdleSlaves()
{
   kdDebug(7006) << "Clean Idle Slaves" << endl;
   for(Slave *slave = idleSlaves->first();slave;)
   {
      kdDebug(7006) << "Slave: " << slave->protocol() << " " << slave->host() 
	            << " Idle for " << slave->idleTime() << "secs" << endl;
      if (slave->idleTime() >= MAX_SLAVE_IDLE)
      {
         kdDebug(7006) << "Removing idle slave: " << slave->protocol() << " " << slave->host() << endl;
         Slave *removeSlave = slave;
         ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
         protInfo->idleSlaves--;
         slave = idleSlaves->next();
         idleSlaves->removeRef(removeSlave);
         slaveList->removeRef(removeSlave);
         delete removeSlave;
      }
      else
      {
         slave = idleSlaves->next();
      }
   }
   _scheduleCleanup();
}

void Scheduler::_scheduleCleanup()
{
   if (idleSlaves->count())
   {
      if (!cleanupTimer.isActive())
         cleanupTimer.start( MAX_SLAVE_IDLE*1000, true );
   }   
}

Scheduler* Scheduler::self() {
    if ( !instance )
	instance = new Scheduler;

    return instance;
}

#include "scheduler.moc"
