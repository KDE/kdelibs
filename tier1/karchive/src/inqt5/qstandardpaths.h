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

#ifndef QSTANDARDPATHS_H
#define QSTANDARDPATHS_H

#include <QtCore/qstringlist.h>
#include "qmime_global.h" // LOCAL HACK

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

#ifndef QT_NO_STANDARDPATHS

class QStringList;

class QMIME_EXPORT QStandardPaths
{
public:
    // Do not re-order, must match QDesktopServices
    enum StandardLocation {
        DesktopLocation,
        DocumentsLocation,
        FontsLocation,
        ApplicationsLocation,
        MusicLocation,
        MoviesLocation,
        PicturesLocation,
        TempLocation,
        HomeLocation,
        DataLocation,
        CacheLocation,
        GenericDataLocation,
        RuntimeLocation,
        ConfigLocation
    };

    static QString writableLocation(StandardLocation type);
    static QStringList standardLocations(StandardLocation type);

    enum LocateOption {
        LocateFile = 0x0,
        LocateDirectory = 0x1
    };
    Q_DECLARE_FLAGS(LocateOptions, LocateOption)

    static QString locate(StandardLocation type, const QString &fileName, LocateOptions options = LocateFile);
    static QStringList locateAll(StandardLocation type, const QString &fileName, LocateOptions options = LocateFile);
    static QString displayName(StandardLocation type);

    static QString findExecutable(const QString &executableName, const QStringList &paths = QStringList());

private:
    // prevent construction
    QStandardPaths();
    ~QStandardPaths();
};

#endif // QT_NO_STANDARDPATHS

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSTANDARDPATHS_H
