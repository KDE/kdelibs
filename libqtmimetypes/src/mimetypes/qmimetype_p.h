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

#ifndef QMIMETYPE_P_H_INCLUDED
#define QMIMETYPE_P_H_INCLUDED

#include "qmimetype.h"

#include <QtCore/QHash>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

struct QMIME_EXPORT QMimeTypePrivate : public QSharedData
{
    typedef QHash<QString, QString> LocaleHash;

    QMimeTypePrivate();
    explicit QMimeTypePrivate(const QMimeType &other);

    void clear();

    bool operator==(const QMimeTypePrivate &other) const;

    void addGlobPattern(const QString &pattern);

    //unsigned matchesFileBySuffix(const QString &fileName) const;
    //unsigned matchesData(const QByteArray &data) const;

    QString name;
    //QString comment; // mostly unused, only for tests and qml bindings
    LocaleHash localeComments;
    QString genericIconName;
    QString iconName;
    QStringList globPatterns;
};

QT_END_NAMESPACE

#define QMIMETYPE_BUILDER \
    QT_BEGIN_NAMESPACE \
    static QMimeType buildQMimeType ( \
                         const QString &name, \
                         /*const QStringList &aliases,*/ \
                         /*const QString &comment,*/ \
                         const QString &genericIconName, \
                         const QString &iconName, \
                         const QStringList &globPatterns \
                     ) \
    { \
        QMimeTypePrivate qMimeTypeData; \
        qMimeTypeData.name = name; \
        /*qMimeTypeData.aliases = aliases;*/ \
        /*qMimeTypeData.comment = comment;*/ \
        qMimeTypeData.genericIconName = genericIconName; \
        qMimeTypeData.iconName = iconName; \
        qMimeTypeData.globPatterns = globPatterns; \
        return QMimeType(qMimeTypeData); \
    } \
    QT_END_NAMESPACE

#ifdef Q_COMPILER_RVALUE_REFS
#define QMIMETYPE_BUILDER_FROM_RVALUE_REFS \
    QT_BEGIN_NAMESPACE \
    static QMimeType buildQMimeType ( \
                         QString &&name, \
                         /*QStringList &&aliases,*/ \
                         /*QString &&comment,*/ \
                         QString &&genericIconName, \
                         QString &&iconName, \
                         QStringList &&globPatterns \
                     ) \
    { \
        QMimeTypePrivate qMimeTypeData; \
        qMimeTypeData.name = std::move(name); \
        /*qMimeTypeData.aliases = std::move(aliases);*/ \
        /*qMimeTypeData.comment = std::move(comment);*/ \
        qMimeTypeData.genericIconName = std::move(genericIconName); \
        qMimeTypeData.iconName = std::move(iconName); \
        qMimeTypeData.globPatterns = std::move(globPatterns); \
        return QMimeType(qMimeTypeData); \
    } \
    QT_END_NAMESPACE
#endif

#endif   // QMIMETYPE_P_H_INCLUDED
