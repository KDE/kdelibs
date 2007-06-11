/*
   $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $

   This file is part of the Nepomuk KDE project.
   Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "kmetadatatagcloud.h"

#include "../generated/tag.h"

#include <QtCore/QTimer>


class Nepomuk::TagCloud::Private
{
public:
    QTimer* updateTimer;
};


Nepomuk::TagCloud::TagCloud( QWidget* parent )
    : KTagCloudWidget( parent ),
      d( new Private() )
{
    d->updateTimer = new QTimer( this );
    connect( d->updateTimer, SIGNAL(timeout()), this, SLOT(updateTags()) );
    updateTags();
    setAutoUpdate( true );
}


Nepomuk::TagCloud::~TagCloud()
{
    delete d;
}


bool Nepomuk::TagCloud::autoUpdate() const
{
    return d->updateTimer->isActive();
}


void Nepomuk::TagCloud::updateTags()
{
    // clear the tag cloud
    clear();

    // retrieve the list of all tags
    QList<Tag> tags = Tag::allTags();

    // count the number of usages of the tags and add them to the cloud
    for( QList<Tag>::iterator it = tags.begin();
         it != tags.end(); ++it ) {
        Tag& tag = *it;
        if ( tag.label().isEmpty() ) {
            tag.setLabel( tag.identifiers().isEmpty() ? tag.uri() : tag.identifiers().first() );
        }
        addTag( tag.label(), tag.tagOf().count() );
    }
}


void Nepomuk::TagCloud::setAutoUpdate( bool enable )
{
    if( !enable )
        d->updateTimer->stop();
    else if( !autoUpdate() )
        d->updateTimer->start( 10*1000 ); // update every 10 seconds
}


void Nepomuk::TagCloud::slotTagClicked( const QString& tag )
{
    emit tagClicked( Tag(tag) );
}

#include "kmetadatatagcloud.moc"
