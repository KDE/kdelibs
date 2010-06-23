/* This file is part of the KDE libraries
 *  Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
 *  Copyright (C) 2000,2010 Dawit Alemayehu <adawit at kde.org>
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
#include <QtCore/QHashIterator>

typedef QList<KUriFilterPlugin *> KUriFilterPluginList;

static QString lookupIconNameFor(const KUrl &url, KUriFilterData::UriTypes type)
{
    QString iconName;

    switch ( type )
    {
        case KUriFilterData::NetProtocol:
            iconName = KMimeType::favIconForUrl(url);
            if (iconName.isEmpty())
                iconName = KMimeType::iconNameForUrl( url );
            break;
        case KUriFilterData::LocalFile:
        case KUriFilterData::LocalDir:
        {
            iconName = KMimeType::iconNameForUrl( url );
            break;
        }
        case KUriFilterData::Executable:
        {
            QString exeName = url.url();
            exeName = exeName.mid( exeName.lastIndexOf( '/' ) + 1 ); // strip path if given
            KService::Ptr service = KService::serviceByDesktopName( exeName );
            if (service && service->icon() != QLatin1String( "unknown" ))
                iconName = service->icon();
            // Try to find an icon with the same name as the binary (useful for non-kde apps)
            else if ( !KIconLoader::global()->loadIcon( exeName, KIconLoader::NoGroup, 16, KIconLoader::DefaultState, QStringList(), 0, true ).isNull() )
                iconName = exeName;
            else
                // not found, use default
                iconName = QLatin1String("system-run");
            break;
        }
        case KUriFilterData::Help:
        {
            iconName = QLatin1String("khelpcenter");
            break;
        }
        case KUriFilterData::Shell:
        {
            iconName = QLatin1String("konsole");
            break;
        }
        case KUriFilterData::Error:
        case KUriFilterData::Blocked:
        {
            iconName = QLatin1String("error");
            break;
        }
        default:
            break;
    }

    return iconName;
}

class KUriFilterDataPrivate
{
public:
    explicit KUriFilterDataPrivate( const KUrl& u, const QString& typedUrl )
      : checkForExecs(true), wasModified(true), uriType(KUriFilterData::Unknown),
        url(u), typedString(typedUrl)
    {
    }

    void setData( const KUrl& u, const QString& typedUrl )
    {
        checkForExecs = true;
        wasModified = true;
        uriType = KUriFilterData::Unknown;

        url = u;
        typedString = typedUrl;

        errMsg.clear();
        iconName.clear();
        absPath.clear();
        args.clear();
        searchTerm.clear();
        searchProvider.clear();
        alternateDefaultSearchProvider.clear();
        alternateSearchProviders.clear();
        searchProviderInfoList.clear();
    }

    KUriFilterDataPrivate( KUriFilterDataPrivate * data )
    {
        wasModified = data->wasModified;
        checkForExecs = data->checkForExecs;
        uriType = data->uriType;

        url = data->url;
        typedString = data->typedString;

        errMsg = data->errMsg;
        iconName = data->iconName;
        absPath = data->absPath;
        args = data->args;
        searchTerm = data->searchTerm;
        searchProvider = data->searchProvider;
        alternateDefaultSearchProvider = data->alternateDefaultSearchProvider;
        alternateSearchProviders = data->alternateSearchProviders;
        searchProviderInfoList = data->searchProviderInfoList;
    }

    bool checkForExecs;
    bool wasModified;
    KUriFilterData::UriTypes uriType;

    KUrl url;
    QString typedString;
    QString errMsg;
    QString iconName;
    QString absPath;
    QString args;
    QString searchTerm;
    QString searchProvider;
    QString alternateDefaultSearchProvider;
    QChar searchTermSeparator;

    QStringList alternateSearchProviders;
    KUriFilterPlugin::ProviderInfoList searchProviderInfoList;
};

KUriFilterData::KUriFilterData()
               :d( new KUriFilterDataPrivate( KUrl(), QString() ) )
{
}

KUriFilterData::KUriFilterData( const KUrl& url )
               :d( new KUriFilterDataPrivate( url, url.url() ) )
{
}

KUriFilterData::KUriFilterData( const QString& url )
               :d( new KUriFilterDataPrivate( KUrl(url), url ) )
{
}


KUriFilterData::KUriFilterData( const KUriFilterData& other )
               :d( new KUriFilterDataPrivate( other.d ) )
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

QString KUriFilterData::searchTerm() const
{
    return d->searchTerm;
}

QChar KUriFilterData::searchTermSeparator() const
{
    return d->searchTermSeparator;
}

QString KUriFilterData::searchProvider() const
{
    return d->searchProvider;
}

QStringList KUriFilterData::preferredSearchProviders() const
{
    return d->searchProviderInfoList.keys();
}

QString KUriFilterData::queryForPreferredSearchProvider(const QString &provider) const
{
    return d->searchProviderInfoList.value(provider).first;
}

QString KUriFilterData::iconNameForPreferredSearchProvider(const QString &provider) const
{
    return d->searchProviderInfoList.value(provider).second;
}

QStringList KUriFilterData::alternateSearchProviders() const
{
    return d->alternateSearchProviders;
}

QString KUriFilterData::alternateDefaultSearchProvider() const
{
    return d->alternateDefaultSearchProvider;
}

QString KUriFilterData::iconName()
{
    if (d->wasModified) {
        d->iconName = lookupIconNameFor(d->url, d->uriType);
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

void KUriFilterData::setAlternateSearchProviders(const QStringList &providers)
{
    d->alternateSearchProviders = providers;
}

void KUriFilterData::setAlternateDefaultSearchProvider(const QString &provider)
{
    d->alternateDefaultSearchProvider = provider;
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

void KUriFilterPlugin::setSearchProvider( KUriFilterData &data, const QString& provider,
                                          const QString &term, const QChar &separator) const
{
    data.d->searchProvider = provider;
    data.d->searchTerm = term;
    data.d->searchTermSeparator = separator;
}

void KUriFilterPlugin::setPreferredSearchProviders(KUriFilterData &data, const ProviderInfoList &providers) const
{
    data.d->searchProviderInfoList = providers;
}

QString KUriFilterPlugin::iconNameFor(const KUrl& url, KUriFilterData::UriTypes type) const
{
    return lookupIconNameFor(url, type);
}


/*******************************  KUriFilter ******************************/

class KUriFilterPrivate
{
public:
    KUriFilterPrivate() {}
    ~KUriFilterPrivate()
    {
        qDeleteAll(plugins);
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
            if ( it.value()->filterUri( data ) )
                filtered = true;
        }
    } else {
        QListIterator<QString> it (filters);
        while (it.hasNext()) {
            KUriFilterPlugin* plugin = d->plugins.value(it.next());
            if (plugin &&  plugin->filterUri( data ))
                filtered = true;
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

bool KUriFilter::filterSearchUri(KUriFilterData &data)
{
    return filterUri(data, QStringList() << "kuriikwsfilter" << "kurisearchfilter");
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

    // NOTE: Plugin priority is determined by the InitialPreference entry in
    // the .desktop files, so the trader result is already sorted and should
    // not be manually sorted.
}

#include "kurifilter.moc"
