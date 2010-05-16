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
#include <config.h>

#include "kurifilter.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <kservicetypetrader.h>
#include <kmimetype.h>

#include <QtGui/QPixmap>
#include <QtCore/QHash>
#include <QtCore/QHashIterator>

typedef QList<KUriFilterPlugin *> KUriFilterPluginList;

class KUriFilterDataPrivate
{
public:
    explicit KUriFilterDataPrivate( const KUrl& u, const QString& typedUrl )
      : checkForExecs(true), wasModified(true), url(u),
        uriType(KUriFilterData::Unknown), typedString(typedUrl)
    {
    }

    void setData( const KUrl& u, const QString& typedUrl )
    {
        checkForExecs = true;
        wasModified = true;
        errMsg.clear();
        iconName.clear();
        url = u;
        uriType = KUriFilterData::Unknown;
        typedString = typedUrl;
    }

    KUriFilterDataPrivate( KUriFilterDataPrivate * data )
    {
        uriType = data->uriType;
        url = data->url;
        errMsg = data->errMsg;
        iconName = data->iconName;
        wasModified = data->wasModified;
        checkForExecs = data->checkForExecs;
        absPath = data->absPath;
        typedString = data->typedString;
        args = data->args;
    }

    bool checkForExecs;
    bool wasModified;

    KUrl url;
    KUriFilterData::UriTypes uriType;

    QString errMsg;
    QString iconName;
    QString absPath;
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


KUriFilterData::KUriFilterData( const KUriFilterData& other )
    : d( new KUriFilterDataPrivate( other.d ) )
{
}

KUriFilterData::~KUriFilterData()
{
    delete d;
}

KUrl KUriFilterData::uri() const
{
    return d->url;
}

QString KUriFilterData::errorMsg() const
{
    return d->errMsg;
}

KUriFilterData::UriTypes KUriFilterData::uriType() const
{
    return d->uriType;
}

QString KUriFilterData::absolutePath() const
{
    return d->absPath;
}

bool KUriFilterData::hasAbsolutePath() const
{
    return !d->absPath.isEmpty();
}

QString KUriFilterData::argsAndOptions() const
{
    return d->args;
}

bool KUriFilterData::hasArgsAndOptions() const
{
    return !d->args.isEmpty();
}

bool KUriFilterData::checkForExecutables() const
{
    return d->checkForExecs;
}

QString KUriFilterData::typedString() const
{
    return d->typedString;
}

QString KUriFilterData::iconName()
{
    if( d->wasModified )
    {
        switch ( d->uriType )
        {
            case KUriFilterData::LocalFile:
            case KUriFilterData::LocalDir:
            case KUriFilterData::NetProtocol:
            {
                d->iconName = KMimeType::iconNameForUrl( d->url );
                break;
            }
            case KUriFilterData::Executable:
            {
                QString exeName = d->url.url();
                exeName = exeName.mid( exeName.lastIndexOf( '/' ) + 1 ); // strip path if given
                KService::Ptr service = KService::serviceByDesktopName( exeName );
                if (service && service->icon() != QLatin1String( "unknown" ))
                    d->iconName = service->icon();
                // Try to find an icon with the same name as the binary (useful for non-kde apps)
                else if ( !KIconLoader::global()->loadIcon( exeName, KIconLoader::NoGroup, 16, KIconLoader::DefaultState, QStringList(), 0, true ).isNull() )
                    d->iconName = exeName;
                else
                    // not found, use default
                    d->iconName = QLatin1String("system-run");
                break;
            }
            case KUriFilterData::Help:
            {
                d->iconName = QLatin1String("khelpcenter");
                break;
            }
            case KUriFilterData::Shell:
            {
                d->iconName = QLatin1String("konsole");
                break;
            }
            case KUriFilterData::Error:
            case KUriFilterData::Blocked:
            {
                d->iconName = QLatin1String("error");
                break;
            }
            default:
                d->iconName.clear();
                break;
        }
        d->wasModified = false;
    }
    return d->iconName;
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
    if( (d->url.protocol().isEmpty() || d->url.isLocalFile()) )
    {
        d->absPath = absPath;
        return true;
    }
    return false;
}

void KUriFilterData::setCheckForExecutables( bool check )
{
    d->checkForExecs = check;
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
                 :QObject( parent ), d( 0 )
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
        data.d->url = uri;
        data.d->wasModified = true;
    }
}

void KUriFilterPlugin::setErrorMsg ( KUriFilterData& data,
                                     const QString& errmsg ) const
{
    data.d->errMsg = errmsg;
}

void KUriFilterPlugin::setUriType ( KUriFilterData& data,
                                    KUriFilterData::UriTypes type) const
{
    data.d->uriType = type;
    data.d->wasModified = true;
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
        qDeleteAll(plugins.values());
        plugins.clear();
    }
    QHash<QString, KUriFilterPlugin *> plugins;
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

bool KUriFilter::filterUri( KUriFilterData& data, const QStringList& filters )
{
    bool filtered = false;

    // If no specific filters were requested, iterate through all the plugins.
    // Otherwise, only use available filters.
    if( filters.isEmpty() ) {
        QHashIterator<QString, KUriFilterPlugin *> it (d->plugins);
        while (it.hasNext()) {
            it.next();
            filtered |= it.value()->filterUri( data );
        }
    } else {
        QListIterator<QString> it (filters);
        while (it.hasNext()) {
            KUriFilterPlugin* plugin = d->plugins.value(it.next());
            if (plugin)
                filtered |= plugin->filterUri( data );
        }
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
    return d->plugins.keys();
}

void KUriFilter::loadPlugins()
{
    const KService::List offers = KServiceTypeTrader::self()->query( "KUriFilter/Plugin" );

    Q_FOREACH (const KService::Ptr &ptr, offers) {
        KUriFilterPlugin *plugin = ptr->createInstance<KUriFilterPlugin>();
        if (plugin) {
            Q_ASSERT( !plugin->objectName().isEmpty() );
            d->plugins.insert(plugin->objectName(), plugin );
        }
    }

    // NOTE: Plugin priority is determined by
    // the InitialPreference entry in the .desktop files,
    // so the trader result is already sorted.

    // TODO: Config dialog to differentiate "system"
    // plugins from "user-defined" ones...
    // d->plugins.sort();
}

#include "kurifilter.moc"
