/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kmetadatatagcloud.h"
#include "resourcemanager.h"

#include "tag.h"

#include <QtCore/QTimer>

#include <Soprano/Model>


class Nepomuk::TagCloud::Private
{
public:
    Private()
        : autoUpdate(false) {
    }
    QTimer* updateTimer;
    bool autoUpdate;
};


Nepomuk::TagCloud::TagCloud( QWidget* parent )
    : KTagCloudWidget( parent ),
      d( new Private() )
{
    updateTags();
    setAutoUpdate( true );
}


Nepomuk::TagCloud::~TagCloud()
{
    delete d;
}


bool Nepomuk::TagCloud::autoUpdate() const
{
    return d->autoUpdate;
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
            tag.setLabel( tag.genericLabel() );
        }
        addTag( tag.label(), tag.tagOf().count() );
    }
}


void Nepomuk::TagCloud::setAutoUpdate( bool enable )
{
    if ( enable != d->autoUpdate ) {
        if( !enable ) {
            disconnect( ResourceManager::instance()->mainModel(), SIGNAL(statementsAdded()),
                        this, SLOT(updateTags()) );
            disconnect( ResourceManager::instance()->mainModel(), SIGNAL(statementsRemoved()),
                        this, SLOT(updateTags()) );
        }
        else {
            connect( ResourceManager::instance()->mainModel(), SIGNAL(statementsAdded()),
                     this, SLOT(updateTags()) );
            connect( ResourceManager::instance()->mainModel(), SIGNAL(statementsRemoved()),
                     this, SLOT(updateTags()) );
        }

        d->autoUpdate = enable;
    }
}


void Nepomuk::TagCloud::slotTagClicked( const QString& tag )
{
    emit tagClicked( Tag(tag) );
}

#include "kmetadatatagcloud.moc"
