/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QURLPATHINFO_H
#define QURLPATHINFO_H

#include "kdeqt5staging_export.h"
#include <QtCore/qurl.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QUrlPathInfoPrivate;

class KDEQT5STAGING_EXPORT QUrlPathInfo
{
public:
    enum PathFormattingOption {
        None = 0x0,
        StripTrailingSlash = 0x200, // compatible with QUrl
        AppendTrailingSlash = 0x400
    };
    Q_DECLARE_FLAGS(PathFormattingOptions, PathFormattingOption)

    /*!
     * Flags to be used in URL comparison functions like equals, or urlcmp
     */
    enum EqualsOption
    {
        StrictComparison = 0x00,
        CompareWithoutTrailingSlash = 0x01,
        CompareWithoutFragment = 0x02,
        AllowEmptyPath = 0x04
    };
    Q_DECLARE_FLAGS(EqualsOptions, EqualsOption)


    QUrlPathInfo();
    explicit QUrlPathInfo(const QUrl &url);
    QUrlPathInfo(const QUrlPathInfo &other);
    QUrlPathInfo &operator =(const QUrlPathInfo &other);
#ifdef Q_COMPILER_RVALUE_REFS
    inline QUrlPathInfo &operator=(QUrlPathInfo &&other)
    { qSwap(d, other.d); return *this; }
#endif
    ~QUrlPathInfo();

    inline void swap(QUrlPathInfo &other) { qSwap(d, other.d); }

    QUrl url(PathFormattingOption options = None) const;
    void setUrl(const QUrl &u);

    QString path(PathFormattingOptions options = None) const;
    void setPath(const QString &path);
    void addPath(const QString &relativePath);
    void adjustPath(PathFormattingOptions options);
    static QUrl addPathToUrl(const QUrl &url, const QString& relativePath) Q_REQUIRED_RESULT;

    QString localPath(PathFormattingOptions options = None) const;

    bool isEmpty() const;
    bool equals(const QUrl& u, EqualsOptions options = StrictComparison) const;
    bool isParentOfOrEqual(const QUrl &child) const;

    void clear();

    QString fileName() const;
    void setFileName(const QString &name);

    QString directory(PathFormattingOptions options = None) const;
    QUrl directoryUrl() const;

private:
    QSharedDataPointer<QUrlPathInfoPrivate> d;
//public:
    //typedef QUrlPathInfoPrivate * DataPtr;
    //inline DataPtr &data_ptr() { return d; }
};

Q_DECLARE_TYPEINFO(QUrlPathInfo, Q_MOVABLE_TYPE);
// Q_DECLARE_SHARED(QUrlPathInfo)
Q_DECLARE_OPERATORS_FOR_FLAGS(QUrlPathInfo::PathFormattingOptions)

QT_END_NAMESPACE

QT_END_HEADER

#endif // QURLPATHINFO_H
