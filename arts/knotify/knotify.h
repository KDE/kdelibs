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
 
        kaudioserver.cpp: Audio server starter for libmediatool/maudio
*/

#ifndef KNOTIFY_H
#define KNOTIFY_H

#include <qobject.h>
#include <dcopobject.h>

class KNotify : public QObject, DCOPObject
{
Q_OBJECT
K_DCOP

public:
	KNotify();
//	~KNotify();
	enum Presentation
	{
		Default=-1,
		None=0,
		Sound=1,
		Messagebox=2,
		Logwindow=4,
		Logfile=8,
		Stderr=16
	};

protected:
	void processNotification(const QString &event, const QString &fromApp,
	                         const QString &text, QString sound,
	                         Presentation present);

	bool notifyBySound(const QString &sound);
	bool notifyByMessagebox(const QString &text);
	bool notifyByLogwindow(const QString &text);
	bool notifyByLogfile(const QString &text);
	bool notifyByStderr(const QString &text);
};


#endif

