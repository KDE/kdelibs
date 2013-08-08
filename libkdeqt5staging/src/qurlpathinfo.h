/****************************************************************************
**
** Copyright (C) 2013 David Faure <faure@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
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

QT_BEGIN_NAMESPACE

class QUrlPathInfoPrivate;

class KDEQT5STAGING_EXPORT QUrlPathInfo
{
public:
    enum PathFormattingOption {
        None = 0x0,
        StripTrailingSlash = 0x200 // compatible with QUrl
    };
    Q_DECLARE_FLAGS(PathFormattingOptions, PathFormattingOption)

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

    QString localPath(PathFormattingOptions options = None) const;

private:
    QSharedDataPointer<QUrlPathInfoPrivate> d;
};

Q_DECLARE_TYPEINFO(QUrlPathInfo, Q_MOVABLE_TYPE);
// Q_DECLARE_SHARED(QUrlPathInfo)
Q_DECLARE_OPERATORS_FOR_FLAGS(QUrlPathInfo::PathFormattingOptions)

QT_END_NAMESPACE

#endif // QURLPATHINFO_H
