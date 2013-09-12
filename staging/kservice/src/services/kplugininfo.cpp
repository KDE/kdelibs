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

static const char HIDDEN_KEY[] = "Hidden";
static const char NAME_KEY[] = "Name";
static const char COMMENT_KEY[] = "Comment";
static const char ICON_KEY[] = "Icon";
static const char AUTHOR_KEY[] = "X-KDE-PluginInfo-Author";
static const char EMAIL_KEY[] = "X-KDE-PluginInfo-Email";
static const char PLUGINNAME_KEY[] = "X-KDE-PluginInfo-Name";
static const char VERSION_KEY[] = "X-KDE-PluginInfo-Version";
static const char WEBSITE_KEY[] = "X-KDE-PluginInfo-Website";
static const char CATEGORY_KEY[] = "X-KDE-PluginInfo-Category";
static const char LICENSE_KEY[] = "X-KDE-PluginInfo-License";
static const char DEPENDENCIES_KEY[] = "X-KDE-PluginInfo-Depends";
static const char SERVICETYPES_KEY[] = "ServiceTypes";
static const char XKDESERVICETYPES_KEY[] = "X-KDE-ServiceTypes";
static const char ENABLED_KEYBYDEFAULT_KEY[] = "X-KDE-PluginInfo-EnabledByDefault";
static const char ENABLED_KEY[] = "Enabled";


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
    d->hidden = cg.readEntry(HIDDEN_KEY, false);
    if( d->hidden )
        return;

    d->metaData.insert(NAME_KEY, file.readName());
    d->metaData.insert(COMMENT_KEY, file.readComment());
    d->metaData.insert(ICON_KEY, cg.readEntryUntranslated(ICON_KEY));
    d->metaData.insert(AUTHOR_KEY, cg.readEntryUntranslated(AUTHOR_KEY));
    d->metaData.insert(EMAIL_KEY, cg.readEntryUntranslated(EMAIL_KEY ));
    d->metaData.insert(PLUGINNAME_KEY, cg.readEntryUntranslated(PLUGINNAME_KEY));
    d->metaData.insert(VERSION_KEY, cg.readEntryUntranslated(VERSION_KEY));
    d->metaData.insert(WEBSITE_KEY, cg.readEntryUntranslated(WEBSITE_KEY));
    d->metaData.insert(CATEGORY_KEY, cg.readEntryUntranslated(CATEGORY_KEY));
    d->metaData.insert(LICENSE_KEY, cg.readEntryUntranslated(LICENSE_KEY));
    d->metaData.insert(DEPENDENCIES_KEY, cg.readEntryUntranslated(DEPENDENCIES_KEY));
    d->metaData.insert(ENABLED_KEYBYDEFAULT_KEY, cg.readEntryUntranslated(ENABLED_KEYBYDEFAULT_KEY));
    d->enabledbydefault = cg.readEntry(ENABLED_KEYBYDEFAULT_KEY, false);
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

    d->hidden = d->metaData.value(HIDDEN_KEY).toBool();
    if (d->hidden) {
        return;
    }
    d->enabledbydefault = d->metaData.value(ENABLED_KEYBYDEFAULT_KEY).toBool();
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

    d->metaData.insert(NAME_KEY, service->name());
    d->metaData.insert(COMMENT_KEY, service->comment());
    d->metaData.insert(ICON_KEY, service->icon());
    d->metaData.insert(AUTHOR_KEY, service->property(AUTHOR_KEY));
    d->metaData.insert(EMAIL_KEY, service->property(EMAIL_KEY));
    d->metaData.insert(PLUGINNAME_KEY, service->property(PLUGINNAME_KEY));
    d->metaData.insert(VERSION_KEY, service->property(VERSION_KEY));
    d->metaData.insert(WEBSITE_KEY, service->property(WEBSITE_KEY));
    d->metaData.insert(CATEGORY_KEY, service->property(CATEGORY_KEY));
    d->metaData.insert(LICENSE_KEY, service->property(LICENSE_KEY));
    d->metaData.insert(DEPENDENCIES_KEY, service->property(DEPENDENCIES_KEY));
    QVariant tmp = service->property(ENABLED_KEYBYDEFAULT_KEY);
    d->metaData.insert(ENABLED_KEYBYDEFAULT_KEY, (tmp.isValid() ? tmp.toBool() : false));
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
    return d->metaData.value(NAME_KEY).toString();
}

QString KPluginInfo::comment() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(COMMENT_KEY).toString();
}

QString KPluginInfo::icon() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(ICON_KEY).toString();
}

QString KPluginInfo::entryPath() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->entryPath;
}

QString KPluginInfo::author() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(AUTHOR_KEY).toString();
}

QString KPluginInfo::email() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(EMAIL_KEY).toString();
}

QString KPluginInfo::category() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(CATEGORY_KEY).toString();
}

QString KPluginInfo::pluginName() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(PLUGINNAME_KEY).toString();
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
    return d->metaData.value(VERSION_KEY).toString();
}

QString KPluginInfo::website() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(WEBSITE_KEY).toString();
}

QString KPluginInfo::license() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(LICENSE_KEY).toString();
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
    return d->metaData.value(DEPENDENCIES_KEY).toStringList();
}

QStringList KPluginInfo::serviceTypes() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    QStringList serviceTypes = d->metaData.value(XKDESERVICETYPES_KEY).toStringList();
    if (serviceTypes.isEmpty()) {
        serviceTypes = d->metaData.value(SERVICETYPES_KEY).toStringList();
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
        config.writeEntry(pluginName() + ENABLED_KEY, isPluginEnabled());
    } else {
        if (!d->config.isValid()) {
            qWarning() << "no KConfigGroup, cannot save";
            return;
        }
        d->config.writeEntry(pluginName() + ENABLED_KEY, isPluginEnabled());
    }
}

void KPluginInfo::load(const KConfigGroup &config)
{
    KPLUGININFO_ISVALID_ASSERTION;
    //qDebug() << Q_FUNC_INFO;
    if (config.isValid()) {
        setPluginEnabled(config.readEntry(pluginName() + ENABLED_KEY, isPluginEnabledByDefault()));
    } else {
        if (!d->config.isValid()) {
            qWarning() << "no KConfigGroup, cannot load";
            return;
        }
        setPluginEnabled(d->config.readEntry(pluginName() + ENABLED_KEY, isPluginEnabledByDefault()));
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
