/**
 * backgroundthread.cpp
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
#include "backgroundthread.h"

#include "threadevents.h"
#include "broker.h"
#include "filter.h"
#include "dictionary.h"

#include <kdebug.h>
#include <qapplication.h>

using namespace KSpell2;

BackgroundThread::BackgroundThread()
    : QThread(), m_broker( 0 ), m_dict( 0 )
{
    m_recv   = 0;
    m_filter = Filter::defaultFilter();
    m_done   = false;
}

void BackgroundThread::setReceiver( QObject *recv )
{
    m_recv = recv;
}

void BackgroundThread::setBroker( const Broker::Ptr& broker )
{
    stop();
    m_broker = broker;
    delete m_dict;
    m_dict   = m_broker->dictionary();
    m_filter->restart();
}

QStringList BackgroundThread::suggest( const QString& word ) const
{
    return m_dict->suggest( word );
}

void BackgroundThread::run()
{
    m_mutex.lock();
    m_done = false;
    for ( Word w = m_filter->nextWord(); !m_done && !w.end;
          w = m_filter->nextWord() ) {
        if ( !m_dict->check( w.word ) && !m_done ) {
            MisspellingEvent *event = new MisspellingEvent( w.word, w.start );
            QApplication::postEvent( m_recv, event );
        }
    }
    m_mutex.unlock();
    FinishedCheckingEvent *event = new FinishedCheckingEvent();
    QApplication::postEvent( m_recv, event );
}

void BackgroundThread::setText( const QString& buff )
{
    stop();
    m_mutex.lock();
    m_filter->setBuffer( buff );
    m_mutex.unlock();
    start();
}

void BackgroundThread::setFilter( Filter *filter )
{
    stop();
    m_mutex.lock();
    Filter *oldFilter = m_filter;
    m_filter = filter;
    if ( oldFilter ) {
        m_filter->setBuffer( oldFilter->buffer() );
        oldFilter->setBuffer( QString::null );
    }
    m_mutex.unlock();
    start();
}

void BackgroundThread::changeLanguage( const QString& lang )
{
    stop();
    m_mutex.lock();
    delete m_dict;
    m_dict = m_broker->dictionary( lang );
    m_filter->restart();
    m_mutex.unlock();
    start();
}

void BackgroundThread::stop()
{
    //### maybe terminate() would be better than using m_done
    m_done = true;
    wait();
}
