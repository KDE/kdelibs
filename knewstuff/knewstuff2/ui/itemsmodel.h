/***************************************************************************
 *   Copyright (C) 2008 Jeremy Whiting <jeremy@scitools.com>               *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 ***************************************************************************/

#ifndef KNEWSTUFF2_UI_ITEMSMODEL_H
#define KNEWSTUFF2_UI_ITEMSMODEL_H

#include <QAbstractListModel>
#include <QtGui/QImage>

#include <knewstuff2/core/entry.h>
#include <knewstuff2/ui/qasyncpixmap.h>

namespace KNS
{

class ItemsModel: public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ItemsModel(QObject * parent = NULL, bool hasWebService = false);
    ~ItemsModel();

    enum EntryRoles {
        /** the name of the entry */
        kNameRole = Qt::UserRole,
        /** the category of the entry */
        kCategory,
        /** the name of the author of the entry */
        kAuthorName,
        /** the e-mail address of the author */
        kAuthorEmail,
        /** the jabber of the author */
        kAuthorJabber,
        /** the authors homepage */
        kAuthorHomepage,
        /** the license of the entry */
        kLicense,
        /** a summary of the entry */
        kSummary,
        /** a string of the version e.g. "v1.0 alpha" */
        kVersion,
        /** the release number */
        kRelease,
        /** the release date */
        kReleaseDate,
        /** the payload */
        kPayload,
        /** the preview url */
        kPreview,
        /** the preview pixmap */
        kPreviewPixmap,
        /** the preview image unscaled */
        kLargePreviewPixmap,
        /** the rating of the entry */
        kRating,
        /** the number of downloads for the entry */
        kDownloads,
        /** the status of this entry */
        kStatus
    };

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    void addEntry(Entry * entry);

    void removeEntry(Entry * entry);

    KNS::Entry* entryForIndex(const QModelIndex & index) const;

    bool hasPreviewImages() const;

    bool hasWebService() const;

public slots:
    void slotEntryChanged(KNS::Entry * entry);
private slots:
    void slotEntryPreviewLoaded(const QString &url, const QPixmap & pix);
private:

    // the list of entries
    QList<Entry *> m_entries;
    QMap<QString, QImage> m_previewImages;
    QMap<QString, QImage> m_largePreviewImages;
    QMap<QString, QModelIndex> m_imageIndexes;
    bool m_hasPreviewImages;
    bool m_hasWebService;
};

} // end KNS namespace

#endif
