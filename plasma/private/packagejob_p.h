/******************************************************************************
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

#ifndef PLASMA_PACKAGEJOB_P_H
#define PLASMA_PACKAGEJOB_P_H

#include "kjob.h"


namespace Plasma
{

class PackageJobPrivate;

class PackageJob : public KJob
{
    Q_OBJECT


    public:
        PackageJob(const QString& packageRoot, const QString& serviceRoot, QObject* parent = 0);
        ~PackageJob();

        //void init();

        virtual void start(); // ??

        void install(const QString& archivePath);
        void uninstall(const QString& packageName);

    Q_SIGNALS:
        void finished(bool success);

    private:
        PackageJobPrivate* d;
};

}

#endif
