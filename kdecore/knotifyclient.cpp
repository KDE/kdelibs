/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>
                 2000 Malte Starostik <starosti@zedat.fu-berlin.de>
		 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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
#include <qptrstack.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

static const char * const daemonName="knotify";

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
  QString appname = KNotifyClient::instance()->instanceName();
  ds << message << appname << text << sound << file << present << level;

  if ( !KNotifyClient::startDaemon() )
      return false;

  return client->send(daemonName, "Notify", "notify(QString,QString,QString,QString,QString,int,int)", data);
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

	KConfig eventsfile( KNotifyClient::instance()->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("presentation", -1);

	return present;
}

QString KNotifyClient::getFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile( KNotifyClient::instance()->instanceName()+".eventsrc", true, false);
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

	KConfig eventsfile( KNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("default_presentation", -1);

	return present;
}

QString KNotifyClient::getDefaultFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile( KNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
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
    return KApplication::startServiceByDesktopName(daemonName) == 0;
  return true;
}


void KNotifyClient::beep(const QString& reason)
{
  if ( KNotifyClient::Instance::currentInstance()->useSystemBell() ) {
    QApplication::beep();
    return;
  }

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


KInstance * KNotifyClient::instance() {
    return KNotifyClient::Instance::current();
}


class KNotifyClient::InstanceStack
{
public:
	InstanceStack() { m_defaultInstance = 0; }
	virtual ~InstanceStack() { delete m_defaultInstance; }
	void push(Instance *instance) { m_instances.push(instance); }

	void pop(Instance *instance)
	{
		if (m_instances.top() == instance)
			m_instances.pop();
		else if (!m_instances.isEmpty())
		{
			kdWarning(160) << "Tried to remove an Instance that is not the current," << endl;
			kdWarning(160) << "Resetting to the main KApplication." << endl;
			m_instances.clear();
		}
		else
			kdWarning(160) << "Tried to remove an Instance, but the stack was empty." << endl;
    }

	Instance *currentInstance()
	{
		if (m_instances.isEmpty())
		{
			m_defaultInstance = new Instance(kapp);
		}
		return m_instances.top();
	}

private:
	QPtrStack<Instance> m_instances;
	Instance *m_defaultInstance;
};

KNotifyClient::InstanceStack * KNotifyClient::Instance::s_instances = 0L;
static KStaticDeleter<KNotifyClient::InstanceStack > instancesDeleter;

struct KNotifyClient::InstancePrivate
{
    KInstance *instance;
    bool useSystemBell;
};

KNotifyClient::Instance::Instance(KInstance *instance)
{
    d = new InstancePrivate;
    d->instance = instance;
    instances()->push(this);

    KConfig *config = instance->config();
    KConfigGroupSaver cs( config, "General" );
    d->useSystemBell = config->readBoolEntry( "UseSystemBell", false );
}

KNotifyClient::Instance::~Instance()
{
	if (s_instances)
		s_instances->pop(this);
	delete d;
}

KNotifyClient::InstanceStack *KNotifyClient::Instance::instances()
{
	if (!s_instances)
		instancesDeleter.setObject(s_instances, new InstanceStack);
	return s_instances;
}

bool KNotifyClient::Instance::useSystemBell() const
{
    return d->useSystemBell;
}


// static methods

// We always return a valid KNotifyClient::Instance here. If no special one
// is available, we have a default-instance with kapp as KInstance.
// We make sure to always have that default-instance in the stack, because
// the stack might have gotten cleared in the destructor.
// We can't use QStack::setAutoDelete( true ), because no instance besides
// our default instance is owned by us.
KNotifyClient::Instance * KNotifyClient::Instance::currentInstance()
{
	return instances()->currentInstance();
}

KInstance *KNotifyClient::Instance::current()
{
    return currentInstance()->d->instance;
}
