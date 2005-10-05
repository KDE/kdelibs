/**
 * test.cpp
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
#include "broker.h"
#include "dictionary.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <qdatetime.h>

using namespace KSpell2;

int main( int argc, char** argv )
{
    KCmdLineArgs::init( argc, argv, "KSpell2Test", 0, 0, 0, 0);
    KApplication app;

    Broker::Ptr broker = Broker::openBroker();

    kdDebug()<< "Clients are "   << broker->clients()   << endl;
    kdDebug()<< "Languages are " << broker->languages() << endl;

    Dictionary *dict = broker->dictionary( "en_US" );

    QStringList words;

    words << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted"
          << "hello" << "helo" << "enviroment" << "guvernment" << "farted";

    QTime mtime;
    mtime.start();
    for ( QStringList::Iterator itr = words.begin(); itr != words.end(); ++itr ) {
        if ( dict && !dict->check( *itr ) ) {
            //kdDebug()<<"Word " << *itr <<" is misspelled"<<endl;
            QStringList sug = dict->suggest( *itr );
            //kdDebug()<<"Suggestions : "<<sug<<endl;
        }
    }
    //mtime.stop();
    kdDebug()<<"Elapsed time is "<<mtime.elapsed()<<endl;

    delete dict;

    return 0;
}
