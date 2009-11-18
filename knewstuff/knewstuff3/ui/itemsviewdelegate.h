/*
    This file is part of KNewStuff2.
    Copyright (C) 2008 Jeremy Whiting <jeremy@scitools.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_UI_ITEMSVIEWDELEGATE_H
#define KNEWSTUFF3_UI_ITEMSVIEWDELEGATE_H

#include <kwidgetitemdelegate.h>

#include <QtCore/QMap>
#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QtGui/QImage>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>

#include "core/engine.h"
#include "entry.h"
#include "downloaddialog.h"

#include <kicon.h>
#include <kmenu.h>

namespace KNS3
{
class ItemsViewDelegate: public KWidgetItemDelegate
{
    Q_OBJECT
public:
    explicit ItemsViewDelegate(QAbstractItemView *itemView, QObject * parent = 0);
    ~ItemsViewDelegate();

    // paint the item at index with all its attributes shown
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

    // get the list of widgets
    virtual QList<QWidget*> createItemWidgets() const;

    // update the widgets
    virtual void updateItemWidgets(const QList<QWidget*> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

signals:
    void performAction(KNS3::Engine::EntryAction action, KNS3::Entry entry);

private slots:

    void slotActionTriggered(QAction *action);
    void slotInstallClicked();
    void slotLinkClicked(const QString & url);

private:
    KMenu * InstallMenu(const QToolButton* button, Entry::Status status) const;

    QList<KIcon> m_statusicons;
    QImage m_frameImage;
};
}

#endif
