/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Dario Freddi <drf@kde.org>
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

        QMap< QString, QString > attributes() const;
        QString label() const;

        QDateTime created() const;
        QDateTime lastModified() const;

        QByteArray secret() const;

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

    KJob *writeEntry(const QString &label, const QByteArray &secret, const QVariantMap &attributes = QVariantMap());
    ReadEntryJob *readEntry(const QString &label);
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
