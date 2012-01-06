/****************************************************************************
**
** This file is part of QMime
**
** Based on Qt Creator source code
**
** Qt Creator Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#ifndef QMIMEDATABASE_H_INCLUDED
#define QMIMEDATABASE_H_INCLUDED

#include "qmime_global.h"

#include "qmimetype.h"

#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QByteArray;
class QFileInfo;
class QIODevice;
class QUrl;

struct QMimeDatabasePrivate;
class QMIME_EXPORT QMimeDatabase
{
    Q_DISABLE_COPY(QMimeDatabase)

public:
    QMimeDatabase();
    ~QMimeDatabase();

    QMimeType mimeTypeForName(const QString &nameOrAlias) const;

    QMimeType findByName(const QString &fileName) const;
    QList<QMimeType> findMimeTypesByFileName(const QString &fileName) const;

    QMimeType findByData(const QByteArray &data) const;
    QMimeType findByData(QIODevice *device) const;

    QMimeType findByFile(const QString &fileName) const;
    QMimeType findByFile(const QFileInfo &fileInfo) const;
    QMimeType findByUrl(const QUrl &url) const;
    QMimeType findByNameAndData(const QString &fileName, QIODevice *device) const;
    QMimeType findByNameAndData(const QString &fileName, const QByteArray &data) const;

    QString suffixForFileName(const QString &fileName) const;

    QList<QMimeType> allMimeTypes() const;

#if 0
    // This must be a huge list, why would anyone ever want this?
    QStringList filterStrings() const;
    QString allFiltersString(QString *allFilesFilter = 0) const;
#endif

    QMimeDatabasePrivate *data_ptr() { return d; }

private:
    QMimeDatabasePrivate *d;
};

QT_END_NAMESPACE

#endif   // QMIMEDATABASE_H_INCLUDED
