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

#include "qurlpathinfo.h"

/*!
    \class QUrlPathInfo

    \since 5.0

    \brief The QUrlPathInfo class provides a convenient interface for working
    with URL paths.

    \reentrant
    \ingroup io
    \ingroup network
    \ingroup shared


    QUrlPathInfo allows to manipulate URLs which represent a file or a
    directory in a local or remote filesystem. It offers similar
    path-manipulation methods as QFileInfo, but for paths in URLs.

    The most common way to use QUrlPathInfo is to initialize it via the
    constructor by passing a QUrl. Otherwise, setUrl() can also be used.

    QUrl defaults to encoded paths, so a '%' in a filename becomes a %25
    in the url, and therefore in QUrl::path(). However for file management
    on local or remote filesystems, decoded paths are wanted, to display
    '%' to the user when the filename contains '%'. For this reason,
    QUrlPathInfo works with decoded paths.

    \sa QUrl
*/

QT_BEGIN_NAMESPACE

class QUrlPathInfoPrivate : public QSharedData
{
public:
    QUrlPathInfoPrivate() {}

    QUrl url;
};

/*!
    Constructs an empty QUrlPathInfo object.
*/
QUrlPathInfo::QUrlPathInfo()
    : d(new QUrlPathInfoPrivate)
{
}

/*!
    Constructs a QUrlPathInfo object from a \a url.
*/
QUrlPathInfo::QUrlPathInfo(const QUrl &url)
    : d(new QUrlPathInfoPrivate)
{
    d->url = url;
}

/*!
    Constructs a copy of \a other.
*/
QUrlPathInfo::QUrlPathInfo(const QUrlPathInfo &other)
    : d(other.d)
{
}

/*!
    Destructor; called immediately before the object is deleted.
*/
QUrlPathInfo::~QUrlPathInfo()
{
}

/*!
    Returns the full URL, with the path modifications taken into account, if any.

    \sa setUrl()
*/
QUrl QUrlPathInfo::url() const
{
    return d->url;
}

/*!
    Sets the URL to manipulate.

    \sa url()
*/
void QUrlPathInfo::setUrl(const QUrl &u)
{
    d->url = u;
}

/*!
    Returns the path of the URL, formatted using \a options

    Unlike QUrl, QUrlPathInfo works with decoded paths.

    Do not use this as a local path, this is not portable.
    Use toLocalPath(options) or url().toLocalFile() instead.

    \sa setPath(), toLocalPath(), url()
*/
QString QUrlPathInfo::path(PathFormattingOptions options) const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QString path = d->url.path(QUrl::FullyDecoded);
#else
    QString path = d->url.path();
#endif
    while ((options & StripTrailingSlash) && path.endsWith(QLatin1Char('/')) && path.length() > 1)
        path.chop(1);
    if ((options & AppendTrailingSlash) && !path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    return path;
}

/*!
    Sets the path of the URL.

    Unlike QUrl, QUrlPathInfo works with decoded paths.

    The other components of the URL (scheme, host, query, fragment...) remain unchanged.

    \sa path(), addPath()
*/
void QUrlPathInfo::setPath(const QString &path)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    d->url.setPath(path, QUrl::DecodedMode);
#else
    d->url.setPath(path);
#endif
}

/*!
    Returns the path for this URL, formatted as a local file path, using \a options

    The path returned will use forward slashes, even if it was originally created
    from one with backslashes.

    \sa setPath(), url(), QUrl::toLocalFile()
*/
QString QUrlPathInfo::localPath(PathFormattingOptions options) const
{
    QString path = d->url.toLocalFile();
    while ((options & StripTrailingSlash) && path.endsWith(QLatin1Char('/')) && path.length() > 1)
        path.chop(1);
    if ((options & AppendTrailingSlash) && !path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    return path;
}

/*!
    Returns true if the path is empty

    \sa path()
*/
bool QUrlPathInfo::isEmpty() const
{
    return d->url.path().isEmpty();
}

/*!
   Sets the path to be empty

   \sa path(), setPath()
*/
void QUrlPathInfo::clear()
{
    d->url.setPath(QString());
}

/*!
   Returns the name of the file, excluding the directory.

   Note that, if this QUrl object is given a path ending in a slash, the name of the file is considered empty.

   If the path doesn't contain any slash, it is fully returned as part of fileName, and directory() will be empty.

   \sa path(), setPath()
*/
QString QUrlPathInfo::fileName() const
{
    const QString ourPath = path();
    const int slash = ourPath.lastIndexOf(QLatin1Char('/'));
    if (slash == -1)
        return ourPath;
    return ourPath.mid(slash + 1);
}

/*!
   Sets the name of the file, keeping the directory unchanged.

   \sa path(), fileName(), directory()
*/
void QUrlPathInfo::setFileName(const QString &fileName)
{
    const QString ourPath = path();
    const int slash = ourPath.lastIndexOf(QLatin1Char('/'));
    if (slash == -1)
        setPath(fileName);
    else if (slash == 0)
        setPath(QLatin1Char('/') + fileName);
    else
        setPath(ourPath.left(slash + 1) + fileName);
}

/*!
   Returns the directory path, excluding the file name returned by fileName().

   Example:

   \snippet doc/src/snippets/code/src_corelib_io_qurl.cpp 7

   If the path doesn't contain any slash, it is fully returned as part of fileName, and directoryPath will be empty.

   \sa path(), fileName(), setFileName(), directoryUrl()
*/
QString QUrlPathInfo::directory() const
{
    const QString ourPath = path();
    const int slash = ourPath.lastIndexOf(QLatin1Char('/'));
    if (slash == -1)
        return QString();
    else if (slash == 0)
        return QString(QLatin1Char('/'));
    return ourPath.left(slash);
}

/*!
   Returns a URL for the parent directory of this URL.

   \sa directory()
*/
QUrl QUrlPathInfo::directoryUrl() const
{
    QUrl url = d->url;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    url.setPath(directory(), QUrl::DecodedMode);
#else
    url.setPath(directory());
#endif
    return url;
}

/*!
   Adds to the current path.

   Assumes that the current path is a directory.
   \p relativePath is appended to the current path.
   This method adds '/' if needed while concatenating.
   This means it does not matter whether the current path has a trailing
   '/' or not. If there is none, it becomes appended. If \p relativePath
   has a leading '/' then this one is stripped.

   \param relativePath The relative path to add to the current directory path.
   Usually this is simply a file name.

   \sa path(), setPath()
*/
void QUrlPathInfo::addPath(const QString &relativePath)
{
    if (relativePath.isEmpty())
        return;
    QString p = path();
    // "The function adds '/' if needed while concatenating."
    if (!p.endsWith(QLatin1Char('/')))
        p += QLatin1Char('/');

    // If \p relativePath has a leading '/' then this one is stripped.
    int i = 0;
    if (relativePath.startsWith(QLatin1Char('/')))
        ++i;

    setPath(p + relativePath.mid(i));
}

/*!
  Convenience method, for adding a relative path to a url.

  \param url the initial URL
  \param relativePath the relative path to add (often just a file name)
  \return the modified URL
*/
QUrl QUrlPathInfo::addPathToUrl(const QUrl &url, const QString& relativePath)
{
    QUrlPathInfo info(url);
    info.addPath(relativePath);
    return info.url();
}
