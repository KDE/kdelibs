/*****************************************************************************
 * Copyright (C) 2011 by Peter Penz <peter.penz19@gmail.com>                 *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#ifndef KFILEMETADATAREADER_H
#define KFILEMETADATAREADER_H

#include <nepomuk/core/variant.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QUrl>

/**
 * @brief Provides metadata extracted from files.
 *
 * The reading of the metadata is done asynchronously in a process.
 * This assures that the caller won't get blocked and also prevents
 * that the caller crashes in case if a metadata-analyzer plugin is instable.
 *
 * @since 4.7
 * @internal
 */
class KFileMetaDataReader : public QObject
{
    Q_OBJECT

public:
    /**
     * @param urls   List of files where the metadata should be extracted from.
     * @param parent Parent object.
     */
    explicit KFileMetaDataReader(const QList<QUrl>& urls, QObject* parent = 0);
    virtual ~KFileMetaDataReader();

    /**
     * If \p read is set to true also metadata that is persisted outside the
     * files itself (like e.g. rating, comments or tags) are read. Per
     * default the reading of context data is enabled. Pass false if only the metadata
     * persisted inside the file should be read.
     */
    void setReadContextData(bool read);
    bool readContextData() const;

    /**
     * Starts the reading of the metadata inside a custom process.
     * The signal finished() will get emitted if the reading has been finished.
     * Use metaData() to access the read metadata.
     */
    void start();

    /**
     * @return The read metadata of the given files. The method provides valid values
     *         after the signal finished() has been emitted. If it is invoked before
     *         an empty hash-table will be returned.
     */
    QHash<QUrl, Nepomuk::Variant> metaData() const;

Q_SIGNALS:
    /**
     * Is emitted if the reading of the metadata inside a custom process has been finished.
     * The method metaData() can be used afterwards to access the metadata.
     */
    void finished();

private:
    class Private;
    Private* d;

    Q_PRIVATE_SLOT(d, void slotLoadingFinished(int, QProcess::ExitStatus))
};

#endif
