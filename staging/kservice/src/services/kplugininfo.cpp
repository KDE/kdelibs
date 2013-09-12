/*  This file is part of the KDE project
    Copyright (C) 2003,2007 Matthias Kretz <kretz@kde.org>
    Copyright 2013 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kplugininfo.h"
#include <kservicetypetrader.h>
#include <kdesktopfile.h>
#include <kservice.h>
#include <QList>
#include <QDebug>
#include <QDirIterator>
#include <kconfiggroup.h>
#include <qstandardpaths.h>
#include <qdebug.h>

//#ifndef NDEBUG
#define KPLUGININFO_ISVALID_ASSERTION \
    do { \
        if (!d) { \
            qFatal("Accessed invalid KPluginInfo object"); \
        } \
    } while (false)
//#else
//#define KPLUGININFO_ISVALID_ASSERTION
//#endif

static const char HIDDEN[] = "Hidden";
static const char NAME[] = "Name";
static const char COMMENT[] = "Comment";
static const char ICON[] = "Icon";
static const char AUTHOR[] = "X-KDE-PluginInfo-Author";
static const char EMAIL[] = "X-KDE-PluginInfo-Email";
static const char PLUGINNAME[] = "X-KDE-PluginInfo-Name";
static const char VERSION[] = "X-KDE-PluginInfo-Version";
static const char WEBSITE[] = "X-KDE-PluginInfo-Website";
static const char CATEGORY[] = "X-KDE-PluginInfo-Category";
static const char LICENSE[] = "X-KDE-PluginInfo-License";
static const char DEPENDENCIES[] = "X-KDE-PluginInfo-Depends";
static const char SERVICETYPES[] = "ServiceTypes";
static const char XKDESERVICETYPES[] = "X-KDE-ServiceTypes";
static const char ENABLEDBYDEFAULT[] = "X-KDE-PluginInfo-EnabledByDefault";
static const char ENABLED[] = "Enabled";


class KPluginInfoPrivate : public QSharedData
{
    public:
        KPluginInfoPrivate()
            : hidden(false)
            , enabledbydefault(false)
            , pluginenabled(false)
            , kcmservicesCached(false)
            {}

        QString entryPath; // the filename of the file containing all the info
        QString libraryPath;

        bool hidden : 1;
        bool enabledbydefault : 1;
        bool pluginenabled : 1;
        mutable bool kcmservicesCached : 1;

        QVariantMap metaData;
        KConfigGroup config;
        KService::Ptr service;
        mutable QList<KService::Ptr> kcmservices;
};

KPluginInfo::KPluginInfo(const QString & filename /*, QStandardPaths::StandardLocation resource*/)
: d( new KPluginInfoPrivate )
{
    KDesktopFile file(/*resource,*/ filename);

    d->entryPath = filename;

    KConfigGroup cg = file.desktopGroup();
    d->hidden = cg.readEntry(HIDDEN, false);
    if( d->hidden )
        return;

    d->metaData.insert(NAME, file.readName());
    d->metaData.insert(COMMENT, file.readComment());
    d->metaData.insert(ICON, cg.readEntryUntranslated(ICON));
    d->metaData.insert(AUTHOR, cg.readEntryUntranslated(AUTHOR));
    d->metaData.insert(EMAIL, cg.readEntryUntranslated(EMAIL ));
    d->metaData.insert(PLUGINNAME, cg.readEntryUntranslated(PLUGINNAME));
    d->metaData.insert(VERSION, cg.readEntryUntranslated(VERSION));
    d->metaData.insert(WEBSITE, cg.readEntryUntranslated(WEBSITE));
    d->metaData.insert(CATEGORY, cg.readEntryUntranslated(CATEGORY));
    d->metaData.insert(LICENSE, cg.readEntryUntranslated(LICENSE));
    d->metaData.insert(DEPENDENCIES, cg.readEntryUntranslated(DEPENDENCIES));
    d->metaData.insert(ENABLEDBYDEFAULT, cg.readEntryUntranslated(ENABLEDBYDEFAULT));
    d->enabledbydefault = cg.readEntry(ENABLEDBYDEFAULT, false);
}

KPluginInfo::KPluginInfo(const QVariantList &args)
: d( new KPluginInfoPrivate )
{
    static const QString metaData = QStringLiteral("MetaData");

    foreach (const QVariant &v, args) {
        if (v.canConvert<QVariantMap>()) {
            const QVariantMap &m = v.toMap();
            const QVariant &_metadata = m.value(metaData);
            if (_metadata.canConvert<QVariantMap>()) {
                d->metaData = _metadata.value<QVariantMap>();
                break;
            }
        }
    }

    d->hidden = d->metaData.value(HIDDEN).toBool();
    if (d->hidden) {
        return;
    }
    d->enabledbydefault = d->metaData.value(ENABLEDBYDEFAULT).toBool();
}

#ifndef KDE_NO_DEPRECATED
KPluginInfo::KPluginInfo( const KService::Ptr service )
: d( new KPluginInfoPrivate )
{
    if (!service) {
        d = 0; // isValid() == false
        return;
    }
    d->service = service;
    d->entryPath = service->entryPath();

    if ( service->isDeleted() )
    {
        d->hidden = true;
        return;
    }

    d->metaData.insert(NAME, service->name());
    d->metaData.insert(COMMENT, service->comment());
    d->metaData.insert(ICON, service->icon());
    d->metaData.insert(AUTHOR, service->property(AUTHOR));
    d->metaData.insert(EMAIL, service->property(EMAIL));
    d->metaData.insert(PLUGINNAME, service->property(PLUGINNAME));
    d->metaData.insert(VERSION, service->property(VERSION));
    d->metaData.insert(WEBSITE, service->property(WEBSITE));
    d->metaData.insert(CATEGORY, service->property(CATEGORY));
    d->metaData.insert(LICENSE, service->property(LICENSE));
    d->metaData.insert(DEPENDENCIES, service->property(DEPENDENCIES));
    QVariant tmp = service->property(ENABLEDBYDEFAULT);
    d->metaData.insert(ENABLEDBYDEFAULT, (tmp.isValid() ? tmp.toBool() : false));
}
#endif

KPluginInfo::KPluginInfo()
    : d(0) // isValid() == false
{
}

bool KPluginInfo::isValid() const
{
    return d.data() != 0;
}

KPluginInfo::KPluginInfo(const KPluginInfo &rhs)
    : d(rhs.d)
{
}

KPluginInfo &KPluginInfo::operator=(const KPluginInfo &rhs)
{
    d = rhs.d;
    return *this;
}

bool KPluginInfo::operator==(const KPluginInfo &rhs) const
{
    return d == rhs.d;
}

bool KPluginInfo::operator!=(const KPluginInfo &rhs) const
{
    return d != rhs.d;
}

bool KPluginInfo::operator<(const KPluginInfo &rhs) const
{
    if (category() < rhs.category()) {
        return true;
    }
    if (category() == rhs.category()) {
        return name() < rhs.name();
    }
    return false;
}

bool KPluginInfo::operator>(const KPluginInfo &rhs) const
{
    if (category() > rhs.category()) {
        return true;
    }
    if (category() == rhs.category()) {
        return name() > rhs.name();
    }
    return false;
}

KPluginInfo::~KPluginInfo()
{
}

QList<KPluginInfo> KPluginInfo::fromServices(const KService::List &services, const KConfigGroup &config)
{
    QList<KPluginInfo> infolist;
    for( KService::List::ConstIterator it = services.begin();
            it != services.end(); ++it )
    {
        KPluginInfo info(*it);
        info.setConfig(config);
        infolist += info;
    }
    return infolist;
}

QList<KPluginInfo> KPluginInfo::fromFiles(const QStringList &files, const KConfigGroup &config)
{
    QList<KPluginInfo> infolist;
    for( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
    {
        KPluginInfo info(*it);
        info.setConfig(config);
        infolist += info;
    }
    return infolist;
}

QList<KPluginInfo> KPluginInfo::fromKPartsInstanceName(const QString &name, const KConfigGroup &config)
{
    QStringList files;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, name + QStringLiteral("/kpartplugins"), QStandardPaths::LocateDirectory);
    Q_FOREACH(const QString& dir, dirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.desktop"));
        while (it.hasNext()) {
            files.append(it.next());
        }
    }
    return fromFiles(files, config);
}

bool KPluginInfo::isHidden() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->hidden;
}

void KPluginInfo::setPluginEnabled( bool enabled )
{
    KPLUGININFO_ISVALID_ASSERTION;
    //qDebug() << Q_FUNC_INFO;
    d->pluginenabled = enabled;
}

bool KPluginInfo::isPluginEnabled() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    //qDebug() << Q_FUNC_INFO;
    return d->pluginenabled;
}

bool KPluginInfo::isPluginEnabledByDefault() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    //qDebug() << Q_FUNC_INFO;
    return d->enabledbydefault;
}

QString KPluginInfo::name() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(NAME).toString();
}

QString KPluginInfo::comment() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(COMMENT).toString();
}

QString KPluginInfo::icon() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(ICON).toString();
}

QString KPluginInfo::entryPath() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->entryPath;
}

QString KPluginInfo::author() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(AUTHOR).toString();
}

QString KPluginInfo::email() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(EMAIL).toString();
}

QString KPluginInfo::category() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(CATEGORY).toString();
}

QString KPluginInfo::pluginName() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(PLUGINNAME).toString();
}

QString KPluginInfo::libraryPath() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->libraryPath;
}

void KPluginInfo::setLibraryPath(const QString& filename)
{
    KPLUGININFO_ISVALID_ASSERTION;
    d->libraryPath = filename;
}

QString KPluginInfo::version() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(VERSION).toString();
}

QString KPluginInfo::website() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(WEBSITE).toString();
}

QString KPluginInfo::license() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(LICENSE).toString();
}

#if 0
KAboutLicense KPluginInfo::fullLicense() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return KAboutLicense::byKeyword(d->license);
}
#endif

QStringList KPluginInfo::dependencies() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(DEPENDENCIES).toStringList();
}

QStringList KPluginInfo::serviceTypes() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    QStringList serviceTypes = d->metaData.value(XKDESERVICETYPES).toStringList();
    if (serviceTypes.isEmpty()) {
        serviceTypes = d->metaData.value(SERVICETYPES).toStringList();
    }
    return serviceTypes;
}

KService::Ptr KPluginInfo::service() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->service;
}

QList<KService::Ptr> KPluginInfo::kcmServices() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    if ( !d->kcmservicesCached )
    {
        d->kcmservices = KServiceTypeTrader::self()->query( QStringLiteral("KCModule"), QLatin1Char('\'') + pluginName() +
            QStringLiteral("' in [X-KDE-ParentComponents]") );
        //qDebug() << "found" << d->kcmservices.count() << "offers for" << d->pluginName;

        d->kcmservicesCached = true;
    }

    return d->kcmservices;
}

void KPluginInfo::setConfig(const KConfigGroup &config)
{
    KPLUGININFO_ISVALID_ASSERTION;
    d->config = config;
}

KConfigGroup KPluginInfo::config() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->config;
}

QVariant KPluginInfo::property(const QString &key) const
{
    KPLUGININFO_ISVALID_ASSERTION;
    if(d->service) {
        return d->service->property(key);
    }
    return d->metaData.value(key);
}

void KPluginInfo::save(KConfigGroup config)
{
    KPLUGININFO_ISVALID_ASSERTION;
    //qDebug() << Q_FUNC_INFO;
    if (config.isValid()) {
        config.writeEntry(pluginName() + ENABLED, isPluginEnabled());
    } else {
        if (!d->config.isValid()) {
            qWarning() << "no KConfigGroup, cannot save";
            return;
        }
        d->config.writeEntry(pluginName() + ENABLED, isPluginEnabled());
    }
}

void KPluginInfo::load(const KConfigGroup &config)
{
    KPLUGININFO_ISVALID_ASSERTION;
    //qDebug() << Q_FUNC_INFO;
    if (config.isValid()) {
        setPluginEnabled(config.readEntry(pluginName() + ENABLED, isPluginEnabledByDefault()));
    } else {
        if (!d->config.isValid()) {
            qWarning() << "no KConfigGroup, cannot load";
            return;
        }
        setPluginEnabled(d->config.readEntry(pluginName() + ENABLED, isPluginEnabledByDefault()));
    }
}

void KPluginInfo::defaults()
{
    //qDebug() << Q_FUNC_INFO;
    setPluginEnabled( isPluginEnabledByDefault() );
}

uint qHash(const KPluginInfo &p)
{
    return qHash(reinterpret_cast<quint64>(p.d.data()));
}

#undef KPLUGININFO_ISVALID_ASSERTION

// vim: sw=4 sts=4 et
