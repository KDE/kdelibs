/* This file is part of the KDE libraries

   Copyright (c) 2007 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KFILEWRITEPLUGIN_H
#define KFILEWRITEPLUGIN_H

#include <kio/kio_export.h>
#include <QtCore/QVariant>

class KUrl;

class KIO_EXPORT KFileWritePlugin : public QObject {
    Q_OBJECT
    friend class KFileWriterProvider;
public:
    /**
     * @brief Constructor that initializes the object as a QObject.
     **/
    KFileWritePlugin(QObject* parent, const QStringList& args);
    /**
     * @brief Destructor
     **/
    virtual ~KFileWritePlugin();
    /**
     * @brief determine if this plugin can write a value into a particular
     * resource.
     **/
    virtual bool canWrite(const KUrl& file, const QString& key) = 0;
    /**
     * @brief Write a set of values into a resource pointed to by @p file.
     **/
    virtual bool write(const KUrl& file, const QVariantMap& data) = 0;

private:
    class Private;
    Private* const d;
};

#endif
