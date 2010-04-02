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
#include "itemsviewbasedelegate.h"

#include "itemsmodel.h"
#include "entrydetailsdialog.h"

#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

namespace KNS3
{
ItemsViewBaseDelegate::ItemsViewBaseDelegate(QAbstractItemView *itemView, Engine* engine, QObject * parent)
        : KWidgetItemDelegate(itemView, parent)
        , m_engine(engine)
        , m_iconInvalid(KIcon("dialog-error"))
        , m_iconInstall(KIcon("dialog-ok"))
        , m_iconUpdate(KIcon("system-software-update"))
        , m_iconDelete(KIcon("edit-delete"))
        , m_noImage(SmallIcon( "image-missing", KIconLoader::SizeLarge, KIconLoader::DisabledState ))
{
    QString framefile = KStandardDirs::locate("data", "knewstuff/pics/thumb_frame.png");
    m_frameImage = QPixmap(framefile);   
}

ItemsViewBaseDelegate::~ItemsViewBaseDelegate()
{
}

bool ItemsViewBaseDelegate::eventFilter(QObject *watched, QEvent *event)
{
   if (event->type() == QEvent::MouseButtonDblClick) {
       slotDetailsClicked();
       return true;
   }

   return KWidgetItemDelegate::eventFilter(watched, event);
}

void ItemsViewBaseDelegate::slotLinkClicked(const QString & url)
{
    Q_UNUSED(url)
    QModelIndex index = focusedIndex();
    Q_ASSERT(index.isValid());

    KNS3::EntryInternal entry = index.data(Qt::UserRole).value<KNS3::EntryInternal>();
    m_engine->contactAuthor(entry);
}

void ItemsViewBaseDelegate::slotInstallClicked()
{
    QModelIndex index = focusedIndex();
    if (index.isValid()) {
        KNS3::EntryInternal entry = index.data(Qt::UserRole).value<KNS3::EntryInternal>();
        if (!entry.isValid()) {
            kDebug() << "Invalid entry: " << entry.name();
            return;
        }

        if (entry.status() == Entry::Installed) {
            m_engine->uninstall(entry);
        } else {
            m_engine->install(entry);
        }
    }
}

void ItemsViewBaseDelegate::slotInstallActionTriggered(QAction* action)
{
    QPoint rowDownload = action->data().toPoint();
    int row = rowDownload.x();
    QModelIndex index = focusedIndex().model()->index(row, 0);
    if (index.isValid()) {
        KNS3::EntryInternal entry = index.data(Qt::UserRole).value<KNS3::EntryInternal>();
        m_engine->install(entry, rowDownload.y());
    }
}

void ItemsViewBaseDelegate::slotDetailsClicked()
{
    QModelIndex index = focusedIndex();
    slotDetailsClicked(index);
}

void ItemsViewBaseDelegate::slotDetailsClicked(const QModelIndex& index)
{
    if (index.isValid()) {
        KNS3::EntryInternal entry = index.data(Qt::UserRole).value<KNS3::EntryInternal>();
        if ( !entry.isValid() )
            return;
kDebug() << "Details: " << entry.name();
        emit signalShowDetails(entry);
    }
}
}
