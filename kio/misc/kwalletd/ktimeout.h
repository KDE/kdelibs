/*
   This file is part of the KDE libraries

   Copyright (c) 2003 George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/
#ifndef _KTIMEOUT_H_
#define _KTIMEOUT_H_

#include <qintdict.h>
#include <qobject.h>
#include <qtimer.h>

// @internal
class KTimeout : public QObject {
	Q_OBJECT
	public:
		KTimeout(int size = 29);
		virtual ~KTimeout();

	signals:
		void timedOut(int id);

	public slots:
		void resetTimer(int id, int timeout);
		void addTimer(int id, int timeout);
		void removeTimer(int id);
		void clear();

	private slots:
		void timeout();

	private:
		QIntDict<QTimer> _timers;
};

#endif
