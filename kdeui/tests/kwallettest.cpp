/*
    This file is part of the KDE Libraries

    Copyright (C) 2007 Thomas McGuire (thomas.mcguire@gmx.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kwallettest.h"

#include <QtTest>
#include <QWidget>

#include <kwallet.h>
#include <kdebug.h>
#include <klauncher_iface.h>
#include <ktoolinvocation.h>

using namespace KWallet;

void KWalletTest::testWallet()
{
  QString testWallet = "kdewallet";
  QString testFolder = "wallettestfolder";
  QString testKeys[] = { "testKey", "account-302948", "\\", "/abc",
                         "a@b.c" };
  QByteArray testValues[] = { "test", "@(!§\"%&", "", ".test", "\\" };
  int numTests = 5;

  // Start kdeinit4, so that the wallet damon is started in the background
  OrgKdeKLauncherInterface *launcher = KToolInvocation::klauncher();
  launcher->autoStart();

  // Create a widget to serve as the wallet's parent widget, to get rid of a
  // warning
  QWidget *w = new QWidget();

  // open
  Wallet *wallet = Wallet::openWallet( testWallet, w->winId(), Wallet::Synchronous );
  if ( wallet == 0 )
    kDebug() << "Couldn't open the wallet. Maybe the wallet daemon is not running?";
  QVERIFY( wallet != 0 );
  QVERIFY( Wallet::isOpen( testWallet ) );

  // create folder
  wallet->createFolder( testFolder );
  QVERIFY( wallet->hasFolder( testFolder ) );
  wallet->setFolder( testFolder );
  QVERIFY( wallet->currentFolder() == testFolder );
  QVERIFY( wallet->folderList().contains( testFolder ) );

  // write & read many entries
  for ( int i = 0; i < numTests; i++ ) {
    wallet->writeEntry( testKeys[i], testValues[i] );
    QVERIFY( wallet->hasEntry( testKeys[i] ) );
    QByteArray readEntry;
    wallet->readEntry( testKeys[i], readEntry );
    QVERIFY( readEntry == testValues[i] );
  }

  // close
  wallet->sync();
  Wallet::closeWallet( "kdewallet", true );
  QVERIFY( !Wallet::isOpen( "kdewallet" ) );

  // test for key - closed wallet
  for ( int i = 0; i < 5; i++ ) {
    QVERIFY( !Wallet::keyDoesNotExist( testWallet, testFolder, testKeys[i] ) );
    QVERIFY( Wallet::keyDoesNotExist( testWallet, testFolder, "madeUpKey" ) );
    QVERIFY( Wallet::keyDoesNotExist( testWallet, "madeUpFolderName", "madeUpKey" ) );
    QVERIFY( Wallet::keyDoesNotExist( testWallet, "madeUpFolderName", testKeys[i] ) );
  }

  // open
  wallet = Wallet::openWallet( testWallet, w->winId(), Wallet::Synchronous );
  QVERIFY( wallet != 0 );
  QVERIFY( Wallet::isOpen( testWallet ) );

  // set folder
  QVERIFY( wallet->hasFolder( testFolder ) );
  wallet->setFolder( testFolder );
  QVERIFY( wallet->currentFolder() == testFolder );

  // test for key - opened wallet
  for ( int i = 0; i < numTests; i++ ) {
    QVERIFY( !Wallet::keyDoesNotExist( testWallet, testFolder, testKeys[i] ) );
    QVERIFY( Wallet::keyDoesNotExist( testWallet, testFolder, "madeUpKey" ) );
    QVERIFY( Wallet::keyDoesNotExist( testWallet, "madeUpFolderName", "madeUpKey" ) );
    QVERIFY( Wallet::keyDoesNotExist( testWallet, "madeUpFolderName", testKeys[i] ) );
  }

  // read many keys
  for ( int i = 0; i < numTests; i++ ) {
    QByteArray readEntry;
    wallet->readEntry( testKeys[i], readEntry );
    QVERIFY( readEntry == testValues[i] );
  }

  // delete folder
  wallet->removeFolder( testFolder );
  QVERIFY( !wallet->hasFolder( testFolder ) );

  // close
  Wallet::closeWallet( "kdewallet", true );
  QVERIFY( !Wallet::isOpen( "kdewallet" ) );
}

QTEST_KDEMAIN(KWalletTest, GUI)

