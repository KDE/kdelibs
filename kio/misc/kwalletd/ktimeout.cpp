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
#include <QtCore/QEvent>

KTimeout::KTimeout()
: QObject() {
}

KTimeout::~KTimeout() {
}

void KTimeout::clear() {
    _timers.clear();
}

void KTimeout::removeTimer(int id) {
    _timers.remove(id);
}

void KTimeout::addTimer(int id, int timeout) {
    if (_timers.contains(id)) {
        return;
    }
    _timers.insert(id, startTimer(timeout));
}

void KTimeout::resetTimer(int id, int timeout) {
    int timerId = _timers.value(id, 0);
    if (timerId != 0) {
            killTimer(timerId);
            _timers.insert(id, startTimer(timeout));
    }
}

void KTimeout::timerEvent(QTimerEvent* ev) {
    QHash<int, int>::const_iterator it = _timers.constBegin();
    for ( ; it != _timers.constEnd(); ++it) {
        if (it.value() == ev->timerId()) {
            emit timedOut(it.key());
            return;
        }
    }
}

#include "ktimeout.moc"

