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

#include "filter_p.h"

#include "settings_p.h"

#include <kglobal.h>
#include <kdebug.h>

namespace Sonnet
{

static Word endWord;

K_GLOBAL_STATIC(Filter, s_defFilter)

class Filter::Private
{
public:
    // The reason it's not in the class directly is that
    // I'm not 100% sure that having the settings() here is
    // the way i want to be doing this.
    Settings *settings;
};

Filter* Filter::defaultFilter()
{
    return new Filter();
}

Word Filter::end()
{
    return endWord;
}

Filter::Filter()
    : m_currentPosition( 0 ),d(new Private)
{
    d->settings = 0;
}

Filter::~Filter()
{
    delete d;
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

    if ( m_currentPosition >= m_buffer.length() || currentChar.isNull() ) {
        return Filter::end();
    }

    bool allUppercase = currentChar.category() & QChar::Letter_Uppercase;
    bool runTogether = false;

    QString foundWord;
    int start = m_currentPosition;

    // Loop through the chars of the word, until the current char is not a letter
    // anymore.
    // Include apostrophes in the word, but not when it is the first character,
    // as it might be used as 'quotes'.
    // This way, we'll pass contractions like "I've" to the spellchecker, and
    // only the word inside apostrophe-quotes, without the apostrophes.
    while ( currentChar.isLetter() ||
            ( currentChar == '\'' && start != m_currentPosition ) ) {
        if ( currentChar.category() & QChar::Letter_Lowercase )
            allUppercase = false;

	/* FIXME: this does not work for Hebrew for example
        //we consider run-together words as mixed-case words
        if ( !allUppercase &&
             currentChar.category() & QChar::Letter_Uppercase )
            runTogether = true;
	*/

        foundWord += currentChar;
        //Test if currentPosition exists, otherwise go out
        if( (m_currentPosition + 1) >= m_buffer.length()) {

            // Remove apostrophes at the end of the word, it probably comes from
            // quoting with apostrophes.
            if ( foundWord.endsWith( '\'' ) )
                foundWord.chop( 1 );

            if ( shouldBeSkipped( allUppercase, runTogether, foundWord ) ) {
                ++m_currentPosition;
                return nextWord();
            }
            else {
                ++m_currentPosition;
                return Word( foundWord, start );
            }
        }
        ++m_currentPosition;
        currentChar = m_buffer.at( m_currentPosition );
    }

    // Remove apostrophes at the end of the word, it probably comes from
    // quoting with apostrophes.
    if ( foundWord.endsWith( '\'' ) )
        foundWord.chop( 1 );

    if ( shouldBeSkipped( allUppercase, runTogether, foundWord ) )
        return nextWord();
    return Word( foundWord, start );
}

Word Filter::previousWord() const
{
    while ( !m_buffer.at( m_currentPosition ).isLetter() &&
            m_currentPosition != 0) {
        --m_currentPosition;
    }

    if ( m_currentPosition == 0 ) {
        return Filter::end();
    }

    QString foundWord;
    int start = m_currentPosition;
    while ( m_buffer.at( start ).isLetter() ) {
        foundWord.prepend( m_buffer.at( m_currentPosition ) );
        --start;
    }

    return Word( foundWord, start );
}

Word Filter::wordAtPosition( unsigned int pos ) const
{
    if ( (int)pos > m_buffer.length() )
        return Filter::end();

    int currentPosition = pos - 1;
    QString foundWord;
    while ( currentPosition >= 0 &&
            m_buffer.at( currentPosition ).isLetter() ) {
        foundWord.prepend( m_buffer.at( currentPosition ) );
        --currentPosition;
    }

    // currentPosition == 0 means the first char is not letter
    // currentPosition == -1 means we reached the beginning
    int start = (currentPosition < 0) ? 0 : ++currentPosition;
    currentPosition = pos ;
    if ( currentPosition < m_buffer.length() && m_buffer.at( currentPosition ).isLetter() ) {
        while ( m_buffer.at( currentPosition ).isLetter() ) {
            foundWord.append( m_buffer.at( currentPosition ) );
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
    while ( m_buffer.at( m_currentPosition ).isLetter() && m_currentPosition > 0 )
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
    QChar currentChar = m_buffer.at( m_currentPosition );

    int length = m_buffer.length();
    //URL - if so skip
    if ( currentChar == ':'
         && (m_currentPosition+1 < length)
         && (m_buffer.at( ++m_currentPosition ) == '/' || ( m_currentPosition + 1 ) >= length ) ) {
        //in both cases url is considered finished at the first whitespace occurrence
        //TODO hey, "http://en.wikipedia.org/wiki/Main Page" --Nick Shaforostoff
        while ( !m_buffer.at( m_currentPosition++ ).isSpace() && m_currentPosition < length )
            ;
        return true;
    }

    //Email - if so skip
    if ( currentChar == '@') {
        while ( ++m_currentPosition < length && !m_buffer.at( m_currentPosition ).isSpace() )
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

QChar Filter::skipToLetter( int &fromPosition ) const
{
    //if( m_buffer.isEmpty())
    if (fromPosition>=m_buffer.size())
        return QChar();
    QChar currentChar = m_buffer.at( fromPosition );
    while ( !currentChar.isLetter() &&
            (int)++fromPosition < m_buffer.length() ) {
        currentChar = m_buffer.at( fromPosition );
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
