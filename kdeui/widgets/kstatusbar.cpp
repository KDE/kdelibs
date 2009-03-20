/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
              (C) 1997,1998, 2000 Sven Radej (radej@kde.org)
              (C) 2007 Aron Boström (aron.bostrom@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kstatusbar.h"

#include <QtCore/QHash>
#include <QtCore/QEvent>
#include <QtGui/QLabel>

#include <kdebug.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "ksqueezedtextlabel.h"


class KStatusBarPrivate
{
public:
    int id(QObject* object) const
    {
        QHash<int, QLabel*>::const_iterator it = qFind(items, object);
        if (it != items.constEnd())
            return it.key();
        // Not found. This happens when a subclass uses an eventFilter too,
        // on objects not registered here.
        return -1;
    }

    QHash<int, QLabel*> items;
};


bool KStatusBar::eventFilter(QObject* object, QEvent* event)
{
    if ( event->type() == QEvent::MouseButtonPress ) {
        const int id = d->id(object);
        if (id > -1) {
            emit pressed( d->id( object ) );
            return true;
        }
    }
    else if ( event->type() == QEvent::MouseButtonRelease ) {
        const int id = d->id(object);
        if (id > -1) {
            emit released( d->id( object ) );
            return true;
        }
    }
    return QStatusBar::eventFilter(object, event);
}

KStatusBar::KStatusBar( QWidget *parent )
  : QStatusBar( parent ),
    d(new KStatusBarPrivate)
{
    // make the size grip stuff configurable
    // ...but off by default (sven)
    // ...but on by default on OSX, else windows with a KStatusBar are not resizable at all (marijn)
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group( config, QLatin1String("StatusBar style") );
#ifdef Q_WS_MAC
    bool grip_enabled = group.readEntry(QLatin1String("SizeGripEnabled"), true);
#else
    bool grip_enabled = group.readEntry(QLatin1String("SizeGripEnabled"), false);
#endif
    setSizeGripEnabled(grip_enabled);
}

KStatusBar::~KStatusBar ()
{
  delete d;
}

void KStatusBar::insertItem( const QString& text, int id, int stretch)
{
    if ( d->items[id] ) {
        kDebug() << "KStatusBar::insertItem: item id " << id << " already exists.";
    }

    KSqueezedTextLabel *l = new KSqueezedTextLabel( text, this );
    l->installEventFilter( this );
    l->setFixedHeight( fontMetrics().height() + 2 );
    l->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    d->items.insert( id, l );
    addPermanentWidget( l, stretch );
    l->show();
}

void KStatusBar::insertFixedItem( const QString& text, int id )
{
    insertItem( text, id, 0 );
    setItemFixed( id );
}


void KStatusBar::insertPermanentItem( const QString& text, int id, int stretch)
{
    if (d->items[id]) {
        kDebug() << "KStatusBar::insertPermanentItem: item id " << id << " already exists.";
    }

    QLabel *l = new QLabel( text, this );
    l->installEventFilter( this );
    l->setFixedHeight( fontMetrics().height() + 2 );
    l->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    d->items.insert( id, l );
    addPermanentWidget( l, stretch );
    l->show();
}

void KStatusBar::insertPermanentFixedItem( const QString& text, int id )
{
    insertPermanentItem( text, id, 0 );
    setItemFixed( id );
}

void KStatusBar::removeItem (int id)
{
    if ( d->items.contains( id ) ) {
        QLabel *label = d->items[id];
        removeWidget( label );
        d->items.remove( id );
        delete label;
    } else {
        kDebug() << "KStatusBar::removeItem: bad item id: " << id;
    }
}

bool KStatusBar::hasItem( int id ) const
{
    return d->items.contains(id);
}

QString KStatusBar::itemText( int id ) const
{
    if ( !hasItem( id ) ) {
        return QString();
    }

    return d->items[id]->text();
}

void KStatusBar::changeItem( const QString& text, int id )
{
    QLabel *label = d->items[id];
    KSqueezedTextLabel *squeezed = qobject_cast<KSqueezedTextLabel*>( label );

    if ( squeezed ) {
        squeezed->setText( text );
    } else if ( label ) {
        label->setText( text );
        if ( label->minimumWidth () != label->maximumWidth () ) {
            reformat();
        }
    } else {
        kDebug() << "KStatusBar::changeItem: bad item id: " << id;
    }
}

void KStatusBar::setItemAlignment (int id, Qt::Alignment alignment)
{
    QLabel *label = qobject_cast<QLabel*>( d->items[id] );
    if ( label ) {
        label->setAlignment( alignment );
    } else {
        kDebug() << "KStatusBar::setItemAlignment: bad item id: " << id;
    }
}

void KStatusBar::setItemFixed(int id, int w)
{
    QLabel *label = qobject_cast<QLabel*>(d->items[id]);
    if ( label ) {
        if ( w == -1 ) {
            w = fontMetrics().boundingRect(label->text()).width()+3;
        }

        label->setFixedWidth(w);
    } else {
        kDebug() << "KStatusBar::setItemFixed: bad item id: " << id;
    }
}

#include "kstatusbar.moc"

