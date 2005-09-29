// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * filter.cpp
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

#include "filter.h"

#include "settings.h"

#include <kstaticdeleter.h>
#include <kdebug.h>

#include <qstring.h>

namespace KSpell2
{

static Word endWord;
static KStaticDeleter<Filter> sd;
static Filter* defFilter = 0;

class Filter::Private
{
public:
    // The reason it's not in the class directly is that
    // i'm not 100% sure that having the settings() here is
    // the way i want to be doing this.
    Settings *settings;
};

Filter* Filter::defaultFilter()
{
    if ( !defFilter )
        sd.setObject( defFilter, new Filter() );
    return defFilter;
}

Word Filter::end()
{
    return endWord;
}

Filter::Filter()
    : m_currentPosition( 0 )
{
    d = new Private;
    d->settings = 0;
}

Filter::~Filter()
{
    delete d; d = 0;
}

void Filter::setSettings( Settings *conf )
{
    d->settings = conf;
}

Settings *Filter::settings() const
{
    return d->settings;
}

void Filter::restart()
{
    m_currentPosition = 0;
}

void Filter::setBuffer( const QString& buffer )
{
    m_buffer          = buffer;
    m_currentPosition = 0;
}

QString Filter::buffer() const
{
    return m_buffer;
}

bool Filter::atEnd() const
{
    if ( m_currentPosition >= m_buffer.length() ) {
        return true;
    } else
        return false;
}

Word Filter::nextWord() const
{
    QChar currentChar = skipToLetter( m_currentPosition );

    if ( m_currentPosition >= m_buffer.length() ) {
        return Filter::end();
    }

    bool allUppercase = currentChar.category() & QChar::Letter_Uppercase;
    bool runTogether = false;

    QString foundWord;
    int start = m_currentPosition;
    while ( currentChar.isLetter() ) {
        if ( currentChar.category() & QChar::Letter_Lowercase )
            allUppercase = false;

	/* FIXME: this does not work for Hebrew for example
        //we consider run-together words as mixed-case words
        if ( !allUppercase &&
             currentChar.category() & QChar::Letter_Uppercase )
            runTogether = true;
	*/

        foundWord += currentChar;
        ++m_currentPosition;
        currentChar = m_buffer[ m_currentPosition ];
    }

    if ( shouldBeSkipped( allUppercase, runTogether, foundWord ) )
        return nextWord();

    return Word( foundWord, start );
}

Word Filter::previousWord() const
{
    while ( !m_buffer[ m_currentPosition ].isLetter() &&
            m_currentPosition != 0) {
        --m_currentPosition;
    }

    if ( m_currentPosition == 0 ) {
        return Filter::end();
    }

    QString foundWord;
    int start = m_currentPosition;
    while ( m_buffer[ start ].isLetter() ) {
        foundWord.prepend( m_buffer[ m_currentPosition ] );
        --start;
    }

    return Word( foundWord, start );
}

Word Filter::wordAtPosition( unsigned int pos ) const
{
    if ( pos > m_buffer.length() )
        return Filter::end();

    int currentPosition = pos - 1;
    QString foundWord;
    while ( currentPosition >= 0 &&
            m_buffer[ currentPosition ].isLetter() ) {
        foundWord.prepend( m_buffer[ currentPosition ] );
        --currentPosition;
    }

    int start = (!currentPosition) ? 0 : ++currentPosition;
    currentPosition = pos ;
    if ( m_buffer[ currentPosition ].isLetter() ) {
        while ( m_buffer[ currentPosition ].isLetter() ) {
            foundWord.append( m_buffer[ currentPosition ] );
            ++currentPosition;
        }
    }

    return Word( foundWord, start );
}


void Filter::setCurrentPosition( int i )
{
    m_currentPosition = i;

    //go back to the last word so that next word returns something
    //useful
    while ( m_buffer[m_currentPosition].isLetter() && m_currentPosition > 0 )
        --m_currentPosition;
}

int Filter::currentPosition() const
{
    return m_currentPosition;
}

void Filter::replace( const Word& w, const QString& newWord)
{
    int oldLen = w.word.length();
    int newLen = newWord.length();

    if ( oldLen != newLen && m_currentPosition > w.start ) {
        if ( m_currentPosition > w.start ) {
            int len = newLen - oldLen;
            m_currentPosition += len;
        }
    }
    m_buffer = m_buffer.replace( w.start, oldLen, newWord );
}

QString Filter::context() const
{
    int len = 60;
    //we don't want the expression underneath casted to an unsigned int
    //which would cause it to always evaluate to false
    int signedPosition = m_currentPosition;
    bool begin = ( (signedPosition - len/2)<=0 ) ? true : false;


    QString buffer = m_buffer;
    Word word = wordAtPosition( m_currentPosition );
    buffer = buffer.replace( word.start, word.word.length(),
                             QString( "<b>%1</b>" ).arg( word.word ) );

    QString context;
    if ( begin )
        context = QString( "%1...")
                  .arg( buffer.mid(  0, len ) );
    else
        context = QString( "...%1..." )
                  .arg( buffer.mid(  m_currentPosition - 20, len ) );

    context = context.replace( '\n', ' ' );

    return context;
}

bool Filter::trySkipLinks() const
{
    QChar currentChar = m_buffer[ m_currentPosition ];

    uint length = m_buffer.length();
    //URL - if so skip
    if ( currentChar == ':' &&
         ( m_buffer[ ++m_currentPosition] == '/' || ( m_currentPosition + 1 ) >= length ) ) {
        //in both cases url is considered finished at the first whitespace occurence
        while ( !m_buffer[ m_currentPosition++ ].isSpace() && m_currentPosition < length )
            ;
        return true;
    }

    //Email - if so skip
    if ( currentChar == '@' ) {
        while ( !m_buffer[ ++m_currentPosition ].isSpace() && m_currentPosition < length )
            ;
        return true;
    }

    return false;
}

bool Filter::ignore( const QString& word ) const
{
    if ( d->settings ) {
        return d->settings->ignore( word );
    }
    return false;
}

QChar Filter::skipToLetter( uint &fromPosition ) const
{

    QChar currentChar = m_buffer[ fromPosition ];
    while ( !currentChar.isLetter() &&
            fromPosition < m_buffer.length() ) {
        ++fromPosition;
        currentChar = m_buffer[ fromPosition ];
    }
    return currentChar;
}

bool Filter::shouldBeSkipped( bool wordWasUppercase, bool wordWasRunTogether,
                             const QString& foundWord ) const
{
    bool checkUpper = ( d->settings ) ?
                      d->settings->checkUppercase () : true;
    bool skipRunTogether = ( d->settings ) ?
                           d->settings->skipRunTogether() : true;

    if ( trySkipLinks() )
        return true;

    if ( wordWasUppercase && !checkUpper )
        return true;

    if ( wordWasRunTogether && skipRunTogether )
        return true;

    return ignore( foundWord );
}

}
