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

#ifndef KSECRETSERVICE_LIB_H
#define KSECRETSERVICE_LIB_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

class KJob;
class KSecretServiceCollection;
class KSecretService : public QObject
{
    Q_OBJECT

public:
    static KSecretService *instance();

    virtual ~KSecretService();

    void init();

    QStringList collections() const;
    KSecretServiceCollection *openCollection(const QString &label);

    KJob *createCollection(const QString &label, const QVariantMap &properties = QVariantMap());
    KJob *deleteCollection(const QString &label);

private:
    KSecretService(QObject *parent = 0);

    class Private;
    Private * const d;

    friend class CreateCollectionJob;
    friend class DeleteCollectionJob;
    friend class ReadEntryJob;
    friend class WriteEntryJob;
};

#endif // KSECRETSERVICE_H
