/**
 * highlighter.cpp
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

#include "highlighter.h"
#include "broker.h"
#include "dictionary.h"
#include "settings.h"

#include <kconfig.h>
#include <kdebug.h>

#include <q3textedit.h>
#include <qtimer.h>
#include <qcolor.h>
#include <q3dict.h>

namespace KSpell2 {

class Highlighter::Private
{
public:
    Filter     *filter;
    Broker::Ptr broker;
    Dictionary *dict;
    Q3Dict<Dictionary>  dictCache;
};

Highlighter::Highlighter( Q3TextEdit *textEdit,
                          const QString& configFile,
                          Filter *filter)
    : Q3SyntaxHighlighter( textEdit )
{
    d = new Private;
    d->filter = filter;
    if ( !configFile.isEmpty() )
        d->broker = Broker::openBroker( KSharedConfig::openConfig( configFile ) );
    else
        d->broker = Broker::openBroker();

    d->filter->setSettings( d->broker->settings() );
    d->dict   = d->broker->dictionary();
    Q_ASSERT( d->dict );
    d->dictCache.insert( d->broker->settings()->defaultLanguage(),
                         d->dict );
}

Highlighter::~Highlighter()
{
    delete d; d = 0;
}

int Highlighter::highlightParagraph( const QString& text,
                                     int endStateOfLastPara )
{
    Q_UNUSED( endStateOfLastPara );
    int para, index;
    textEdit()->getCursorPosition( &para, &index );
    int lengthPosition = text.length() - 1;

    if ( index != lengthPosition ||
         !text[text.length()-2].isLetter() ) {
        d->filter->setBuffer( text );
        Word w = d->filter->nextWord();
        while ( !w.end ) {
            if ( !d->dict->check( w.word ) ) {
                setMisspelled( w.start, w.word.length() );
            } else
                unsetMisspelled( w.start, w.word.length() );
            w = d->filter->nextWord();
        }
    }
    //QTimer::singleShot( 0, this, SLOT(checkWords()) );

    return 0;
}

Filter *Highlighter::currentFilter() const
{
    return d->filter;
}

void Highlighter::setCurrentFilter( Filter *filter )
{
    d->filter = filter;
    d->filter->setSettings( d->broker->settings() );
}

QString Highlighter::currentLanguage() const
{
    return d->dict->language();
}

void Highlighter::setCurrentLanguage( const QString& lang )
{
    if ( !d->dictCache.find( lang ) ) {
        Dictionary *dict = d->broker->dictionary( lang );
        if ( dict ) {
            d->dictCache.insert( lang, dict );
        } else {
            kdDebug()<<"No dictionary for \""
                     <<lang
                     <<"\" staying with the current language."
                     <<endl;
            return;
        }
    }
    d->dict = d->dictCache[lang];
}

void Highlighter::setMisspelled( int start, int count )
{
    setFormat( start , count, Qt::red );
}

void Highlighter::unsetMisspelled( int start, int count )
{
    setFormat( start, count, Qt::black );
}

/*
void Highlighter::checkWords()
{
    Word w = d->filter->nextWord();
    if ( !w.end ) {
        if ( !d->dict->check( w.word ) ) {
            setFormat( w.start, w.word.length(),
                       Qt::red );
        }
    }
}*/

}
