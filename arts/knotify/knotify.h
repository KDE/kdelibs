/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef KNOTIFY_H
#define KNOTIFY_H

#include <qobject.h>
#include <knotifyclient.h>
#include <dcopobject.h>
#include <soundserver.h>

class KNotifyPrivate;

class KNotify : public QObject, public DCOPObject
{
Q_OBJECT
K_DCOP

public:
	KNotify();
	~KNotify();

	static void crashHandler(int);

protected:
k_dcop:
	void notify(const QString &event, const QString &fromApp,
                         const QString &text, QString sound, QString file,
                         int present, int level);
    
	void reconfigure();
	void setVolume( int volume );

protected:
	bool notifyBySound(const QString &sound);
	bool notifyByMessagebox(const QString &text, int level);
	bool notifyByLogfile(const QString &text, const QString &file);
	bool notifyByStderr(const QString &text);
	bool notifyByPassivePopup(const QString &text, const QString &appName);
	bool notifyByExecute(const QString &command);
	
	void connectSoundServer();
	
public:
	/**
	 * checks if eventname is a global event (exists in config/eventsrc)
	 **/
	bool isGlobal(const QString &eventname);

private slots:
    void playTimeout();
    
private:
    KNotifyPrivate* d;
    void loadConfig();
};


#endif

