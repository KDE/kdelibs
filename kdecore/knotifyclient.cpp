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
  QString appname = kapp->name();
  ds << message << appname << text << sound << file << present << level;

  return client->send(daemonName, "Notify", "notify(QString,QString,QString,QString,QString,int,int)", data, true);
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
	
	KConfig eventsfile(locate("data", QString(KApplication::kApplication()->name())+"/eventsrc"));
	eventsfile.setGroup(eventname);
	
	present=eventsfile.readNumEntry("presentation", -1);
	
	return present;
}

QString KNotifyClient::getFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return 0;

	KConfig eventsfile(locate("data", QString(KApplication::kApplication()->name())+"/eventsrc"));
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readEntry("sound", 0);
	case (Logfile):
		return eventsfile.readEntry("logfile", 0);
	}
		
	return 0;
}

int KNotifyClient::getDefaultPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;
		
	KConfig eventsfile(locate("data", QString(KApplication::kApplication()->name())+"/eventsrc"));
	eventsfile.setGroup(eventname);
	
	present=eventsfile.readNumEntry("default_presentation", -1);
	
	return present;
}

QString KNotifyClient::getDefaultFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return 0;

	KConfig eventsfile(locate("data", QString(KApplication::kApplication()->name())+"/eventsrc"));
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readEntry("default_sound", 0);
	case (Logfile):
		return eventsfile.readEntry("default_logfile", 0);
	}
		
	return 0;
}

bool KNotifyClient::startDaemon()
{
  if (!kapp->dcopClient()->isApplicationRegistered(daemonName))
    return KApplication::startServiceByDesktopName(daemonName);
  return true;
}


