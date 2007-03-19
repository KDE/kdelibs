/* This file is part of the KDE libraries
  Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>
  Copyright (c) 2004 Gustavo Sverzut Barbieri <gsbarbieri@users.sourceforge.net>

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

#include "klistwidgetsearchline.h"

#include <qlistwidget.h>
#include <klocale.h>
#include <qtimer.h>
#include <kdebug.h>

#define DEFAULT_CASESENSITIVE Qt::CaseInsensitive

typedef QList <QListWidgetItem *> QListWidgetItemList;

class KListWidgetSearchLine::KListWidgetSearchLinePrivate
{
public:
    KListWidgetSearchLinePrivate(KLineEdit *parent) :
            lineEdit( parent ),
            listWidget( 0 ),
            caseSensitivity( DEFAULT_CASESENSITIVE ),
            activeSearch( false ),
            queuedSearches( 0 )
    {}

    void listWidgetDeleted();

    KLineEdit *lineEdit;
    QListWidget *listWidget;
    Qt::CaseSensitivity caseSensitivity;
    bool activeSearch;
    QString search;
    int queuedSearches;
    QListWidgetItemList hiddenItems;
};

/******************************************************************************
 * Public Methods                                                             *
 *****************************************************************************/
KListWidgetSearchLine::KListWidgetSearchLine( QWidget *parent, QListWidget *listWidget ) :
        KLineEdit( parent )
{
    d = 0;
    init( listWidget );
}

KListWidgetSearchLine::~KListWidgetSearchLine()
{
    clear(); // empty hiddenItems, returning items back to listWidget
    delete d;
}

Qt::CaseSensitivity KListWidgetSearchLine::caseSensitive() const
{
    return d->caseSensitivity;
}

QListWidget *KListWidgetSearchLine::listWidget() const
{
    return d->listWidget;
}

/******************************************************************************
 * Public Slots                                                               *
 *****************************************************************************/
void KListWidgetSearchLine::updateSearch( const QString &s )
{
    QListWidget *lw = d->listWidget;
    if ( !lw )
        return ; // disabled

    QString search = d->search = s.isNull() ? text() : s;

    QListWidgetItemList *hi = &(d->hiddenItems);

    QListWidgetItem *currentItem = lw->currentItem();

    QListWidgetItem *item = 0;

    // Remove Non-Matching items, add them them to hidden list
    int index = 0;
    while ( index < lw->count() ) {
        QListWidgetItem *item = lw->item(index);
        if ( ! itemMatches( item, search ) ) {
            hideItem( item );

            if ( item == currentItem ) {
                currentItem = 0; // It's not in listWidget anymore.
            }
        } else {
            index++;
        }
    }

    // Add Matching items, remove from hidden list
    QListWidgetItemList::iterator it = hi->begin();
    while ( it != hi->end() ) {
        item = *it;
        ++it;
        if ( itemMatches( item, search ) )
            showItem( item );
    }

    lw->sortItems();

    if ( currentItem != 0 )
        lw->scrollToItem( currentItem );
}

void KListWidgetSearchLine::clear()
{
    // Clear hidden list, give items back to QListWidget, if it still exists
    QListWidgetItem *item = 0;
    QListWidgetItemList::iterator it = d->hiddenItems.begin();
    while ( it != d->hiddenItems.end() ) {
        item = *it;
        ++it;
        if ( item != 0 ) {
            if ( d->listWidget != 0 )
                showItem( item );
            else
                delete item;
        }
    }
    if ( ! d->hiddenItems.isEmpty() )
        kDebug() << __FILE__ << ":" << __LINE__ <<
        "hiddenItems is not empty as it should be. " <<
        d->hiddenItems.count() << " items are still there.\n" << endl;

    d->search = "";
    d->queuedSearches = 0;
    KLineEdit::clear();
}

void KListWidgetSearchLine::setCaseSensitivity( Qt::CaseSensitivity cs )
{
    d->caseSensitivity = cs;
}

void KListWidgetSearchLine::setListWidget( QListWidget *lw )
{
    if ( d->listWidget != 0 )
        disconnect( d->listWidget, SIGNAL( destroyed() ),
                    this, SLOT( listWidgetDeleted() ) );

    d->listWidget = lw;

    if ( lw != 0 ) {
        connect( d->listWidget, SIGNAL( destroyed() ),
                 this, SLOT( listWidgetDeleted() ) );
        setEnabled( true );
    } else
        setEnabled( false );
}

/******************************************************************************
 * Protected Methods                                                          *
 *****************************************************************************/
bool KListWidgetSearchLine::itemMatches( const QListWidgetItem *item,
        const QString &s ) const
{
    if ( s.isEmpty() )
        return true;

    if ( item == 0 )
        return false;

    return ( item->text().indexOf( s, 0,
                                   caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive ) >= 0 );
}

void KListWidgetSearchLine::init( QListWidget *listWidget )
{
    delete d;
    d = new KListWidgetSearchLinePrivate(this);

    d->listWidget = listWidget;

    connect( this, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( queueSearch( const QString & ) ) );

    if ( listWidget != 0 ) {
        connect( listWidget, SIGNAL( destroyed() ),
                 this, SLOT( listWidgetDeleted() ) );
        setEnabled( true );
    } else {
        setEnabled( false );
    }

    setClearButtonShown(true);
}

void KListWidgetSearchLine::hideItem( QListWidgetItem *item )
{
    if ( ( item == 0 ) || ( d->listWidget == 0 ) )
        return ;

    d->hiddenItems.append( item );
    d->listWidget->takeItem( d->listWidget->row( item ) );
}

void KListWidgetSearchLine::showItem( QListWidgetItem *item )
{
    if ( d->listWidget == 0 ) {
        kDebug() << __FILE__ << ":" << __LINE__ <<
        "showItem() could not be called while there's no listWidget set." <<
        endl;
        return ;
    }
    d->listWidget->addItem( item );
    d->hiddenItems.removeAll( item );
}

/******************************************************************************
 * Protected Slots                                                            *
 *****************************************************************************/
void KListWidgetSearchLine::queueSearch( const QString &s )
{
    d->queuedSearches++;
    d->search = s;
    QTimer::singleShot( 200, this, SLOT( activateSearch() ) );
}

void KListWidgetSearchLine::activateSearch()
{
    d->queuedSearches--;

    if ( d->queuedSearches <= 0 ) {
        updateSearch( d->search );
        d->queuedSearches = 0;
    }
}

/******************************************************************************
 * Private Slots                                                              *
 *****************************************************************************/
void KListWidgetSearchLine::KListWidgetSearchLinePrivate::listWidgetDeleted()
{
    listWidget = 0;
    lineEdit->setEnabled( false );
}

#include "klistwidgetsearchline.moc"
