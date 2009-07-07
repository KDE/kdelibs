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
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kservice.h>
#include <QList>
#include <kconfiggroup.h>

//#ifndef NDEBUG
#define KPLUGININFO_ISVALID_ASSERTION \
    do { \
        if (!d) { \
            kFatal(703) << "Accessed invalid KPluginInfo object"; \
        } \
    } while (false)
//#else
//#define KPLUGININFO_ISVALID_ASSERTION
//#endif

class KPluginInfoPrivate : public QSharedData
{
    public:
        KPluginInfoPrivate()
            : hidden( false )
            , enabledbydefault( false )
            , pluginenabled( false )
            , kcmservicesCached( false )
            {}

        QString entryPath; // the filename of the file containing all the info
        QString name;
        QString comment;
        QString icon;
        QString author;
        QString email;
        QString pluginName; // the name attribute in the .rc file
        QString version;
        QString website; // URL to the website of the plugin/author
        QString category;
        QString license;
        QStringList dependencies;

        bool hidden : 1;
        bool enabledbydefault : 1;
        bool pluginenabled : 1;
        mutable bool kcmservicesCached : 1;

        KConfigGroup config;
        KService::Ptr service;
        mutable QList<KService::Ptr> kcmservices;

    static int debugArea() {
        static int s_area = KDebug::registerArea("kdecore (KPluginInfo)");
        return s_area;
    }
};

KPluginInfo::KPluginInfo( const QString & filename, const char* resource )
: d( new KPluginInfoPrivate )
{
    KDesktopFile file( resource, filename );

    d->entryPath = filename;

    KConfigGroup cg = file.desktopGroup();
    d->hidden = cg.readEntry("Hidden", false);
    if( d->hidden )
        return;

    d->name = file.readName();
    d->comment = file.readComment();
    d->icon = cg.readEntryUntranslated( "Icon" );
    d->author = cg.readEntryUntranslated( "X-KDE-PluginInfo-Author" );
    d->email = cg.readEntryUntranslated( "X-KDE-PluginInfo-Email" );
    d->pluginName = cg.readEntryUntranslated( "X-KDE-PluginInfo-Name" );
    d->version = cg.readEntryUntranslated( "X-KDE-PluginInfo-Version" );
    d->website = cg.readEntryUntranslated( "X-KDE-PluginInfo-Website" );
    d->category = cg.readEntryUntranslated( "X-KDE-PluginInfo-Category" );
    d->license = cg.readEntryUntranslated( "X-KDE-PluginInfo-License" );
    d->dependencies = cg.readEntry( "X-KDE-PluginInfo-Depends", QStringList() );
    d->enabledbydefault = cg.readEntry(
            "X-KDE-PluginInfo-EnabledByDefault", false);
}

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

    d->name = service->name();
    d->comment = service->comment();
    d->icon = service->icon();
    d->author = service->property( "X-KDE-PluginInfo-Author" ).toString();
    d->email = service->property( "X-KDE-PluginInfo-Email" ).toString();
    d->pluginName = service->property( "X-KDE-PluginInfo-Name" ).toString();
    d->version = service->property( "X-KDE-PluginInfo-Version" ).toString();
    d->website = service->property( "X-KDE-PluginInfo-Website" ).toString();
    d->category = service->property( "X-KDE-PluginInfo-Category" ).toString();
    d->license = service->property( "X-KDE-PluginInfo-License" ).toString();
    d->dependencies =
        service->property( "X-KDE-PluginInfo-Depends" ).toStringList();
    QVariant tmp = service->property( "X-KDE-PluginInfo-EnabledByDefault" );
    d->enabledbydefault = tmp.isValid() ? tmp.toBool() : false;
}

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
    QStringList files = KGlobal::dirs()->findAllResources( "data",
                                                           name + "/kpartplugins/*.desktop",
                                                           KStandardDirs::Recursive );
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
    //kDebug( d->debugArea() ) ;
    d->pluginenabled = enabled;
}

bool KPluginInfo::isPluginEnabled() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    //kDebug( d->debugArea() ) ;
    return d->pluginenabled;
}

bool KPluginInfo::isPluginEnabledByDefault() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    //kDebug( d->debugArea() ) ;
    return d->enabledbydefault;
}

QString KPluginInfo::name() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->name;
}

QString KPluginInfo::comment() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->comment;
}

QString KPluginInfo::icon() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->icon;
}

QString KPluginInfo::entryPath() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->entryPath;
}

QString KPluginInfo::author() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->author;
}

QString KPluginInfo::email() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->email;
}

QString KPluginInfo::category() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->category;
}

QString KPluginInfo::pluginName() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->pluginName;
}

QString KPluginInfo::version() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->version;
}

QString KPluginInfo::website() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->website;
}

QString KPluginInfo::license() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->license;
}

KAboutLicense KPluginInfo::fullLicense() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return KAboutLicense::byKeyword(d->license);
}

QStringList KPluginInfo::dependencies() const
{
    KPLUGININFO_ISVALID_ASSERTION;
    return d->dependencies;
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
        d->kcmservices = KServiceTypeTrader::self()->query( "KCModule", '\'' + d->pluginName +
            "' in [X-KDE-ParentComponents]" );
        kDebug( d->debugArea() ) << "found " << d->kcmservices.count() << " offers for " <<
            d->pluginName << endl;

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
        return QVariant();
}

void KPluginInfo::save(KConfigGroup config)
{
    KPLUGININFO_ISVALID_ASSERTION;
    //kDebug( d->debugArea() ) ;
    if (config.isValid()) {
        config.writeEntry(d->pluginName + "Enabled", isPluginEnabled());
    } else {
        if (!d->config.isValid()) {
            kWarning( d->debugArea() ) << "no KConfigGroup, cannot save";
            return;
        }
        d->config.writeEntry(d->pluginName + "Enabled", isPluginEnabled());
    }
}

void KPluginInfo::load(const KConfigGroup &config)
{
    KPLUGININFO_ISVALID_ASSERTION;
    //kDebug( d->debugArea() ) ;
    if (config.isValid()) {
        setPluginEnabled(config.readEntry(d->pluginName + "Enabled", isPluginEnabledByDefault()));
    } else {
        if (!d->config.isValid()) {
            kWarning( d->debugArea() ) << "no KConfigGroup, cannot load";
            return;
        }
        setPluginEnabled(d->config.readEntry(d->pluginName + "Enabled", isPluginEnabledByDefault()));
    }
}

void KPluginInfo::defaults()
{
    //kDebug( d->debugArea() ) ;
    setPluginEnabled( isPluginEnabledByDefault() );
}

uint qHash(const KPluginInfo &p)
{
    return qHash(reinterpret_cast<quint64>(p.d.data()));
}

#undef KPLUGININFO_ISVALID_ASSERTION

// vim: sw=4 sts=4 et
