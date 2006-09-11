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

#include <kdebug.h>
#include <kiconloader.h>
#include <kservicetypetrader.h>
#include <kmimetype.h>
#include <klibloader.h>
#include <kstaticdeleter.h>

#include "kurifilter.h"

template class Q3PtrList<KUriFilterPlugin>;

#ifdef KDE3_SUPPORT
KUriFilterPlugin::KUriFilterPlugin( QObject *parent, const char *name, double pri )
#else
KUriFilterPlugin::KUriFilterPlugin( const QString & name, QObject *parent, double pri )
#endif
                 :QObject( parent ), m_strName( name )
{
    m_dblPriority = pri;
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
    KUriFilterDataPrivate() {};
    QString abs_path;
    QString args;
    QString typedString;
};

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
    d = new KUriFilterDataPrivate;
    d->typedString = url.url();
}

void KUriFilterData::init( const QString& url )
{
    m_iType = KUriFilterData::UNKNOWN;
    m_pURI = url; // QString -> KUrl
    m_strErrMsg.clear();
    m_strIconName.clear();
    m_bCheckForExecutables = true;
    m_bChanged = true;
    d = new KUriFilterDataPrivate;
    d->typedString = url;
}

QString KUriFilterData::typedString() const
{
    return d->typedString;
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
                else if ( !KGlobal::iconLoader()->loadIcon( exeName, K3Icon::NoGroup, 16, K3Icon::DefaultState, 0, true ).isNull() )
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
KUriFilter *KUriFilter::m_self;
static KStaticDeleter<KUriFilter> kurifiltersd;

KUriFilter *KUriFilter::self()
{
    if (!m_self)
        m_self = kurifiltersd.setObject(m_self, new KUriFilter);
    return m_self;
}

KUriFilter::KUriFilter()
{
    m_lstPlugins.setAutoDelete(true);
    loadPlugins();
}

KUriFilter::~KUriFilter()
{
    m_self = 0;
}

bool KUriFilter::filterUri( KUriFilterData& data, const QStringList& filters )
{
    bool filtered = false;
    KUriFilterPluginList use_plugins;

    // If we have a filter list, only include the once
    // explicitly specified by it. Otherwise, use all available filters...
    if( filters.isEmpty() )
        use_plugins = m_lstPlugins;  // Use everything that is loaded...
    else
    {
        //kDebug() << "Named plugins requested..."  << endl;
        for( QStringList::ConstIterator lst = filters.begin(); lst != filters.end(); ++lst )
        {
            Q3PtrListIterator<KUriFilterPlugin> it( m_lstPlugins );
            for( ; it.current() ; ++it )
            {
                if( (*lst) == it.current()->name() )
                {
                    //kDebug() << "Will use filter plugin named: " << it.current()->name() << endl;
                    use_plugins.append( it.current() );
                    break;  // We already found it ; so lets test the next named filter...
                }
            }
        }
    }

    Q3PtrListIterator<KUriFilterPlugin> it( use_plugins );
    //kDebug() << "Using " << use_plugins.count() << " out of the "
    //          << m_lstPlugins.count() << " available plugins" << endl;
    for (; it.current() && !filtered; ++it)
    {
        //kDebug() << "Using a filter plugin named: " << it.current()->name() << endl;
        filtered |= it.current()->filterUri( data );
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

Q3PtrListIterator<KUriFilterPlugin> KUriFilter::pluginsIterator() const
{
    return Q3PtrListIterator<KUriFilterPlugin>(m_lstPlugins);
}

QStringList KUriFilter::pluginNames() const
{
    QStringList list;
    for(Q3PtrListIterator<KUriFilterPlugin> i = pluginsIterator(); *i; ++i)
        list.append((*i)->name());
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
        plugin->setObjectName( (*it)->desktopEntryName() );
        m_lstPlugins.append( plugin );
      }
    }

    // NOTE: Plugin priority is now determined by
    // the entry in the .desktop files...
    // TODO: Config dialog to differentiate "system"
    // plugins from "user-defined" ones...
    // m_lstPlugins.sort();
}

#include "kurifilter.moc"
