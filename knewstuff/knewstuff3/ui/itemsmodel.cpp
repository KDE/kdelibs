/*
    knewstuff3/ui/itemsmodel.cpp.
    Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>

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

#include "kdebug.h"
#include "klocalizedstring.h"

#include "core/entryinternal.h"
#include "core/engine.h"
#include "imageloader.h"

namespace KNS3
{
ItemsModel::ItemsModel(Engine* engine, QObject* parent)
        : QAbstractListModel(parent)
        , m_engine(engine)
        , m_hasPreviewImages(false)
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
    if (role != Qt::UserRole) {
        return QVariant();
    }
    EntryInternal entry = m_entries[index.row()];
    return QVariant::fromValue(entry);
}

void ItemsModel::slotEntriesLoaded(EntryInternal::List entries)
{
    foreach(const KNS3::EntryInternal &entry, entries) {
        addEntry(entry);
    }
}

void ItemsModel::addEntry(const EntryInternal& entry)
{
    QString preview = entry.previewUrl(EntryInternal::PreviewSmall1);
    if (!m_hasPreviewImages && !preview.isEmpty()) {
        m_hasPreviewImages = true;
        if (rowCount() > 0) {
            emit dataChanged(index(0,0), index(rowCount()-1,0));
        }
    }

    //kDebug(551) << "adding entry " << entry.name() << " to the model";
    beginInsertRows(QModelIndex(), m_entries.count(), m_entries.count());
    m_entries.append(entry);
    endInsertRows();

    if (!preview.isEmpty() && entry.previewImage(EntryInternal::PreviewSmall1).isNull()) {
        m_engine->loadPreview(entry, EntryInternal::PreviewSmall1);
    }
}

void ItemsModel::removeEntry(const EntryInternal& entry)
{
    kDebug(551) << "removing entry " << entry.name() << " from the model";
    int index = m_entries.indexOf(entry);
    if (index > -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_entries.removeAt(index);
        endRemoveRows();
    }
}

void ItemsModel::slotEntryChanged(const EntryInternal& entry)
{
    int i = m_entries.indexOf(entry);
    QModelIndex entryIndex = index(i, 0);
    emit dataChanged(entryIndex, entryIndex);
}

void ItemsModel::clearEntries()
{
    m_entries.clear();
    reset();
}

void ItemsModel::slotEntryPreviewLoaded(const EntryInternal& entry, EntryInternal::PreviewType type)
{
    // we only care about the first small preview in the list
    if (type != EntryInternal::PreviewSmall1) {
        return;
    }
    slotEntryChanged(entry);
}

/*
void ItemsModel::slotEntryPreviewLoaded(const QString &url, const QImage & pix)
{
    if (pix.isNull()) {
        return;
    }

    QImage image = pix;
    m_largePreviewImages.insert(url, image);
    if (image.width() > PreviewWidth || image.height() > PreviewHeight) {
        // if the preview is really big, first scale fast to a smaller size, then smooth to desired size
        if (image.width() > 4 * PreviewWidth || image.height() > 4 * PreviewHeight) {
            image = image.scaled(2 * PreviewWidth, 2 * PreviewHeight, Qt::KeepAspectRatio, Qt::FastTransformation);
        }
        m_previewImages.insert(url, image.scaled(PreviewWidth, PreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else if (image.width() <= PreviewWidth / 2 && image.height() <= PreviewHeight / 2) {
        // upscale tiny previews to double size
        m_previewImages.insert(url, image.scaled(2 * image.width(), 2 * image.height()));
    } else {
        m_previewImages.insert(url, image);
    }

    QModelIndex thisIndex = m_imageIndexes[url];

    emit dataChanged(thisIndex, thisIndex);
}*/

bool ItemsModel::hasPreviewImages() const
{
    return m_hasPreviewImages;
}

} // end KNS namespace

#include "itemsmodel.moc"
