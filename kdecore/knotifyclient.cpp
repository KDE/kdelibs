/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>

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

#include "knotifyclient.h"

#include <qdatastream.h>

#include <kstddirs.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kdebug.h>

static const char *daemonName="knotify";

static bool sendNotifyEvent(const QString &message, const QString &text,
                 int present, int level, const QString &sound,
                 const QString &file)
{
  DCOPClient *client=kapp->dcopClient();
  if (!client->isAttached())
  {
    client->attach();
    if (!client->isAttached())
      return false;
  }

  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  QString appname = KNotifyClient::Instance::current()->instanceName();
  ds << message << appname << text << sound << file << present << level;

  if ( !KNotifyClient::startDaemon() )
      return false;

  return client->send(daemonName, "Notify", "notify(QString,QString,QString,QString,QString,int,int)", data, true);
}

bool KNotifyClient::event( StandardEvent type, const QString& text )
{
    QString message;
    switch ( type ) {
    case cannotOpenFile:
	message = QString::fromLatin1("cannotopenfile");
	break;
    case warning:
	message = QString::fromLatin1("warning");
	break;
    case fatalError:
	message = QString::fromLatin1("fatalerror");
	break;
    case catastrophe:
	message = QString::fromLatin1("catastrophe");
	break;
    case notification: // fall through
    default:
	message = QString::fromLatin1("notification");
	break;
    }

    return sendNotifyEvent( message, text, Default, Default,
			    QString::null, QString::null);
}

bool KNotifyClient::event(const QString &message, const QString &text)
{
  return sendNotifyEvent(message, text, Default, Default, QString::null, QString::null);
}

bool KNotifyClient::userEvent(const QString &text, int present, int level,
                              const QString &sound, const QString &file)
{
  return sendNotifyEvent(QString::null, text, present, level, sound, file);
}

int KNotifyClient::getPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;
	
	KConfig eventsfile(locate("data", KNotifyClient::Instance::current()->instanceName() + "/eventsrc"));
	eventsfile.setGroup(eventname);
	
	present=eventsfile.readNumEntry("presentation", -1);
	
	return present;
}

QString KNotifyClient::getFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile(locate("data", KNotifyClient::Instance::current()->instanceName() + "/eventsrc"));
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readEntry("soundfile");
	case (Logfile):
		return eventsfile.readEntry("logfile");
	}
		
	return QString::null;
}

int KNotifyClient::getDefaultPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;
		
	KConfig eventsfile(locate("data", KNotifyClient::Instance::current()->instanceName() + "/eventsrc"));
	eventsfile.setGroup(eventname);
	
	present=eventsfile.readNumEntry("default_presentation", -1);
	
	return present;
}

QString KNotifyClient::getDefaultFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile(locate("data", KNotifyClient::Instance::current()->instanceName() + "/eventsrc"));
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readEntry("default_sound");
	case (Logfile):
		return eventsfile.readEntry("default_logfile");
	}
		
	return QString::null;
}

bool KNotifyClient::startDaemon()
{
  if (!kapp->dcopClient()->isApplicationRegistered(daemonName))
    return KApplication::startServiceByDesktopName(daemonName);
  return true;
}


void KNotifyClient::beep(const QString& reason)
{
  DCOPClient *client=kapp->dcopClient();
  if (!client->isAttached())
  {
    client->attach();
    if (!client->isAttached() || !client->isApplicationRegistered(daemonName))
      {
        QApplication::beep();
        return;
      }
  }

  KNotifyClient::event(KNotifyClient::notification, reason);
}

QStack<KNotifyClient::Instance> KNotifyClient::Instance::s_instances;

struct KNotifyClient::InstancePrivate
{
    KInstance *instance;
};

KNotifyClient::Instance::Instance(KInstance *instance)
{
    d = new InstancePrivate;
    d->instance = instance;
    s_instances.push(this);
}

KNotifyClient::Instance::~Instance()
{
    if (s_instances.top() == this)
        s_instances.pop();
    else if (s_instances.count())
    {
        kdWarning(160) << "Tried to remove an Instance that is not the current," << endl;
        kdWarning(160) << "Resetting to the main KApplication." << endl;
        kdWarning(160) << "Offending instance is: " << d->instance->instanceName() << ", fix it!" << endl;
        s_instances.clear();
    }
    else {
        kdWarning(160) << "Tried to remove an Instance, but the stack was empty." << endl;
        kdWarning(160) << "Offending instance is: " << d->instance->instanceName() << ", fix it!" << endl;
    }
    delete d;
}

KInstance *KNotifyClient::Instance::current()
{
    return s_instances.top() ? s_instances.top()->d->instance : kapp;
}

