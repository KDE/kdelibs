/*  This file is part of the KDE libraries
 *  Copyright (C) 1999, 2007 David Faure <faure@kde.org>
 *                1999 Waldo Bastian <bastian@kde.org>
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

#include "kbuildservicefactory.h"
#include "kbuildservicegroupfactory.h"
#include "kbuildmimetypefactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"
#include "kdesktopfile.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <assert.h>
#include <kmimetypefactory.h>

KBuildServiceFactory::KBuildServiceFactory( KSycocaFactory *serviceTypeFactory,
                                            KBuildMimeTypeFactory *mimeTypeFactory,
                                            KBuildServiceGroupFactory *serviceGroupFactory ) :
    KServiceFactory(),
    m_nameMemoryHash(),
    m_relNameMemoryHash(),
    m_menuIdMemoryHash(),
    m_dupeDict(),
    m_serviceTypeFactory( serviceTypeFactory ),
    m_mimeTypeFactory( mimeTypeFactory ),
    m_serviceGroupFactory( serviceGroupFactory )
{
    m_resourceList = new KSycocaResourceList();
    // We directly care about services desktop files.
    // All the application desktop files are parsed on demand from the vfolder menu code.
    m_resourceList->add( "services", "*.desktop" );

    m_nameDict = new KSycocaDict();
    m_relNameDict = new KSycocaDict();
    m_menuIdDict = new KSycocaDict();
}

// return all service types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildServiceFactory::resourceTypes()
{
    return QStringList() << "services";
}

KBuildServiceFactory::~KBuildServiceFactory()
{
    delete m_resourceList;
}

KService::Ptr KBuildServiceFactory::findServiceByDesktopName(const QString &name)
{
    return m_nameMemoryHash.value(name);
}

KService::Ptr KBuildServiceFactory::findServiceByDesktopPath(const QString &name)
{
    return m_relNameMemoryHash.value(name);
}

KService::Ptr KBuildServiceFactory::findServiceByMenuId(const QString &menuId)
{
    return m_menuIdMemoryHash.value(menuId);
}

KSycocaEntry* KBuildServiceFactory::createEntry( const QString& file, const char *resource ) const
{
    QString name = file;
    int pos = name.lastIndexOf('/');
    if (pos != -1) {
        name = name.mid(pos+1);
    }
    // Is it a .desktop file?
    if (name.endsWith(QLatin1String(".desktop"))) {
        KDesktopFile desktopFile(resource, file);

        KService * serv = new KService(&desktopFile);
        //kDebug(7021) << "Creating KService from" << file << "entryPath=" << serv->entryPath();
        // Note that the menuId will be set by the vfolder_menu.cpp code just after
        // createEntry returns.

        if ( serv->isValid() && !serv->isDeleted() ) {
            return serv;
        } else {
            if (!serv->isDeleted()) {
                kWarning(7012) << "Invalid Service : " << file;
            }
            delete serv;
            return 0;
        }
    } // TODO else if a Windows application,  new KService(name, exec, icon)
    return 0;
}

void KBuildServiceFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);

    str << (qint32) m_nameDictOffset;
    str << (qint32) m_relNameDictOffset;
    str << (qint32) m_offerListOffset;
    str << (qint32) m_menuIdDictOffset;
}

void KBuildServiceFactory::save(QDataStream &str)
{
    KSycocaFactory::save(str);

    m_nameDictOffset = str.device()->pos();
    m_nameDict->save(str);

    m_relNameDictOffset = str.device()->pos();
    m_relNameDict->save(str);

    saveOfferList(str);

    m_menuIdDictOffset = str.device()->pos();
    m_menuIdDict->save(str);

    int endOfFactoryData = str.device()->pos();

    // Update header (pass #3)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
}

void KBuildServiceFactory::collectInheritedServices()
{
    // For each mimetype, go up the parent-mimetype chains and collect offers.
    // For "removed associations" to work, we can't just grab everything from all parents.
    // We need to process parents before children, hence the recursive call in
    // collectInheritedServices(mime) and the QSet to process a given parent only once.
    QSet<KMimeType::Ptr> visitedMimes;
    const KMimeType::List allMimeTypes = m_mimeTypeFactory->allMimeTypes();
    KMimeType::List::const_iterator itm = allMimeTypes.begin();
    for( ; itm != allMimeTypes.end(); ++itm ) {
        const KMimeType::Ptr mimeType = *itm;
        collectInheritedServices(mimeType, visitedMimes);
    }
    // TODO do the same for all/all and all/allfiles, if (!KServiceTypeProfile::configurationMode())
}

void KBuildServiceFactory::collectInheritedServices(KMimeType::Ptr mimeType, QSet<KMimeType::Ptr>& visitedMimes)
{
    if (visitedMimes.contains(mimeType))
        return;
    visitedMimes.insert(mimeType);

    // With multiple inheritance, the "mimeTypeInheritanceLevel" isn't exactly
    // correct (it should only be increased when going up a level, not when iterating
    // through the multiple parents at a given level). I don't think we care, though.
    int mimeTypeInheritanceLevel = 0;

    const QString mimeTypeName = mimeType->name();
    Q_FOREACH(const QString& parent, mimeType->parentMimeTypes()) {
        const KMimeType::Ptr parentMimeType =
            m_mimeTypeFactory->findMimeTypeByName(parent, KMimeType::ResolveAliases);

        if ( parentMimeType ) {
            collectInheritedServices(parentMimeType, visitedMimes);

            ++mimeTypeInheritanceLevel;
            const QList<KServiceOffer>& offers = m_offerHash.offersFor(parent);
            QList<KServiceOffer>::const_iterator itserv = offers.begin();
            const QList<KServiceOffer>::const_iterator endserv = offers.end();
            for ( ; itserv != endserv; ++itserv ) {
                if (!m_offerHash.hasRemovedOffer(mimeTypeName, (*itserv).service())) {
                    KServiceOffer offer(*itserv);
                    offer.setMimeTypeInheritanceLevel(mimeTypeInheritanceLevel);
                    //kDebug(7021) << "INHERITANCE: Adding service" << (*itserv).service()->entryPath() << "to" << mimeTypeName << "mimeTypeInheritanceLevel=" << mimeTypeInheritanceLevel;
                    m_offerHash.addServiceOffer( mimeTypeName, offer );
                }
            }
        } else {
            kWarning(7012) << "parent mimetype not found:" << parent;
            break;
        }
    }
}

void KBuildServiceFactory::postProcessServices()
{
    // By doing all this here rather than in addEntry (and removing when replacing
    // with local override), we only do it for the final applications.

    // For every service...
    KSycocaEntryDict::Iterator itserv = m_entryDict->begin();
    const KSycocaEntryDict::Iterator endserv = m_entryDict->end();
    for( ; itserv != endserv ; ++itserv ) {

        KSycocaEntry::Ptr entry = *itserv;
        KService::Ptr service = KService::Ptr::staticCast(entry);

        if (!service->isDeleted()) {
            const QString parent = service->parentApp();
            if (!parent.isEmpty())
                m_serviceGroupFactory->addNewChild(parent, KSycocaEntry::Ptr::staticCast(service));
        }

        const QString name = service->desktopEntryName();
        m_nameDict->add(name, entry);
        m_nameMemoryHash.insert(name, service);

        const QString relName = service->entryPath();
        //kDebug(7021) << "adding service" << service.data() << service->type() << "menuId=" << service->menuId() << "name=" << name << "relName=" << relName;
        m_relNameDict->add(relName, entry);
        m_relNameMemoryHash.insert(relName, service); // for KMimeAssociations

        const QString menuId = service->menuId();
        if (!menuId.isEmpty()) { // empty for services, non-empty for applications
            m_menuIdDict->add(menuId, entry);
            m_menuIdMemoryHash.insert(menuId, service); // for KMimeAssociations
        }
    }
    populateServiceTypes();
}

void KBuildServiceFactory::populateServiceTypes()
{
    // For every service...
    KSycocaEntryDict::Iterator itserv = m_entryDict->begin();
    const KSycocaEntryDict::Iterator endserv = m_entryDict->end();
    for( ; itserv != endserv ; ++itserv ) {

        KService::Ptr service = KService::Ptr::staticCast(*itserv);
        QVector<KService::ServiceTypeAndPreference> serviceTypeList = service->_k_accessServiceTypes();
        //bool hasAllAll = false;
        //bool hasAllFiles = false;

        // Add this service to all its servicetypes (and their parents) and to all its mimetypes
        for (int i = 0; i < serviceTypeList.count() /*don't cache it, it can change during iteration!*/; ++i) {
            const QString stName = serviceTypeList[i].serviceType;
            // It could be a servicetype or a mimetype.
            KServiceType::Ptr serviceType = KServiceType::serviceType(stName);
            if (!serviceType) {
                serviceType = KServiceType::Ptr::staticCast(m_mimeTypeFactory->findMimeTypeByName(stName, KMimeType::ResolveAliases));
            }
            // TODO. But maybe we should rename all/all to */*, to also support image/*?
            // Not sure how to model all/allfiles then, though
            // Also this kind of thing isn't in the XDG standards...
#if 0
            if (!serviceType) {
                if ( stName == QLatin1String( "all/all" ) ) {
                    hasAllAll = true;
                    continue;
                } else if ( stName == QLatin1String( "all/allfiles" ) ) {
                    hasAllFiles = true;
                    continue;
                }
            }
#endif

            if (!serviceType) {
                kDebug(7021) << service->entryPath() << "specifies undefined mimetype/servicetype" << stName;
                continue;
            }

            const int preference = serviceTypeList[i].preference;
            const QString parent = serviceType->parentServiceType();
            if (!parent.isEmpty())
                serviceTypeList.append(KService::ServiceTypeAndPreference(preference, parent));

            //kDebug(7021) << "Adding service" << service->entryPath() << "to" << serviceType->name() << "pref=" << preference;
            m_offerHash.addServiceOffer(stName, KServiceOffer(service, preference, 0, service->allowAsDefault()) );
        }
    }

    // Read user preferences (added/removed associations) and add/remove serviceoffers to m_offerHash
    KMimeAssociations mimeAssociations(m_offerHash, m_mimeTypeFactory);
    mimeAssociations.parseAllMimeAppsList();

    // Now for each mimetype, collect services from parent mimetypes
    collectInheritedServices();

    // Now collect the offsets into the (future) offer list
    // The loops look very much like the ones in saveOfferList obviously.
    int offersOffset = 0;
    const int offerEntrySize = sizeof( qint32 ) * 4; // four qint32s, see saveOfferList.

    KSycocaEntryDict::const_iterator itstf = m_serviceTypeFactory->entryDict()->constBegin();
    const KSycocaEntryDict::const_iterator endstf = m_serviceTypeFactory->entryDict()->constEnd();
    for( ; itstf != endstf; ++itstf ) {
        KServiceType::Ptr entry = KServiceType::Ptr::staticCast( *itstf );
        const int numOffers = m_offerHash.offersFor(entry->name()).count();
        if ( numOffers ) {
            entry->setServiceOffersOffset( offersOffset );
            offersOffset += offerEntrySize * numOffers;
        }
    }
    KSycocaEntryDict::const_iterator itmtf = m_mimeTypeFactory->entryDict()->constBegin();
    const KSycocaEntryDict::const_iterator endmtf = m_mimeTypeFactory->entryDict()->constEnd();
    for( ; itmtf != endmtf; ++itmtf )
    {
        KMimeType::Ptr entry = KMimeType::Ptr::staticCast( *itmtf );
        const int numOffers = m_offerHash.offersFor(entry->name()).count();
        if ( numOffers ) {
            entry->setServiceOffersOffset( offersOffset );
            offersOffset += offerEntrySize * numOffers;
        }
    }
}

void KBuildServiceFactory::saveOfferList(QDataStream &str)
{
    m_offerListOffset = str.device()->pos();

    // For each entry in servicetypeFactory
    KSycocaEntryDict::const_iterator itstf = m_serviceTypeFactory->entryDict()->constBegin();
    const KSycocaEntryDict::const_iterator endstf = m_serviceTypeFactory->entryDict()->constEnd();
    for( ; itstf != endstf; ++itstf ) {
        // export associated services
        const KServiceType::Ptr entry = KServiceType::Ptr::staticCast( *itstf );
        Q_ASSERT( entry );

        QList<KServiceOffer> offers = m_offerHash.offersFor(entry->name());
        qStableSort( offers ); // by initial preference

        for(QList<KServiceOffer>::const_iterator it2 = offers.constBegin();
            it2 != offers.constEnd(); ++it2) {
            //kDebug(7021) << "servicetype offers list:" << entry->name() << "->" << (*it2).service()->entryPath();

            str << (qint32) entry->offset();
            str << (qint32) (*it2).service()->offset();
            str << (qint32) (*it2).preference();
            str << (qint32) 0; // mimeTypeInheritanceLevel
            // update offerEntrySize in populateServiceTypes if you add/remove something here
        }
    }

    // For each entry in mimeTypeFactory
    KSycocaEntryDict::const_iterator itmtf = m_mimeTypeFactory->entryDict()->constBegin();
    const KSycocaEntryDict::const_iterator endmtf = m_mimeTypeFactory->entryDict()->constEnd();
    for( ; itmtf != endmtf; ++itmtf ) {
        // export associated services
        const KMimeType::Ptr entry = KMimeType::Ptr::staticCast( *itmtf );
        Q_ASSERT( entry );
        QList<KServiceOffer> offers = m_offerHash.offersFor(entry->name());
        qStableSort( offers ); // by initial preference

        for(QList<KServiceOffer>::const_iterator it2 = offers.constBegin();
            it2 != offers.constEnd(); ++it2) {
            //kDebug(7021) << "mimetype offers list:" << entry->name() << "->" << (*it2).service()->entryPath() << "pref" << (*it2).preference();
            Q_ASSERT((*it2).service()->offset() != 0);
            str << (qint32) entry->offset();
            str << (qint32) (*it2).service()->offset();
            str << (qint32) (*it2).preference();
            str << (qint32) (*it2).mimeTypeInheritanceLevel();
            // update offerEntrySize in populateServiceTypes if you add/remove something here
        }
    }

    str << (qint32) 0;               // End of list marker (0)
}

void KBuildServiceFactory::addEntry(const KSycocaEntry::Ptr& newEntry)
{
    Q_ASSERT(newEntry);
    if (m_dupeDict.contains(newEntry))
        return;

    const KService::Ptr service = KService::Ptr::staticCast( newEntry );
    m_dupeDict.insert(newEntry);
    KSycocaFactory::addEntry(newEntry);
}
