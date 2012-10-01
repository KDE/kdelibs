/******************************************************************************
*   Copyright 2008 Aaron Seigo <aseigo@kde.org>                               *
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

#include "plasmapkg.h"

#include <kcmdlineargs.h>
#include <kdebug.h>
//#include <KLocale>
//#include <KPluginInfo>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kshell.h>
#include <kstandarddirs.h>
//#include <KSycocai>
#include <klocalizedstring.h>

#include <plasma/packagestructure.h>
#include <plasma/package.h>

#include <klocale.h>
#include <kjob.h>

#include <QDir>
#include <QMap>
#include <QStringList>

#include <iostream>
#include <iomanip>


namespace Plasma
{
class PlasmaPkgPrivate {
public:
    QString packageRoot;
    Plasma::PackageStructure* structure;
    QString installPath;
    void output(const QString &msg);
    void runKbuildsycoca();
    QStringList packages(const QStringList& types);
    void listPackages(const QStringList& types);
    void renderTypeTable(const QMap<QString, QStringList> &plugins);
    void listTypes();
    void coutput(const QString &msg);
    KCmdLineArgs *args;
};

PlasmaPkg::PlasmaPkg(int& argc, char** argv) :
    QCoreApplication(argc, argv)
{
    d = new PlasmaPkgPrivate;
}

PlasmaPkg::~PlasmaPkg()
{
    delete d;
}


void PlasmaPkg::runMain()
{
    d->args = KCmdLineArgs::parsedArgs();

    Plasma::PackageStructure* structure = new Plasma::PackageStructure;
    if (d->args->isSet("hash")) {
        const QString path = d->args->getOption("hash");
        Plasma::Package package(structure);
        package.setPath(path);
        const QString hash = package.contentsHash();
        if (hash.isEmpty()) {
            d->coutput(i18n("Failed to generate a Package hash for %1", path));
            exit(1);
        }

        d->coutput(i18n("SHA1 hash for Package at %1: '%2'", path, hash));
        exit(0);
    }

    if (d->args->isSet("list-types")) {
        d->listTypes();
        exit(0);
    }

    QString type = d->args->getOption("type");
    QString packageRoot = type;
    QString servicePrefix;
    QStringList pluginTypes;
    Plasma::Package *installer = 0;
    QString package;
    QString packageFile;

    if (d->args->isSet("remove")) {
        package = d->args->getOption("remove");
    } else if (d->args->isSet("upgrade")) {
        package = d->args->getOption("upgrade");
    } else if (d->args->isSet("install")) {
        package = d->args->getOption("install");
    }
    if (!QDir::isAbsolutePath(package)) {
        packageFile = QDir(QDir::currentPath() + '/' + package).absolutePath();
    } else {
        packageFile = package;
    }

    if (!packageFile.isEmpty() && (!d->args->isSet("type") ||
        type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0 ||
        type.compare("wallpaper", Qt::CaseInsensitive) == 0)) {
        // Check type for common plasma packages
        Plasma::Package package(structure);
        package.setPath(packageFile);
        QString serviceType = package.metadata().property("X-Plasma-ServiceType").toString();
        if (!serviceType.isEmpty()) {
            if (serviceType.contains("Plasma/Applet") ||
                serviceType.contains("Plasma/PopupApplet") ||
                serviceType.contains("Plasma/Containment")) {
                type = "plasmoid";
            } else if (serviceType == "Plasma/DataEngine") {
                type = "dataengine";
            } else if (serviceType == "Plasma/Runner") {
                type = "runner";
            } else if (serviceType == "Plasma/Wallpaper") {
                // This also changes type to wallpaperplugin when --type wallpaper
                // was specified and we have wallpaper plugin package (instead of
                // wallpaper image package)
                type = "wallpaperplugin";
            } else if (serviceType == "KWin/WindowSwitcher") {
                type = "windowswitcher";
            } else if (serviceType == "KWin/Effect") {
                type = "kwineffect";
            } else if (serviceType == "KWin/Script") {
                type = "kwinscript";
            } else if (serviceType == "Plasma/LayoutTemplate") {
                type = "layout-template";
            } else {
                type = serviceType;
                kDebug() << "fallthrough type is" << serviceType;
            }
        }
    }

    if (type.compare(i18nc("package type", "plasmoid"), Qt::CaseInsensitive) == 0 ||
        type.compare("plasmoid", Qt::CaseInsensitive) == 0) {
        packageRoot = "plasma/plasmoids/";
        servicePrefix = "plasma-applet-";
        pluginTypes << "Plasma/Applet";
        pluginTypes << "Plasma/PopupApplet";
        pluginTypes << "Plasma/Containment";
    } else if (type.compare(i18nc("package type", "theme"), Qt::CaseInsensitive) == 0 ||
               type.compare("theme", Qt::CaseInsensitive) == 0) {
        packageRoot = "desktoptheme/";
    } else if (type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0 ||
               type.compare("wallpaper", Qt::CaseInsensitive) == 0) {
        packageRoot = "wallpapers/";
    } else if (type.compare(i18nc("package type", "dataengine"), Qt::CaseInsensitive) == 0 ||
               type.compare("dataengine", Qt::CaseInsensitive) == 0) {
        packageRoot = "plasma/dataengines/";
        servicePrefix = "plasma-dataengine-";
        pluginTypes << "Plasma/DataEngine";
    } else if (type.compare(i18nc("package type", "runner"), Qt::CaseInsensitive) == 0 ||
               type.compare("runner", Qt::CaseInsensitive) == 0) {
        packageRoot = "plasma/runners/";
        servicePrefix = "plasma-runner-";
        pluginTypes << "Plasma/Runner";
    } else if (type.compare(i18nc("package type", "wallpaperplugin"), Qt::CaseInsensitive) == 0 ||
               type.compare("wallpaperplugin", Qt::CaseInsensitive) == 0) {
        packageRoot = "plasma/wallpapers/";
        servicePrefix = "plasma-wallpaper-";
        pluginTypes << "Plasma/Wallpaper";
    } else if (type.compare(i18nc("package type", "layout-template"), Qt::CaseInsensitive) == 0 ||
               type.compare("layout-template", Qt::CaseInsensitive) == 0) {
        packageRoot = "plasma/layout-templates/";
        servicePrefix = "plasma-layout-";
        pluginTypes << "Plasma/LayoutTemplate";
    } else if (type.compare(i18nc("package type", "kwineffect"), Qt::CaseInsensitive) == 0 ||
               type.compare("kwineffect", Qt::CaseInsensitive) == 0) {
        packageRoot = "kwin/effects/";
        servicePrefix = "kwin-effect-";
        pluginTypes << "KWin/Effect";
    } else if (type.compare(i18nc("package type", "windowswitcher"), Qt::CaseInsensitive) == 0 ||
               type.compare("windowswitcher", Qt::CaseInsensitive) == 0) {
        packageRoot = "kwin/tabbox/";
        servicePrefix = "kwin-windowswitcher-";
        pluginTypes << "KWin/WindowSwitcher";
    } else if (type.compare(i18nc("package type", "kwinscript"), Qt::CaseInsensitive) == 0 ||
               type.compare("kwinscript", Qt::CaseInsensitive) == 0) {
        packageRoot = "kwin/scripts/";
        servicePrefix = "kwin-script-";
        pluginTypes << "KWin/Script";
    } else /*if (KSycoca::isAvailable())*/ {
        const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(type);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure", constraint);
        if (offers.isEmpty()) {
            d->coutput(i18n("Could not find a suitable installer for package of type %1", type));
            return;
            //return 1;
        }
        kWarning() << "custom PackageStructure plugins not ported";
        KService::Ptr offer = offers.first();
        QString error;

        //installer = new Plasma::Package(offer->createInstance<Plasma::PackageStructure>(0, QVariantList(), &error));

        if (!installer) {
            d->coutput(i18n("Could not load installer for package of type %1. Error reported was: %2",
                        d->args->getOption("type"), error));
            return;
            //return 1;
        }

        //packageRoot = installer->defaultPackageRoot();
        //pluginTypes << installer->type();
        kDebug() << "we have: " << packageRoot << pluginTypes;
    }

    if (d->args->isSet("list")) {
        d->listPackages(pluginTypes);
    } else {
        // install, remove or upgrade
        if (!installer) {
            installer = new Plasma::Package(new Plasma::PackageStructure());
            //installer->setServicePrefix(servicePrefix);
        }

        if (d->args->isSet("packageroot") && d->args->isSet("global")) {
            KCmdLineArgs::usageError(i18nc("The user entered conflicting options packageroot and global, this is the error message telling the user he can use only one", "The packageroot and global options conflict each other, please select only one."));
        } else if (d->args->isSet("packageroot")) {
            packageRoot = d->args->getOption("packageroot");
        } else if (d->args->isSet("global")) {
            packageRoot = KStandardDirs::locate("data", packageRoot);
        } else {
            packageRoot = KStandardDirs::locateLocal("data", packageRoot);
        }

        if (d->args->isSet("remove") || d->args->isSet("upgrade")) {
            installer->setPath(packageFile);
            KPluginInfo metadata = installer->metadata();

            QString pluginName;
            if (metadata.pluginName().isEmpty()) {
                // plugin name given in command line
                pluginName = package;
            } else {
                // Parameter was a plasma package, get plugin name from the package
                pluginName = metadata.pluginName();
            }

            QStringList installed = d->packages(pluginTypes);
            if (installed.contains(pluginName)) {
                kWarning() << " is now async.";
//                 if (installer->uninstall(pluginName, packageRoot)) {
//                     output(i18n("Successfully removed %1", pluginName));
//                 } else if (!d->args->isSet("upgrade")) {
//                     output(i18n("Removal of %1 failed.", pluginName));
//                     delete installer;
//                     return 1;
//                 }
            } else {
                d->coutput(i18n("Plugin %1 is not installed.", pluginName));
            }
        }
        if (d->args->isSet("install") || d->args->isSet("upgrade")) {
            if (installer->install(packageFile, packageRoot)) {
                kWarning() << " is now async.";
                d->coutput(i18n("Successfully installed %1", packageFile));
            } else {
                d->coutput(i18n("Installation of %1 failed.", packageFile));
                kWarning() << " is now async.";
                delete installer;
                return;
                //return 1;
            }
        }
        if (package.isEmpty()) {
            KCmdLineArgs::usageError(i18nc("No option was given, this is the error message telling the user he needs at least one, do not translate install, remove, upgrade nor list", "One of install, remove, upgrade or list is required."));
        } else {
            d->runKbuildsycoca();
        }
    }
    delete installer;

}

void PlasmaPkgPrivate::coutput(const QString &msg)
{
    std::cout << msg.toLocal8Bit().constData() << std::endl;
}

void PlasmaPkgPrivate::runKbuildsycoca()
{
    return;
//     if (KSycoca::isAvailable()) {
//         QDBusInterface dbus("org.kde.kded", "/kbuildsycoca", "org.kde.kbuildsycoca");
//         dbus.call(QDBus::NoBlock, "recreate");
//     }
}

QStringList PlasmaPkgPrivate::packages(const QStringList& types)
{
    QStringList result;

//     if (KSycoca::isAvailable()) {
        foreach (const QString& type, types) {
            const KService::List services = KServiceTypeTrader::self()->query(type);
            foreach (const KService::Ptr &service, services) {
                result << service->property("X-KDE-PluginInfo-Name", QVariant::String).toString();
            }
        }
//     }

    return result;
}

void PlasmaPkgPrivate::listPackages(const QStringList& types)
{
    QStringList list = packages(types);
    list.sort();
    foreach (const QString& package, list) {
        coutput(package);
    }
}

void PlasmaPkgPrivate::renderTypeTable(const QMap<QString, QStringList> &plugins)
{
    const QString nameHeader = i18n("Addon Name");
    const QString pluginHeader = i18n("Service Type");
    const QString pathHeader = i18n("Path");
    int nameWidth = nameHeader.length();
    int pluginWidth = pluginHeader.length();
    int pathWidth = pathHeader.length();

    QMapIterator<QString, QStringList> pluginIt(plugins);
    while (pluginIt.hasNext()) {
        pluginIt.next();
        if (pluginIt.key().length() > nameWidth) {
            nameWidth = pluginIt.key().length();
        }

        if (pluginIt.value()[0].length() > pluginWidth) {
            pluginWidth = pluginIt.value()[0].length();
        }

        if (pluginIt.value()[1].length() > pathWidth) {
            pathWidth = pluginIt.value()[1].length();
        }
    }

    std::cout << nameHeader.toLocal8Bit().constData() << std::setw(nameWidth - nameHeader.length() + 2) << ' '
              << pluginHeader.toLocal8Bit().constData() << std::setw(pluginWidth - pluginHeader.length() + 2) << ' '
              << pathHeader.toLocal8Bit().constData() << std::endl;
    std::cout << std::setfill('-') << std::setw(nameWidth) << '-' << "  "
              << std::setw(pluginWidth) << '-' << "  "
              << std::setw(pathWidth) << '-' << std::endl;
    std::cout << std::setfill(' ');

    pluginIt.toFront();
    while (pluginIt.hasNext()) {
        pluginIt.next();
        std::cout << pluginIt.key().toLocal8Bit().constData() << std::setw(nameWidth - pluginIt.key().length() + 2) << ' '
                  << pluginIt.value()[0].toLocal8Bit().constData() << std::setw(pluginWidth - pluginIt.value()[0].length() + 2) << ' '
                  << pluginIt.value()[1].toLocal8Bit().constData() << std::endl;
    }
}

void PlasmaPkgPrivate::listTypes()
{
    coutput(i18n("Package types that are installable with this tool:"));
    coutput(i18n("Built in:"));

    QMap<QString, QStringList> builtIns;
    builtIns.insert(i18n("DataEngine"), QStringList() << "Plasma/DataEngine" << "plasma/dataengines/");
    builtIns.insert(i18n("Layout Template"), QStringList() << "Plasma/LayoutTemplate" << "plasma/layout-templates/");
    builtIns.insert(i18n("Plasmoid"), QStringList() << "Plasma/Applet" << "plasma/plasmoids/");
    builtIns.insert(i18n("Runner"), QStringList() << "Plasma/Runner" << "plasma/runners/");
    builtIns.insert(i18n("Theme"), QStringList() << "" << "desktoptheme/");
    builtIns.insert(i18n("Wallpaper Images"), QStringList() << "" << "wallpapers/");
    builtIns.insert(i18n("Wallpaper Plugin"), QStringList() << "Plasma/Wallpaper" << "plasma/wallpapers/");
    builtIns.insert(i18n("KWin Effect"), QStringList() << "KWin/Effect" << "kwin/effects/");
    builtIns.insert(i18n("KWin Window Switcher"), QStringList() << "KWin/WindowSwitcher" << "kwin/tabbox/");
    builtIns.insert(i18n("KWin Script"), QStringList() << "KWin/Script" << "kwin/scripts/");
    renderTypeTable(builtIns);

    KService::List offers;
    //if (KSycoca::isAvailable()) {
        offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure");
    //}
    /*
    if (!offers.isEmpty()) {
        std::cout << std::endl;
        output(i18n("Provided by plugins:"));

        QMap<QString, QStringList> plugins;
        foreach (const KService::Ptr service, offers) {
            KPluginInfo info(service);
            const QString proot = "";
            Plasma::PackageStructure* structure = Plasma::PackageStructure::load(info.pluginName());
            QString name = info.name();
            QString plugin = info.pluginName();
            //QString path = structure->defaultPackageRoot();
            QString path =
            plugins.insert(name, QStringList() << plugin << path);
        }

        renderTypeTable(plugins);
    }

    QStringList desktopFiles = KGlobal::dirs()->findAllResources("data", "plasma/packageformats/*rc", KStandardDirs::NoDuplicates);
    if (!desktopFiles.isEmpty()) {
        output(i18n("Provided by .desktop files:"));
        Plasma::PackageStructure structure;
        QMap<QString, QStringList> plugins;
        foreach (const QString &file, desktopFiles) {
            // extract the type
            KConfig config(file, KConfig::SimpleConfig);
            structure.read(&config);
            // get the name based on the rc file name, just as Plasma::PackageStructure does
            const QString name = file.left(file.length() - 2);
            plugins.insert(name, QStringList() << structure.type() << structure.defaultPackageRoot());
        }
    }
    */
}

void PlasmaPkg::install(const QString& src, const QString &dest)
{
    QString archivePath = "/home/sebas/kde5/src/kdelibs/plasma/tests/microblog.plasmoid";

    d->structure = new Plasma::PackageStructure(this);
    Plasma::Package *p = new Plasma::Package(d->structure);
    kDebug() << "Installing " << archivePath;
//     p->setPath(,z
    //const QString packageRoot = "plasma/plasmoids/";
    //const QString servicePrefix = "plasma-applet-";
    KJob* job = p->install(archivePath, d->packageRoot);
    connect(job, SIGNAL(finished(KJob*)), SLOT(packageInstalled(KJob*)));

}

void PlasmaPkg::uninstall(const QString& installationPath)
{
}

void PlasmaPkg::packageInstalled(KJob* j)
{
    kDebug() << "!!!!!!!!!!!!!!!!!!!! package installed" << (j->error() == KJob::NoError);
    //QVERIFY(j->error() == KJob::NoError);
    //QVERIFY(p->path());

//     Plasma::Package *p = new Plasma::Package(d->structure);
//     KJob* jj = p->uninstall();
//     //QObject::disconnect(j, SIGNAL(finished(KJob*)), this, SLOT(packageInstalled(KJob*)));
//     connect(jj, SIGNAL(finished(KJob*)), SLOT(packageInstalled(KJob*)));
    quit();
}

void PlasmaPkg::packageUninstalled(KJob* j)
{
    kDebug() << "!!!!!!!!!!!!!!!!!!!!! package uninstalled";
    //QVERIFY(j->error() == KJob::NoError);

    quit();
}

} // namespace Plasma

#include "moc_plasmapkg.cpp"

