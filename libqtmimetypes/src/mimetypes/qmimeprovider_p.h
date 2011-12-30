/**************************************************************************
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
**************************************************************************/

#ifndef QMIMEPROVIDER_P_H
#define QMIMEPROVIDER_P_H

#include "qmimedatabase_p.h"
#include <QtCore/QSet>
class QMimeMagicRuleMatcher;

class QMimeProviderBase
{
public:
    QMimeProviderBase(QMimeDatabasePrivate *db);
    virtual ~QMimeProviderBase() {}

    virtual bool isValid() = 0;
    virtual QMimeType mimeTypeForName(const QString &name) = 0;
    virtual QStringList findByName(const QString &fileName, QString *foundSuffix) = 0;
    virtual QStringList parents(const QString &mime) = 0;
    virtual QString resolveAlias(const QString &name) = 0;
    virtual QMimeType findByMagic(const QByteArray &data, int *accuracyPtr) = 0;
    virtual QList<QMimeType> allMimeTypes() = 0;
    virtual void loadMimeTypePrivate(QMimeTypePrivate &) {}
    virtual void loadIcon(QMimeTypePrivate &) {}
    virtual void loadGenericIcon(QMimeTypePrivate &) {}

    QMimeDatabasePrivate* m_db;
};

/*
   Parses the files 'mime.cache' and 'types' on demand
 */
class QMimeBinaryProvider : public QMimeProviderBase
{
public:
    QMimeBinaryProvider(QMimeDatabasePrivate *db);
    virtual ~QMimeBinaryProvider();

    virtual bool isValid();
    virtual QMimeType mimeTypeForName(const QString &name);
    virtual QStringList findByName(const QString &fileName, QString *foundSuffix);
    virtual QStringList parents(const QString &mime);
    virtual QString resolveAlias(const QString &name);
    virtual QMimeType findByMagic(const QByteArray &data, int *accuracyPtr);
    virtual QList<QMimeType> allMimeTypes();
    virtual void loadMimeTypePrivate(QMimeTypePrivate &);
    virtual void loadIcon(QMimeTypePrivate &);
    virtual void loadGenericIcon(QMimeTypePrivate &);

private:
    struct CacheFile;

    void matchGlobList(QMimeGlobMatchResult &result, CacheFile *cacheFile, int offset, const QString &fileName);
    bool matchSuffixTree(QMimeGlobMatchResult &result, CacheFile *cacheFile, int numEntries, int firstOffset, const QString &fileName, int charPos, bool caseSensitiveCheck);
    bool matchMagicRule(CacheFile *cacheFile, int numMatchlets, int firstOffset, const QByteArray &data);
    QString iconForMime(CacheFile *cacheFile, int posListOffset, const QByteArray& inputMime);
    void loadMimeTypeList();

    QList<CacheFile *> m_cacheFiles;
    QSet<QString> m_mimetypeNames;
    bool m_mimetypeListLoaded;
};

/*
   Parses the raw XML files (slower)
 */
class QMimeXMLProvider : public QMimeProviderBase
{
public:
    QMimeXMLProvider(QMimeDatabasePrivate *db);

    virtual bool isValid();
    virtual QMimeType mimeTypeForName(const QString &name);
    virtual QStringList findByName(const QString &fileName, QString *foundSuffix);
    virtual QStringList parents(const QString &mime);
    virtual QString resolveAlias(const QString &name);
    virtual QMimeType findByMagic(const QByteArray &data, int *accuracyPtr);
    virtual QList<QMimeType> allMimeTypes();

    bool load(const QString &fileName, QString *errorMessage);

    // Called by the mimetype xml parser
    void addMimeType(const QMimeType &mt);
    void addGlobPattern(const QMimeGlobPattern& glob);
    void addParent(const QString &child, const QString &parent);
    void addAlias(const QString &alias, const QString &name);
    void addMagicMatcher(const QMimeMagicRuleMatcher &matcher);

private:
    void ensureLoaded();
    void load(const QString &fileName);

    bool m_loaded;

    typedef QHash<QString, QMimeType> NameMimeTypeMap;
    NameMimeTypeMap m_nameMimeTypeMap;

    typedef QHash<QString, QString> AliasHash;
    AliasHash m_aliases;

    typedef QHash<QString, QStringList> ParentsHash;
    ParentsHash m_parents;
    QMimeAllGlobPatterns m_mimeTypeGlobs;

    QList<QMimeMagicRuleMatcher> m_magicMatchers;
};

#endif // QMIMEPROVIDER_P_H
