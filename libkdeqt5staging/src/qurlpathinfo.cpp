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

#include "qurlpathinfo.h"

#include <qfileinfo.h>

/*!
    \class QUrlPathInfo
    \inmodule QtCore
    \since 5.1

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

/*!
    \enum QUrlPathInfo::PathFormattingOption

    The path formatting options define transformations for the path.

    \value None The path is unchanged.
    \value StripTrailingSlash  The trailing slash is removed if one is present.

    Note that a path of "/" will always remain unchanged, since an empty path
    has a different meaning.
*/

/*!
    \enum QUrlPathInfo::EqualsOption

    Options for the equals() method.

    \value CompareWithoutTrailingSlash Ignore trailing '/' in paths.
    The paths "dir" and "dir/" are treated the same.
    Note however, that by default, the paths "" and "/" are not the same
    (For instance ftp://user@host redirects to ftp://user@host/home/user (on a linux server),
    while ftp://user@host/ is the root dir).
    This is also why path(StripTrailingSlash) for "/" returns "/" and not "".
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
QUrl QUrlPathInfo::url(PathFormattingOption options) const
{
    if (options == None) {
        return d->url;
    } else {
        QUrl url(d->url);
        url.setPath(path(options), QUrl::DecodedMode);
        return url;
    }
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
    QString path = d->url.path(QUrl::FullyDecoded);
    while ((options & StripTrailingSlash) && path.endsWith(QLatin1Char('/')) && path.length() > 1)
        path.chop(1);
    return path;
}

/*!
    Sets the path of the URL.

    Unlike QUrl, QUrlPathInfo works with decoded paths.

    The other components of the URL (scheme, host, query, fragment...) remain unchanged.

    \sa path()
*/
void QUrlPathInfo::setPath(const QString &path)
{
    d->url.setPath(path, QUrl::DecodedMode);
}

/*!
    Returns the path for this URL, formatted as a local file path, using \a options

    This method only makes sense for local urls.

    The path returned will use forward slashes, even if it was originally created
    from one with backslashes.

    \sa setPath(), url(), QUrl::toLocalFile()
*/
QString QUrlPathInfo::localPath(PathFormattingOptions options) const
{
    QString path = d->url.toLocalFile();
    while ((options & StripTrailingSlash) && path.endsWith(QLatin1Char('/')) && path.length() > 1)
        path.chop(1);
    return path;
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
    Return true if this URL is a parent of \a child, or if they are equal.
    Trailing slashes are ignored.
*/
bool QUrlPathInfo::isParentOfOrEqual(const QUrl &child) const
{
    return d->url.isParentOf(child) || equals(child, CompareWithoutTrailingSlash);
}

/*!
    Compares this url with \a url.
    \param u the URL to compare this one with.
    \param options a set of EqualsOption flags
    \return True if both urls are the same. If at least one of the urls is invalid,
    false is returned.
    \see operator==. This function should be used if you want to
    set additional options, like ignoring trailing '/' characters.
*/
bool QUrlPathInfo::equals(const QUrl& url, EqualsOptions options) const
{
    if (!d->url.isValid() || !url.isValid())
        return false;

    if (options != StrictComparison) {
        if (d->url.scheme() != url.scheme() ||
            d->url.authority() != url.authority() || // user+pass+host+port
            d->url.query() != url.query() ||
            d->url.fragment() != url.fragment())
            return false;

        const bool bLocal1 = d->url.isLocalFile();
        const bool bLocal2 = url.isLocalFile();
        if (bLocal1 != bLocal2)
            return false;

        // local files: use QFileInfo in case the filesystem is case sensitive
        if (bLocal1 && bLocal2)
            return QFileInfo(d->url.toLocalFile()) == QFileInfo(url.toLocalFile());

        QString path1 = path((options & CompareWithoutTrailingSlash) ? StripTrailingSlash : None);
        QString path2 = QUrlPathInfo(url).path((options & CompareWithoutTrailingSlash) ? StripTrailingSlash : None);

        return path1 == path2;
    }

    return d->url == url;
}
