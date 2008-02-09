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

#include "ktagdisplaywidget.h"

#include <kurllabel.h>

#include <QtGui/QBoxLayout>


class KTagDisplayWidget::Private
{
public:
    Private( KTagDisplayWidget* _parent )
        : parent( _parent ) {
        layout = new QHBoxLayout( parent );
        layout->setMargin( 0 );
    }

    void buildDisplay() {
        // delete all the children
        foreach( QWidget* w, wl )
            delete w;
        wl.clear();

        // remove the stretch we added in the last call
        if ( QLayoutItem* item = layout->takeAt( 0 ) )
            delete item;

        // create new labels
        for( QStringList::const_iterator it = tags.constBegin();
             it != tags.constEnd(); ++it ) {
            if (  it != tags.constBegin() ) {
                QLabel* label = new QLabel( "-", parent );
                wl.append( label );
                layout->addWidget( label ); // FIXME: display some nicer symbol like a big dot
            }
            KUrlLabel* label = new KUrlLabel( *it, *it, parent );
            wl.append( label );
            label->setUnderline( false );
            layout->addWidget( label );

            connect( label, SIGNAL(leftClickedUrl(const QString&)),
                     parent, SIGNAL(tagClicked(const QString&)) );
        }

        layout->addStretch( 1 );
    }

    QStringList tags;
    QList<QWidget*> wl;
    QHBoxLayout* layout;

private:
    KTagDisplayWidget* parent;
};


KTagDisplayWidget::KTagDisplayWidget( QWidget* parent )
    : QWidget( parent ),
      d( new Private( this ) )
{
}


KTagDisplayWidget::~KTagDisplayWidget()
{
    delete d;
}


void KTagDisplayWidget::setTags( const QStringList& tags )
{
    d->tags = tags;
    d->buildDisplay();
}


void KTagDisplayWidget::addTag( const QString& tag )
{
    if ( !d->tags.contains( tag ) )
        d->tags.append( tag );
    d->buildDisplay();
}


void KTagDisplayWidget::addTags( const QStringList& tags )
{
    foreach( const QString &tag, tags )
        if ( !d->tags.contains( tag ) )
            d->tags.append( tag );
    d->buildDisplay();
}


void KTagDisplayWidget::clear()
{
    d->tags.clear();
    d->buildDisplay();
}

#include "ktagdisplaywidget.moc"
