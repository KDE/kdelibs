/* This file is part of the KDE libraries
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "ktoolbarspaceraction.h"

#include <QToolBar>

class KToolBarSpacerAction::Private
{
  public:
    Private()
      : width( 0 ),
        minimumWidth( -1 ),
        maximumWidth( -1 )
    {
    }

    void _k_spacerDestroyed( QObject* spacer )
    {
      spacers.removeAll( static_cast<QWidget*>( spacer ) );
    }

    int width, minimumWidth, maximumWidth;
    QList<QWidget*> spacers;
};

KToolBarSpacerAction::KToolBarSpacerAction(QObject *parent)
  : KAction(parent),
    d( new Private )
{
}

KToolBarSpacerAction::~KToolBarSpacerAction()
{
    delete d;
}

int KToolBarSpacerAction::width() const
{
  return d->width;
}

void KToolBarSpacerAction::setWidth( int width )
{
  if ( d->width == width )
    return;

  d->width = width;

  foreach ( QWidget* spacer, d->spacers )
    spacer->resize( width, spacer->height() );
}

int KToolBarSpacerAction::minimumWidth() const
{
  return d->minimumWidth;
}

void KToolBarSpacerAction::setMinimumWidth( int width )
{
  if ( d->minimumWidth == width )
    return;

  d->minimumWidth = width;

  foreach ( QWidget* spacer, d->spacers )
    spacer->setMinimumWidth( width );
}

int KToolBarSpacerAction::maximumWidth( ) const
{
  return d->maximumWidth;
}

void KToolBarSpacerAction::setMaximumWidth( int width )
{
  if ( d->maximumWidth == width )
    return;

  d->maximumWidth = width;

  foreach ( QWidget* spacer, d->spacers )
    spacer->setMaximumWidth( width );
}

QWidget * KToolBarSpacerAction::createWidget( QWidget * _parent )
{
  QToolBar *parent = qobject_cast<QToolBar *>(_parent);
  if (!_parent)
    return KAction::createWidget(_parent);
  QWidget* spacer = new QWidget( parent );
  spacer->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );

  d->spacers.append( spacer );
  connect( spacer, SIGNAL( destroyed( QObject* ) ),
           SLOT( _k_spacerDestroyed( QObject* ) ) );

  return spacer;
}

void KToolBarSpacerAction::deleteWidget(QWidget *widget)
{
    d->spacers.removeAll(widget);
    KAction::deleteWidget(widget);
}

#include "ktoolbarspaceraction.moc"
