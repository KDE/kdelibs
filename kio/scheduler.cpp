/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

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
    kdDebug(7006) << "Scheduler Info" << endl;
    kdDebug(7006) << "==========" << endl;
    kdDebug(7006) << "Total Slaves: " << slaveList->count() << endl;
    Slave *slave = slaveList->first();
    for(; slave; slave = slaveList->next())
    {
        ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
        kdDebug(7006) << " Slave: " << slave->protocol() << " " << slave->host() << slave->port() << endl;
        kdDebug(7006) << " -- activeSlaves: " << protInfo->activeSlaves << endl;
    }
    kdDebug(7006) << "Idle Slaves: " << idleSlaves->count() << endl;
    slave = idleSlaves->first();
    for(; slave; slave = idleSlaves->next())
    {
        kdDebug(7006) << " IdleSlave: " << slave->protocol() << " " << slave->host() << slave->port() << endl;

    }
    kdDebug(7006) << "Jobs in Queue: " << joblist.count() << endl;
    SimpleJob *job = joblist.first();
    for(; job; job = joblist.next())
    {
        kdDebug(7006) << " Job: " << job->url().url() << endl;
    }
}

void Scheduler::_doJob(SimpleJob *job) {
    joblist.append(job);
    kdDebug(7006) << "Scheduler has now " << joblist.count() << " jobs " << job << endl;
    mytimer.start(0, true);
}
	
void Scheduler::_cancelJob(SimpleJob *job) {
    kdDebug(7006) << "Scheduler: canceling job " << job << endl;
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
       kdDebug(7006) << "Scheduling job" << endl;
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
                if (!slave) kdFatal() << "couldn't create slave " << error 
                                      << error << " "
                                      << debugString(errortext) << endl;
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
          kdDebug(7006) << "No slaves available" << endl;
          debug_info();
	  return;
       }

       protInfo->idleSlaves--;
       protInfo->activeSlaves++;
       idleSlaves->removeRef(slave);
       joblist.removeRef(job);
       kdDebug(7006) << "scheduler: job started " << job << endl;
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
    kdDebug(7006) << "slave status" << endl;
    Slave *slave = (Slave*)sender();
    kdDebug(7006) << "Slave = " << slave << " (PID = " << pid
                  << ") protocol = " << protocol.data() << " host = "
                  << ( host.ascii() ? host.ascii() : "[None]" ) << " "
                  << ( connected ? "Connected" : "Not connected" ) << endl;
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
           kdDebug(7006) << "Scheduler has now " << joblist.count() << " jobs" << endl;
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
