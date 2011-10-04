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

#include <qsettings.h>
#include <qdir.h>
#include <private/qsystemlibrary_p.h>
#include <qurl.h>
#include <qstringlist.h>
#include <qprocess.h>
#include <qtemporaryfile.h>
#include <qcoreapplication.h>

#include <qt_windows.h>
#include <shlobj.h>
#if !defined(Q_OS_WINCE)
#  include <intshcut.h>
#else
#  include <qguifunctions_wince.h>
#  if !defined(STANDARDSHELL_UI_MODEL)
#    include <winx.h>
#  endif
#endif

#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC	13
#define CSIDL_MYVIDEO	14
#endif

#ifndef QT_NO_STANDARDPATHS

QT_BEGIN_NAMESPACE

typedef BOOL (WINAPI*GetSpecialFolderPath)(HWND, LPWSTR, int, BOOL);
static GetSpecialFolderPath resolveGetSpecialFolderPath()
{
    static GetSpecialFolderPath gsfp = 0;
    if (!gsfp) {
#ifndef Q_OS_WINCE
        QSystemLibrary library(QLatin1String("shell32"));
#else
        QSystemLibrary library(QLatin1String("coredll"));
#endif // Q_OS_WINCE
        gsfp = (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathW");
    }
    return gsfp;
}

static QString convertCharArray(const wchar_t *path)
{
    return QDir::fromNativeSeparators(QString::fromWCharArray(path));
}

QString QStandardPaths::storageLocation(StandardLocation type)
{
    QString result;

    static GetSpecialFolderPath SHGetSpecialFolderPath = resolveGetSpecialFolderPath();
    if (!SHGetSpecialFolderPath)
        return QString();

    wchar_t path[MAX_PATH];

    switch (type) {
    case ConfigLocation: // same as DataLocation, on Windows
    case DataLocation:
    case GenericDataLocation:
#if defined Q_WS_WINCE
        if (SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, FALSE))
#else
        if (SHGetSpecialFolderPath(0, path, CSIDL_LOCAL_APPDATA, FALSE))
#endif
            result = convertCharArray(path);
        if (type != GenericDataLocation) {
            if (!QCoreApplication::organizationName().isEmpty())
                result += QLatin1Char('/') + QCoreApplication::organizationName();
            if (!QCoreApplication::applicationName().isEmpty())
                result += QLatin1Char('/') + QCoreApplication::applicationName();
        }
        break;

    case DesktopLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, FALSE))
            result = convertCharArray(path);
        break;

    case DocumentsLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_PERSONAL, FALSE))
            result = convertCharArray(path);
        break;

    case FontsLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_FONTS, FALSE))
            result = convertCharArray(path);
        break;

    case ApplicationsLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_PROGRAMS, FALSE))
            result = convertCharArray(path);
        break;

    case MusicLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_MYMUSIC, FALSE))
            result = convertCharArray(path);
        break;

    case MoviesLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_MYVIDEO, FALSE))
            result = convertCharArray(path);
        break;

    case PicturesLocation:
        if (SHGetSpecialFolderPath(0, path, CSIDL_MYPICTURES, FALSE))
            result = convertCharArray(path);
        break;

    case CacheLocation:
        // Although Microsoft has a Cache key it is a pointer to IE's cache, not a cache
        // location for everyone.  Most applications seem to be using a
        // cache directory located in their AppData directory
        return storageLocation(DataLocation) + QLatin1String("\\cache");

    case RuntimeLocation:
    case HomeLocation:
        result = QDir::homePath();
        break;

    case TempLocation:
        result = QDir::tempPath();
        break;
    }
    return result;
}

QStringList QStandardPaths::standardLocations(StandardLocation type)
{
    QStringList dirs;

    // type-specific handling goes here

#ifndef Q_WS_WINCE
    static GetSpecialFolderPath SHGetSpecialFolderPath = resolveGetSpecialFolderPath();
    if (SHGetSpecialFolderPath) {
        wchar_t path[MAX_PATH];
        switch (type) {
        case ConfigLocation: // same as DataLocation, on Windows
        case DataLocation:
        case GenericDataLocation:
            if (SHGetSpecialFolderPath(0, path, CSIDL_COMMON_APPDATA, FALSE)) {
                QString result = convertCharArray(path);
                if (type != GenericDataLocation) {
                    if (!QCoreApplication::organizationName().isEmpty())
                        result += QLatin1Char('/') + QCoreApplication::organizationName();
                    if (!QCoreApplication::applicationName().isEmpty())
                        result += QLatin1Char('/') + QCoreApplication::applicationName();
                }
                dirs.append(result);
            }
            break;
        default:
            break;
        }
    }
#endif

    const QString localDir = storageLocation(type);
    dirs.prepend(localDir);
    return dirs;
}

QString QStandardPaths::displayName(StandardLocation type)
{
    Q_UNUSED(type);
    return QString();
}

QT_END_NAMESPACE

#endif // QT_NO_STANDARDPATHS
