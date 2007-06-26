/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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
#include <kconfig.h>
#include <kservicetypetrader.h>
#include <kdebug.h>
#include <kconfigbase.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kservice.h>
#include <QList>
#include <kconfiggroup.h>

class KPluginInfo::KPluginInfoPrivate
{
    public:
        KPluginInfoPrivate()
            : hidden( false )
            , enabledbydefault( false )
            , pluginenabled( false )
            , kcmservicesCached( false )
            {}

        QString specfile; // the filename of the file containing all the info
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

        bool hidden;
        bool enabledbydefault;
        bool pluginenabled;

        bool kcmservicesCached;
        KConfigGroup config;
        KService::Ptr service;
        QList<KService::Ptr> kcmservices;
};

KPluginInfo::KPluginInfo( const QString & filename, const char* resource )
: d( new KPluginInfoPrivate )
{
    KDesktopFile file( resource, filename );

    d->specfile = filename;

    KConfigGroup cg = file.desktopGroup();
    d->hidden = cg.readEntry("Hidden", false);
    if( d->hidden )
        return;

    d->name = file.readName();
    d->comment = file.readComment();
    d->icon = cg.readEntry( "Icon" );
    d->author = cg.readEntry( "X-KDE-PluginInfo-Author" );
    d->email = cg.readEntry( "X-KDE-PluginInfo-Email" );
    d->pluginName = cg.readEntry( "X-KDE-PluginInfo-Name" );
    d->version = cg.readEntry( "X-KDE-PluginInfo-Version" );
    d->website = cg.readEntry( "X-KDE-PluginInfo-Website" );
    d->category = cg.readEntry( "X-KDE-PluginInfo-Category" );
    d->license = cg.readEntry( "X-KDE-PluginInfo-License" );
    d->dependencies = cg.readEntry( "X-KDE-PluginInfo-Depends", QStringList() );
    d->enabledbydefault = cg.readEntry(
            "X-KDE-PluginInfo-EnabledByDefault", false);
}

KPluginInfo::KPluginInfo( const KService::Ptr service )
: d( new KPluginInfoPrivate )
{
    d->service = service;
    d->specfile = service->desktopEntryPath();

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

//X KPluginInfo::KPluginInfo()
//X : d( new KPluginInfoPrivate )
//X {
//X     d->hidden = true;
//X }

KPluginInfo::~KPluginInfo()
{
    delete d;
}

QList<KPluginInfo*> KPluginInfo::fromServices(const KService::List &services, const KConfigGroup &config)
{
    QList<KPluginInfo*> infolist;
    KPluginInfo * info;
    for( KService::List::ConstIterator it = services.begin();
            it != services.end(); ++it )
    {
        info = new KPluginInfo( *it );
        info->setConfig(config);
        infolist += info;
    }
    return infolist;
}

QList<KPluginInfo*> KPluginInfo::fromFiles(const QStringList &files, const KConfigGroup &config)
{
    QList<KPluginInfo*> infolist;
    for( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
    {
        KPluginInfo * info = new KPluginInfo( *it );
        info->setConfig(config);
        infolist += info;
    }
    return infolist;
}

QList<KPluginInfo*> KPluginInfo::fromKPartsInstanceName(const QString &name, const KConfigGroup &config)
{
    QStringList files = KGlobal::dirs()->findAllResources( "data",
                                                           name + "/kpartplugins/*.desktop",
                                                           KStandardDirs::Recursive );
    return fromFiles(files, config);
}

bool KPluginInfo::isHidden() const
{
    return d->hidden;
}

void KPluginInfo::setPluginEnabled( bool enabled )
{
    kDebug( 703 ) << k_funcinfo << endl;
    d->pluginenabled = enabled;
}

bool KPluginInfo::isPluginEnabled() const
{
    kDebug( 703 ) << k_funcinfo << endl;
    return d->pluginenabled;
}

bool KPluginInfo::isPluginEnabledByDefault() const
{
    kDebug( 703 ) << k_funcinfo << endl;
    return d->enabledbydefault;
}

QString KPluginInfo::name() const
{
    return d->name;
}

QString KPluginInfo::comment() const
{
    return d->comment;
}

QString KPluginInfo::icon() const
{
    return d->icon;
}

QString KPluginInfo::specfile() const
{
    return d->specfile;
}

QString KPluginInfo::author() const
{
    return d->author;
}

QString KPluginInfo::email() const
{
    return d->email;
}

QString KPluginInfo::category() const
{
    return d->category;
}

QString KPluginInfo::pluginName() const
{
    return d->pluginName;
}

QString KPluginInfo::version() const
{
    return d->version;
}

QString KPluginInfo::website() const
{
    return d->website;
}

QString KPluginInfo::license() const
{
    return d->license;
}

QStringList KPluginInfo::dependencies() const
{
    return d->dependencies;
}

KService::Ptr KPluginInfo::service() const
{
    return d->service;
}

QList<KService::Ptr> KPluginInfo::kcmServices() const
{
    if ( !d->kcmservicesCached )
    {
        d->kcmservices = KServiceTypeTrader::self()->query( "KCModule", '\'' + d->pluginName +
            "' in [X-KDE-ParentComponents]" );
        kDebug( 703 ) << "found " << d->kcmservices.count() << " offers for " <<
            d->pluginName << endl;

        d->kcmservicesCached = true;
    }

    return d->kcmservices;
}

void KPluginInfo::setConfig(const KConfigGroup &config)
{
    d->config = config;
}

KConfigGroup KPluginInfo::config() const
{
    return d->config;
}

QVariant KPluginInfo::property( const QString & key ) const
{
    if( d->service )
        return d->service->property( key );
    else
        return QVariant();
}

QVariant KPluginInfo::operator[]( const QString & key ) const
{
    return property( key );
}

void KPluginInfo::save(KConfigGroup config)
{
    kDebug( 703 ) << k_funcinfo << endl;
    if (config.isValid()) {
        config.writeEntry(d->pluginName + "Enabled", isPluginEnabled());
    } else {
        if (!d->config.isValid()) {
            kWarning( 703 ) << "no KConfigGroup, cannot save" << endl;
            return;
        }
        d->config.writeEntry(d->pluginName + "Enabled", isPluginEnabled());
    }
}

void KPluginInfo::load(const KConfigGroup &config)
{
    kDebug( 703 ) << k_funcinfo << endl;
    if (config.isValid()) {
        setPluginEnabled(config.readEntry(d->pluginName + "Enabled", isPluginEnabledByDefault()));
    } else {
        if (!d->config.isValid()) {
            kWarning( 703 ) << "no KConfigGroup, cannot load" << endl;
            return;
        }
        setPluginEnabled(d->config.readEntry(d->pluginName + "Enabled", isPluginEnabledByDefault()));
    }
}

void KPluginInfo::defaults()
{
    kDebug( 703 ) << k_funcinfo << endl;
    setPluginEnabled( isPluginEnabledByDefault() );
}

// vim: sw=4 sts=4 et
