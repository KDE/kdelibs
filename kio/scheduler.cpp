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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "scheduler.h"
#include "slave.h"
#include <qlist.h>
#include <qdict.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kprotocolmanager.h>
#include <assert.h>
#include <kstaticdeleter.h>

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
      DCOPObject( "KIO::Scheduler" ),
    mytimer(this, "Scheduler::mytimer"),
    cleanupTimer(this, "Scheduler::cleanupTimer")
{
    slaveOnHold = 0;
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

Scheduler::~Scheduler()
{
//fprintf(stderr, "Destructing KIO::Scheduler...\n");
    protInfoDict->setAutoDelete(true);
    delete protInfoDict; protInfoDict = 0;
    delete idleSlaves; idleSlaves = 0;
    slaveList->setAutoDelete(true);
    delete slaveList; slaveList = 0;
    instance = 0;
}

void
Scheduler::debug_info()
{
#if 0
    kdDebug(7006) << "Scheduler Info" << endl;
    kdDebug(7006) << "==========" << endl;
    kdDebug(7006) << "Total Slaves: " << slaveList->count() << endl;
    Slave *slave = slaveList->first();
    for(; slave; slave = slaveList->next())
    {
        ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
        kdDebug(7006) << " Slave: " << slave->protocol() << " " << slave->host() << " " << slave->port() << " pid: " << slave->slave_pid() << endl;
        kdDebug(7006) << " -- active: " << protInfo->activeSlaves << " idle: " << protInfo->idleSlaves << endl;
    }
    kdDebug(7006) << "Idle Slaves: " << idleSlaves->count() << endl;
    slave = idleSlaves->first();
    for(; slave; slave = idleSlaves->next())
    {
        kdDebug(7006) << " IdleSlave: " << slave->protocol() << " " << slave->host() << " " << slave->port() << endl;

    }
    kdDebug(7006) << "Jobs in Queue: " << joblist.count() << endl;
    SimpleJob *job = joblist.first();
    for(; job; job = joblist.next())
    {
        kdDebug(7006) << " Job: " << job << " " << job->url().url() << endl;
    }
#endif
}

bool Scheduler::process(const QCString &fun, const QByteArray &data, QCString &replyType, QByteArray &replyData )
{
  if ( fun != "reparseSlaveConfiguration(QString)" )
    return DCOPObject::process( fun, data, replyType, replyData );

  replyType = "void";

  QDataStream stream( data, IO_ReadOnly );

  QString proto;

  stream >> proto;

  kdDebug( 7006 ) << "reparseConfiguration( " << proto << " )" << endl;

  KProtocolManager::reparseConfiguration();

  Slave *slave = slaveList->first();
  for (; slave; slave = slaveList->next() )
    if ( slave->protocol() == proto || proto.isEmpty() )
      slave->connection()->send( CMD_REPARSECONFIGURATION );

  return true;
}

QCStringList Scheduler::functions()
{
    QCStringList funcs = DCOPObject::functions();
    funcs << "void reparseSlaveConfiguration(QString)";
    return funcs;
}

void Scheduler::_doJob(SimpleJob *job) {
    joblist.append(job);
    mytimer.start(0, true);
}
	
void Scheduler::_cancelJob(SimpleJob *job) {
//    kdDebug(7006) << "Scheduler: canceling job " << job << endl;
    if ( job->slave() ) // was running
    {
	Slave *slave = job->slave();
//        kdDebug(7006) << "Scheduler: killing slave " << slave->slave_pid() << endl;
        slave->kill();
        _jobFinished( job, slave );
	slotSlaveDied( slave);
    } else { // was not yet running (don't call this on a finished job!)
        joblist.remove(job);
    }
}

void Scheduler::startStep()
{
    while (joblist.count())
    {
//       kdDebug(7006) << "Scheduling job" << endl;
       debug_info();
       SimpleJob *job = joblist.at(0);
       QString protocol = job->url().protocol();
       ProtocolInfo *protInfo = protInfoDict->get(protocol);
       QString host = job->url().host();
       int port = job->url().port();
       QString user = job->url().user();
       QString passwd = job->url().pass();

       bool newSlave = false;

       // Look for matching slave
       Slave *slave = 0;
       if (!slave)
       {
//WABA:
// Make somehow sure that the job wants to do a GET.
          if (slaveOnHold)
          {
             if (job->url() == urlOnHold)
             {
//kdDebug(7006) << "HOLD: Reusing hold slave." << endl;
                slave = slaveOnHold;
                protInfo->idleSlaves++; // Will be decreased later on.
             }
             else
             {
//kdDebug(7006) << "HOLD: Discarding hold slave." << endl;
                slaveOnHold->kill();
             }
             slaveOnHold = 0;
             urlOnHold = KURL();
          }
       }

       if (!slave)
       {
          for( slave = idleSlaves->first();
               slave;
               slave = idleSlaves->next())
          {
             if ((protocol == slave->protocol()) &&
                  (host == slave->host()) &&
                  (port == slave->port()) &&
                  (user == slave->user()))
                 break;
          }
       }

       if (!slave)
       {
          if ((protInfo->activeSlaves+protInfo->idleSlaves) < protInfo->maxSlaves)
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
                 kdError() << "ERROR " << error << ": couldn't create slave : "
                           << errortext << endl;
                 joblist.remove(job);
                 job->slotError( error, errortext );
                 return;
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
//          kdDebug(7006) << "No slaves available" << endl;
//          kdDebug(7006) << " -- active: " << protInfo->activeSlaves << " idle: " << protInfo->idleSlaves << endl;
	  return;
       }

       protInfo->idleSlaves--;
       protInfo->activeSlaves++;
       idleSlaves->removeRef(slave);
       joblist.removeRef(job);
//       kdDebug(7006) << "scheduler: job started " << job << endl;
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

#if 0
void Scheduler::slotSlaveStatus(pid_t pid, const QCString &protocol, const QString &host, bool connected) {
    Slave *slave = (Slave*)sender();
    kdDebug(7006) << "Slave = " << slave << " (PID = " << pid
                  << ") protocol = " << protocol.data() << " host = "
                  << ( !host.isNull() ? host : QString::fromLatin1("[None]") ) << " "
                  << ( connected ? "Connected" : "Not connected" ) << endl;
}
#else
void Scheduler::slotSlaveStatus(pid_t, const QCString &, const QString &, bool) {}
#endif

void Scheduler::_jobFinished(SimpleJob *job, Slave *slave)
{
    ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
    slave->disconnect(job);
    protInfo->activeSlaves--;
//    kdDebug(7006) << "Scheduler: job finished job = " << job << " pid = " << slave->slave_pid() << endl;
    if (slave->isAlive())
    {
       idleSlaves->append(slave);
       protInfo->idleSlaves++;
       slave->setIdle();
       _scheduleCleanup();
       if (joblist.count())
       {
//           kdDebug(7006) << "Scheduler has now " << joblist.count() << " jobs" << endl;
           mytimer.start(0, true);
       }
       slave->connection()->send( CMD_SLAVE_STATUS );
    }
    else
    {
//       kdDebug(7006) << "Scheduler: Slave is dead pid = " << slave->slave_pid() << endl;
    }
}

void Scheduler::slotSlaveDied(KIO::Slave *slave)
{
    ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
    if (slave == slaveOnHold)
    {
       slaveOnHold = 0;
       urlOnHold = KURL();
    }
    if (idleSlaves->removeRef(slave))
    {
       protInfo->idleSlaves--;
//       kdDebug(7006) << "Scheduler: Slave died while in idleSlaves list! PID=" << slave->slave_pid() << endl;
    }
    else
    {
//       kdDebug(7006) << "Scheduler: Slave died while NOT in idleSlaves list! PID=" << slave->slave_pid() << endl;
    }

    if (!slaveList->removeRef(slave))
       kdDebug(7006) << "Scheduler: BUG!! Slave died, but is NOT in slaveList!!!\n" << endl;
//    kdDebug(7006) << " -- active: " << protInfo->activeSlaves << " idle: " << protInfo->idleSlaves << endl;
    delete slave;
}

void Scheduler::slotCleanIdleSlaves()
{
   for(Slave *slave = idleSlaves->first();slave;)
   {
      if (slave->idleTime() >= MAX_SLAVE_IDLE)
      {
//         kdDebug(7006) << "Removing idle slave: " << slave->protocol() << " " << slave->host() << endl;
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

void Scheduler::_putSlaveOnHold(KIO::SimpleJob *job, const KURL &url)
{
   Slave *slave = job->slave();
   ProtocolInfo *protInfo = protInfoDict->get(slave->protocol());
   slave->disconnect(job);
//WABA:
// Maybe it's better to count 'slaveOnHold' as an active slave.
// For now, this is needed to keep the count consistent.
   protInfo->activeSlaves--;

   if (slaveOnHold)
   {
      slaveOnHold->kill();
   }
   slaveOnHold = slave;
   urlOnHold = url;
   slaveOnHold->suspend();
}

void Scheduler::_removeSlaveOnHold()
{
   if (slaveOnHold)
   {
      slaveOnHold->kill();
   }
   slaveOnHold = 0;
   urlOnHold = KURL();
}

static KStaticDeleter<Scheduler> ksds;

Scheduler* Scheduler::self() {
    if ( !instance )
	instance = ksds.setObject(new Scheduler);

    return instance;
}

#include "scheduler.moc"
