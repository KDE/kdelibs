/*
    knewstuff3/ui/itemsmodel.cpp.
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

#include <KDebug>
#include <KLocalizedString>

#include "entry.h"

#include "imageloader.h"

namespace KNS3
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
    Entry entry = m_entries[index.row()];
    switch (role) {
    case Qt::DisplayRole:
    case kNameRole:
        return entry.name();
        break;
    case kCategory:
        return entry.category();
        break;
    case kHomepage:
        kDebug() << "HP: " << entry.homepage().url();
        return QUrl(entry.homepage());
        break;
    case kAuthorName:
        return entry.author().name();
        break;
    case kAuthorEmail:
        return entry.author().email();
        break;
    case kAuthorJabber:
        return entry.author().jabber();
        break;
    case kAuthorHomepage:
        return entry.author().homepage();
        break;
    case kLicense:
        return entry.license();
        break;
        //case Qt::ToolTipRole:
    case kSummary:
        return entry.summary();
        break;
    case kVersion:
        return entry.version();
        break;
    case kReleaseDate:
        return entry.releaseDate();
        break;
    case kPayload:
        return entry.payload();
        break;
    case kPreview:
        return entry.previewSmall();
        break;
    case kPreviewPixmap:
        if (m_previewImages.contains(entry.previewSmall())) {
            return m_previewImages[entry.previewSmall()];
        }
        break;
    case kLargePreviewPixmap:
        if (m_largePreviewImages.contains(entry.previewBig())) {
            return m_largePreviewImages[entry.previewBig()];
        }
        break;
    case kRating:
        return entry.rating();
        break;
    case kDownloads:
        return entry.downloads();
        break;
    case kStatus:
        return entry.status();
        break;
    }
    return QVariant();
}

KNS3::Entry ItemsModel::entryForIndex(const QModelIndex & index) const
{
    if (index.row() < 0)
        return Entry();
    else
        return m_entries[index.row()];
}

void ItemsModel::slotEntriesLoaded(Entry::List entries)
{
    foreach(KNS3::Entry entry, entries) {
        addEntry(entry);
    }
}

void ItemsModel::addEntry(const Entry& entry)
{
    QString preview = entry.previewSmall();
    if (!preview.isEmpty()) {
        m_hasPreviewImages = true;
    }

    //kDebug(551) << "adding entry " << entry.name() << " to the model";
    beginInsertRows(QModelIndex(), m_entries.count(), m_entries.count());
    m_entries.append(entry);
    endInsertRows();

    if (!preview.isEmpty()) {
        m_imageIndexes.insert(preview, index(m_entries.count() - 1, 0));
        ImageLoader *pix = new ImageLoader(preview, this);
        emit jobStarted(pix->job(), i18n("Loading preview..."));
        connect(pix, SIGNAL(signalLoaded(const QString &, const QImage&)),
                this, SLOT(slotEntryPreviewLoaded(const QString &, const QImage&)));
    }
}

void ItemsModel::removeEntry(const Entry& entry)
{
    kDebug(551) << "removing entry " << entry.name() << " from the model";
    int index = m_entries.indexOf(entry);
    if (index > -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_entries.removeAt(index);
        endRemoveRows();
    }
}

void ItemsModel::slotEntryChanged(const Entry& entry)
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

void ItemsModel::slotEntryPreviewLoaded(const QString &url, const QImage & pix)
{
    if( pix.isNull())
        return;
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
