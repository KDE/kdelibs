/**
 * backgroundthread.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef KSPELL_BACKGROUNDTHREAD_H
#define KSPELL_BACKGROUNDTHREAD_H

#include "broker.h"

#include <qthread.h>
#include <qmutex.h>

class QObject;

namespace KSpell2
{
    class Filter;
    class Broker;
    class Dictionary;
    class BackgroundThread : public QThread
    {
    public:
        BackgroundThread();
        void setReceiver( QObject *parent );
        QObject *receiver() const { return m_recv; }

        void setBroker( const Broker::Ptr& broker );
        Broker *broker() const { return m_broker.get(); }

        void setText( const QString& );
        QString text() const;

        void changeLanguage( const QString& );
        QString language() const;

        void setFilter( Filter *filter );
        Filter *filter() const { return m_filter; }

        QStringList suggest( const QString& ) const;

        virtual void run();
        void stop();
    private:
        QObject    *m_recv;
        QMutex      m_mutex;
        Filter     *m_filter;
        Broker::Ptr m_broker;
        Dictionary *m_dict;
        bool        m_done;
    };
}

#endif
