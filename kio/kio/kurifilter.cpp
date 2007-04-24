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
#include <QPixmap>

#include <kdebug.h>
#include <kiconloader.h>
#include <kservicetypetrader.h>
#include <kmimetype.h>
#include <klibloader.h>

#include "kurifilter.h"

typedef QList<KUriFilterPlugin *> KUriFilterPluginList;

KUriFilterPlugin::KUriFilterPlugin( const QString & name, QObject *parent )
    : QObject( parent )
{
    setObjectName( name );
}

void KUriFilterPlugin::setFilteredUri( KUriFilterData& data, const KUrl& uri ) const
{
    if ( data.uri() != uri )
    {
        data.m_pURI = uri;
        data.m_bChanged = true;
    }
}

class KUriFilterDataPrivate
{
public:
    KUriFilterDataPrivate() {}
    QString abs_path;
    QString args;
    QString typedString;
};

KUriFilterData::KUriFilterData()
    : d( 0 )
{
    init();
}

KUriFilterData::KUriFilterData( const KUrl& url )
    : d( 0 )
{
    init( url );
}

KUriFilterData::KUriFilterData( const QString& url )
    : d( 0 )
{
    init( url );
}


KUriFilterData::KUriFilterData( const KUriFilterData& data )
{
    m_iType = data.m_iType;
    m_pURI = data.m_pURI;
    m_strErrMsg = data.m_strErrMsg;
    m_strIconName = data.m_strIconName;
    m_bChanged = data.m_bChanged;
    m_bCheckForExecutables = data.m_bCheckForExecutables;
    d = new KUriFilterDataPrivate;
    d->abs_path = data.absolutePath();
    d->typedString = data.typedString();
    d->args = data.argsAndOptions();
}

KUriFilterData::~KUriFilterData()
{
    delete d;
    d = 0;
}

void KUriFilterData::init( const KUrl& url )
{
    m_iType = KUriFilterData::UNKNOWN;
    m_pURI = url;
    m_strErrMsg.clear();
    m_strIconName.clear();
    m_bCheckForExecutables = true;
    m_bChanged = true;
    delete d;
    d = new KUriFilterDataPrivate;
    d->typedString = url.url();
}

void KUriFilterData::init( const QString& url )
{
    init( KUrl( url ) );
    d->typedString = url;
}

KUrl KUriFilterData::uri() const
{
    return m_pURI;
}

bool KUriFilterData::checkForExecutables() const
{
    return m_bCheckForExecutables;
}

QString KUriFilterData::errorMsg() const
{
    return m_strErrMsg;
}

KUriFilterData::UriTypes KUriFilterData::uriType() const
{
    return m_iType;
}

QString KUriFilterData::typedString() const
{
    return d->typedString;
}

KUriFilterData& KUriFilterData::operator=( const KUrl& url )
{
    init( url );
    return *this;
}

KUriFilterData& KUriFilterData::operator=( const QString& url )
{
    init( url );
    return *this;
}

void KUriFilterData::setCheckForExecutables( bool check )
{
    m_bCheckForExecutables = check;
}

bool KUriFilterData::hasArgsAndOptions() const
{
    return !d->args.isEmpty();
}

bool KUriFilterData::hasAbsolutePath() const
{
    return !d->abs_path.isEmpty();
}

void KUriFilterData::setData( const QString& url )
{
    init( url );
}

void KUriFilterData::setData( const KUrl& url )
{
    init( url );
}

bool KUriFilterData::setAbsolutePath( const QString& absPath )
{
    // Since a malformed URL could possibly be a relative
    // URL we tag it as a possible local resource...
    if( (m_pURI.protocol().isEmpty() || m_pURI.isLocalFile()) )
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

QString KUriFilterData::argsAndOptions() const
{
    return d->args;
}

QString KUriFilterData::iconName()
{
    if( m_bChanged )
    {
        switch ( m_iType )
        {
            case KUriFilterData::LOCAL_FILE:
            case KUriFilterData::LOCAL_DIR:
            case KUriFilterData::NET_PROTOCOL:
            {
                m_strIconName = KMimeType::iconNameForUrl( m_pURI );
                break;
            }
            case KUriFilterData::EXECUTABLE:
            {
                QString exeName = m_pURI.url();
                exeName = exeName.mid( exeName.lastIndexOf( '/' ) + 1 ); // strip path if given
                KService::Ptr service = KService::serviceByDesktopName( exeName );
                if (service && service->icon() != QLatin1String( "unknown" ))
                    m_strIconName = service->icon();
                // Try to find an icon with the same name as the binary (useful for non-kde apps)
                else if ( !KIconLoader::global()->loadIcon( exeName, K3Icon::NoGroup, 16, K3Icon::DefaultState, 0, true ).isNull() )
                    m_strIconName = exeName;
                else
                    // not found, use default
                    m_strIconName = QLatin1String("exec");
                break;
            }
            case KUriFilterData::HELP:
            {
                m_strIconName = QLatin1String("khelpcenter");
                break;
            }
            case KUriFilterData::SHELL:
            {
                m_strIconName = QLatin1String("konsole");
                break;
            }
            case KUriFilterData::ERROR:
            case KUriFilterData::BLOCKED:
            {
                m_strIconName = QLatin1String("error");
                break;
            }
            default:
                m_strIconName.clear();
                break;
        }
        m_bChanged = false;
    }
    return m_strIconName;
}

//********************************************  KUriFilterPlugin **********************************************
void KUriFilterPlugin::setArguments( KUriFilterData& data, const QString& args ) const
{
    data.d->args = args;
}

//********************************************  KUriFilter **********************************************
class KUriFilter::Private
{
public:
    Private() {}
    QList<KUriFilterPlugin *> lstPlugins;
};

KUriFilter *KUriFilter::self()
{
    K_GLOBAL_STATIC(KUriFilter, m_self)
    return m_self;
}

KUriFilter::KUriFilter()
    : d(new Private())
{
    loadPlugins();
}

KUriFilter::~KUriFilter()
{
    qDeleteAll(d->lstPlugins);
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
        //kDebug() << "Named plugins requested..."  << endl;
        for( QStringList::ConstIterator lst = filters.begin(); lst != filters.end(); ++lst ) {
            KUriFilterPlugin* plugin = findPluginByName( d->lstPlugins, *lst );
            if (plugin) {
                //kDebug() << "Will use filter plugin named: " << plugin->name() << endl;
                use_plugins.append(plugin);
            }
        }
    }

    //kDebug() << "Using " << use_plugins.count() << " out of the "
    //          << d->lstPlugins.count() << " available plugins" << endl;
    bool filtered = false;
    for ( KUriFilterPluginList::const_iterator it = use_plugins.begin(), end = use_plugins.end();
          it != end; ++it ) {
        //kDebug() << "Using a filter plugin named: " << (*it)->name() << endl;
        if( (*it)->filterUri( data ))
            filtered = true;
    }
    return filtered;
}

bool KUriFilter::filterUri( KUrl& uri, const QStringList& filters )
{
    KUriFilterData data = uri;
    bool filtered = filterUri( data, filters );
    if( filtered ) uri = data.uri();
    return filtered;
}

bool KUriFilter::filterUri( QString& uri, const QStringList& filters )
{
    KUriFilterData data = uri;
    bool filtered = filterUri( data, filters );
    if( filtered )  uri = data.uri().url();
    return filtered;
}

KUrl KUriFilter::filteredUri( const KUrl &uri, const QStringList& filters )
{
    KUriFilterData data = uri;
    filterUri( data, filters );
    return data.uri();
}

QString KUriFilter::filteredUri( const QString &uri, const QStringList& filters )
{
    KUriFilterData data = uri;
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

    KService::List::ConstIterator it = offers.begin();
    const KService::List::ConstIterator end = offers.end();

    for (; it != end; ++it )
    {
        KUriFilterPlugin *plugin = KService::createInstance<KUriFilterPlugin>( *it );

        if ( plugin ) {
            // plugins set their name already
            //plugin->setObjectName( (*it)->desktopEntryName() );
            Q_ASSERT( !plugin->objectName().isEmpty() );
            d->lstPlugins.append( plugin );
        }
    }

    // NOTE: Plugin priority is determined by
    // the InitialPreference entry in the .desktop files,
    // so the trader result is already sorted.

    // TODO: Config dialog to differentiate "system"
    // plugins from "user-defined" ones...
    // d->lstPlugins.sort();
}

#include "kurifilter.moc"
