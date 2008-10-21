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
#include "nepomukmassupdatejob.h"

#include "../tools.h"
#include "../tag.h"

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
    return qHash( res.resourceUri().toString() );
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

        tagMenu = new QMenu( parent );
        button->setMenu( tagMenu );

        lay->addWidget( sa, 1 );
        lay->addWidget( button );

        connect( label, SIGNAL( tagClicked( const QString& ) ),
                 parent, SLOT( slotTagClicked( const QString& ) ) );
        connect( tagMenu, SIGNAL( aboutToShow() ), parent, SLOT( fillTagMenu() ) );
    }

    QStringList extractTagNames( QList<Tag> tags ) {
        QStringList tagStrings;
        // convert the tag list to keywords
        for( QList<Tag>::const_iterator it = tags.constBegin();
             it != tags.constEnd(); ++it ) {
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
            QList<Resource>::const_iterator it = res.constBegin();
            for ( ++it; it != res.constEnd(); ++it ) {
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
    QMenu* tagMenu;
    QMap<QAction*, Tag> tagFromAction;
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
        d->label->setTags( d->extractTagNames( tags ) );
        Nepomuk::MassUpdateJob* job = Nepomuk::MassUpdateJob::tagResources( d->res, tags );
        connect( job, SIGNAL( result( KJob* ) ),
                 this, SLOT( slotTagUpdateDone() ) );
        setEnabled( false ); // no updates during execution
        job->start();
    }
}


void Nepomuk::TagWidget::slotTagUpdateDone()
{
    setEnabled( true );
}

void Nepomuk::TagWidget::slotTagClicked( const QString& text) 
{
    Nepomuk::Tag tag( text );
    emit tagClicked( tag );
}

static bool tagLabelLessThan( const Nepomuk::Tag& t1, const Nepomuk::Tag& t2 )
{
    return t1.label() < t2.label();
}


void Nepomuk::TagWidget::fillTagMenu()
{
    QList<Tag> allTags = Tag::allTags();
    // Prepare allTags list
    foreach( Tag tag, allTags ) {
        if ( tag.label().isEmpty() ) {
            tag.setLabel( tag.genericLabel() );
        }
    }
    qSort( allTags.begin(), allTags.end(), tagLabelLessThan );

    QList<Tag> assignedTags = d->intersectTags();

    d->tagMenu->clear();
    d->tagFromAction.clear();
    foreach( const Tag &tag,  allTags ) {
        QAction* a = d->tagMenu->addAction( tag.label(), this, SLOT( updateAssignedTagsFromMenu() ) );
        d->tagFromAction.insert( a, tag );
        a->setCheckable( true );
        a->setChecked( assignedTags.contains( tag ) );
    }

    d->tagMenu->addAction( i18n( "Create New Tag..."), this, SLOT(createTag()) );
}


void Nepomuk::TagWidget::createTag()
{
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


void Nepomuk::TagWidget::updateAssignedTagsFromMenu()
{
    QList<Tag> tags;
    for ( QMap<QAction*, Tag>::const_iterator it = d->tagFromAction.constBegin();
          it != d->tagFromAction.constEnd(); ++it )
        if ( it.key()->isChecked() )
            tags.append( it.value() );

    setAssignedTags( tags );
}


#include "kmetadatatagwidget.moc"
