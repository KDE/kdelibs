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

#include "itemsmodel.h"

#include <kdebug.h>

namespace KNS
{
ItemsModel::ItemsModel(QObject * parent, bool hasWebService)
        : QAbstractListModel(parent), m_hasPreviewImages(false), m_hasWebService(hasWebService)
{
}

ItemsModel::~ItemsModel()
{
}

int ItemsModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_entries.count();
}

QVariant ItemsModel::data(const QModelIndex & index, int role) const
{
    Entry * entry = m_entries[index.row()];
    switch (role) {
    case Qt::DisplayRole:
    case kNameRole:
        return entry->name().representation();
        break;
    case kCategory:
        return entry->category();
        break;
    case kAuthorName:
        return entry->author().name();
        break;
    case kAuthorEmail:
        return entry->author().email();
        break;
    case kAuthorJabber:
        return entry->author().jabber();
        break;
    case kAuthorHomepage:
        return entry->author().homepage();
        break;
    case kLicense:
        return entry->license();
        break;
        //case Qt::ToolTipRole:
    case kSummary:
        return entry->summary().representation();
        break;
    case kVersion:
        return entry->version();
        break;
    case kRelease:
        return entry->release();
        break;
    case kReleaseDate:
        return entry->releaseDate();
        break;
    case kPayload:
        return entry->payload().representation();
        break;
    case kPreview:
        return entry->preview().representation();
        break;
    case kPreviewPixmap:
        if (m_previewImages.contains(entry->preview().representation())) {
            return m_previewImages[entry->preview().representation()];
        }
        break;
    case kLargePreviewPixmap:
        if (m_largePreviewImages.contains(entry->preview().representation())) {
            return m_largePreviewImages[entry->preview().representation()];
        }
        break;
    case kRating:
        return entry->rating();
        break;
    case kDownloads:
        return entry->downloads();
        break;
    case kStatus:
        return entry->status();
        break;
    }
    return QVariant();
}

KNS::Entry* ItemsModel::entryForIndex(const QModelIndex & index) const
{
    if (index.row() < 0)
        return 0;
    else
        return m_entries[index.row()];
}

void ItemsModel::addEntry(Entry * entry)
{
    //kDebug(551) << "adding entry " << entry->name().representation() << " to the model";
    beginInsertRows(QModelIndex(), m_entries.count(), m_entries.count());
    m_entries.append(entry);
    endInsertRows();

    QString preview = entry->preview().representation();
    if (!preview.isEmpty()) {
        m_hasPreviewImages = true;
        m_imageIndexes.insert(preview, index(m_entries.count() - 1, 0));
        QAsyncPixmap *pix = new QAsyncPixmap(preview, this);
        connect(pix, SIGNAL(signalLoaded(const QString &, const QPixmap&)),
                this, SLOT(slotEntryPreviewLoaded(const QString &, const QPixmap&)));
    }
}

void ItemsModel::removeEntry(Entry * entry)
{
    kDebug(551) << "removing entry " << entry->name().representation() << " from the model";
    int index = m_entries.indexOf(entry);
    if (index > -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_entries.removeAt(index);
        endRemoveRows();
    }
}

void ItemsModel::slotEntryChanged(Entry * entry)
{
    int i = m_entries.indexOf(entry);
    QModelIndex entryIndex = index(i, 0);
    emit dataChanged(entryIndex, entryIndex);
}

void ItemsModel::slotEntryPreviewLoaded(const QString &url, const QPixmap & pix)
{
    if( pix.isNull())
        return;
    QImage image = pix.toImage();
    m_largePreviewImages.insert(url, image);
    m_previewImages.insert(url, image.scaled(64, 64, Qt::KeepAspectRatio));

    QModelIndex thisIndex = m_imageIndexes[url];

    emit dataChanged(thisIndex, thisIndex);
}

bool ItemsModel::hasPreviewImages() const
{
    return m_hasPreviewImages;
}

bool ItemsModel::hasWebService() const
{
    return m_hasWebService;
}

} // end KNS namespace
