/* This file is part of the KDE libraries
    Copyright (C) 2004 Felix Berger <felixberger@beldesign.de>

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

#include <QtCore/QPointer>
#include <QtGui/QApplication>
#include <QtGui/QLabel>

#include "ktoolbar.h"
#include "ktoolbarlabelaction.h"

class KToolBarLabelAction::Private
{
  public:
    QPointer<QAction> buddy;
    QString oldText;
};

KToolBarLabelAction::KToolBarLabelAction( const QString &text,
                                          KActionCollection *parent,
                                          const QString& name )
  : KAction( text, parent, name ),
    d( new Private )
{
  setToolBarWidgetFactory( this );
  d->oldText = KToolBarLabelAction::text();
}

KToolBarLabelAction::KToolBarLabelAction( QAction* buddy, const QString &text,
                                          KActionCollection *parent, const QString& name )
  : KAction( text, parent, name ),
    d( new Private )
{
  setToolBarWidgetFactory( this );

  setBuddy( buddy );

  d->oldText = KToolBarLabelAction::text();
}

KToolBarLabelAction::~KToolBarLabelAction()
{
  delete d;
}

void KToolBarLabelAction::setBuddy( QAction* buddy )
{
  d->buddy = buddy;

  QList<QLabel*> labels;
  foreach ( QWidget* widget, associatedWidgets() )
    if ( QToolBar* toolBar = qobject_cast<QToolBar*>( widget ) )
      if ( QLabel* label = qobject_cast<QLabel*>( toolBar->widgetForAction( this ) ) )
        labels.append( label );

  foreach ( QWidget* widget, buddy->associatedWidgets() )
    if ( QToolBar* toolBar = qobject_cast<QToolBar*>( widget ) ) {
      QWidget* newBuddy = toolBar->widgetForAction( buddy );
      foreach ( QLabel* label, labels )
        label->setBuddy( newBuddy );
      return;
    }
}

QAction* KToolBarLabelAction::buddy() const
{
  return d->buddy;
}

bool KToolBarLabelAction::event( QEvent *event )
{
  if ( event->type() == QEvent::ActionChanged ) {
    if ( text() != d->oldText ) {
      emit textChanged( text() );
      d->oldText = text();
    }
  }

  return KAction::event( event );
}

QWidget *KToolBarLabelAction::createToolBarWidget( QToolBar* parent )
{
  QLabel* newLabel = new QLabel( parent );

  /**
   * These lines were copied from Konqueror's KonqDraggableLabel class in
   * konq_misc.cc
   */
  newLabel->setBackgroundRole( QPalette::Button );
  newLabel->setAlignment( (QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft) |
                          Qt::AlignVCenter );
  newLabel->adjustSize();

  if ( d->buddy )
    foreach ( QWidget* widget, d->buddy->associatedWidgets() )
      if ( QToolBar* toolBar = qobject_cast<QToolBar*>( widget ) ) {
        QWidget* newBuddy = toolBar->widgetForAction( d->buddy );
        newLabel->setBuddy( newBuddy );
        break;
      }

  return newLabel;
}

#include "ktoolbarlabelaction.moc"
