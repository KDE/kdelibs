/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kmtimer.h"
#include "kmfactory.h"

#include <kconfig.h>

KMTimer* KMTimer::m_self = 0;

KMTimer* KMTimer::self()
{
	if (!m_self)
	{
		m_self = new KMTimer(KMFactory::self(), "InternalTimer");
		Q_CHECK_PTR(m_self);
	}
	return m_self;
}

KMTimer::KMTimer(QObject *parent, const char *name)
: QTimer(parent, name), m_count(0)
{
	connect(this, SIGNAL(timeout()), SLOT(slotTimeout()));
}

KMTimer::~KMTimer()
{
	m_self = 0;
}

void KMTimer::hold()
{
	if ((m_count++) == 0)
		stop();
}

void KMTimer::release()
{
	releaseTimer(false);
}

void KMTimer::release(bool do_emit)
{
	releaseTimer(do_emit);
}

void KMTimer::releaseTimer(bool do_emit)
{
	m_count = QMAX(0, m_count-1);
	if (m_count == 0)
	{
		if (do_emit)
			emit timeout();
		startTimer();
	}
}

void KMTimer::delay(int t)
{
	startTimer(t);
}

void KMTimer::slotTimeout()
{
	startTimer();
}

void KMTimer::startTimer(int t)
{
	if (t == -1)
	{
		KConfig	*conf = KMFactory::self()->printConfig();
		conf->setGroup("General");
		t = conf->readEntry("TimerDelay", 5) * 1000;
	}
	start(t, true);
}

#include "kmtimer.moc"
