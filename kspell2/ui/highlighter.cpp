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

#include <qtextedit.h>
#include <qtimer.h>
#include <qcolor.h>
#include <QHash>
#include <QTextCursor>

namespace KSpell2 {

class Highlighter::Private
{
public:
    Filter     *filter;
    Broker::Ptr broker;
    Dictionary *dict;
    QHash<QString, Dictionary*>dictCache;
    QTextEdit *edit;
};

Highlighter::Highlighter( QTextEdit *textEdit,
                          const QString& configFile,
                          Filter *filter)
    : QSyntaxHighlighter( textEdit ),d(new Private)
{
    d->filter = filter;
    d->edit = textEdit;
    if ( !configFile.isEmpty() )
        d->broker = Broker::openBroker( KSharedConfig::openConfig( configFile ).data() );
    else
        d->broker = Broker::openBroker();

    d->filter->setSettings( d->broker->settings() );
    d->dict   = d->broker->dictionary();
    Q_ASSERT( d->dict );
    d->dictCache.insert( d->broker->settings()->defaultLanguage(),
                         d->dict );

    //Add kde personal word
    const QStringList l = Highlighter::personalWords();
    for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        d->dict->addToSession( *it );
    }

}

Highlighter::~Highlighter()
{
    delete d;
}

QStringList Highlighter::personalWords()
{
    QStringList l;
    l.append( "KMail" );
    l.append( "KOrganizer" );
    l.append( "KAddressBook" );
    l.append( "KHTML" );
    l.append( "KIO" );
    l.append( "KJS" );
    l.append( "Konqueror" );
    l.append( "KSpell" );
    l.append( "Kontact" );
    l.append( "Qt" );
    return l;
}

void Highlighter::highlightBlock ( const QString & text )
{
    if ( text.isEmpty() )
        return;
    QTextCursor cursor = d->edit->textCursor();
    int index = cursor.position();

    const int lengthPosition = text.length() - 1;

    if ( index != lengthPosition ||
         ( lengthPosition > 0 && !text[lengthPosition-1].isLetter() ) ) {
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
    setCurrentBlockState ( 0 );
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
