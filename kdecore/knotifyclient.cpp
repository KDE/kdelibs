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

#include <dcopclient.h>


KNotifyClient::KNotifyClient(const QString &message, const QString &text,
                             Presentation present, const QString &soundfile,
                             DCOPClient* client)
{
	levent=new Event;
	levent->message=message;
	levent->text=text;
	levent->present=present;
	levent->soundfile=soundfile;
	levent->client=client;
}

KNotifyClient::~KNotifyClient()
{
	delete levent;
}

bool KNotifyClient::send()
{
	DCOPClient *client=levent->client;
	if (!client->isAttached())
		client->attach();
	if (!client->isAttached())
		return false;

	QByteArray data;
	QDataStream ds(data, IO_WriteOnly);
	ds << levent->message << KApplication::kApplication()->argv()[0] << levent->text << levent->soundfile << (int)levent->present;

	return client->send("knotify", "Notify", "notify(QString,QString,QString,QString,Presentation)", data);
}

bool KNotifyClient::event(const QString &message, const QString &text)
{
	KNotifyClient c(message, text);
	return c.send();
}

bool KNotifyClient::userEvent(const QString &text, Presentation present,
                              const QString &soundfile)
{
	KNotifyClient c(0, text, present, soundfile);
	return c.send();
}

#include "knotifyclient.moc"
