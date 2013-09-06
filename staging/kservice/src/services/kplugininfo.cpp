/*  This file is part of the KDE project
    Copyright (C) 2003,2007 Matthias Kretz <kretz@kde.org>

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

class KPluginInfoPrivate : public QSharedData
{
    public:
        KPluginInfoPrivate()
            : _hidden(QStringLiteral("Hidden"))
            , _name(QStringLiteral("Name"))
            , _comment(QStringLiteral("Comment"))
            , _icon(QStringLiteral("Icon"))
            , _author(QStringLiteral("X-KDE-PluginInfo-Author"))
            , _email(QStringLiteral("X-KDE-PluginInfo-Email"))
            , _pluginName(QStringLiteral("X-KDE-PluginInfo-Name"))
            , _version(QStringLiteral("X-KDE-PluginInfo-Version"))
            , _website(QStringLiteral("X-KDE-PluginInfo-Website"))
            , _category(QStringLiteral("X-KDE-PluginInfo-Category"))
            , _license(QStringLiteral("X-KDE-PluginInfo-License"))
            , _dependencies(QStringLiteral("X-KDE-PluginInfo-Depends"))
            , _serviceTypes(QStringLiteral("ServiceTypes"))
            , _xKdeServiceTypes(QStringLiteral("X-KDE-ServiceTypes"))
            , _enabledbydefault(QStringLiteral("X-KDE-PluginInfo-EnabledByDefault"))
            , hidden(false)
            , enabledbydefault(false)
            , pluginenabled(false)
            , kcmservicesCached(false)
            {}

        QString _hidden;
        QString _name;
        QString _comment;
        QString _icon;
        QString _author;
        QString _email;
        QString _pluginName; // the name attribute in the .rc file
        QString _version;
        QString _website; // URL to the website of the plugin/author
        QString _category;
        QString _license;
        QString _dependencies;
        QString _serviceTypes;
        QString _xKdeServiceTypes;
        QString _enabledbydefault;


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
    d->hidden = cg.readEntry(d->_hidden, false);
    if( d->hidden )
        return;

    d->metaData.insert(d->_name, file.readName());
    d->metaData.insert(d->_comment, file.readComment());
    d->metaData.insert(d->_icon, cg.readEntryUntranslated("Icon"));
    d->metaData.insert(d->_author, cg.readEntryUntranslated(d->_author));
    d->metaData.insert(d->_email, cg.readEntryUntranslated( d->_email ));
    d->metaData.insert(d->_pluginName, cg.readEntryUntranslated(d->_pluginName));
    d->metaData.insert(d->_version, cg.readEntryUntranslated(d->_version));
    d->metaData.insert(d->_website, cg.readEntryUntranslated(d->_website));
    d->metaData.insert(d->_category, cg.readEntryUntranslated(d->_category));
    d->metaData.insert(d->_license, cg.readEntryUntranslated(d->_license));
    d->metaData.insert(d->_dependencies, cg.readEntryUntranslated(d->_dependencies));
    d->metaData.insert(d->_enabledbydefault, cg.readEntryUntranslated(d->_enabledbydefault));
    d->enabledbydefault = cg.readEntry(d->_enabledbydefault, false);
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

    d->hidden = d->metaData.value(QStringLiteral("Hidden")).toBool();
    if (d->hidden) {
        return;
    }
    d->enabledbydefault = d->metaData.value(d->_enabledbydefault).toBool();
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

    d->metaData.insert(d->_name, service->name());
    d->metaData.insert(d->_comment, service->comment());
    d->metaData.insert(d->_icon, service->icon());
    d->metaData.insert(d->_author, service->property(d->_author));
    d->metaData.insert(d->_email, service->property(d->_email));
    d->metaData.insert(d->_pluginName, service->property(d->_pluginName));
    d->metaData.insert(d->_version, service->property(d->_version));
    d->metaData.insert(d->_website, service->property(d->_website));
    d->metaData.insert(d->_category, service->property(d->_category));
    d->metaData.insert(d->_license, service->property(d->_license));
    d->metaData.insert(d->_dependencies, service->property(d->_dependencies));
    QVariant tmp = service->property(d->_enabledbydefault);
    d->metaData.insert(d->_enabledbydefault, (tmp.isValid() ? tmp.toBool() : false));
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
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, name + QLatin1String("/kpartplugins"), QStandardPaths::LocateDirectory);
    Q_FOREACH(const QString& dir, dirs) {
        QDirIterator it(dir, QStringList() << QLatin1String("*.desktop"));
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
    return d->metaData.value(d->_name).toString();
}

QString KPluginInfo::comment() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(d->_comment).toString();
}

QString KPluginInfo::icon() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(d->_icon).toString();
}

QString KPluginInfo::entryPath() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->entryPath;
}

QString KPluginInfo::author() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(d->_author).toString();
}

QString KPluginInfo::email() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(d->_email).toString();
}

QString KPluginInfo::category() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(d->_category).toString();
}

QString KPluginInfo::pluginName() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(d->_pluginName).toString();
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
    return d->metaData.value(d->_version).toString();
}

QString KPluginInfo::website() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(d->_website).toString();
}

QString KPluginInfo::license() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->metaData.value(d->_license).toString();
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
    return d->metaData.value(d->_dependencies).toStringList();
}

QStringList KPluginInfo::serviceTypes() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    QStringList serviceTypes = d->metaData.value(d->_xKdeServiceTypes).toStringList();
    if (serviceTypes.isEmpty()) {
        serviceTypes = d->metaData.value(d->_serviceTypes).toStringList();
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
        d->kcmservices = KServiceTypeTrader::self()->query( QLatin1String("KCModule"), QLatin1Char('\'') + pluginName() +
            QString::fromLatin1("' in [X-KDE-ParentComponents]") );
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

QVariant KPluginInfo::property( const QString & key ) const
{
    KPLUGININFO_ISVALID_ASSERTION;
    if( d->service )
        return d->service->property( key );
    else
        return d->metaData.value(key);
}

void KPluginInfo::save(KConfigGroup config)
{
    KPLUGININFO_ISVALID_ASSERTION;
    //qDebug() << Q_FUNC_INFO;
    if (config.isValid()) {
        config.writeEntry(pluginName() + QString::fromLatin1("Enabled"), isPluginEnabled());
    } else {
        if (!d->config.isValid()) {
            qWarning() << "no KConfigGroup, cannot save";
            return;
        }
        d->config.writeEntry(pluginName() + QString::fromLatin1("Enabled"), isPluginEnabled());
    }
}

void KPluginInfo::load(const KConfigGroup &config)
{
    KPLUGININFO_ISVALID_ASSERTION;
    //qDebug() << Q_FUNC_INFO;
    if (config.isValid()) {
        setPluginEnabled(config.readEntry(pluginName() + QString::fromLatin1("Enabled"), isPluginEnabledByDefault()));
    } else {
        if (!d->config.isValid()) {
            qWarning() << "no KConfigGroup, cannot load";
            return;
        }
        setPluginEnabled(d->config.readEntry(pluginName() + QString::fromLatin1("Enabled"), isPluginEnabledByDefault()));
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
