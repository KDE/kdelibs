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

class KToolbarSpacerActionPrivate
{
  public:
    KToolbarSpacerActionPrivate()
      : width(0)
      , minimumWidth(-1)
      , maximumWidth(-1)
    {
    }

    int width, minimumWidth, maximumWidth;
    QList<QWidget*> spacers;
};

KToolbarSpacerAction::KToolbarSpacerAction(KActionCollection* parent, const char* name)
  : KAction(parent, name)
  , d(new KToolbarSpacerActionPrivate)
{
  setToolBarWidgetFactory(this);
}

int KToolbarSpacerAction::width( )
{
  return d->width;
}

void KToolbarSpacerAction::setWidth( int width )
{
  if (d->width == width)
    return;

  d->width = width;

  foreach (QWidget* spacer, d->spacers)
    spacer->resize(width, spacer->height());
}

int KToolbarSpacerAction::minimumWidth( ) const
{
  return d->minimumWidth;
}

void KToolbarSpacerAction::setMinimumWidth( int width )
{
  if (d->minimumWidth == width)
    return;

  d->minimumWidth = width;

  foreach (QWidget* spacer, d->spacers)
    spacer->setMinimumWidth(width);
}

int KToolbarSpacerAction::maximumWidth( ) const
{
  return d->maximumWidth;
}

void KToolbarSpacerAction::setMaximumWidth( int width )
{
  if (d->maximumWidth == width)
    return;

  d->maximumWidth = width;

  foreach (QWidget* spacer, d->spacers)
    spacer->setMaximumWidth(width);
}

QWidget * KToolbarSpacerAction::createToolBarWidget( QToolBar * parent )
{
  QWidget* spacer = new QWidget(parent);
  spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

  d->spacers.append(spacer);
  connect(spacer, SIGNAL(destroyed(QObject*)), SLOT(spacerDestroyed(QObject*)));

  return spacer;
}

void KToolbarSpacerAction::spacerDestroyed( QObject * spacer )
{
  d->spacers.removeAll(static_cast<QWidget*>(spacer));
}

#include "ktoolbarspaceraction.moc"
