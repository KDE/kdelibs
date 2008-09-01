/**
 * test_filter.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "filter_p.h"
#include "test_filter.h"

#include <qtest_kde.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

QTEST_KDEMAIN_CORE( SonnetFilterTest )

using namespace Sonnet;

struct Hit {
    Hit( const QString& w, int s ) : word( w ), start( s ) {}
    QString word;
    int start;
};

void SonnetFilterTest::testFilter()
{
    QString buffer( "This is     a sample buffer.      Please test me. He's don't Le'Clerk." );
    QList<Hit> hits;
    hits.append( Hit( "This", 0 ) );
    hits.append( Hit( "is", 5 ) );
    hits.append( Hit( "a", 12 ) );
    hits.append( Hit( "sample", 14 ) );
    hits.append( Hit( "buffer", 21 ) );
    hits.append( Hit( "Please", 34 ) );
    hits.append( Hit( "test", 41 ) );
    hits.append( Hit( "me", 46 ) );
    hits.append( Hit( "He's", 50 ) );
    hits.append( Hit( "don't", 55 ) );
    hits.append( Hit( "Le'Clerk", 61 ) );

    Filter filter;
    filter.setBuffer( buffer );

    Word w;
    int hitNumber = 0;
    while ( ! (w=filter.nextWord()).end ) {
        QCOMPARE( w.word, hits[hitNumber].word );
        QCOMPARE( w.start, hits[hitNumber].start );
        //kDebug()<< "Found word \""<< w.word << "\" which starts at position " << w.start;
        ++hitNumber;
    }
    QCOMPARE( hitNumber, hits.count() );

    // ? filter.setBuffer( buffer );
}

#include "test_filter.moc"
