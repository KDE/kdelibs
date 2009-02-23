/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kbuildservicetypefactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <QtCore/QHash>

KBuildServiceTypeFactory::KBuildServiceTypeFactory() :
    KServiceTypeFactory()
{
    m_resourceList = new KSycocaResourceList;
    m_resourceList->add("servicetypes", "*.desktop");
}

// return all service types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildServiceTypeFactory::resourceTypes()
{
    return QStringList() << "servicetypes";
}

KBuildServiceTypeFactory::~KBuildServiceTypeFactory()
{
    delete m_resourceList;
}

KServiceType::Ptr KBuildServiceTypeFactory::findServiceTypeByName(const QString &_name)
{
    assert (KSycoca::self()->isBuilding());
    // We're building a database - the service type must be in memory
    KSycocaEntry::Ptr servType = m_entryDict->value( _name );
    return KServiceType::Ptr::staticCast( servType );
}


KSycocaEntry* KBuildServiceTypeFactory::createEntry(const QString &file, const char *resource) const
{
    QString name = file;
    int pos = name.lastIndexOf('/');
    if (pos != -1) {
        name = name.mid(pos+1);
    }

    if (name.isEmpty())
        return 0;

    KDesktopFile desktopFile(resource, file);
    const KConfigGroup desktopGroup = desktopFile.desktopGroup();

    if ( desktopGroup.readEntry( "Hidden", false ) == true )
        return 0;

    const QString type = desktopGroup.readEntry( "Type" );
    if ( type != QLatin1String( "ServiceType" ) ) {
        kWarning(7012) << "The service type config file " << desktopFile.fileName() << " has Type=" << type << " instead of Type=ServiceType";
        return 0;
    }

    const QString serviceType = desktopGroup.readEntry( "X-KDE-ServiceType" );

    if ( serviceType.isEmpty() ) {
        kWarning(7012) << "The service type config file " << desktopFile.fileName() << " does not contain a ServiceType=... entry";
        return 0;
    }

    KServiceType* e = new KServiceType( &desktopFile );

    if (e->isDeleted()) {
        delete e;
        return 0;
    }

    if ( !(e->isValid()) ) {
        kWarning(7012) << "Invalid ServiceType : " << file;
        delete e;
        return 0;
    }

    return e;
}

void
KBuildServiceTypeFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);
    str << (qint32) m_propertyTypeDict.count();
    for (QMap<QString, int>::ConstIterator it = m_propertyTypeDict.constBegin(); it != m_propertyTypeDict.constEnd(); ++it) {
        str << it.key() << (qint32)it.value();
    }
}

void
KBuildServiceTypeFactory::save(QDataStream &str)
{
    KSycocaFactory::save(str);
#if 0 // not needed since don't have any additional index anymore
    int endOfFactoryData = str.device()->pos();

    // Update header (pass #3)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
#endif
}

void
KBuildServiceTypeFactory::addEntry(const KSycocaEntry::Ptr& newEntry)
{
    KSycocaFactory::addEntry(newEntry);

    KServiceType::Ptr serviceType = KServiceType::Ptr::staticCast( newEntry );

    const QMap<QString,QVariant::Type>& pd = serviceType->propertyDefs();
    QMap<QString,QVariant::Type>::ConstIterator pit = pd.begin();
    for( ; pit != pd.end(); ++pit ) {
        const QString property = pit.key();
        QMap<QString, int>::iterator dictit = m_propertyTypeDict.find(property);
        if (dictit == m_propertyTypeDict.end())
            m_propertyTypeDict.insert(property, pit.value());
        else if (*dictit != static_cast<int>(pit.value()))
            kWarning(7021) << "Property '"<< property << "' is defined multiple times ("<< serviceType->name() <<")";
    }
}

