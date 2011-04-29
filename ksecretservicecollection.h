/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Dario Freddi <drf@kde.org>
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef KSECRETSERVICECOLLECTION_H
#define KSECRETSERVICECOLLECTION_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QSharedDataPointer>

#include <kjob.h>

#include "../daemon/frontend/secret/adaptors/dbustypes.h"

class KSecretService;
class ReadEntryJob;
class KSecretServiceCollection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KSecretServiceCollection)
public:
    class Entry {
    public:
        Entry();
        Entry(const Entry &other);
        ~Entry();

        QVariantMap properties() const;
        StringStringMap attributes() const;
        QString label() const;

        QDateTime created() const;
        QDateTime lastModified() const;

        QByteArray secret() const;
        QString contentType() const;

    private:
        Entry(const QString &path, const QString &session);

        class Data;
        QSharedDataPointer<Data> d;

        friend class ReadEntryJob;
    };

    KSecretServiceCollection(const QString &path, KSecretService* parent);
    virtual ~KSecretServiceCollection();

    QString label() const;

    QDateTime created() const;
    QDateTime lastModified() const;

    QStringList entries() const;

    KJob *writeEntryAsync(const QByteArray &secret, const QVariantMap &properties);
    
    /**
     * This convenience overload synchronously writes an entry into this collection.
     * @param entry reference to an Entry instance describing the data to be written into the collection
     * @return true if the entry was successfully written
     */
    bool writeEntry(const Entry &entry );
    
    ReadEntryJob *readEntry(const QString &label);
    
    /**
     * This method will synchrounously read the specified entry into the referenced instance.
     * @param label is the to be read entry's label
     * @param entry is the reference where the entry must be read
     * @return true if the entry was correctly read
     */
    bool readEntry(const QString &label, Entry &entry);
    
    /**
     * Asynchrounously delete and entry from the collection
     * @param label is the entry's label
     * @return true if the entry has been successfully deleted
     */
    KJob *deleteEntry(const QString &label);
    
private:
    class Private;
    Private * const d;

    friend class ReadEntryJob;
    friend class WriteEntryJob;
    friend class SearchEntriesJob;
    friend class DeleteEntryJob;
};

class ReadEntryJob : public KJob
{
    Q_OBJECT

public:
    virtual ~ReadEntryJob();

    virtual void start();

    KSecretServiceCollection::Entry entry() const;

private:
    explicit ReadEntryJob(const QString &label, KSecretServiceCollection* parent);

    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void run())
    Q_PRIVATE_SLOT(d, void onSearchFinished(KJob*))

    friend class KSecretServiceCollection;
};

#endif // KSECRETSERVICECOLLECTION_H
