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

#ifndef QMIMEDATABASE_P_H_INCLUDED
#define QMIMEDATABASE_P_H_INCLUDED

#include <QtCore/QMultiHash>
#include <QtCore/QMutex>

#include "qmimetype.h"
#include "qmimetype_p.h"
#include "qmimeglobpattern_p.h"

// ------------------------------------------------------------------------------------------------

QT_BEGIN_NAMESPACE

class QMimeDatabase;
class QMimeProviderBase;

struct QMimeDatabasePrivate
{
    Q_DISABLE_COPY(QMimeDatabasePrivate)

    QMimeDatabasePrivate();
    ~QMimeDatabasePrivate();

    static QMimeDatabasePrivate *instance();

    QMimeProviderBase *provider();
    void setProvider(QMimeProviderBase *theProvider);

    QString defaultMimeType() const { return m_defaultMimeType; }

#if 0
    QStringList filterStrings() const;
#endif

    bool inherits(const QString &mime, const QString &parent);

    QList<QMimeType> allMimeTypes();


    QMimeType mimeTypeForName(const QString &nameOrAlias);
    QMimeType findByNameAndData(const QString &fileName, QIODevice *device, int *priorityPtr);
    QMimeType findByData(const QByteArray &data, int *priorityPtr);
    QStringList findByName(const QString &fileName, QString *foundSuffix = 0);

    mutable QMimeProviderBase *m_provider;
    const QString m_defaultMimeType;
    QMutex mutex;
};

QT_END_NAMESPACE

#endif   // QMIMEDATABASE_P_H_INCLUDED
