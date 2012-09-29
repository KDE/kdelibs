/*
 * Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>
#include <iomanip>

#include <QDir>
#include <QDBusInterface>

//#include <KApplication>
//#include <KAboutData>
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
//#include <plasma/packagemetadata.h>

// static const char description[] = I18N_NOOP("Install, list, remove Plasma packages");
// static const char version[] = "0.2";

void output(const QString &msg)
{
    std::cout << msg.toLocal8Bit().constData() << std::endl;
}

void runKbuildsycoca()
{
    return;
//     if (KSycoca::isAvailable()) {
//         QDBusInterface dbus("org.kde.kded", "/kbuildsycoca", "org.kde.kbuildsycoca");
//         dbus.call(QDBus::NoBlock, "recreate");
//     }
}

QStringList packages(const QStringList& types)
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

void listPackages(const QStringList& types)
{
    QStringList list = packages(types);
    list.sort();
    foreach (const QString& package, list) {
        output(package);
    }
}

void renderTypeTable(const QMap<QString, QStringList> &plugins)
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

void listTypes()
{
    output(i18n("Package types that are installable with this tool:"));
    output(i18n("Built in:"));

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

    if (!offers.isEmpty()) {
        std::cout << std::endl;
        output(i18n("Provided by plugins:"));

        QMap<QString, QStringList> plugins;
        foreach (const KService::Ptr service, offers) {
            KPluginInfo info(service);
            Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load(info.pluginName());
            QString name = info.name();
            QString plugin = info.pluginName();
            QString path = structure->defaultPackageRoot();
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
}

int main(int argc, char **argv)
{
//     KAboutData aboutData("plasmapkg", 0, ki18n("Plasma Package Manager"),
//                          version, ki18n(description), KAboutData::License_GPL,
//                          ki18n("(C) 2008, Aaron Seigo"));
//     aboutData.addAuthor( ki18n("Aaron Seigo"),
//                          ki18n("Original author"),
//                         "aseigo@kde.org" );
//
//     KComponentData componentData(aboutData);
    QLocalizedString description = qi18n("Plasma Package Manager");

    const char version[] = "1.90";

    KCmdLineArgs::init(argc, argv, "plasmapkg", "plasmapkg", qi18n("Plasma Package Manager"), version, description);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("h");
    options.add("hash <path>", qi18nc("Do not translate <path>", "Generate a SHA1 hash for the package at <path>"));
    options.add("g");
    options.add("global", ki18n("For install or remove, operates on packages installed for all users."));
    options.add("t");
    options.add("type <type>",
                qi18nc("theme, wallpaper, etc. are keywords, but they may be translated, as both versions "
                       "are recognized by the application "
                       "(if translated, should be same as messages with 'package type' context below)",
                       "The type of package, e.g. theme, wallpaper, plasmoid, dataengine, runner, layout-template, etc."),
                "plasmoid");
    options.add("s");
    options.add("i");
    options.add("install <path>", qi18nc("Do not translate <path>", "Install the package at <path>"));
    options.add("u");
    options.add("upgrade <path>", qi18nc("Do not translate <path>", "Upgrade the package at <path>"));
    options.add("l");
    options.add("list", ki18n("List installed packages"));
    options.add("list-types", ki18n("lists all known Package types that can be installed"));
    options.add("r");
    options.add("remove <name>", qi18nc("Do not translate <name>", "Remove the package named <name>"));
    options.add("p");
    options.add("packageroot <path>", ki18n("Absolute path to the package root. If not supplied, then the standard data directories for this KDE session will be searched instead."));
    KCmdLineArgs::addCmdLineOptions( options );

    QCoreApplication app(argc, argv);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("hash")) {
        const QString path = args->getOption("hash");
        Plasma::PackageStructure::Ptr structure(new Plasma::PackageStructure);
        Plasma::Package package(path, structure);
        const QString hash = package.contentsHash();
        if (hash.isEmpty()) {
            output(i18n("Failed to generate a Package hash for %1", path));
            exit(1);
        }

        output(i18n("SHA1 hash for Package at %1: '%2'", path, hash));
        exit(0);
    }

    if (args->isSet("list-types")) {
        listTypes();
        exit(0);
    }

    QString type = args->getOption("type");
    QString packageRoot = type;
    QString servicePrefix;
    QStringList pluginTypes;
    Plasma::PackageStructure *installer = 0;
    QString package;
    QString packageFile;

    if (args->isSet("remove")) {
        package = args->getOption("remove");
    } else if (args->isSet("upgrade")) {
        package = args->getOption("upgrade");
    } else if (args->isSet("install")) {
        package = args->getOption("install");
    }
    if (!QDir::isAbsolutePath(package)) {
        packageFile = QDir(QDir::currentPath() + '/' + package).absolutePath();
    } else {
        packageFile = package;
    }

    if (!packageFile.isEmpty() && (!args->isSet("type") ||
        type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0 ||
        type.compare("wallpaper", Qt::CaseInsensitive) == 0)) {
        // Check type for common plasma packages
        Plasma::PackageStructure package;
        package.setPath(packageFile);
        QString serviceType = package.metadata().serviceType();
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
            output(i18n("Could not find a suitable installer for package of type %1", type));
            return 1;
        }

        KService::Ptr offer = offers.first();
        QString error;
        installer = offer->createInstance<Plasma::PackageStructure>(0, QVariantList(), &error);

        if (!installer) {
            output(i18n("Could not load installer for package of type %1. Error reported was: %2",
                        args->getOption("type"), error));
            return 1;
        }

        packageRoot = installer->defaultPackageRoot();
        pluginTypes << installer->type();
        kDebug() << "we have: " << packageRoot << pluginTypes;
    }

    if (args->isSet("list")) {
        listPackages(pluginTypes);
    } else {
        // install, remove or upgrade
        if (!installer) {
            installer = new Plasma::PackageStructure();
            installer->setServicePrefix(servicePrefix);
        }

        if (args->isSet("packageroot") && args->isSet("global")) {
            KCmdLineArgs::usageError(i18nc("The user entered conflicting options packageroot and global, this is the error message telling the user he can use only one", "The packageroot and global options conflict each other, please select only one."));
        } else if (args->isSet("packageroot")) {
            packageRoot = args->getOption("packageroot");
        } else if (args->isSet("global")) {
            packageRoot = KStandardDirs::locate("data", packageRoot);
        } else {
            packageRoot = KStandardDirs::locateLocal("data", packageRoot);
        }

        if (args->isSet("remove") || args->isSet("upgrade")) {
            installer->setPath(packageFile);
            Plasma::PackageMetadata metadata = installer->metadata();

            QString pluginName;
            if (metadata.pluginName().isEmpty()) {
                // plugin name given in command line
                pluginName = package;
            } else {
                // Parameter was a plasma package, get plugin name from the package
                pluginName = metadata.pluginName();
            }

            QStringList installed = packages(pluginTypes);
            if (installed.contains(pluginName)) {
                if (installer->uninstallPackage(pluginName, packageRoot)) {
                    output(i18n("Successfully removed %1", pluginName));
                } else if (!args->isSet("upgrade")) {
                    output(i18n("Removal of %1 failed.", pluginName));
                    delete installer;
                    return 1;
                }
            } else {
                output(i18n("Plugin %1 is not installed.", pluginName));
            }
        }
        if (args->isSet("install") || args->isSet("upgrade")) {
            if (installer->installPackage(packageFile, packageRoot)) {
                output(i18n("Successfully installed %1", packageFile));
            } else {
                output(i18n("Installation of %1 failed.", packageFile));
                delete installer;
                return 1;
            }
        }
        if (package.isEmpty()) {
            KCmdLineArgs::usageError(i18nc("No option was given, this is the error message telling the user he needs at least one, do not translate install, remove, upgrade nor list", "One of install, remove, upgrade or list is required."));
        } else {
            runKbuildsycoca();
        }
    }
    delete installer;
    return 0;
}

