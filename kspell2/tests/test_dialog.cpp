/**
 * test_dialog.cpp
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
#include "test_dialog.h"
#include "test_dialog.moc"

#include "backgroundchecker.h"
#include "dictionary.h"
#include "filter.h"

#include <kapplication.h>
#include <kdebug.h>
using namespace KSpell2;

TestDialog::TestDialog()
    : QObject( 0, "testdialog" )
{

}

void TestDialog::check( const QString& buffer )
{
    KSpell2::Dialog *dlg = new KSpell2::Dialog(
        new BackgroundChecker( Broker::openBroker(), this ),
        0 );
    connect( dlg, SIGNAL(done(const QString&)),
             SLOT(doneChecking(const QString&)) );
    dlg->setBuffer( buffer );
    dlg->show();
}

void TestDialog::doneChecking( const QString& buf )
{
    kdDebug()<<"Done with :"<<buf<<endl;
    qApp->quit();
}

int main( int argc, char** argv )
{
    KApplication app;

    TestDialog test;
    test.check( "This is a sample buffer. Whih this thingg will "
                "be checkin for misstakes. Whih, Enviroment, govermant. Whih."
        );

    return app.exec();
}
