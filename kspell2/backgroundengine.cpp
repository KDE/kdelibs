/**
 * backgroundengine.cpp
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
#include "backgroundengine.h"

#include "defaultdictionary.h"
#include "dictionary.h"
#include "filter.h"

#include <kdebug.h>

#include <qtimer.h>

using namespace KSpell2;

BackgroundEngine::BackgroundEngine( QObject *parent )
    : QObject( parent )
{
    m_filter = Filter::defaultFilter();
    m_dict = 0;
}

BackgroundEngine::~BackgroundEngine()
{
    delete m_dict; m_dict = 0;
}

void BackgroundEngine::setBroker( const Broker::Ptr& broker )
{
    m_broker = broker;
    delete m_dict;
    m_defaultDict = m_broker->defaultDictionary();
    m_filter->setSettings( m_broker->settings() );
}

void BackgroundEngine::setText( const QString& text )
{
    m_filter->setBuffer( text );
}

QString BackgroundEngine::text() const
{
    return m_filter->buffer();
}

void BackgroundEngine::changeLanguage( const QString& lang )
{
    delete m_dict;
    if ( lang.isEmpty() ) {
        m_dict = 0;
    } else {
        m_dict = m_broker->dictionary( lang );
    }
}

QString BackgroundEngine::language() const
{
    if ( m_dict )
        return m_dict->language();
    else
        return m_defaultDict->language();
}

void BackgroundEngine::setFilter( Filter *filter )
{
    QString oldText = m_filter->buffer();
    m_filter = filter;
    m_filter->setBuffer( oldText );
}

void BackgroundEngine::start()
{
    QTimer::singleShot( 0, this, SLOT(checkNext()) );
}

void BackgroundEngine::stop()
{
}

void BackgroundEngine::continueChecking()
{
    QTimer::singleShot( 0, this, SLOT(checkNext()) );
}

void BackgroundEngine::checkNext()
{
    Word w = m_filter->nextWord();
    if ( w.end ) {
        emit done();
        return;
    }

    Dictionary *dict = ( m_dict ) ? m_dict : static_cast<Dictionary*>( m_defaultDict );

    if ( !dict->check( w.word ) ) {
        //kdDebug()<<"found misspelling "<< w.word <<endl;
        emit misspelling( w.word, w.start );
        //wait for the handler. the parent will decide itself when to continue
    } else
        continueChecking();
}

bool BackgroundEngine::checkWord( const QString& word )
{
    Dictionary *dict = ( m_dict ) ? m_dict : static_cast<Dictionary*>( m_defaultDict );
    return dict->check( word );
}

bool BackgroundEngine::addWord( const QString& word )
{
    Dictionary *dict = ( m_dict ) ? m_dict : static_cast<Dictionary*>( m_defaultDict );
    return dict->addToPersonal( word );
}

QStringList BackgroundEngine::suggest( const QString& word )
{
    Dictionary *dict = ( m_dict ) ? m_dict : static_cast<Dictionary*>( m_defaultDict );
    return dict->suggest( word );
}

#include "backgroundengine.moc"
