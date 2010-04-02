/*
    Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_UI_ITEMSVIEWBASEDELEGATE_H
#define KNEWSTUFF3_UI_ITEMSVIEWBASEDELEGATE_H

#include <QList>
#include <QtCore/QMap>
#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QtGui/QImage>

#include "core/engine.h"
#include "core/entryinternal.h"

#include <kicon.h>
#include <kwidgetitemdelegate.h>

namespace KNS3
{
    class ItemsModel;
    class Engine;    
    
class ItemsViewBaseDelegate: public KWidgetItemDelegate
{
    Q_OBJECT
public:
    explicit ItemsViewBaseDelegate(QAbstractItemView* itemView, Engine* engine, QObject* parent = 0);
    virtual ~ItemsViewBaseDelegate();
    // paint the item at index with all its attributes shown
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const = 0;

    // get the list of widgets
    virtual QList<QWidget*> createItemWidgets() const = 0;

    // update the widgets
    virtual void updateItemWidgets(const QList<QWidget*> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const =0;

    virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const = 0;
    
Q_SIGNALS:
    void signalShowDetails(const KNS3::EntryInternal& entry);
    
protected slots:
    bool eventFilter(QObject *watched, QEvent *event);
    void slotInstallClicked();
    void slotInstallActionTriggered(QAction* action);
    void slotLinkClicked(const QString & url);
    void slotDetailsClicked(const QModelIndex& index);
    void slotDetailsClicked();

protected:
    Engine* m_engine;
    KIcon m_iconInvalid;
    KIcon m_iconDownloadable;
    KIcon m_iconInstall;
    KIcon m_iconUpdate;
    KIcon m_iconDelete;
    QPixmap m_frameImage;
    QPixmap m_noImage;
    QSize m_buttonSize;
};
}
#endif
