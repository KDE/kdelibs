/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kcomprmanager.h"
#include <klibloader.h>
#include <kstaticdeleter.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <kdebug.h>

KComprManager * KComprManager::s_comprManager = 0L;
KStaticDeleter<KComprManager> comprsd;

KComprManager * KComprManager::self()
{
    if ( !s_comprManager )
    {
        s_comprManager = comprsd.setObject(new KComprManager);
    }
    return s_comprManager;
}

KComprManager::KComprManager()
{
}

KComprManager::~KComprManager()
{

}

KFilterBase * KComprManager::findFilterByFileName( const QString & fileName )
{
    KURL url;
    url.setPath( fileName );
    KMimeType::Ptr mime = KMimeType::findByURL( url );
    return findFilterByMimeType(mime->name());
}

KFilterBase * KComprManager::findFilterByMimeType( const QString & mimeType )
{
    KTrader::OfferList offers = KTrader::self()->query( "KDECompressionFilter",
                                                        QString("'") + mimeType + "' in ServiceTypes" );
    KTrader::OfferList::ConstIterator it = offers.begin();
    KTrader::OfferList::ConstIterator end = offers.end();

    kdDebug() << "KComprManager::findFilterByMimeType got " << offers.count() << " offers" << endl;
    for (; it != end; ++it )
    {
        if ((*it)->library().isEmpty()) { continue; }
        KLibFactory *factory = KLibLoader::self()->factory((*it)->library().latin1());
        if (!factory) { continue; }
        KFilterBase *filter = static_cast<KFilterBase*>( factory->create(0, (*it)->desktopEntryName().latin1()) );
        if ( filter )
            return filter;
    }
    kdWarning() << "KComprManager::findFilterByExtension : not filter found for " << mimeType << endl;
    return 0L;
}
