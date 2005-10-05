/*
   This file is part of the KDE libraries
   Copyright (C) 2005 Andreas Roth <aroth@arsoft-online.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#if !defined(__QEVENTLOOPEX_H_)
#define __QEVENTLOOPEX_H_

#include <qsocketnotifier.h>
#include <qeventloop.h>
#include <qthread.h>

#include <winsock2.h>

class QEventLoopExPrivate;

class QEventLoopEx : public QEventLoop
{
	Q_OBJECT

public:
	QEventLoopEx( QObject *parent = 0, const char *name = 0 );
	virtual ~QEventLoopEx();

public:
	virtual void registerSocketNotifier( QSocketNotifier * );
	virtual void unregisterSocketNotifier( QSocketNotifier * );
	virtual bool processEvents( ProcessEventsFlags flags );

	void setSocketNotifierPending( QSocketNotifier *notifier );
	int activateSocketNotifiers();

protected:
	void run();

private:
	static DWORD WINAPI ThreadProc(void * p);

	// data for the default implementation - other implementations should not
	// use/need this data
	QEventLoopExPrivate *d;
};

#endif // __QEVENTLOOPEX_H_
