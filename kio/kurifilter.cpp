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

#include <klibloader.h>
#include <ktrader.h>

#include "kurifilter.h"

template class QList<KURIFilterPlugin>;

bool KURIFilterPlugin::filterURI( QString &uri ) {
    KURL kuri(uri);
    bool filtered = filterURI(kuri);
    uri = kuri.url();
    return filtered;
}

KURIFilter *KURIFilter::ms_pFilter = 0;

KURIFilter::KURIFilter() {
    m_lstPlugins.setAutoDelete(true);

    loadPlugins();
}

KURIFilter *KURIFilter::filter() {
    if (!ms_pFilter) {
	ms_pFilter = new KURIFilter();
    }

    return ms_pFilter;
}

bool KURIFilter::filterURI(KURL &uri) {
    bool filtered = false;
    QListIterator<KURIFilterPlugin> it(m_lstPlugins);

    for (; it.current(); ++it) {
	filtered |= it.current()->filterURI(uri);
    }

    return filtered;
}

bool KURIFilter::filterURI(QString &uri) {
    bool filtered = false;
    QListIterator<KURIFilterPlugin> it(m_lstPlugins);

    for (; it.current(); ++it) {
	filtered |= it.current()->filterURI(uri);
    }

    return filtered;
}

KURL KURIFilter::filteredURI(const KURL &uri) {
    KURL filtered = uri;
    filterURI(filtered);
    return filtered;
}

QString KURIFilter::filteredURI(const QString &uri) {
    QString filtered = uri;
    filterURI(filtered);
    return filtered;
}

QListIterator<KURIFilterPlugin> KURIFilter::pluginsIterator() const {
    return QListIterator<KURIFilterPlugin>(m_lstPlugins);
};

void KURIFilter::loadPlugins() {
    KTrader::OfferList offers = KTrader::self()->query( "KURIFilter/Plugin" );
    KTrader::OfferList::ConstIterator it = offers.begin();
    KTrader::OfferList::ConstIterator end = offers.end();

    for (; it != end; ++it ) {
	if ((*it)->library().isEmpty()) {
	    continue;
	}

	KLibFactory *factory = KLibLoader::self()->factory((*it)->library());
						
	if (!factory) {
	    continue;
	}

	KURIFilterPlugin *plugin = (KURIFilterPlugin *) factory->create(0, 0, "KURIFilterPlugin");
	if (plugin) {
	    m_lstPlugins.append(plugin);
	}
    }

    m_lstPlugins.sort();
}

#include "kurifilter.moc"

