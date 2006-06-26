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

#ifndef KTOOLBARSPACERACTION_H
#define KTOOLBARSPACERACTION_H

#include "kaction.h"

/**
 * An extension to KAction which allows for adding a spacer item into a
 * QToolBar / KToolBar.
 *
 * Use this as the replacement for the old KToolBar::alignItemRight().
 *
 * @author Hamish Rodda <rodda@kde.org>
 */
class KDEUI_EXPORT KToolBarSpacerAction : public KAction, public QActionWidgetFactory
{
  Q_OBJECT
  Q_PROPERTY( int width READ width WRITE setWidth )
  Q_PROPERTY( int minimumWidth READ minimumWidth WRITE setMinimumWidth )
  Q_PROPERTY( int maximumWidth READ maximumWidth WRITE setMaximumWidth )

  public:
    /**
     * Creates a new toolbar spacer action.
     *
     * @param parent The parent @see KActionCollection of the action.
     * @param name The name of the action.
     */
    explicit KToolBarSpacerAction( KActionCollection* parent, const QString& name = QString() );

    /**
     * Returns the width of the spacer item.
     */
    int width() const;

    /**
     * Sets the @param width of the spacer item.
     */
    void setWidth( int width );

    /**
     * Returns the minimum width of the spacer item.
     */
    int minimumWidth() const;

    /**
     * Sets the minimum @param width of the spacer item.
     */
    void setMinimumWidth( int width );

    /**
     * Returns the maximum width of the spacer item.
     */
    int maximumWidth() const;

    /**
     * Sets the maximum @param width of the spacer item.
     */
    void setMaximumWidth( int width );

    /**
     * Reimplemented from @see QActionWidgetFactory.
     */
    virtual QWidget* createToolBarWidget( QToolBar* parent );


  private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY( KToolBarSpacerAction )

    Q_PRIVATE_SLOT( d, void spacerDestroyed( QObject* ) )
};

#endif
