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
#include "filter.h"


#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
using namespace KSpell2;

int main( int argc, char** argv )
{
    KCmdLineArgs::init( argc, argv, "KSpell2Test", 0, 0, 0, 0);

    KApplication app;

    QString buffer = QString( "This is     a sample buffer.      Please test me." );

    Filter filter;
    filter.setBuffer( buffer );

    Word w;

    while ( ! (w=filter.nextWord()).end ) {
        kdDebug()<< "Found word \""<< w.word << "\" which starts at position "
                 << w.start <<endl;
    }

    filter.setBuffer( buffer );



    return 0;
}
