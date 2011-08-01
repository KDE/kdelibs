/*
 * Copyright 2011, Valentin Rusu <kde@rusu.info>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ksecretsservice-test.h"
#include "../ksecretsservicecollection.h"
#include "../ksecretscollectionjobs.h"
#include "../ksecretsservicesecret.h"
#include "../ksecretsserviceitem.h"

#include <qtest_kde.h>
#include <ktoolinvocation.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtTest/QTest>
#include <kdebug.h>

QTEST_KDEMAIN_CORE(KSecretServiceTest)

#define SERVICE_NAME "org.freedesktop.secrets"

using namespace KSecretsService;

#define DONT_TEST_CREATEANDDELETE 
#define DONT_TEST_RENAMECOLLECTION 
//#define DONT_TEST_CREATEITEM 

KSecretServiceTest::KSecretServiceTest(QObject* parent): QObject(parent)
{

}

void KSecretServiceTest::initTestCase()
{
    // launch the daemon if it's not yet started
/*    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1( SERVICE_NAME )))
    {
        QString error;
        // FIXME: find out why this is not working
        int ret = KToolInvocation::startServiceByDesktopPath("ksecretsserviced.desktop", QStringList(), &error);
        QVERIFY2( ret == 0, qPrintable( error ) );
        
        QVERIFY2( QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1( SERVICE_NAME )),
                 "Secret Service was started but the service is not registered on the DBus");
    }*/
}

void KSecretServiceTest::testCreateAndDelete()
{
    Collection *createdColl = Collection::findCollection( 0, "test collection" );
    ReadCollectionPropertyJob *isValidJob = createdColl->isValid();
    QVERIFY2( isValidJob->exec(), qPrintable( isValidJob->errorText() ) );
    
    Collection *existingColl = Collection::findCollection( 0, "test collection", Collection::OpenOnly );
    isValidJob = existingColl->isValid();
    QVERIFY2( isValidJob->exec(), qPrintable( isValidJob->errorText() ) );
    
    KJob* deleteJob = existingColl->deleteCollection();
    deleteJob->exec();
    QVERIFY2( (deleteJob->error() == 0), qPrintable( deleteJob->errorText() ) );
}

void KSecretServiceTest::testRenameCollection()
{
    Collection *coll = Collection::findCollection( 0, "test name1" );
    KJob *renameJob = coll->renameCollection( "test name2" );
    renameJob->exec();
    QVERIFY2( (renameJob->error() == 0), qPrintable( renameJob->errorText() ) );
    ReadCollectionPropertyJob *readLabelJob = coll->label();
    QVERIFY2( readLabelJob->exec(), qPrintable( readLabelJob->errorText() ) );
    QVERIFY2( (readLabelJob->propertyValue() == "test name2"), "Collection won't change it's name!" );
    
    // finally, delete the collection
    KJob *deleteJob = coll->deleteCollection();
    deleteJob->exec();
    QVERIFY2( (deleteJob->error() == 0), qPrintable( deleteJob->errorText() ) );
}

void KSecretServiceTest::testCreateItem()
{
    Collection *coll = Collection::findCollection( 0, "test collection" );
    QStringStringMap attributes;
    attributes.insert( "test-attribute", "test-attribute-value" );
    Secret newSecret;
    newSecret.setValue( QVariant("test-secret"), "stringVariant" );
    KSecretsService::CreateItemJob *createItemJob = coll->createItem( "test label", attributes, newSecret );
    QVERIFY2( createItemJob->exec(), qPrintable( createItemJob->errorText() ) );
    
    // first method, try to directly read the SecretStruct
    KSecretsService::SearchSecretsJob *searchJob = coll->searchSecrets( attributes );
    QVERIFY2( searchJob->exec(), qPrintable( searchJob->errorText() ) );

    bool found = false;
    foreach( Secret secret, searchJob->secrets() ) {
        if ( secret == newSecret ) {
            found = true;
            break;
        }
    }
    QVERIFY2( found, "The new secret was not found in the collection (via searchSecrets()) !");
    
    // second method, try to read the SecretItem
    KSecretsService::SearchItemsJob *searchItemsJob = coll->searchItems( attributes );
    QVERIFY2( searchItemsJob->exec(), qPrintable( searchItemsJob->errorText() ) );
    
    foreach ( SecretItem item, searchItemsJob->items() ) {
        KSecretsService::GetSecretItemSecretJob *getSecretJob = item.getSecret();
        QVERIFY2( getSecretJob->exec(), qPrintable( getSecretJob->errorText() ) );
        if ( getSecretJob->secret() == newSecret ) {
            found = true;
            break;
        }
    }
    QVERIFY2( found, "The new secret was not found in the collection (via searchItems()) !");
    
    // third method, use the items() method
    KSecretsService::ReadItemsJob *readItemsJob = coll->items();
    QVERIFY2( readItemsJob->exec(), qPrintable( readItemsJob->errorText() ) );
    
    foreach ( SecretItem item, readItemsJob->items() ) {
        KSecretsService::GetSecretItemSecretJob *getSecretJob = item.getSecret();
        QVERIFY2( getSecretJob->exec(), qPrintable( getSecretJob->errorText() ) );
        if ( getSecretJob->secret() == newSecret ) {
            found = true;
            break;
        }
    }
    QVERIFY2( found, "The new secret was not found in the collection (via items()) !");
    
    //bool collLocked = coll->isLocked();
    
    // finally, delete the collection
    KJob *deleteJob = coll->deleteCollection();
    deleteJob->exec();
    QVERIFY2( (deleteJob->error() == 0), qPrintable( deleteJob->errorText() ) );
}

void KSecretServiceTest::testItems()
{
    Collection *coll = Collection::findCollection( 0, "test collection" );
    QStringStringMap attributes;
    attributes.insert( "test-attribute", "test-attribute-value" );
    Secret newSecret;
    newSecret.setValue( QVariant("test-secret"), "stringVariant" );
    KSecretsService::CreateItemJob *createItemJob = coll->createItem( "test label", attributes, newSecret );
    QVERIFY2( createItemJob->exec(), qPrintable( createItemJob->errorText() ) );

    SecretItem * createdItem = createItemJob->item();
    ReadItemPropertyJob * readLabelJob = createdItem->label();
    QVERIFY( readLabelJob->exec() );
    QVERIFY( readLabelJob->propertyValue() == "test label" );
    
    KSecretsService::GetSecretItemSecretJob *getSecretJob = createdItem->getSecret();
    QVERIFY2( getSecretJob->exec(), qPrintable( getSecretJob->errorText() ) );
    QVERIFY( getSecretJob->secret() == newSecret );
    
    Secret secondSecret;
    secondSecret.setValue( QVariant("second secret"), "stringVariant" );
    KSecretsService::SetSecretItemSecretJob * setSecretJob = createdItem->setSecret( secondSecret );
    QVERIFY2( setSecretJob->exec(), qPrintable( setSecretJob->errorText() ) );
    getSecretJob = createdItem->getSecret();
    QVERIFY2( getSecretJob->exec(), qPrintable( getSecretJob->errorText() ) );
    QVERIFY( getSecretJob->secret() == secondSecret );
    
    // try to delete the item
    KSecretsService::SecretItemDeleteJob *itemDeleteJob = createdItem->deleteItem( 0 );
    QVERIFY2( itemDeleteJob->exec(), qPrintable( itemDeleteJob->errorText() ) );
    
    // finally, delete the collection
    KJob *deleteJob = coll->deleteCollection();
    deleteJob->exec();
    QVERIFY2( (deleteJob->error() == 0), qPrintable( deleteJob->errorText() ) );
}

void KSecretServiceTest::cleanupTestCase()
{
}

#include "ksecretsservice-test.moc"
