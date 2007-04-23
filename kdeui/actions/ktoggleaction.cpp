/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#include "ktoggleaction.h"

#include <kdebug.h>
#include <kguiitem.h>
#include <kicon.h>
#include <klocale.h>

class KToggleAction::Private
{
  public:
    Private( KToggleAction *_parent )
      : parent( _parent ), checkedGuiItem( 0L )
    {
    }

    ~Private()
    {
      delete checkedGuiItem;
    }

    void init()
    {
      parent->setCheckable( true );
      connect( parent, SIGNAL( toggled( bool ) ),
               parent, SLOT( slotToggled( bool ) ) );
    }

    KToggleAction* parent;
    KGuiItem* checkedGuiItem;
};


KToggleAction::KToggleAction( QObject *parent )
  : KAction( parent ),
    d( new Private( this ) )
{
  d->init();
}

KToggleAction::KToggleAction( const QString & text, QObject *parent )
  : KAction( text, parent ),
    d( new Private( this ) )
{
  d->init();
}

KToggleAction::KToggleAction( const KIcon & icon, const QString & text, QObject *parent )
  : KAction( icon, text, parent ),
    d( new Private( this ) )
{
  d->init();
}

KToggleAction::~KToggleAction()
{
  delete d;
}


void KToggleAction::setCheckedState( const KGuiItem& checkedItem )
{
  delete d->checkedGuiItem;
  d->checkedGuiItem = new KGuiItem( checkedItem );
}

void KToggleAction::slotToggled( bool )
{
  if ( d->checkedGuiItem ) {
    QString string = d->checkedGuiItem->text();
    d->checkedGuiItem->setText( text() );
    setText( string );

    string = d->checkedGuiItem->toolTip();
    d->checkedGuiItem->setToolTip( toolTip() );
    setToolTip( string );

    if ( d->checkedGuiItem->hasIcon() ) {
      KIcon icon = d->checkedGuiItem->icon();
      d->checkedGuiItem->setIcon( KIcon(this->icon()) );
      QAction::setIcon( icon );
    }
  }
}

#include "ktoggleaction.moc"
