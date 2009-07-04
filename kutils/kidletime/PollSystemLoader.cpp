/***************************************************************************
 *   Copyright (C) 2008 by Dario Freddi <drf@kdemod.ath.cx>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 **************************************************************************/

#include "KIdleTime.h"

#include "WidgetBasedPoller.h"
#include "XSyncBasedPoller.h"
#include "TimerBasedPoller.h"

class Private {
  public:
    Private() {};
    
    void loadSystem();
    void unloadCurrentSystem();
    
    QPointer<AbstractSystemPoller> poller;  
};

KIdleTime::KIdleTime()
        : d(new Private())
{
    d->loadSystem();
}

KIdleTime::~KIdleTime()
{
    d->unloadCurrentSystem();
}

void KIdleTime::Private::loadSystem()
{
    if (m_poller) {
        unloadCurrentSystem();
    }

    // Priority order
    
    if (XSyncBasedPoller::instance()->isAvailable()) {
        XSyncBasedPoller::instance()->setUpPoller();
	poller = XSyncBasedPoller::instance();
    } else {
        poller = new WidgetBasedPoller();
	
	if (!poller->isAvailable()) {
	    poller->deleteLater();
	    poller = new TimerBasedPoller();
	}
	
	poller->setUpPoller();
    }
}

bool KIdleTime::Private::unloadCurrentSystem()
{
    if (poller) {
        poller->unloadPoller();

        if (poller->getPollingType() != AbstractSystemPoller::XSyncBased) {
            poller->deleteLater();
        }
    }

    return true;
}

#include "KIdleTime.moc"
