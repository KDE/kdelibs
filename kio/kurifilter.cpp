/* This file is part of the KDE libraries
 *  Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <kmimetype.h>
#include <klibloader.h>
#include <ktrader.h>

#include "kurifilter.h"

template class QList<KURIFilterPlugin>;

KURIFilterPlugin::KURIFilterPlugin( QObject *parent, const char *name, double pri )
                 :QObject( parent, name )
{
    m_strName = QString::fromLatin1( name );
    m_dblPriority = pri;
}

void KURIFilterPlugin::setFilteredURI( KURIFilterData& data, const KURL& uri ) const
{
    if ( data.uri() != uri )
    {
        data.m_pURI = uri;
        data.m_bFiltered = true;
        data.m_bChanged = true;
    }
}

KURIFilterData::KURIFilterData( const KURIFilterData& data )
{
    m_iType = data.m_iType;
    m_pURI = data.m_pURI;
    m_strErrMsg = data.m_strErrMsg;
    m_strIconName = data.m_strIconName;
    m_bFiltered = data.m_bFiltered;
    m_bChanged = data.m_bChanged;
    
}

void KURIFilterData::init( const KURL& url )
{
    m_iType = KURIFilterData::UNKNOWN;
    m_pURI = url;
    m_strErrMsg = QString::null;
    m_strIconName = QString::null;    
    m_bFiltered = false;
    m_bChanged = true;
}

QString KURIFilterData::iconName()
{
    if( m_bChanged )
    {      
        switch ( m_iType )
        {
    	    case KURIFilterData::LOCAL_FILE:
        	case KURIFilterData::LOCAL_DIR:
        	case KURIFilterData::NET_PROTOCOL:
        	{
	            KMimeType::Ptr mimetype = KMimeType::findByURL( m_pURI );
        	    if (mimetype)
            		m_strIconName = mimetype->icon( m_pURI, false );
                break;
        	}
        	case KURIFilterData::EXECUTABLE:
        	{
	            KService::Ptr service = KService::serviceByDesktopName( m_pURI.url() );
        	    if (service)
            		m_strIconName = service->icon();
        	    else 
            		m_strIconName = QString::fromLatin1("exec");
                break;
            }
            case KURIFilterData::HELP:
            {
                m_strIconName = QString::fromLatin1("khelpcenter");
        	    break;
            }
            case KURIFilterData::SHELL:
            {
                m_strIconName = QString::fromLatin1("konsole");
        	    break;
            }            
            case KURIFilterData::ERROR:
            case KURIFilterData::BLOCKED:
            {
                m_strIconName = QString::fromLatin1("error");
                break;
            }
            default:
                m_strIconName = QString::null;
                break;
        }
        m_bChanged = false;   
    }
    return m_strIconName;
}

//********************************************  KURIFilter **********************************************
KURIFilter *KURIFilter::ms_pFilter = 0;

KURIFilter::KURIFilter()
{
    m_lstPlugins.setAutoDelete(true);
    loadPlugins();
}

KURIFilter *KURIFilter::self()
{
    if (!ms_pFilter)
        ms_pFilter = new KURIFilter();

    return ms_pFilter;
}

bool KURIFilter::filterURI( KURIFilterData& data, const QStringList& filters )
{
    bool filtered = false;
    KURIFilterPluginList plugins = m_lstPlugins;
    // If we have a filter list, only include
    // those specifically requested.
    if( filters.count() > 0 )
    {
        for( KURIFilterPlugin* it = plugins.first(); it != 0; it = plugins.next() )
        {
            bool found = false;
            for ( QStringList::ConstIterator lst = filters.begin(); lst != filters.end(); ++lst )
            {
                if( (*lst) == it->name() ) { found = true; break; }
            }
            if( !found ) plugins.remove(); // remove current item from the list if not found!!!
        }
    }
    QListIterator<KURIFilterPlugin> it( plugins );
    for (; it.current(); ++it) filtered |= it.current()->filterURI( data );
    return filtered;
}

bool KURIFilter::filterURI( KURL& uri, const QStringList& filters )
{
    KURIFilterData data = uri;
    bool filtered = filterURI( data, filters );
    if( filtered ) uri = data.uri();
    return filtered;
}

bool KURIFilter::filterURI( QString& uri, const QStringList& filters )
{
    KURIFilterData data = uri;
    bool filtered = filterURI( data, filters );
    if( filtered )  uri = data.uri().url();
    return filtered;

}

KURL KURIFilter::filteredURI( const KURL &uri, const QStringList& filters )
{
    KURIFilterData data = uri;
    filterURI( data, filters );
    return data.uri();
}

QString KURIFilter::filteredURI( const QString &uri, const QStringList& filters )
{
    KURIFilterData data = uri;
    filterURI( data, filters );
    return data.uri().url();
}

QListIterator<KURIFilterPlugin> KURIFilter::pluginsIterator() const
{
    return QListIterator<KURIFilterPlugin>(m_lstPlugins);
};

void KURIFilter::loadPlugins()
{
    KTrader::OfferList offers = KTrader::self()->query( "KURIFilter/Plugin" );
    KTrader::OfferList::ConstIterator it = offers.begin();
    KTrader::OfferList::ConstIterator end = offers.end();

    for (; it != end; ++it )
    {
	    if ((*it)->library().isEmpty()) { continue; }
        KLibFactory *factory = KLibLoader::self()->factory((*it)->library());						
    	if (!factory) { continue; }
    	KURIFilterPlugin *plugin = (KURIFilterPlugin *) factory->create(0, (*it)->name(), "KURIFilterPlugin");
	    if ( plugin ) { m_lstPlugins.append( plugin ); }
    }
    m_lstPlugins.sort(); // TODO: Prioritize based on the user preference from control module.
}

#include "kurifilter.moc"

