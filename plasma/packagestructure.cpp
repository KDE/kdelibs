/******************************************************************************
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>                            *
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

#include "packagestructure.h"

#include "private/packages_p.h"
#include "private/package_p.h"
#include <private/packagejob_p.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>

namespace Plasma
{

class PackageStructurePrivate {
public:
    static PackageStructure* defaultPackageStructure(ComponentType type);
};

PackageStructure::PackageStructure(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(0)
{
    d = new PackageStructurePrivate;
    Q_UNUSED(args)
}

PackageStructure::~PackageStructure()
{
}

void PackageStructure::initPackage(Package *package)
{
    Q_UNUSED(package)
}

void PackageStructure::pathChanged(Package *package)
{
//     if (package) {
//         package->setPath();
//     }
    Q_UNUSED(package)
}

KJob* PackageStructure::install(Package *package, const QString &archivePath, const QString &packageRoot)
{
    PackageJob* j = new PackageJob(package->servicePrefix(), this);
    j->install(archivePath, packageRoot);
    return j;
//     return PackagePrivate::installPackage(archivePath, packageRoot, package->servicePrefix());
}

KJob* PackageStructure::uninstall(Package *package, const QString &packageRoot)
{
    PackageJob* j = new PackageJob(package->servicePrefix(), this);
    j->uninstall(packageRoot);
    return j;
}

PackageStructure* PackageStructure::load(const QString &packageFormat)
{
    if (packageFormat.isEmpty()) {
        return new PackageStructure();
    }

    PackageStructure* structure = 0;

    if (packageFormat == "Plasma/Applet") {
        structure = PackageStructurePrivate::defaultPackageStructure(AppletComponent);
        //structure->d->type = "Plasma/Applet";
    } else if (packageFormat == "Plasma/DataEngine") {
        structure = PackageStructurePrivate::defaultPackageStructure(DataEngineComponent);
        //structure->d->type = "Plasma/DataEngine";
    } else if (packageFormat == "Plasma/Runner") {
        structure = PackageStructurePrivate::defaultPackageStructure(RunnerComponent);
        //structure->d->type = "Plasma/Runner";
    } else if (packageFormat == "Plasma/Wallpaper") {
        structure = PackageStructurePrivate::defaultPackageStructure(WallpaperComponent);
        //structure->d->type = "Plasma/Wallpaper";
    } else if (packageFormat == "Plasma/Theme") {
        structure = new ThemePackage();
        //structure->d->type = "Plasma/Theme";
    } else if (packageFormat == "Plasma/Generic") {
        structure = PackageStructurePrivate::defaultPackageStructure(GenericComponent);
        //structure->d->type = "Plasma/Generic";
        //structure->setDefaultPackageRoot(KStandardDirs::locate("data", "plasma/packages/"));
    }

    if (structure) {
        return structure;
    }

    // first we check for plugins in sycoca
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(packageFormat);
    KService::List offers =
        KServiceTypeTrader::self()->query("Plasma/PackageStructure", constraint);


    QVariantList args;
    QString error;
    foreach (const KService::Ptr &offer, offers) {
        PackageStructure* structure(
            offer->createInstance<Plasma::PackageStructure>(0, args, &error));

        if (structure) {
            return structure;
        }

        kDebug() << "Couldn't load PackageStructure for" << packageFormat
                 << "! reason given: " << error;
    }
    /*
    // if that didn't give us any love, then we try to load from a config file
    structure = new PackageStructure();
    QString configPath("plasma/packageformats/%1rc");
    configPath = KStandardDirs::locate("data", configPath.arg(packageFormat));
    kDebug() << "configPath: " << configPath;

    if (!configPath.isEmpty()) {
        KConfig config(configPath);
        structure->read(&config);
        return structure;
    }

    // try to load from absolute file path
    KUrl url(packageFormat);
    if (url.isLocalFile()) {
        KConfig config(url.toLocalFile(), KConfig::SimpleConfig);
        structure->read(&config);
    }
    */
#warning "FIXME PLASMA_NO_KIO force-enabled"
#define PLASMA_NO_KIO 1
#ifndef PLASMA_NO_KIO
    else {
        KTemporaryFile tmp;
        if (tmp.open()) {
            KIO::Job *job = KIO::file_copy(url, KUrl(tmp.fileName()),
                                           -1, KIO::Overwrite | KIO::HideProgressInfo);
            if (job->exec()) {
                KConfig config(tmp.fileName(), KConfig::SimpleConfig);
                structure->read(&config);
            }
        }
    }
#endif

    return structure;
}

PackageStructure* PackageStructurePrivate::defaultPackageStructure(ComponentType type)
{
    switch (type) {
    case AppletComponent:
    case WallpaperComponent:
    case RunnerComponent:
    case GenericComponent:
        return new PlasmoidPackage();
        break;
    case DataEngineComponent:
        return new DataEnginePackage();
        break;
    default:
        // TODO: we don't have any special structures for other components yet
        break;
    }

    return new PackageStructure();
}


}



#include "moc_packagestructure.cpp"
