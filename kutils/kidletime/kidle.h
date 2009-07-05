/***************************************************************************
 *   Copyright (C) 2009 by Dario Freddi <drf@kde.org>                *
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

#ifndef KIDLETIME_H
#define KIDLETIME_H

#include <QObject>

class KIdleTimePrivate;

/**
 * KIdleTime is a singleton reporting information on idle time. It is useful not
 * only for finding out about the current idle time of the PC, but also for getting
 * notified upon idle time events, such as custom timeouts, or user activity.
 *
 * It also includes a number of 
 *
 * @author Dario Freddi
 */
class KIdleTime : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KIdleTime)
    Q_DISABLE_COPY(KIdleTime)

public:
    static KIdleTime *instance();
  
    virtual ~KIdleTime();
    
    int getIdleTime();
    void simulateUserActivity();
    
public slots:
    void catchIdleTimeout(int msec);
    void stopCatchingIdleTimeout();
    void catchNextResumeEvent();
    
signals:
    void resumingFromIdle();
    void timeoutReached(int msec);

private:
    KIdleTime();
  
    KIdleTimePrivate * const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _k_resumingFromIdle())
    
};

#endif /* KIDLETIME_H */
