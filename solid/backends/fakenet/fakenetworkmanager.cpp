/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/
#include <QFile>
#include <QtXml>
#include <QLatin1String>

#include <kstandarddirs.h>
#include <kdebug.h>

#include "fakenetworkmanager.h"
#include "fakenetwork.h"
#include "fakewirelessnetwork.h"

FakeAuthenticationValidator::FakeAuthenticationValidator( QObject * parent ) : QObject( parent )
{
}

FakeAuthenticationValidator::~FakeAuthenticationValidator()
{
}

bool FakeAuthenticationValidator::validate( const Solid::Authentication * )
{
    return true;
}

FakeNetworkManager::FakeNetworkManager( QObject * parent, const QStringList & ) : Solid::Ifaces::NetworkManager( parent ), mAuthValidator( 0 )
{
    mUserNetworkingEnabled = true;
    mUserWirelessEnabled = true;

    mXmlFile = KStandardDirs::locate("data", "solidfakebackend/fakenetworking.xml");

//     QDBusConnection::sessionBus().registerObject( "/org/kde/solid/fake", this, QDBusConnection::ExportNonScriptableSlots );

    parseNetworkingFile();
}

FakeNetworkManager::FakeNetworkManager( QObject * parent, const QStringList &, const QString& xmlFile ) : Solid::Ifaces::NetworkManager( parent ), mAuthValidator( 0 )
{
    mUserNetworkingEnabled = true;
    mUserWirelessEnabled = true;

    mXmlFile = xmlFile;
    if( mXmlFile.isEmpty() )
    {
       kDebug() << "Falling back to installed networking xml" << endl;
       mXmlFile = KStandardDirs::locate("data", "solidfakebackend/fakenetworking.xml");
    }
    parseNetworkingFile();
}

FakeNetworkManager::~FakeNetworkManager()
{
    delete mAuthValidator;
}

QStringList FakeNetworkManager::networkInterfaces() const
{
    return mNetworkInterfaces.keys();
}

QStringList FakeNetworkManager::activeNetworkInterfaces() const
{
    QStringList activeDevices;
    QMapIterator<QString, FakeNetworkInterface *> it( mNetworkInterfaces );
    while ( it.hasNext() )
    {
        it.next();
        FakeNetworkInterface * netDevice = it.value();
        if ( netDevice->isActive() )
            activeDevices.append( netDevice->uni() );
    }
    return activeDevices;
}

QObject * FakeNetworkManager::createNetworkInterface( const QString & undi)
{
    if ( mNetworkInterfaces.contains( undi ) )
        return mNetworkInterfaces[ undi ];
    else
        return 0;
}

QObject * FakeNetworkManager::createAuthenticationValidator()
{
    if ( mAuthValidator == 0 )
        mAuthValidator = new FakeAuthenticationValidator( this );
    return mAuthValidator;
}

bool FakeNetworkManager::isWirelessEnabled() const
{
    QMapIterator<QString, FakeNetworkInterface *> it( mNetworkInterfaces );
    while ( it.hasNext() )
    {
        it.next();
        FakeNetworkInterface * netDevice = it.value();
        if ( netDevice->type() == Solid::NetworkInterface::Ieee80211 )
            if ( netDevice->isActive() )
                return true;
    }
    return false;
}

bool FakeNetworkManager::isNetworkingEnabled() const
{
    QMapIterator<QString, FakeNetworkInterface *> it( mNetworkInterfaces );
    while ( it.hasNext() )
    {
        it.next();
        FakeNetworkInterface * netDevice = it.value();
        if ( netDevice->isActive() )
            return true;
    }
    return false;
}

void FakeNetworkManager::setWirelessEnabled( bool enabled )
{
    QMapIterator<QString, FakeNetworkInterface *> it( mNetworkInterfaces );
    if ( mUserNetworkingEnabled )
    {
        while ( it.hasNext() )
        {
            it.next();
            FakeNetworkInterface * netDevice = it.value();
            if ( netDevice->type() == Solid::NetworkInterface::Ieee80211 )
                netDevice->setActive( enabled );
        }
    }
    mUserWirelessEnabled = enabled;
}

void FakeNetworkManager::setNetworkingEnabled( bool enabled )
{
    QMapIterator<QString, FakeNetworkInterface *> it( mNetworkInterfaces );
    while ( it.hasNext() )
    {
        it.next();
        FakeNetworkInterface * netDevice = it.value();
        if ( ( netDevice->type() == Solid::NetworkInterface::Ieee80211 && mUserWirelessEnabled )
           || netDevice->type() == Solid::NetworkInterface::Ieee8023 )
            netDevice->setActive( enabled );
    }
    mUserNetworkingEnabled = enabled;
}

void FakeNetworkManager::notifyHiddenNetwork( const QString & essid)
{
    // look up the device hosting the hidden net.
    // move the hidden net into the device's networks list
}

void FakeNetworkManager::parseNetworkingFile()
{
    QFile machineFile( mXmlFile );
    if( !machineFile.open(QIODevice::ReadOnly) )
    {
        kDebug() << k_funcinfo << "Error while opening " << mXmlFile << endl;
        return;
    }

    QDomDocument fakeDocument;
    if( !fakeDocument.setContent(&machineFile) )
    {
        kDebug() << k_funcinfo << "Error while creating the QDomDocument." << endl;
        machineFile.close();
        return;
    }
    machineFile.close();

    kDebug() << k_funcinfo << "Parsing fake computer XML: " << mXmlFile << endl;
    QDomElement mainElement = fakeDocument.documentElement();
    QDomNode node = mainElement.firstChild();
    while( !node.isNull() )
    {
        QDomElement tempElement = node.toElement();
        if( !tempElement.isNull() && tempElement.tagName() == QLatin1String("device") )
        {
            FakeNetworkInterface *tempDevice = parseDeviceElement(tempElement);
            if(tempDevice)
            {
               mNetworkInterfaces.insert(tempDevice->uni(), tempDevice);
// Use the DeviceManager for now, the udi/uni should
//                emit deviceAdded( tempDevice->uni());
            }
        }
        node = node.nextSibling();
    }
}

FakeNetworkInterface *FakeNetworkManager::parseDeviceElement(const QDomElement &deviceElement)
{
    FakeNetworkInterface *device = 0;
    QMap<QString,QVariant> propertyMap;
    QString uni = deviceElement.attribute("uni");
    propertyMap.insert( "uni", uni );
    kDebug() << k_funcinfo << "Listing device: " << uni << endl;
    propertyMap.insert( "uni", QVariant(uni) );
    QList< FakeNetwork * > networks;

    QDomNode childNode = deviceElement.firstChild();
    while( !childNode.isNull() )
    {
        QDomElement childElement = childNode.toElement();
        //kDebug() << "found child=" << childElement.tagName() << endl;
        if ( !childElement.isNull() && childElement.tagName() == QLatin1String("property") )
        {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = childElement.attribute("key");
            propertyValue = QVariant(childElement.text());
            //kDebug() << "Got property key=" << propertyKey << ", value=" << propertyValue.toString() << endl;
            propertyMap.insert( propertyKey, propertyValue );
        }
        else if ( !childElement.isNull() && childElement.tagName() == QLatin1String("network") )
        {
            QString uni = childElement.attribute("uni");
            kDebug() << k_funcinfo << "Listing properties: " << uni << endl;
            FakeNetwork * net = new FakeNetwork( uni, parseNetworkElement(childElement ));
            networks.append( net );
        }
        else if ( !childElement.isNull() && childElement.tagName() == QLatin1String("wireless") )
        {
            QString uni = childElement.attribute("uni");
            kDebug() << k_funcinfo << "Listing properties: " << uni << endl;
            FakeNetwork * wifi = new FakeWirelessNetwork( uni, parseNetworkElement( childElement ));
            networks.append( wifi );
        }
        childNode = childNode.nextSibling();
    }
    //kDebug() << "Done listing. " << endl;

/*    if( !propertyMap.isEmpty() )
    {*/
        kDebug() << k_funcinfo << "Creating FakeNetworkDevice for " << uni << endl;
        device = new FakeNetworkInterface(propertyMap);
        QListIterator< FakeNetwork * > it (networks );
        while ( it.hasNext() )
        {
            FakeNetwork * net = it.next();
            kDebug() << "Injecting " << net->uni() << endl;
            device->injectNetwork( net->uni(), net );
        }

//     }

    return device;
}

QMap<QString,QVariant> FakeNetworkManager::parseNetworkElement(const QDomElement &deviceElement)
{
    QMap<QString,QVariant> propertyMap;

    QDomNode propertyNode = deviceElement.firstChild();
    while( !propertyNode.isNull() )
    {
        QDomElement propertyElement = propertyNode.toElement();
        if( !propertyElement.isNull() && propertyElement.tagName() == QLatin1String("property") )
        {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = propertyElement.attribute("key");
            propertyValue = QVariant(propertyElement.text());
            //kDebug() << "Got property key=" << propertyKey << ", value=" << propertyValue.toString() << endl;
            propertyMap.insert( propertyKey, propertyValue );
        }

        propertyNode = propertyNode.nextSibling();
    }
    return propertyMap;
}


#include "fakenetworkmanager.moc"

