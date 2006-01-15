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

#include "ktimeout.h"

KTimeout::KTimeout(int size)
: QObject(){
	_timers.reserve(size);
}


KTimeout::~KTimeout() {
	clear();
}


void KTimeout::clear() {
	qDeleteAll(_timers);
	_timers.clear();
}


void KTimeout::removeTimer(int id) {
	QTimer *t = _timers.value(id);
	if (t != 0L) {
		_timers.remove(id);
		delete t;
	}
}


void KTimeout::addTimer(int id, int timeout) {
	if (_timers.find(id) != 0L) {
		return;
	}

	QTimer *t = new QTimer;
	connect(t, SIGNAL(timeout()), this, SLOT(timeout()));
	t->start(timeout);
	_timers.insert(id, t);
}


void KTimeout::resetTimer(int id, int timeout) {
	QTimer *t = _timers.value(id);
	if (t) {
		t->start(timeout);
	}
}


void KTimeout::timeout() {
	const QTimer *t = static_cast<const QTimer*>(sender());
	if (t) {
		QMultiHash<int, QTimer*>::const_iterator it = _timers.constBegin();
		while (it != _timers.constEnd()) {
			if (it.value() == t) {
				emit timedOut(it.key());
				return;
			}
			++it;
		}
	}
}


#include "ktimeout.moc"

