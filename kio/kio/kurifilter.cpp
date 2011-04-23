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

#include "kurifilter.h"

#include "hostinfo_p.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <kservicetypetrader.h>
#include <kmimetype.h>
#include <kstandarddirs.h>

#include <QtGui/QPixmap>
#include <QtCore/QThread>
#include <QtCore/QHashIterator>
#include <QtCore/QStringBuilder>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QHostAddress>

typedef QList<KUriFilterPlugin *> KUriFilterPluginList;
typedef QMap<QString, KUriFilterSearchProvider*> SearchProviderMap;


static QString lookupIconNameFor(const KUrl &url, KUriFilterData::UriTypes type)
{
    QString iconName;

    switch ( type )
    {
        case KUriFilterData::NetProtocol:
            iconName = KMimeType::favIconForUrl(url);
            if (iconName.isEmpty())
                iconName = KMimeType::iconNameForUrl( url );
            else
                iconName = KStandardDirs::locate("cache", iconName + QLatin1String(".png"));
            break;
        case KUriFilterData::LocalFile:
        case KUriFilterData::LocalDir:
        {
            iconName = KMimeType::iconNameForUrl( url );
            break;
        }
        case KUriFilterData::Executable:
        {
            QString exeName = url.path();
            exeName = exeName.mid( exeName.lastIndexOf( '/' ) + 1 ); // strip path if given
            KService::Ptr service = KService::serviceByDesktopName( exeName );
            if (service && service->icon() != QLatin1String( "unknown" ))
                iconName = service->icon();
            // Try to find an icon with the same name as the binary (useful for non-kde apps)
            // Use iconPath rather than loadIcon() as the latter uses QPixmap (not threadsafe)
            else if ( !KIconLoader::global()->iconPath( exeName, KIconLoader::NoGroup, true ).isNull() )
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


class KUriFilterSearchProvider::KUriFilterSearchProviderPrivate
{
public:
    KUriFilterSearchProviderPrivate() {}
    KUriFilterSearchProviderPrivate(const KUriFilterSearchProviderPrivate& other)
      : desktopEntryName(other.desktopEntryName),
        iconName(other.iconName),
        name(other.name),
        keys(other.keys) {}


    QString desktopEntryName;
    QString iconName;
    QString name;
    QStringList keys;
};

KUriFilterSearchProvider::KUriFilterSearchProvider()
                         :d(new KUriFilterSearchProvider::KUriFilterSearchProviderPrivate)
{
}

KUriFilterSearchProvider::KUriFilterSearchProvider(const KUriFilterSearchProvider& other)
                         :d(new KUriFilterSearchProvider::KUriFilterSearchProviderPrivate(*(other.d)))
{
}

KUriFilterSearchProvider::~KUriFilterSearchProvider()
{
    delete d;
}

QString KUriFilterSearchProvider::desktopEntryName() const
{
    return d->desktopEntryName;
}

QString KUriFilterSearchProvider::iconName() const
{
    return d->iconName;
}

QString KUriFilterSearchProvider::name() const
{
    return d->name;
}

QStringList KUriFilterSearchProvider::keys() const
{
    return d->keys;
}

QString KUriFilterSearchProvider::defaultKey() const
{
    if (d->keys.isEmpty())
        return QString();

    return d->keys.first();
}

KUriFilterSearchProvider& KUriFilterSearchProvider::operator=(const KUriFilterSearchProvider& other)
{
    d->desktopEntryName = other.d->desktopEntryName;
    d->iconName = other.d->iconName;
    d->keys = other.d->keys;
    d->name = other.d->name;
    return *this;
}

void KUriFilterSearchProvider::setDesktopEntryName(const QString& desktopEntryName)
{
    d->desktopEntryName = desktopEntryName;
}

void KUriFilterSearchProvider::setIconName(const QString& iconName)
{
    d->iconName = iconName;
}

void KUriFilterSearchProvider::setName(const QString& name)
{
    d->name = name;
}

void KUriFilterSearchProvider::setKeys(const QStringList& keys)
{
    d->keys = keys;
}

class KUriFilterDataPrivate
{
public:
    explicit KUriFilterDataPrivate( const KUrl& u, const QString& typedUrl )
      : checkForExecs(true),
        wasModified(true),
        uriType(KUriFilterData::Unknown),
        searchFilterOptions(KUriFilterData::SearchFilterOptionNone),
        url(u),
        typedString(typedUrl)
    {
    }

    ~KUriFilterDataPrivate()
    {
        qDeleteAll(searchProviderMap.begin(), searchProviderMap.end());
    }

    void setData( const KUrl& u, const QString& typedUrl )
    {
        checkForExecs = true;
        wasModified = true;
        uriType = KUriFilterData::Unknown;
        searchFilterOptions = KUriFilterData::SearchFilterOptionNone;

        url = u;
        typedString = typedUrl;

        errMsg.clear();
        iconName.clear();
        absPath.clear();
        args.clear();
        searchTerm.clear();
        searchProvider.clear();
        searchTermSeparator = QChar();
        alternateDefaultSearchProvider.clear();
        alternateSearchProviders.clear();
        searchProviderMap.clear();
        defaultUrlScheme.clear();
    }

    KUriFilterDataPrivate( KUriFilterDataPrivate * data )
    {
        wasModified = data->wasModified;
        checkForExecs = data->checkForExecs;
        uriType = data->uriType;
        searchFilterOptions = data->searchFilterOptions;

        url = data->url;
        typedString = data->typedString;

        errMsg = data->errMsg;
        iconName = data->iconName;
        absPath = data->absPath;
        args = data->args;
        searchTerm = data->searchTerm;
        searchTermSeparator = data->searchTermSeparator;
        searchProvider = data->searchProvider;
        alternateDefaultSearchProvider = data->alternateDefaultSearchProvider;
        alternateSearchProviders = data->alternateSearchProviders;
        searchProviderMap = data->searchProviderMap;
        defaultUrlScheme = data->defaultUrlScheme;
    }

    bool checkForExecs;
    bool wasModified;
    KUriFilterData::UriTypes uriType;
    KUriFilterData::SearchFilterOptions searchFilterOptions;

    KUrl url;
    QString typedString;
    QString errMsg;
    QString iconName;
    QString absPath;
    QString args;
    QString searchTerm;
    QString searchProvider;
    QString alternateDefaultSearchProvider;
    QString defaultUrlScheme;
    QChar searchTermSeparator;

    QStringList alternateSearchProviders;
    QStringList searchProviderList;
    SearchProviderMap searchProviderMap;
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
    return d->searchProviderList;
}

KUriFilterSearchProvider KUriFilterData::queryForSearchProvider(const QString& provider) const
{
    const KUriFilterSearchProvider* searchProvider = d->searchProviderMap.value(provider);

    if (searchProvider)
        return *(searchProvider);

    return KUriFilterSearchProvider();
}

QString KUriFilterData::queryForPreferredSearchProvider(const QString& provider) const
{
    const KUriFilterSearchProvider* searchProvider = d->searchProviderMap.value(provider);
    if (searchProvider)
        return (searchProvider->defaultKey() % searchTermSeparator() % searchTerm());
    return QString();
}

QStringList KUriFilterData::allQueriesForSearchProvider(const QString& provider) const
{
    const KUriFilterSearchProvider* searchProvider = d->searchProviderMap.value(provider);
    if (searchProvider)
        return searchProvider->keys();
    return QStringList();
}

QString KUriFilterData::iconNameForPreferredSearchProvider(const QString &provider) const
{
    const KUriFilterSearchProvider* searchProvider = d->searchProviderMap.value(provider);
    if (searchProvider)
        return searchProvider->iconName();
    return QString();
}

QStringList KUriFilterData::alternateSearchProviders() const
{
    return d->alternateSearchProviders;
}

QString KUriFilterData::alternateDefaultSearchProvider() const
{
    return d->alternateDefaultSearchProvider;
}

QString KUriFilterData::defaultUrlScheme() const
{
    return d->defaultUrlScheme;
}

KUriFilterData::SearchFilterOptions KUriFilterData::searchFilteringOptions() const
{
    return d->searchFilterOptions;
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

void KUriFilterData::setDefaultUrlScheme(const QString& scheme)
{
    d->defaultUrlScheme = scheme;
}

void KUriFilterData::setSearchFilteringOptions(SearchFilterOptions options)
{
    d->searchFilterOptions = options;
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
    data.d->url = uri;
    data.d->wasModified = true;
    kDebug(7022) << "Got filtered to:" << uri;
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

#ifndef KDE_NO_DEPRECATED
void KUriFilterPlugin::setPreferredSearchProviders(KUriFilterData &data, const ProviderInfoList &providers) const
{
    QHashIterator<QString, QPair<QString, QString> > it (providers);
    while (it.hasNext())
    {
        it.next();
        KUriFilterSearchProvider* searchProvider = data.d->searchProviderMap[it.key()];
        searchProvider->setName(it.key());
        searchProvider->setIconName(it.value().second);
        QStringList keys;
        const QStringList queries = it.value().first.split(QLatin1Char(','));
        Q_FOREACH(const QString& query,  queries)
            keys << query.left(query.indexOf(data.d->searchTermSeparator));
        searchProvider->setKeys(keys);
    }
}
#endif

void KUriFilterPlugin::setSearchProviders(KUriFilterData &data, const QList<KUriFilterSearchProvider*>& providers) const
{
    Q_FOREACH(KUriFilterSearchProvider* searchProvider, providers) {
        data.d->searchProviderList << searchProvider->name();
        data.d->searchProviderMap.insert(searchProvider->name(), searchProvider);
    }
}

QString KUriFilterPlugin::iconNameFor(const KUrl& url, KUriFilterData::UriTypes type) const
{
    return lookupIconNameFor(url, type);
}

class NameLookUpThread : public QThread
{
public:
    NameLookUpThread (const QString& name)
    :QThread (0), m_hostName(name), m_started(false)
    {
    }

    QHostInfo result() const
    {
      return m_hostInfo;
    }

    bool wasStarted() const
    {
        return m_started;
    }

    void run()
    {
        m_started = true;
        m_hostInfo = QHostInfo();

        // Do not perform a reverse lookup here...
        QHostAddress address (m_hostName);
        if (!address.isNull()) {
            QList<QHostAddress> addressList;
            addressList << address;
            m_hostInfo.setAddresses(addressList);
            return;
        }

        // Look up the name in the KIO/KHTML DNS cache...
        m_hostInfo = KIO::HostInfo::lookupCachedHostInfoFor(m_hostName);
        if (!m_hostInfo.hostName().isEmpty() && m_hostInfo.error() == QHostInfo::NoError) {
            return;
        }

        // Failing all of the above, do the lookup...
        m_hostInfo = QHostInfo::fromName(m_hostName);
        if (!m_hostInfo.hostName().isEmpty() && m_hostInfo.error() == QHostInfo::NoError) {
            KIO::HostInfo::cacheLookup(m_hostInfo); // cache the look up...
        }
    }

private:
    QHostInfo m_hostInfo;
    QString m_hostName;
    bool m_started;
};

QHostInfo KUriFilterPlugin::resolveName(const QString& hostname, unsigned long timeout) const
{
    NameLookUpThread lookupThread (hostname);
    lookupThread.start(QThread::TimeCriticalPriority);

    // Wait for it to start...
    while (!lookupThread.wasStarted()) {
       kDebug(7022) << "Waiting for name lookup thread to start";
       lookupThread.wait(1000);
    }

    // Now wait for it to complete...
    if (!lookupThread.wait(timeout)) {
        kDebug(7022) << "Name look up for" << hostname << "failed";
        lookupThread.terminate();
        return QHostInfo();
    }

    //kDebug(7022) << "Name look up succeeded for" << hostname;
    return lookupThread.result();
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
    // NOTE: DO NOT REMOVE this variable! Read the
    // comments in KUriFilter::loadPlugins to understand why...
    QStringList pluginNames; 
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
        QStringListIterator it (d->pluginNames);
        while (it.hasNext()) {
            KUriFilterPlugin* plugin = d->plugins.value(it.next());
            if (plugin &&  plugin->filterUri( data ))
                filtered = true;
        }
    } else {
        QStringListIterator it (filters);
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

#ifndef KDE_NO_DEPRECATED
bool KUriFilter::filterSearchUri(KUriFilterData &data)
{
    return filterSearchUri(data, (NormalTextFilter | WebShortcutFilter));
}
#endif

bool KUriFilter::filterSearchUri(KUriFilterData &data, SearchFilterTypes types)
{
    QStringList filters;

    if (types & WebShortcutFilter)
        filters << "kurisearchfilter";

    if (types & NormalTextFilter)
        filters << "kuriikwsfilter";

    return filterUri(data, filters);
}


QStringList KUriFilter::pluginNames() const
{
    return d->pluginNames;
}

void KUriFilter::loadPlugins()
{
    const KService::List offers = KServiceTypeTrader::self()->query( "KUriFilter/Plugin" );

    // NOTE: Plugin priority is determined by the InitialPreference entry in
    // the .desktop files, so the trader result is already sorted and should
    // not be manually sorted.    
    Q_FOREACH (const KService::Ptr &ptr, offers) {
        KUriFilterPlugin *plugin = ptr->createInstance<KUriFilterPlugin>();
        if (plugin) {
            const QString& pluginName = plugin->objectName();
            Q_ASSERT( !pluginName.isEmpty() );
            d->plugins.insert(pluginName, plugin );
            // Needed to ensure the order of filtering is honored since
            // items are ordered arbitarily in a QHash and QMap always
            // sorts by keys. Both undesired behavior.
            d->pluginNames << pluginName;
        }
    }
}

#include "kurifilter.moc"
