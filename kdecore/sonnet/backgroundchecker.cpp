/**
 * backgroundchecker.cpp
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
#include "backgroundchecker.h"

#include "loader.h"
#include "backgroundengine_p.h"

#include <kdebug.h>

using namespace Sonnet;

class BackgroundChecker::Private
{
public:
    BackgroundEngine *engine;
    QString currentText;
};

BackgroundChecker::BackgroundChecker( const Loader::Ptr& loader, QObject* parent )
    : QObject( parent ),d(new Private)
{
    d->engine = new BackgroundEngine( this );
    d->engine->setLoader( loader );
    connect( d->engine, SIGNAL(misspelling( const QString&, int )),
             SIGNAL(misspelling( const QString&, int )) );
    connect( d->engine, SIGNAL(done()),
             SLOT(slotEngineDone()) );
}

BackgroundChecker::~BackgroundChecker()
{
    delete d;
}

void BackgroundChecker::checkText( const QString& text )
{
    d->currentText = text;
    d->engine->setText( text );
    d->engine->start();
}

void BackgroundChecker::start()
{
    d->currentText = getMoreText();
    // ## what if d->currentText.isEmpty()?
    //kDebug()<<"Sonnet BackgroundChecker: starting with : \"" << d->currentText << "\""<<endl;
    d->engine->setText( d->currentText );
    d->engine->start();
}

void BackgroundChecker::stop()
{
    d->engine->stop();
}

QString BackgroundChecker::getMoreText()
{
    return QString();
}

void BackgroundChecker::finishedCurrentFeed()
{
}

void BackgroundChecker::setFilter( Filter *filter )
{
    d->engine->setFilter( filter );
}

Filter *BackgroundChecker::filter() const
{
    return d->engine->filter();
}

Loader *BackgroundChecker::loader() const
{
    return d->engine->loader();
}

bool BackgroundChecker::checkWord( const QString& word )
{
    return d->engine->checkWord( word );
}

bool BackgroundChecker::addWord( const QString& word )
{
    return d->engine->addWord( word );
}

QStringList BackgroundChecker::suggest( const QString& word ) const
{
    return d->engine->suggest( word );
}

void BackgroundChecker::changeLanguage( const QString& lang )
{
    d->engine->changeLanguage( lang );
}

void BackgroundChecker::continueChecking()
{
    d->engine->continueChecking();
}

void BackgroundChecker::slotEngineDone()
{
    finishedCurrentFeed();
    d->currentText = getMoreText();

    if ( d->currentText.isNull() ) {
        emit done();
    } else {
        d->engine->setText( d->currentText );
        d->engine->start();
    }
}

#include "backgroundchecker.moc"
