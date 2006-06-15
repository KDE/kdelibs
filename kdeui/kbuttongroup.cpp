/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Pino Toscano <toscano.pino@tiscali.it>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kbuttongroup.h"

#include <QChildEvent>
#include <QHash>
#include <QRadioButton>
#include <QSignalMapper>

class KButtonGroup::Private
{
  public:
    Private( KButtonGroup* q )
      : clickedMapper( q ), pressedMapper( q ), releasedMapper( q ),
        currentId( -1 ), nextId( 0 )
    {
      connect( &clickedMapper, SIGNAL( mapped( int ) ), q, SLOT( slotClicked( int ) ) );
      connect( &pressedMapper, SIGNAL( mapped( int ) ), q, SIGNAL( pressed( int ) ) );
      connect( &releasedMapper, SIGNAL( mapped( int ) ), q, SIGNAL( released( int ) ) );
    }

    QSignalMapper clickedMapper;
    QSignalMapper pressedMapper;
    QSignalMapper releasedMapper;

    QHash<QObject*, int> btnMap;
    int currentId;
    int nextId;
};

KButtonGroup::KButtonGroup( QWidget* parent )
  : QGroupBox( parent ), d( new Private( this ) )
{
}

KButtonGroup::~KButtonGroup()
{
  delete d;
}

void KButtonGroup::setSelected( int id )
{
  QHash<QObject*, int>::Iterator it = d->btnMap.begin();
  QHash<QObject*, int>::Iterator itEnd = d->btnMap.end();
  QRadioButton* radio = 0;
  for ( ; it != itEnd; ++it )
  {
    if ( ( it.value() == id ) && ( radio = qobject_cast<QRadioButton*>( it.key() ) ) )
    {
      radio->setChecked( true );
      d->currentId = id;
      emit changed( id );
      return;
    }
  }
}

int KButtonGroup::selected() const
{
  return d->currentId;
}

void KButtonGroup::childEvent( QChildEvent* event )
{
  if ( event->polished() )
  {
    QRadioButton* radio = qobject_cast<QRadioButton*>( event->child() );
    if ( !d->btnMap.contains( event->child() ) && radio )
    {
      connect( radio, SIGNAL( clicked() ), &d->clickedMapper, SLOT( map() ) );
      d->clickedMapper.setMapping( radio, d->nextId );

      connect( radio, SIGNAL( pressed() ), &d->pressedMapper, SLOT( map() ) );
      d->pressedMapper.setMapping( radio, d->nextId );

      connect( radio, SIGNAL( released() ), &d->releasedMapper, SLOT( map() ) );
      d->releasedMapper.setMapping( radio, d->nextId );

      d->btnMap[ radio ] = d->nextId;

      ++d->nextId;
    }
  }
  else if ( event->removed() )
  {
    QObject* obj = event->child();
    QHash<QObject*, int>::Iterator it = d->btnMap.find( obj );
    if ( it != d->btnMap.end() )
    {
      d->clickedMapper.removeMappings( obj );
      d->pressedMapper.removeMappings( obj );
      d->releasedMapper.removeMappings( obj );

      if ( it.value() == d->currentId )
        d->currentId = -1;

      d->btnMap.remove( obj );
    }
  }

  // be transparent
  QGroupBox::childEvent( event );
}

void KButtonGroup::slotClicked( int id )
{
  d->currentId = id;
  emit clicked( id );
  emit changed( id );
}

#include "kbuttongroup.moc"

