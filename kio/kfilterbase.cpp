/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

#include "kfilterbase.h"

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

#include "kfilterbase.h"
#include <klibloader.h>
#include <kmimetype.h>
#include <ktrader.h>
#include <kdebug.h>

KFilterBase::KFilterBase()
{

}


KFilterBase * KFilterBase::findFilterByFileName( const QString & fileName )
{
    KURL url;
    url.setPath( fileName );
    KMimeType::Ptr mime = KMimeType::findByURL( url );
    return findFilterByMimeType(mime->name());
}

KFilterBase * KFilterBase::findFilterByMimeType( const QString & mimeType )
{
    KTrader::OfferList offers = KTrader::self()->query( "KDECompressionFilter",
                                                        QString("'") + mimeType + "' in ServiceTypes" );
    KTrader::OfferList::ConstIterator it = offers.begin();
    KTrader::OfferList::ConstIterator end = offers.end();

    kdDebug() << "KFilterBase::findFilterByMimeType got " << offers.count() << " offers" << endl;
    for (; it != end; ++it )
    {
        if ((*it)->library().isEmpty()) { continue; }
        KLibFactory *factory = KLibLoader::self()->factory((*it)->library().latin1());
        if (!factory) { continue; }
        KFilterBase *filter = static_cast<KFilterBase*>( factory->create(0, (*it)->desktopEntryName().latin1() ) );
        if ( filter )
            return filter;
    }
    kdWarning() << "KFilterBase::findFilterByExtension : not filter found for " << mimeType << endl;
    return 0L;
}

#include "kfilterbase.moc"
