/*
   $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $

   This file is part of the Nepomuk KDE project.
   Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>

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

#include "kmetadatatagwidget.h"
#include "kautoscrollarea.h"
#include "ktagdisplaywidget.h"

#include <kmetadata/tools.h>
#include <kmetadata/tag.h>

#include <karrowbutton.h>
#include <klocale.h>

#include <QPushButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>


class Nepomuk::KMetaData::TagWidget::Private
{
public:
    Private( TagWidget* _parent )
        : parent( _parent ) {
        QHBoxLayout* lay = new QHBoxLayout( parent );
        lay->setMargin( 0 );
        KAutoScrollArea* sa = new KAutoScrollArea( parent );
        label = new KTagDisplayWidget;
        sa->setWidget( label );
        button = new KArrowButton( parent, Qt::DownArrow );

        lay->addWidget( sa, 1 );
        lay->addWidget( button );

        connect( button, SIGNAL( pressed() ), parent, SLOT( slotShowTagMenu() ) );
    }

    Resource res;
    KTagDisplayWidget* label;
    KArrowButton* button;
    TagWidget* parent;
};


Nepomuk::KMetaData::TagWidget::TagWidget( const Resource& resource, QWidget* parent )
    : QWidget( parent )
{
    d = new Private( this );
    setTaggedResource( resource );
}


Nepomuk::KMetaData::TagWidget::TagWidget( QWidget* parent )
    : QWidget( parent )
{
    d = new Private( this );
}


Nepomuk::KMetaData::TagWidget::~TagWidget()
{
    delete d;
}


Nepomuk::KMetaData::Resource Nepomuk::KMetaData::TagWidget::taggedResource() const
{
    return d->res;
}


QList<Nepomuk::KMetaData::Tag> Nepomuk::KMetaData::TagWidget::assignedTags() const
{
    return d->res.getTags();
}


void Nepomuk::KMetaData::TagWidget::setTaggedResource( const Resource& resource )
{
    d->res = resource;
    QStringList tagStrings;
    QList<Tag> tags = d->res.getTags();
    for( QList<Tag>::const_iterator it = tags.constBegin();
         it != tags.constEnd(); ++it )
        tagStrings += ( *it ).getIdentifiers().first();
    d->label->setTags( tagStrings );
}


void Nepomuk::KMetaData::TagWidget::setAssignedTags( const QList<Tag>& tags )
{
    if ( d->res.isValid() )
        d->res.setTags( tags );
    setTaggedResource( d->res );
}


void Nepomuk::KMetaData::TagWidget::slotShowTagMenu()
{
    QList<Tag> allTags = Tag::allTags();

    QMenu* popup = new QMenu( i18n( "Tag resource..." ), this );
    QMap<QAction*, Tag> tagMap;
    foreach( Tag tag,  allTags ) {
        QAction* a = new QAction( tag.getIdentifiers().first(), popup );
        a->setCheckable( true );
        popup->addAction( a );
        tagMap.insert( a,  tag );
        a->setChecked( d->res.getTags().contains( tag ) );
    }

    QAction* newTagAction = new QAction( i18n( "Create new tag..." ), popup );
    popup->addAction( newTagAction );

    if ( QAction* a = popup->exec( mapToGlobal( d->button->geometry().topRight() ) ) ) {
        if ( a == newTagAction ) {
            QString s = QInputDialog::getText( this, i18n("New Tag"), i18n("Please insert the name of the new tag") );
            if( !s.isEmpty() ) {
                // see if the tag exists
                QList<Tag> l = Tag::allTags();
                QListIterator<Tag> tagIt( l );
                while( tagIt.hasNext() ) {
                    const Nepomuk::KMetaData::Tag& tag = tagIt.next();
                    if( tag.getLabels().contains( s ) ||
                        tag.getIdentifiers().contains( s ) ) {
                        QMessageBox::critical( this, i18n("Tag exists"), i18n("The tag %1 already exists", s) );
                        return;
                    }
                }

                Nepomuk::KMetaData::Tag( s ).addLabel( s );
            }
        }
        else {
            QList<Tag> tags;
            for ( QMap<QAction*, Tag>::const_iterator it = tagMap.constBegin();
                  it != tagMap.constEnd(); ++it )
                if ( it.key()->isChecked() )
                    tags.append( it.value() );

            setAssignedTags( tags );
        }
    }

    delete popup;
}

#include "kmetadatatagwidget.moc"
