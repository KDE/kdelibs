/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcoredesktopservices.h"
#include <QDebug>

/*!
    \class QDesktopServices
    \brief The QDesktopServices class provides methods for accessing common desktop services.
    \since 4.2
    \ingroup desktop

    Many desktop environments provide services that can be used by applications to
    perform common tasks, such as opening a web page, in a way that is both consistent
    and takes into account the user's application preferences.

    This class contains functions that provide simple interfaces to these services
    that indicate whether they succeeded or failed.

    The openUrl() function is used to open files located at arbitrary URLs in external
    applications. For URLs that correspond to resources on the local filing system
    (where the URL scheme is "file"), a suitable application will be used to open the
    file; otherwise, a web browser will be used to fetch and display the file.

    The user's desktop settings control whether certain executable file types are
    opened for browsing, or if they are executed instead. Some desktop environments
    are configured to prevent users from executing files obtained from non-local URLs,
    or to ask the user's permission before doing so.

    \section1 URL Handlers

    The behavior of the openUrl() function can be customized for individual URL
    schemes to allow applications to override the default handling behavior for
    certain types of URLs.

    The dispatch mechanism allows only one custom handler to be used for each URL
    scheme; this is set using the setUrlHandler() function. Each handler is
    implemented as a slot which accepts only a single QUrl argument.

    The existing handlers for each scheme can be removed with the
    unsetUrlHandler() function. This returns the handling behavior for the given
    scheme to the default behavior.

    This system makes it easy to implement a help system, for example. Help could be
    provided in labels and text browsers using \gui{help://myapplication/mytopic}
    URLs, and by registering a handler it becomes possible to display the help text
    inside the application:

    \snippet doc/src/snippets/code/src_gui_util_qdesktopservices.cpp 0

    If inside the handler you decide that you can't open the requested
    URL, you can just call QDesktopServices::openUrl() again with the
    same argument, and it will try to open the URL using the
    appropriate mechanism for the user's desktop environment.

    \sa QSystemTrayIcon, QProcess
*/

/*!
    \enum QDesktopServices::StandardLocation
    \since 4.4

    This enum describes the different locations that can be queried by
    QDesktopServices::storageLocation and QDesktopServices::displayName.

    \value DesktopLocation Returns the user's desktop directory.
    \value DocumentsLocation Returns the user's document.
    \value FontsLocation Returns the user's fonts.
    \value ApplicationsLocation Returns the user's applications.
    \value MusicLocation Returns the users music.
    \value MoviesLocation Returns the user's movies.
    \value PicturesLocation Returns the user's pictures.
    \value TempLocation Returns the system's temporary directory.
    \value HomeLocation Returns the user's home directory.
    \value DataLocation Returns a directory location where persistent
           application data can be stored. QCoreApplication::applicationName
           and QCoreApplication::organizationName should work on all
           platforms.
    \value CacheLocation Returns a directory location where user-specific
           non-essential (cached) data should be written.
    \value ConfigLocation Returns a directory location where user-specific
           configuration files should be written.

    \sa storageLocation() displayName()
*/

/*!
    \fn QString QDesktopServices::storageLocation(StandardLocation type)
    \since 4.4

    Returns the default system directory where files of \a type belong, or an empty string
    if the location cannot be determined.

    \note The storage location returned can be a directory that does not exist; i.e., it
    may need to be created by the system or the user.

    \note On Symbian OS, ApplicationsLocation always point /sys/bin folder on the same drive
    with executable. FontsLocation always points to folder on ROM drive. Symbian OS does not
    have desktop concept, DesktopLocation returns same path as DocumentsLocation.
    Rest of the standard locations point to folder on same drive with executable, except
    that if executable is in ROM the folder from C drive is returned.
*/

/*!
    \fn QString QDesktopServices::displayName(StandardLocation type)

    Returns a localized display name for the given location \a type or
    an empty QString if no relevant location can be found.
*/

//// HACKS
//// This is the fake implementation for kdelibs-frameworks, temporarily.
//// The goal is to have the API available, but the real implementation will go into Qt5.
#include <QDir>
#include <QFile>
#include <QDesktopServices>

// TODO docu for the option enum

QString QCoreDesktopServices::storageLocation(StandardLocation type)
{
    if (type == ConfigLocation) {
        // TODO: this is a unix-only implementation
        // http://www.freedesktop.org/wiki/Software/xdg-user-dirs
        QString xdgConfigHome = QFile::decodeName(qgetenv("XDG_CONFIG_HOME"));
        if (xdgConfigHome.isEmpty())
            xdgConfigHome = QDir::homePath() + QLatin1String("/.config");
        return xdgConfigHome;
    } else {
        return QDesktopServices::storageLocation(QDesktopServices::StandardLocation(type));
    }
}

QStringList QCoreDesktopServices::standardLocations(StandardLocation type)
{
    QStringList dirs;
    if (type == ConfigLocation) {
        // TODO: this is a unix-only implementation
        QString xdgConfigDirs = QFile::decodeName(qgetenv("XDG_CONFIG_DIRS"));
        if (xdgConfigDirs.isEmpty()) {
            dirs.append(QString::fromLatin1("/etc/xdg"));
        } else {
            dirs = xdgConfigDirs.split(':');
        }
    }
    dirs.append(storageLocation(type));
    return dirs;
}

static bool existsAsSpecified(const QString& path, QCoreDesktopServices::LocateOptions options)
{
    if (options & QCoreDesktopServices::LocateDirectory)
        return QDir(path).exists();
    return QFileInfo(path).isFile();
}

// TODO docu
QString QCoreDesktopServices::locate(StandardLocation type, const QString& fileName, LocateOptions options)
{
    const QStringList dirs = standardLocations(type);
    Q_FOREACH(const QString& dir, dirs) {
        const QString path = dir + '/' + fileName;
        //qDebug() << "Looking at" << path;
        if (existsAsSpecified(path, options))
            return path;
    }
    return QString();
}

//// END HACKS


QT_END_NAMESPACE



