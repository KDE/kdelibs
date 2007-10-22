/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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
#ifndef KFILEPLACESVIEW_H
#define KFILEPLACESVIEW_H

#include <kfile_export.h>

#include <QtGui/QListView>

#include <kurl.h>

class QResizeEvent;
class QContextMenuEvent;

/**
 * This class allows to display a KFilePlacesModel.
 */
class KFILE_EXPORT KFilePlacesView : public QListView
{
    Q_OBJECT
public:
    KFilePlacesView(QWidget *parent = 0);
    ~KFilePlacesView();

public Q_SLOTS:
    void setUrl(const KUrl &url);
    void setShowAll(bool showAll);
    virtual QSize sizeHint() const;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);

protected Q_SLOTS:
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

Q_SIGNALS:
    void urlChanged(const KUrl &url);

private:
    Q_PRIVATE_SLOT(d, void _k_placeClicked(const QModelIndex &))
    Q_PRIVATE_SLOT(d, void _k_placeActivated(const QModelIndex &))
    Q_PRIVATE_SLOT(d, void _k_storageSetupDone(const QModelIndex &, bool))
    Q_PRIVATE_SLOT(d, void _k_adaptItemsUpdate(qreal))
    Q_PRIVATE_SLOT(d, void _k_itemAppearUpdate(qreal))
    Q_PRIVATE_SLOT(d, void _k_itemDisappearUpdate(qreal))
    Q_PRIVATE_SLOT(d, void _k_enableSmoothItemResizing())

    class Private;
    Private * const d;
    friend class Private;
};

#endif
