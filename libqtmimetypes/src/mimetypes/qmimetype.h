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

#ifndef QMIMETYPE_H_INCLUDED
#define QMIMETYPE_H_INCLUDED

#include "qmime_global.h"

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE struct QMimeTypePrivate; QT_END_NAMESPACE

QT_BEGIN_NAMESPACE class QFileinfo; QT_END_NAMESPACE
QT_BEGIN_NAMESPACE class QStringList; QT_END_NAMESPACE

QT_BEGIN_NAMESPACE

class QMIME_EXPORT QMimeType
{
public:
    QMimeType();
    QMimeType(const QMimeType &other);
#if !defined(Q_COMPILER_RVALUE_REFS) || TEST_COMPILER_RVALUE_REFS == 0
    QMimeType &operator=(const QMimeType &other);
#endif
#ifdef Q_COMPILER_RVALUE_REFS
    QMimeType(QMimeType &&other);

    QMimeType &operator=(QMimeType &&other)
    {
        qSwap(d, other.d);
        return *this;
    }
#endif
    void swap(QMimeType &other)
    {
        qSwap(d, other.d);
    }
    explicit QMimeType(const QMimeTypePrivate &dd);
    ~QMimeType();

    bool operator==(const QMimeType &other) const;

    inline bool operator!=(const QMimeType &other) const
    {
        return !operator==(other);
    }

    bool isValid() const;

    bool isDefault() const;

    QString name() const;
#if 0
    QStringList aliases() const;
#endif
    QString comment(const QString &localeName = QString()) const;
    QString genericIconName() const;
    QString iconName() const;
    QStringList globPatterns() const;
    QStringList parentMimeTypes() const;
    QStringList allParentMimeTypes() const;
    QStringList suffixes() const;
    QString preferredSuffix() const;

    bool inherits(const QString &mimeTypeName) const;

    QString filterString() const;

protected:
    friend class BaseMimeTypeParser;
    friend class MimeTypeMapEntry;
    friend struct QMimeDatabasePrivate;
    friend class QMimeXMLProvider;
    friend class QMimeBinaryProvider;
    friend class QMimeTypePrivate;

    QExplicitlySharedDataPointer<QMimeTypePrivate> d;
};

QT_END_NAMESPACE

#endif   // QMIMETYPE_H_INCLUDED
