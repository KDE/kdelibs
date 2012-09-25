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

#include "packagejob_p.h"
#include "packagejobthread_p.h"
#include "config-plasma.h"

namespace Plasma
{
class PackageJobPrivate {
public:
    PackageJobThread *thread;
};

PackageJob::PackageJob(const QString& packageRoot, const QString& serviceRoot, QObject* parent) :
    KJob(parent)
{
    d = new PackageJobPrivate;
    d->thread = new PackageJobThread(packageRoot,  serviceRoot, this);
    d->thread->start();
}

PackageJob::~PackageJob()
{
    delete d;
}

void PackageJob::start()
{

}

void PackageJob::install(const QString& archivePath)
{
    d->thread->install(archivePath);
}

void PackageJob::uninstall(const QString& package)
{
    d->thread->uninstall(package);
}


} // namespace Plasma

#include "moc_packagejob_p.cpp"

