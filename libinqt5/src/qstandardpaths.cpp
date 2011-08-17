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

#include "qstandardpaths.h"
#include <QDebug>

/*!
    \class QStandardPaths
    \brief The QStandardPaths class provides methods for accessing standard paths.
    \since 5.0

    This class contains functions to query standard locations on the local
    filesystem, for common tasks such as user-specific directories or system-wide
    configuration directories.
*/

/*!
    \enum QStandardPaths::StandardLocation

    This enum describes the different locations that can be queried by
    QStandardPaths::storageLocation and QStandardPaths::displayName.

    \value DesktopLocation Returns the user's desktop directory.
    \value DocumentsLocation Returns the user's document.
    \value FontsLocation Returns the user's fonts.
    \value ApplicationsLocation Returns the user's applications.
    \value MusicLocation Returns the user's music.
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
    \fn QString QStandardPaths::storageLocation(StandardLocation type)

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
    \fn QString QStandardPaths::displayName(StandardLocation type)

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

QString QStandardPaths::storageLocation(StandardLocation type)
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

QStringList QStandardPaths::standardLocations(StandardLocation type)
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

static bool existsAsSpecified(const QString& path, QStandardPaths::LocateOptions options)
{
    if (options & QStandardPaths::LocateDirectory)
        return QDir(path).exists();
    return QFileInfo(path).isFile();
}

// TODO docu
QString QStandardPaths::locate(StandardLocation type, const QString& fileName, LocateOptions options)
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



