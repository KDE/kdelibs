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

#include "qmimetype.h"   // Basis

#include "qmimetype_p.h"
#include "qmimedatabase_p.h"
#include "qmimeprovider_p.h"

#include "qmimeglobpattern_p.h"

#include <QtCore/QDebug>
#include <QtCore/QLocale>

QT_BEGIN_NAMESPACE

bool isQMimeTypeDebuggingActivated (false);

#ifndef QT_NO_DEBUG_OUTPUT
#define DBG() if (isQMimeTypeDebuggingActivated) qDebug() << Q_FUNC_INFO
#else
#define DBG() if (0) qDebug() << Q_FUNC_INFO
#endif

QMimeTypePrivate::QMimeTypePrivate()
{}

QMimeTypePrivate::QMimeTypePrivate(const QMimeType &other)
        : name(other.d->name)
        //, comment(other.d->comment)
        , localeComments(other.d->localeComments)
        , genericIconName(other.d->genericIconName)
        , iconName(other.d->iconName)
        , globPatterns(other.d->globPatterns)
{}

void QMimeTypePrivate::clear()
{
    name.clear();
    //comment.clear();
    localeComments.clear();
    genericIconName.clear();
    iconName.clear();
    globPatterns.clear();
}

/*!
    \fn bool QMimeTypePrivate::operator==(const QMimeTypePrivate &other) const;
    \brief Returns if the \a other object has the same value.
 */
bool QMimeTypePrivate::operator==(const QMimeTypePrivate &other) const
{
    return name == other.name &&
            //comment == other.comment &&
            localeComments == other.localeComments &&
            genericIconName == other.genericIconName &&
            iconName == other.iconName &&
            globPatterns == other.globPatterns;
}

void QMimeTypePrivate::addGlobPattern(const QString &pattern)
{
    globPatterns.append(pattern);
}

/*!
    \class QMimeType

    \brief The QMimeType class describes types of file or data, represented by a MIME type string.

    For instance a file named "readme.txt" has the MIME type "text/plain".
    The MIME type can be determined from the file name, or from the file
    contents, or from both. MIME type determination can also be done on
    buffers of data not coming from files.

    Determining the MIME type of a file can be useful to make sure your
    application supports it. It is also useful in file-manager-like applications
    or widgets, in order to display an appropriate icon() for the file, or even
    the descriptive comment() in detailed views.

    To check if a file has the expected MIME type, you should use inherits()
    rather than a simple string comparison based on the name(). This is because
    MIME types can inherit from each other: for instance a C source file is
    a specific type of plain text file, so text/x-csrc inherits text/plain.

    \sa QMimeDatabase
 */

/*!
    \fn QMimeType::QMimeType();
    \brief Initializes the attributes with default values that indicate an invalid MIME type.
 */
QMimeType::QMimeType() :
        d(new QMimeTypePrivate())
{
    DBG() << "name():" << name();
    //DBG() << "aliases():" << aliases();
    //DBG() << "comment():" << comment();
    DBG() << "genericIconName():" << genericIconName();
    DBG() << "iconName():" << iconName();
    DBG() << "globPatterns():" << globPatterns();
    DBG() << "suffixes():" << suffixes();
    DBG() << "preferredSuffix():" << preferredSuffix();
}

/*!
    \fn QMimeType::QMimeType(const QMimeType &other);
    \brief Initializes the attributes with the data of another MIME type.
    \a other The other MIME type the data of which is copied
 */
QMimeType::QMimeType(const QMimeType &other) :
        d(other.d)
{
    DBG() << "name():" << name();
    //DBG() << "aliases():" << aliases();
    //DBG() << "comment():" << comment();
    DBG() << "genericIconName():" << genericIconName();
    DBG() << "iconName():" << iconName();
    DBG() << "globPatterns():" << globPatterns();
    DBG() << "suffixes():" << suffixes();
    DBG() << "preferredSuffix():" << preferredSuffix();
}

#if !defined(Q_COMPILER_RVALUE_REFS) || TEST_COMPILER_RVALUE_REFS == 0
/*!
    \fn QMimeType &QMimeType::operator=(const QMimeType &other);
    \brief Assigns the data of another MIME type.
    \a other The other MIME type the data of which is assigned
 */
QMimeType &QMimeType::operator=(const QMimeType &other)
{
    if (d != other.d)
        d = other.d;
    return *this;
}
#endif

#ifdef Q_COMPILER_RVALUE_REFS
/*!
    \fn QMimeType::QMimeType(QMimeType &&other);
    \brief Moves the data of another rvalue MIME type into the attributes.
    \a other The other rvalue MIME type the data of which is moved
 */
QMimeType::QMimeType(QMimeType &&other) :
        d(std::move(other.d))
{
    DBG() << "name():" << name();
    //DBG() << "aliases():" << aliases();
    //DBG() << "comment():" << comment();
    DBG() << "genericIconName():" << genericIconName();
    DBG() << "iconName():" << iconName();
    DBG() << "globPatterns():" << globPatterns();
    DBG() << "suffixes():" << suffixes();
    DBG() << "preferredSuffix():" << preferredSuffix();
}
#endif

/*!
    \fn QMimeType::QMimeType(const QMimeTypePrivate &dd);
    \brief Assigns the data of a private MIME type object.
    \a dd The private MIME typeobject the data of which is assigned
 */
QMimeType::QMimeType(const QMimeTypePrivate &dd) :
        d(new QMimeTypePrivate(dd))
{
    DBG() << "name():" << name();
    //DBG() << "aliases():" << aliases();
    //DBG() << "comment():" << comment();
    DBG() << "genericIconName():" << genericIconName();
    DBG() << "iconName():" << iconName();
    DBG() << "globPatterns():" << globPatterns();
    DBG() << "suffixes():" << suffixes();
    DBG() << "preferredSuffix():" << preferredSuffix();
}

/*!
    \fn void QMimeType::swap(QMimeType &other);
    \brief Swaps the properties of a MIME type with the properties of another MIME type.
    \a other The other value MIME type the data of which is to be swapped with.
 */

/*!
    \fn QMimeType::~QMimeType();
    \brief Releases the d pointer.
 */
QMimeType::~QMimeType()
{
    DBG() << "name():" << name();
    //DBG() << "aliases():" << aliases();
    //DBG() << "comment():" << comment();
    DBG() << "genericIconName():" << genericIconName();
    DBG() << "iconName():" << iconName();
    DBG() << "globPatterns():" << globPatterns();
    DBG() << "suffixes():" << suffixes();
    DBG() << "preferredSuffix():" << preferredSuffix();
}

/*!
    \fn bool QMimeType::operator==(const QMimeType &other) const;
    \brief Tests for equality with another MIME type.
    \a other The other MIME type that is to be compared with
    Returns if the other MIME type is equal.
 */
bool QMimeType::operator==(const QMimeType &other) const
{
    return d == other.d || *d == *other.d;
}

/*!
    \fn bool QMimeType::operator!=(const QMimeType &other) const;
    \brief Tests for non-equality with another MIME type.
    \a other The other MIME type that is to be compared with
    Returns if the other MIME type is not equal.
 */

/*!
    \fn bool QMimeType::isValid() const;
    \brief Returns true if the MIME type is valid, otherwise returns false.
    A valid MIME type has a non-empty name().
    The invalid MIME type is the default-constructed QMimeType.
 */
bool QMimeType::isValid() const
{
    return !d->name.isEmpty();
}

/*!
    \fn bool QMimeType::isDefault() const;
    Returns true if this MIME type is the default MIME type which
    applies to all files: application/octet-stream.
 */
bool QMimeType::isDefault() const
{
    return d->name == QMimeDatabasePrivate::instance()->defaultMimeType();
}

/*!
    \fn QString QMimeType::name() const;
    \brief Returns the name of the MIME type.
 */
QString QMimeType::name() const
{
    return d->name;
}

#if 0
/*!
    \fn QStringList QMimeType::aliases() const;
    \brief Returns the aliases of the MIME type.
 */
// What is the use case for this? [apart from mimetypeviewer.cpp ...]
QStringList QMimeType::aliases() const
{
    return d->aliases;
}
#endif

/*!
    \fn QString QMimeType::comment(const QString& localeName) const;
    \brief Returns the description of the MIME type to be displayed on user interfaces.

    The system language (QLocale::system().name()) is used to select the appropriate translation.
    Another language can be specified by setting the \a localeName argument.
 */
QString QMimeType::comment(const QString& localeName) const
{
    QMimeDatabasePrivate::instance()->provider()->loadMimeTypePrivate(*d);

    QStringList languageList;
    if (!localeName.isEmpty())
        languageList << localeName;
    languageList << QLocale::system().name();
    languageList << QLocale::system().uiLanguages();
    Q_FOREACH(const QString& lang, languageList) {
        const QString comm = d->localeComments.value(lang);
        if (!comm.isEmpty())
            return comm;
        const int pos = lang.indexOf(QLatin1Char('_'));
        if (pos != -1) {
            // "pt_BR" not found? try just "pt"
            const QString shortLang = lang.left(pos);
            const QString commShort = d->localeComments.value(shortLang);
            if (!commShort.isEmpty())
                return commShort;
        }
    }

    // Use the mimetype name as fallback
    return d->name;
}

/*!
    \fn QString QMimeType::genericIconName() const;
    \brief Returns the file name of an icon image that represents the MIME type.
 */
QString QMimeType::genericIconName() const
{
    QMimeDatabasePrivate::instance()->provider()->loadGenericIcon(*d);
    return d->genericIconName;
}

/*!
    \fn QString QMimeType::iconName() const;
    \brief Returns the file name of an icon image that represents the MIME type.
 */
QString QMimeType::iconName() const
{
    QMimeDatabasePrivate::instance()->provider()->loadIcon(*d);
    if (d->iconName.isEmpty()) {
        // Make default icon name from the mimetype name
        d->iconName = name();
        const int slashindex = d->iconName.indexOf(QLatin1Char('/'));
        if (slashindex != -1)
            d->iconName[slashindex] = QLatin1Char('-');
    }
    return d->iconName;
}

/*!
    \fn QStringList QMimeType::globPatterns() const;
    \brief Returns the list of glob matching patterns.
 */
QStringList QMimeType::globPatterns() const
{
    QMimeDatabasePrivate::instance()->provider()->loadMimeTypePrivate(*d);
    return d->globPatterns;
}

QStringList QMimeType::parentMimeTypes() const
{
    return QMimeDatabasePrivate::instance()->provider()->parents(d->name);
}

static void collectParentMimeTypes(const QString& mime, QStringList& allParents)
{
    QStringList parents = QMimeDatabasePrivate::instance()->provider()->parents(mime);
    foreach(const QString& parent, parents) {
        // I would use QSet, but since order matters I better not
        if (!allParents.contains(parent))
            allParents.append(parent);
    }
    // We want a breadth-first search, so that the least-specific parent (octet-stream) is last
    // This means iterating twice, unfortunately.
    foreach(const QString& parent, parents) {
        collectParentMimeTypes(parent, allParents);
    }
}

QStringList QMimeType::allParentMimeTypes() const
{
    QStringList allParents;
    const QString canonical = d->name;
    if (!canonical.isEmpty())
        allParents.append(canonical);
    collectParentMimeTypes(d->name, allParents);

    return allParents;
}

/*!
    \fn QStringList QMimeType::suffixes() const;
    \brief Returns the known suffixes for the MIME type.
 */
QStringList QMimeType::suffixes() const
{
    QMimeDatabasePrivate::instance()->provider()->loadMimeTypePrivate(*d);

    QStringList result;
    foreach (const QString& pattern, d->globPatterns) {
        // Not a simple suffix if if looks like: README or *. or *.* or *.JP*G or *.JP?
        if (pattern.startsWith(QLatin1String("*.")) &&
            pattern.length() > 2 &&
            pattern.indexOf(QLatin1Char('*'), 2) < 0 && pattern.indexOf(QLatin1Char('?'), 2) < 0) {
            const QString suffix = pattern.mid(2);
            result.append(suffix);
        }
    }

    return result;
}

/*!
    \fn QString QMimeType::preferredSuffix() const;
    \brief Returns the preferred suffix for the MIME type.
 */
QString QMimeType::preferredSuffix() const
{
    const QStringList suffixList = suffixes();
    return suffixList.isEmpty() ? QString() : suffixList.at(0);
}

/*!
    \fn QString QMimeType::filterString() const;
    Returns a filter string usable for a file dialog.
*/
QString QMimeType::filterString() const
{
    QMimeDatabasePrivate::instance()->provider()->loadMimeTypePrivate(*d);
    QString filter;

    if (!d->globPatterns.empty()) {
        filter += comment() + QLatin1String(" (");
        for (int i = 0; i < d->globPatterns.size(); ++i) {
            if (i != 0)
                filter += QLatin1Char(' ');
            filter += d->globPatterns.at(i);
        }
        filter +=  QLatin1Char(')');
    }

    return filter;
}

/*!
    \fn bool QMimeType::inherits(const QString &mimeTypeName) const;
    Returns true if this mimetype is \a mimeTypeName, or inherits \a mimeTypeName,
    or \a mimeTypeName is an alias for this mimetype.
 */
bool QMimeType::inherits(const QString &mimeTypeName) const
{
    if (d->name == mimeTypeName)
        return true;
    return QMimeDatabasePrivate::instance()->inherits(d->name, mimeTypeName);
}

#undef DBG

QT_END_NAMESPACE
