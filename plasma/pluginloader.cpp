/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "pluginloader.h"

#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>
#include <kplugininfo.h>

#include "config-plasma.h"

#ifndef PLASMA_NO_KIO
#include <kio/job.h>
#endif

#include "applet.h"
#include "abstractrunner.h"
#include "containment.h"
#include "containmentactions.h"
#include "package.h"
#include "popupapplet.h"
#include "private/applet_p.h"
#include "private/packages_p.h"
#include "private/service_p.h" // for NullService
#include "private/storage_p.h"

namespace Plasma {

static PluginLoader *s_pluginLoader = 0;

class PluginLoaderPrivate
{
public:
    QHash<QString, QWeakPointer<PackageStructure> > structures;
    bool isDefaultLoader;
};

PluginLoader::PluginLoader()
    : d(new PluginLoaderPrivate)
{
    d->isDefaultLoader = false;
}

PluginLoader::~PluginLoader()
{
    typedef QWeakPointer<PackageStructure> pswp;
    foreach (pswp wp, d->structures) {
        delete wp.data();
    }
    delete d;
}

void PluginLoader::setPluginLoader(PluginLoader* loader)
{
    if (!s_pluginLoader) {
        s_pluginLoader = loader;
    } else {
#ifndef NDEBUG
        kDebug() << "Cannot set pluginLoader, already set!" << s_pluginLoader;
#endif
    }
}

PluginLoader *PluginLoader::self()
{
    if (!s_pluginLoader) {
        // we have been called before any PluginLoader was set, so just use the default
        // implementation. this prevents plugins from nefariously injecting their own
        // plugin loader if the app doesn't
        s_pluginLoader = new PluginLoader;
        s_pluginLoader->d->isDefaultLoader = true;
    }

    return s_pluginLoader;
}

Applet *PluginLoader::loadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    // the application-specific appletLoader failed to create an applet, here we try with our own logic.
    if (name.isEmpty()) {
        return 0;
    }

    Applet *applet = d->isDefaultLoader ? 0 : internalLoadApplet(name, appletId, args);
    if (applet) {
        return applet;
    }

    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    bool isContainment = false;
    if (offers.isEmpty()) {
        //TODO: what would be -really- cool is offer to try and download the applet
        //      from the network at this point
        offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
        if (offers.count() > 0) {
            isContainment = true;
        }
    }

    /* if (offers.count() > 1) {
#ifndef NDEBUG
        kDebug() << "hey! we got more than one! let's blindly take the first one";
#endif
    } */

    AppletPrivate::filterOffers(offers);
    if (offers.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "offers is empty for " << name;
#endif
        return 0;
    }

    KService::Ptr offer = offers.first();

    if (appletId == 0) {
        appletId = ++AppletPrivate::s_maxAppletId;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << appletId << args;

    if (!offer->property("X-Plasma-API").toString().isEmpty()) {
#ifndef NDEBUG
        kDebug() << "we have a script using the"
                 << offer->property("X-Plasma-API").toString() << "API";
#endif
        if (isContainment) {
            return new Containment(0, allArgs);
        } else {
            if (offer->serviceTypes().contains("Plasma/Containment")) {
                return new Containment(0, allArgs);
            } else if (offer->serviceTypes().contains("Plasma/PopupApplet")) {
                return new PopupApplet(0, allArgs);
            } else {
                return new Applet(0, allArgs);
            }
        }
    }

    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
    }


    QString error;
    applet = offer->createInstance<Plasma::Applet>(0, allArgs, &error);

    if (!applet) {
#ifndef NDEBUG
        kDebug() << "Couldn't load applet \"" << name << "\"! reason given: " << error;
#endif
    }

    return applet;
}

DataEngine *PluginLoader::loadDataEngine(const QString &name)
{
    DataEngine *engine = d->isDefaultLoader ? 0 : internalLoadDataEngine(name);
    if (engine) {
        return engine;
    }

    // load the engine, add it to the engines
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine",
                                                              constraint);
    QString error;

    if (offers.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "offers are empty for " << name << " with constraint " << constraint;
#endif
    } else {
        QVariantList allArgs;
        allArgs << offers.first()->storageId();
        QString api = offers.first()->property("X-Plasma-API").toString();
        if (api.isEmpty()) {
            if (offers.first()) {
                KPluginLoader plugin(*offers.first());
                if (Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
                    engine = offers.first()->createInstance<Plasma::DataEngine>(0, allArgs, &error);
                }
            }
        } else {
            engine = new DataEngine(0, offers.first());
        }
    }

    if (!engine) {
#ifndef NDEBUG
        kDebug() << "Couldn't load engine \"" << name << "\". Error given: " << error;
#endif
    }

    return engine;
}

AbstractRunner *PluginLoader::loadRunner(const QString &name)
{
    // FIXME: RunnerManager is all wrapped around runner loading; that should be sorted out
    // and the actual plugin loading added here
    return d->isDefaultLoader ? 0 : internalLoadRunner(name);
}

Service *PluginLoader::loadService(const QString &name, const QVariantList &args, QObject *parent)
{
    Service *service = d->isDefaultLoader ? 0 : internalLoadService(name, args, parent);
    if (service) {
        return service;
    }

    //TODO: scripting API support
    if (name.isEmpty()) {
        return new NullService(QString(), parent);
    } else if (name == "org.kde.servicestorage") {
        return new Storage(parent);
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Service", constraint);

    if (offers.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "offers is empty for " << name;
#endif
        return new NullService(name, parent);
    }

    KService::Ptr offer = offers.first();
    QString error;

    if (Plasma::isPluginVersionCompatible(KPluginLoader(*offer).pluginVersion())) {
        service = offer->createInstance<Plasma::Service>(parent, args, &error);
    }

    if (!service) {
#ifndef NDEBUG
        kDebug() << "Couldn't load Service \"" << name << "\"! reason given: " << error;
#endif
        return new NullService(name, parent);
    }

    if (service->name().isEmpty()) {
        service->setName(name);
    }

    return service;
}

ContainmentActions *PluginLoader::loadContainmentActions(Containment *parent, const QString &name, const QVariantList &args)
{
    if (name.isEmpty()) {
        return 0;
    }

    ContainmentActions *actions = d->isDefaultLoader ? 0 : internalLoadContainmentActions(parent, name, args);
    if (actions) {
        return actions;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContainmentActions", constraint);

    if (offers.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "offers is empty for " << name;
#endif
        return 0;
    }

    KService::Ptr offer = offers.first();
    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << args;
    QString error;
    actions = offer->createInstance<Plasma::ContainmentActions>(parent, allArgs, &error);

    if (!actions) {
#ifndef NDEBUG
        kDebug() << "Couldn't load containmentActions \"" << name << "\"! reason given: " << error;
#endif
    }

    return actions;
}

Package PluginLoader::loadPackage(const QString &packageFormat, const QString &specialization)
{
    if (!d->isDefaultLoader) {
        Package p = internalLoadPackage(packageFormat, specialization);
        if (p.isValid()) {
            return p;
        }
    }

    if (packageFormat.isEmpty()) {
        return Package();
    }

    const QString hashkey = packageFormat + '%' + specialization;
    PackageStructure *structure = d->structures.value(hashkey).data();
    if (structure) {
        return Package(structure);
    }

    if (!specialization.isEmpty()) {
        QRegExp re("[^a-zA-Z0-9\\-_]");
        // check that the provided strings are safe to use in a ServiceType query
        if (re.indexIn(specialization) == -1 && re.indexIn(packageFormat) == -1) {
            // FIXME: The query below is rather spepcific to script engines. generify if possible
            const QString component = packageFormat.right(packageFormat.size() - packageFormat.lastIndexOf('/') - 1);
            const QString constraint = QString("[X-Plasma-API] == '%1' and " "'%2' in [X-Plasma-ComponentTypes]").arg(specialization, component);
            KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);

            if (!offers.isEmpty()) {
                KService::Ptr offer = offers.first();
                QString packageFormat = offer->property("X-Plasma-PackageFormat").toString();
                if (!packageFormat.isEmpty()) {
                    return loadPackage(packageFormat);
                }
            }
        }
    }

    if (packageFormat.startsWith("Plasma")) {
        if (packageFormat.endsWith("/Applet")) {
            structure = new PlasmoidPackage();
        } else if (packageFormat.endsWith("/DataEngine")) {
            structure = new DataEnginePackage();
        } else if (packageFormat.endsWith("/Runner")) {
            structure = new RunnerPackage();
        } else if (packageFormat.endsWith("/Wallpaper")) {
            structure = new WallpaperPackage();
        } else if (packageFormat.endsWith("/Theme")) {
            structure = new ThemePackage();
        } else if (packageFormat.endsWith("/ContainmentActions")) {
            structure = new ContainmentActionsPackage();
        } else if (packageFormat.endsWith("/Generic")) {
            structure = new GenericPackage();
        }

        if (structure) {
            d->structures.insert(hashkey, structure);
            return Package(structure);
        }
    }

    // first we check for plugins in sycoca
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(packageFormat);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure", constraint);

    QVariantList args;
    QString error;
    foreach (const KService::Ptr &offer, offers) {
        structure = qobject_cast<PackageStructure *>(offer->createInstance<PackageStructure>(0, args, &error));

        if (structure) {
            d->structures.insert(hashkey, structure);
            return Package(structure);
        }

#ifndef NDEBUG
        kDebug() << "Couldn't load Package for" << packageFormat << "! reason given: " << error;
#endif
    }

    return Package();
}

KPluginInfo::List PluginLoader::listAppletInfo(const QString &category, const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalAppletInfo(category);
    }

    QString constraint = AppletPrivate::parentAppConstraint(parentApp);

    //note: constraint guaranteed non-empty from here down
    if (category.isEmpty()) { //use all but the excluded categories
        KConfigGroup group(KSharedConfig::openConfig(), "General");
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
        foreach (const QString &category, excluded) {
            constraint.append(" and [X-KDE-PluginInfo-Category] != '").append(category).append("'");
        }
    } else { //specific category (this could be an excluded one - is that bad?)
        constraint.append(" and ").append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    //now we have to do some manual filtering because the constraint can't handle everything
    AppletPrivate::filterOffers(offers);

    //kDebug() << "Applet::listAppletInfo constraint was '" << constraint
    //         << "' which got us " << offers.count() << " matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listDataEngineInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalDataEngineInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);
    return list + KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listRunnerInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalRunnerInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner", constraint);
    return list + KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listContainmentActionsInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalContainmentActionsInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContainmentActions", constraint);
    return KPluginInfo::fromServices(offers);
}

Applet* PluginLoader::internalLoadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    Q_UNUSED(name)
    Q_UNUSED(appletId)
    Q_UNUSED(args)
    return 0;
}

DataEngine *PluginLoader::internalLoadDataEngine(const QString &name)
{
    Q_UNUSED(name)
    return 0;
}

AbstractRunner *PluginLoader::internalLoadRunner(const QString &name)
{
    Q_UNUSED(name)
    return 0;
}

ContainmentActions *PluginLoader::internalLoadContainmentActions(Containment *containment, const QString &name, const QVariantList &args)
{
    Q_UNUSED(containment)
    Q_UNUSED(name)
    Q_UNUSED(args)
    return 0;
}

Service *PluginLoader::internalLoadService(const QString &name, const QVariantList &args, QObject *parent)
{
    Q_UNUSED(name)
    Q_UNUSED(args)
    Q_UNUSED(parent)
    return 0;
}

Package PluginLoader::internalLoadPackage(const QString &name, const QString &specialization)
{
    Q_UNUSED(name);
    Q_UNUSED(specialization);
    return Package();
}

KPluginInfo::List PluginLoader::internalAppletInfo(const QString &category) const
{
    Q_UNUSED(category)
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalDataEngineInfo() const
{
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalRunnerInfo() const
{
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalServiceInfo() const
{
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalContainmentActionsInfo() const
{
    return KPluginInfo::List();
}

static KPluginInfo::List standardInternalInfo(const QString &type, const QString &category = QString())
{
    QStringList files = KGlobal::dirs()->findAllResources("appdata", "plasma/internal/" + type + "/*.desktop", KStandardDirs::NoDuplicates);

    KPluginInfo::List allInfo = KPluginInfo::fromFiles(files);

    if (category.isEmpty() || allInfo.isEmpty()) {
        return allInfo;
    }

    KPluginInfo::List matchingInfo;
    foreach (const KPluginInfo &info, allInfo) {
        if (info.category().compare(category, Qt::CaseInsensitive) == 0) {
            matchingInfo << info;
        }
    }

    return matchingInfo;
}

KPluginInfo::List PluginLoader::standardInternalAppletInfo(const QString &category) const
{
    return standardInternalInfo("applets", category);
}

KPluginInfo::List PluginLoader::standardInternalDataEngineInfo() const
{
    return standardInternalInfo("dataengines");
}

KPluginInfo::List PluginLoader::standardInternalRunnerInfo() const
{
    return standardInternalInfo("runners");
}

KPluginInfo::List PluginLoader::standardInternalServiceInfo() const
{
    return standardInternalInfo("services");
}

} // Plasma Namespace

