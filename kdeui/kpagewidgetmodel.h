/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Tobias Koenig (tokoe@kde.org)

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

#ifndef KPAGEWIDGETMODEL_H
#define KPAGEWIDGETMODEL_H

#include <QtGui/QIcon>

#include <kdelibs_export.h>

#include "kpagemodel.h"

class QWidget;

class KDEUI_EXPORT KPageWidgetItem
{
  public:
    KPageWidgetItem( QWidget *widget, const QString &name );
    ~KPageWidgetItem();

    QWidget* widget() const;

    QString name() const;

    void setHeader( const QString &header );
    QString header() const;

    void setIcon( const QIcon &icon );
    QIcon icon() const;

    void setIsCheckable( bool checkable );
    bool isCheckable() const;

  private:
    class Private;
    Private* const d;
};

class KDEUI_EXPORT KPageWidgetModel : public KPageModel
{
  public:
    KPageWidgetModel( QObject *parent = 0 );
    ~KPageWidgetModel();

    /**
     * Adds a new top level page to the model.
     *
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see KPageWidgetItem.
     */
    KPageWidgetItem* addPage( QWidget *widget, const QString &name );

    /**
     * Adds a new top level page to the model.
     *
     * @param item The @see KPageWidgetItem which describes the page.
     */
    void addPage( KPageWidgetItem *item );

    /**
     * Inserts a new page in the model.
     *
     * @param before The new page will be insert before this @see KPageWidgetItem
     *               on the same level in hierarchy.
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see KPageWidgetItem.
     */
    KPageWidgetItem* insertPage( KPageWidgetItem *before, QWidget *widget, const QString &name );

    /**
     * Inserts a new page in the model.
     *
     * @param before The new page will be insert before this @see KPageWidgetItem
     *               on the same level in hierarchy.
     *
     * @param item The @see KPageWidgetItem which describes the page.
     */
    void insertPage( KPageWidgetItem *before, KPageWidgetItem *item );

    /**
     * Inserts a new sub page in the model.
     *
     * @param parent The new page will be insert as child of this @see KPageWidgetItem.
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated @see KPageWidgetItem.
     */
    KPageWidgetItem* addSubPage( KPageWidgetItem *parent, QWidget *widget, const QString &name );

    /**
     * Inserts a new sub page in the model.
     *
     * @param parent The new page will be insert as child of this @see KPageWidgetItem.
     *
     * @param item The @see KPageWidgetItem which describes the page.
     */
    void addSubPage( KPageWidgetItem *parent, KPageWidgetItem *item );

    /**
     * Removes the page associated with the given @see KPageWidgetItem.
     */
    void removePage( KPageWidgetItem *item );

    /**
     * These methods are reimplemented from QAsbtractItemModel.
     */
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex &index ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * Returns the @see KPageWidgetItem for a given index or 0 if the index is invalid.
     */
    KPageWidgetItem* item( const QModelIndex &index );

    /**
     * Returns the index for a given @see KPageWidgetItem. The index is invalid if the
     * item can't be found in the model.
     */
    QModelIndex index( const KPageWidgetItem *item ) const;

  private:
    class Private;
    Private* const d;
};

#endif
