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

#include "kmetadatatagwidget.h"
#include "kautoscrollarea.h"
#include "ktagdisplaywidget.h"

#include "../tools.h"
#include "../generated/tag.h"

#include <karrowbutton.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtGui/QPushButton>
#include <QtGui/QBoxLayout>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtCore/QSet>


uint qHash( const Nepomuk::Tag& res )
{
    return qHash( res.uri() );
}



class Nepomuk::TagWidget::Private
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

    QStringList extractTagNames( QList<Tag> tags ) {
        QStringList tagStrings;
        // convert the tag list to keywords
        for( QList<Tag>::iterator it = tags.begin();
             it != tags.end(); ++it ) {
            tagStrings += ( *it ).genericLabel();
        }
        return tagStrings;
    }

    QList<Tag> intersectTags() {
        if ( res.count() == 1 ) {
            return res.first().tags();
        }
        else if ( !res.isEmpty() ) {
            // determine the tags used for all resources
            QSet<Tag> tags = QSet<Tag>::fromList( res.first().tags() );
            QList<Resource>::const_iterator it = res.begin();
            for ( ++it; it != res.end(); ++it ) {
                tags.intersect( QSet<Tag>::fromList( (*it).tags() ) );
            }
            return tags.values();
        }
        else {
            return QList<Tag>();
        }
    }

    QList<Resource> res;
    KTagDisplayWidget* label;
    KArrowButton* button;
    TagWidget* parent;
};


Nepomuk::TagWidget::TagWidget( const Resource& resource, QWidget* parent )
    : QWidget( parent ),
      d( new Private( this ) )
{
    setTaggedResource( resource );
}


Nepomuk::TagWidget::TagWidget( QWidget* parent )
    : QWidget( parent ),
      d( new Private( this ) )
{
}


Nepomuk::TagWidget::~TagWidget()
{
    delete d;
}


QList<Nepomuk::Resource> Nepomuk::TagWidget::taggedResources() const
{
    return d->res;
}


QList<Nepomuk::Tag> Nepomuk::TagWidget::assignedTags() const
{
    return d->intersectTags();
}


void Nepomuk::TagWidget::setTaggedResource( const Resource& resource )
{
    QList<Resource> l;
    l.append( resource );
    setTaggedResources( l );
}


void Nepomuk::TagWidget::setTaggedResources( const QList<Resource>& resources )
{
    d->res = resources;
    if ( !resources.isEmpty() ) {
        d->label->setTags( d->extractTagNames( d->intersectTags() ) );
    }
    else {
        d->label->setTags( QStringList() );
    }
}


void Nepomuk::TagWidget::setAssignedTags( const QList<Tag>& tags )
{
    if ( !d->res.isEmpty() ) {
        for ( QList<Resource>::iterator it = d->res.begin(); it != d->res.end(); ++it ) {
            (*it).setTags( tags );
        }
        d->label->setTags( d->extractTagNames( tags ) );
    }
}


void Nepomuk::TagWidget::slotShowTagMenu()
{
    QList<Tag> allTags = Tag::allTags();
    QList<Tag> assignedTags = d->intersectTags();

    QMenu* popup = new QMenu( i18n( "Tag Resource" ), this );
    QMap<QAction*, Tag> tagMap;
    foreach( Tag tag,  allTags ) {
        if ( tag.label().isEmpty() ) {
            tag.setLabel( tag.identifiers().isEmpty() ? tag.uri() : tag.identifiers().first() );
        }
        QAction* a = new QAction( tag.label(), popup );
        a->setCheckable( true );
        popup->addAction( a );
        tagMap.insert( a,  tag );
        a->setChecked( assignedTags.contains( tag ) );
    }

    QAction* newTagAction = new QAction( i18n( "Create New Tag..." ), popup );
    popup->addAction( newTagAction );

    if ( QAction* a = popup->exec( mapToGlobal( d->button->geometry().topRight() ) ) ) {
        if ( a == newTagAction ) {
            QString s = KInputDialog::getText( i18n("New Tag"), i18n("Please insert the name of the new tag:"), QString(), 0, this );
            if( !s.isEmpty() ) {
                // see if the tag exists
                QList<Tag> l = Tag::allTags();
                QListIterator<Tag> tagIt( l );
                while( tagIt.hasNext() ) {
                    const Nepomuk::Tag& tag = tagIt.next();
                    if( tag.label() == s ||
                        tag.identifiers().contains( s ) ) {
                        KMessageBox::sorry( this, i18n("The tag %1 already exists", s), i18n("Tag Exists") );
                        return;
                    }
                }

                Nepomuk::Tag( s ).setLabel( s );
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
