/*
    knewstuff3/ui/itemsmodel.h.
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

#ifndef KNEWSTUFF3_UI_ITEMSMODEL_H
#define KNEWSTUFF3_UI_ITEMSMODEL_H

#include <QAbstractListModel>
#include <QtGui/QImage>

#include "core/entryinternal.h"

class KJob;

namespace KNS3
{
    class Engine;

class ItemsModel: public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ItemsModel(Engine* engine, QObject * parent = 0);
    ~ItemsModel();

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    void addEntry(const EntryInternal& entry);
    void removeEntry(const EntryInternal& entry);

    bool hasPreviewImages() const;
    bool hasWebService() const;

Q_SIGNALS:
    void jobStarted(KJob*, const QString& label);

public Q_SLOTS:
    void slotEntryChanged(const KNS3::EntryInternal& entry);
    void slotEntriesLoaded(KNS3::EntryInternal::List entries);
    void clearEntries();
    void slotEntryPreviewLoaded(const KNS3::EntryInternal& entry, KNS3::EntryInternal::PreviewType type);

private:
    Engine* m_engine;
    // the list of entries
    QList<EntryInternal> m_entries;
    bool m_hasPreviewImages;
};

} // end KNS namespace

Q_DECLARE_METATYPE(KNS3::EntryInternal)

#endif
