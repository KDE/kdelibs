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

class KNotifyClient::KNotifyClientPrivate
{
public:
	QString message;
	QString text;
	int present;
	int level;
	QString sound;
	QString file;
	DCOPClient *client;
};

KNotifyClient::KNotifyClient(QObject *parent, const QString &message, const QString &text,
                             int present, int level, const QString &sound, const QString &file,
                             DCOPClient* client) : QObject(parent)
{
	if (!client) client=KApplication::kApplication()->dcopClient();
	
	d=new KNotifyClientPrivate;
	d->message=message;
	d->text=text;
	d->present=present;
	d->level=level;
	d->sound=sound;
	d->file=file;
	d->client=client;
	startDaemon();
}

KNotifyClient::~KNotifyClient()
{
	delete d;
}

bool KNotifyClient::send()
{
	DCOPClient *client=d->client;
	if (!client->isAttached())
		client->attach();
	if (!client->isAttached())
		return false;

	QByteArray data;
	QDataStream ds(data, IO_WriteOnly);
	QString appname = kapp->name();
	ds << d->message << appname << d->text << d->sound << d->file << d->present << d->level;

	return client->send("knotify", "Notify", "notify(QString,QString,QString,QString,QString,int,int)", data, true);
}

bool KNotifyClient::event(const QString &message, const QString &text)
{
	KNotifyClient c(0,message, text);
	return c.send();
}

bool KNotifyClient::userEvent(const QString &text, int present, int level,
                              const QString &sound, const QString &file)
{
	KNotifyClient c(0,0, text, present, level, sound, file);
	return c.send();
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
	static bool triedItOnceAlready=false;
	if (!triedItOnceAlready)
		KApplication::startServiceByDesktopName("knotify");
	triedItOnceAlready=true;
	return true;
}

#include "knotifyclient.moc"

