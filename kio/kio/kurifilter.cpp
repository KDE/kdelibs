/* This file is part of the KDE libraries
 *  Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
 *  Copyright (C) 2000 Dawit Alemayehu <adawit at kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kurifilter.h"

#include <config.h>
#include <QPixmap>

#include <kdebug.h>
#include <kiconloader.h>
#include <kservicetypetrader.h>
#include <kmimetype.h>

typedef QList<KUriFilterPlugin *> KUriFilterPluginList;

class KUriFilterDataPrivate
{
public:
    explicit KUriFilterDataPrivate( const KUrl& url, const QString& typedUrl ) :
        bCheckForExecutables(true),
        bChanged(true),
        pURI(url),
        iType(KUriFilterData::Unknown),
        typedString(typedUrl)
    {
    }
    void setData( const KUrl& url, const QString& typedUrl )
    {
        bCheckForExecutables = true;
        bChanged = true;
        strErrMsg.clear();
        strIconName.clear();
        pURI = url;
        iType = KUriFilterData::Unknown;
        typedString = typedUrl;
    }
    KUriFilterDataPrivate( KUriFilterDataPrivate * data )
    {
        iType = data->iType;
        pURI = data->pURI;
        strErrMsg = data->strErrMsg;
        strIconName = data->strIconName;
        bChanged = data->bChanged;
        bCheckForExecutables = data->bCheckForExecutables;
        abs_path = data->abs_path;
        typedString = data->typedString;
        args = data->args;
    }
    bool bCheckForExecutables;
    bool bChanged;

    QString strErrMsg;
    QString strIconName;

    KUrl pURI;
    KUriFilterData::UriTypes iType;

    QString abs_path;
    QString args;
    QString typedString;
};

KUriFilterData::KUriFilterData()
    : d( new KUriFilterDataPrivate( KUrl(), QString() ) )
{
}

KUriFilterData::KUriFilterData( const KUrl& url )
    : d( new KUriFilterDataPrivate( url, url.url() ) )
{
}

KUriFilterData::KUriFilterData( const QString& url )
    : d( new KUriFilterDataPrivate( KUrl(url), url ) )
{
    d->typedString = url;
}


KUriFilterData::KUriFilterData( const KUriFilterData& data )
    : d( new KUriFilterDataPrivate( data.d ) )
{
}

KUriFilterData::~KUriFilterData()
{
    delete d;
}

KUrl KUriFilterData::uri() const
{
    return d->pURI;
}

QString KUriFilterData::errorMsg() const
{
    return d->strErrMsg;
}

KUriFilterData::UriTypes KUriFilterData::uriType() const
{
    return d->iType;
}

void KUriFilterData::setData( const KUrl& url )
{
    d->setData(url, url.url());
}

void KUriFilterData::setData( const QString& url )
{
    d->setData(KUrl(url), url);
}

bool KUriFilterData::setAbsolutePath( const QString& absPath )
{
    // Since a malformed URL could possibly be a relative
    // URL we tag it as a possible local resource...
    if( (d->pURI.protocol().isEmpty() || d->pURI.isLocalFile()) )
    {
        d->abs_path = absPath;
        return true;
    }
    return false;
}

QString KUriFilterData::absolutePath() const
{
    return d->abs_path;
}

bool KUriFilterData::hasAbsolutePath() const
{
    return !d->abs_path.isEmpty();
}

QString KUriFilterData::argsAndOptions() const
{
    return d->args;
}

bool KUriFilterData::hasArgsAndOptions() const
{
    return !d->args.isEmpty();
}

QString KUriFilterData::iconName()
{
    if( d->bChanged )
    {
        switch ( d->iType )
        {
            case KUriFilterData::LocalFile:
            case KUriFilterData::LocalDir:
            case KUriFilterData::NetProtocol:
            {
                d->strIconName = KMimeType::iconNameForUrl( d->pURI );
                break;
            }
            case KUriFilterData::Executable:
            {
                QString exeName = d->pURI.url();
                exeName = exeName.mid( exeName.lastIndexOf( '/' ) + 1 ); // strip path if given
                KService::Ptr service = KService::serviceByDesktopName( exeName );
                if (service && service->icon() != QLatin1String( "unknown" ))
                    d->strIconName = service->icon();
                // Try to find an icon with the same name as the binary (useful for non-kde apps)
                else if ( !KIconLoader::global()->loadIcon( exeName, KIconLoader::NoGroup, 16, KIconLoader::DefaultState, QStringList(), 0, true ).isNull() )
                    d->strIconName = exeName;
                else
                    // not found, use default
                    d->strIconName = QLatin1String("system-run");
                break;
            }
            case KUriFilterData::Help:
            {
                d->strIconName = QLatin1String("khelpcenter");
                break;
            }
            case KUriFilterData::Shell:
            {
                d->strIconName = QLatin1String("konsole");
                break;
            }
            case KUriFilterData::Error:
            case KUriFilterData::Blocked:
            {
                d->strIconName = QLatin1String("error");
                break;
            }
            default:
                d->strIconName.clear();
                break;
        }
        d->bChanged = false;
    }
    return d->strIconName;
}

void KUriFilterData::setCheckForExecutables( bool check )
{
    d->bCheckForExecutables = check;
}

bool KUriFilterData::checkForExecutables() const
{
    return d->bCheckForExecutables;
}

QString KUriFilterData::typedString() const
{
    return d->typedString;
}

KUriFilterData& KUriFilterData::operator=( const KUrl& url )
{
    d->setData(url, url.url());
    return *this;
}

KUriFilterData& KUriFilterData::operator=( const QString& url )
{
    d->setData(KUrl(url), url);
    return *this;
}

/*************************  KUriFilterPlugin ******************************/

KUriFilterPlugin::KUriFilterPlugin( const QString & name, QObject *parent )
    : QObject( parent ), d( 0 )
{
    setObjectName( name );
}

KCModule *KUriFilterPlugin::configModule( QWidget*, const char* ) const
{
    return 0;
}

QString KUriFilterPlugin::configName() const
{
    return objectName();
}

void KUriFilterPlugin::setFilteredUri( KUriFilterData& data, const KUrl& uri ) const
{
    if ( data.uri() != uri )
    {
        data.d->pURI = uri;
        data.d->bChanged = true;
    }
}

void KUriFilterPlugin::setErrorMsg ( KUriFilterData& data,
                                     const QString& errmsg ) const
{
    data.d->strErrMsg = errmsg;
}

void KUriFilterPlugin::setUriType ( KUriFilterData& data,
                                    KUriFilterData::UriTypes type) const
{
    data.d->iType = type;
    data.d->bChanged = true;
}

void KUriFilterPlugin::setArguments( KUriFilterData& data,
                                     const QString& args ) const
{
    data.d->args = args;
}

/*******************************  KUriFilter ******************************/

class KUriFilterPrivate
{
public:
    KUriFilterPrivate() {}
    ~KUriFilterPrivate()
    {
        qDeleteAll(lstPlugins);
    }
    QList<KUriFilterPlugin *> lstPlugins;
};

KUriFilter *KUriFilter::self()
{
    K_GLOBAL_STATIC(KUriFilter, m_self)
    return m_self;
}

KUriFilter::KUriFilter()
    : d(new KUriFilterPrivate())
{
    loadPlugins();
}

KUriFilter::~KUriFilter()
{
    delete d;
}

static KUriFilterPlugin* findPluginByName( const KUriFilterPluginList& lst, const QString& name )
{
    for ( KUriFilterPluginList::const_iterator it = lst.begin(), end = lst.end();
          it != end ; ++it ) {
        if ( (*it)->objectName() == name )
            return *it;
    }
    return 0;
}

bool KUriFilter::filterUri( KUriFilterData& data, const QStringList& filters )
{
    KUriFilterPluginList use_plugins;

    // If we have a filter list, only include the once
    // explicitly specified by it. Otherwise, use all available filters...
    if( filters.isEmpty() )
        use_plugins = d->lstPlugins;  // Use everything that is loaded...
    else {
        //kDebug() << "Named plugins requested...";
        for( QStringList::ConstIterator lst = filters.begin(); lst != filters.end(); ++lst ) {
            KUriFilterPlugin* plugin = findPluginByName( d->lstPlugins, *lst );
            if (plugin) {
                //kDebug() << "Will use filter plugin named: " << plugin->objectName();
                use_plugins.append(plugin);
            }
        }
    }

    //kDebug() << "Using" << use_plugins.count() << "out of the"
    //          << d->lstPlugins.count() << "available plugins";
    bool filtered = false;
    for ( KUriFilterPluginList::const_iterator it = use_plugins.constBegin(), end = use_plugins.constEnd();
          it != end; ++it ) {
        //kDebug() << "Using a filter plugin named: " << (*it)->objectName();
        if( (*it)->filterUri( data ))
            filtered = true;
    }
    return filtered;
}

bool KUriFilter::filterUri( KUrl& uri, const QStringList& filters )
{
    KUriFilterData data(uri);
    bool filtered = filterUri( data, filters );
    if( filtered ) uri = data.uri();
    return filtered;
}

bool KUriFilter::filterUri( QString& uri, const QStringList& filters )
{
    KUriFilterData data(uri);
    bool filtered = filterUri( data, filters );
    if( filtered )  uri = data.uri().url();
    return filtered;
}

KUrl KUriFilter::filteredUri( const KUrl &uri, const QStringList& filters )
{
    KUriFilterData data(uri);
    filterUri( data, filters );
    return data.uri();
}

QString KUriFilter::filteredUri( const QString &uri, const QStringList& filters )
{
    KUriFilterData data(uri);
    filterUri( data, filters );
    return data.uri().url();
}

QStringList KUriFilter::pluginNames() const
{
    QStringList list;
    Q_FOREACH( KUriFilterPlugin* plugin, d->lstPlugins )
        list.append(plugin->objectName());
    return list;
}

void KUriFilter::loadPlugins()
{
    const KService::List offers = KServiceTypeTrader::self()->query( "KUriFilter/Plugin" );

    foreach (const KService::Ptr &ptr, offers) {
        KUriFilterPlugin *plugin = ptr->createInstance<KUriFilterPlugin>();
        if (!plugin)
            continue;
        Q_ASSERT( !plugin->objectName().isEmpty() );
        d->lstPlugins.append( plugin );
    }

    // NOTE: Plugin priority is determined by
    // the InitialPreference entry in the .desktop files,
    // so the trader result is already sorted.

    // TODO: Config dialog to differentiate "system"
    // plugins from "user-defined" ones...
    // d->lstPlugins.sort();
}

#include "kurifilter.moc"
