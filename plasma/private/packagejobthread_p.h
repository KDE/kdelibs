/******************************************************************************
*   Copyright 2007-2009 by Aaron Seigo <aseigo@kde.org>                       *
*   Copyright 2012 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef PLASMA_PACKAGEJOBTHREAD_P_H
#define PLASMA_PACKAGEJOBTHREAD_P_H

#include "kjob.h"
#include <QThread>

namespace Plasma
{

class PackageJobThreadPrivate;

class PackageJobThread : public QThread
{
    Q_OBJECT

    public:
        PackageJobThread(const QString& packageRoot, const QString& servicePrefix, QObject* parent = 0);
        virtual ~PackageJobThread();

//        void init();

        bool install(const QString& archivePath);
        bool uninstall(const QString& packageName);

    Q_SIGNALS:
        void finished(bool success);

    private:
        PackageJobThreadPrivate* d;
};

}

#endif
