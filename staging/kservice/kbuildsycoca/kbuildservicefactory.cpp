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
#include "ksycocadict_p.h"
#include "ksycocaresourcelist.h"
#include "kdesktopfile.h"
#include <kservicetype.h>

#include <QDebug>
#include <QDir>
#include <qmimedatabase.h>

#include <klocalizedstring.h>
#include <assert.h>
#include <kmimetypefactory.h>
#include <qstandardpaths.h>

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
    m_resourceList->add("services", "kde5/services", "*.desktop");

    m_nameDict = new KSycocaDict();
    m_relNameDict = new KSycocaDict();
    m_menuIdDict = new KSycocaDict();
}

QStringList KBuildServiceFactory::resourceDirs()
{
    return QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "kde5/services", QStandardPaths::LocateDirectory);
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

KSycocaEntry* KBuildServiceFactory::createEntry(const QString& file) const
{
    Q_ASSERT(!file.startsWith("kde5/services/")); // we add this ourselves, below

    QString name = file;
    int pos = name.lastIndexOf('/');
    if (pos != -1) {
        name = name.mid(pos+1);
    }
    // Is it a .desktop file?
    if (name.endsWith(QLatin1String(".desktop"))) {

        //qDebug() << file;

        KService* serv;
        if (QDir::isAbsolutePath(file)) { // vfolder sends us full paths for applications
            serv = new KService(file);
        } else { // we get relative paths for services
            KDesktopFile desktopFile(QStandardPaths::GenericDataLocation, "kde5/services/" + file);
            // Note that the second arg below MUST be 'file', unchanged.
            // If the entry path doesn't match the 'file' parameter to createEntry, reusing old entries
            // (via time dict, which uses the entry path as key) cannot work.
            serv = new KService(&desktopFile, file);
        }

        //qDebug() << "Creating KService from" << file << "entryPath=" << serv->entryPath();
        // Note that the menuId will be set by the vfolder_menu.cpp code just after
        // createEntry returns.

        if ( serv->isValid() && !serv->isDeleted() ) {
            //qDebug() << "Creating KService from" << file << "entryPath=" << serv->entryPath() << "storageId=" << serv->storageId();
            return serv;
        } else {
            if (!serv->isDeleted()) {
                qWarning() << "Invalid Service : " << file;
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
    QSet<QString> visitedMimes;
    const QStringList allMimeTypes = m_mimeTypeFactory->allMimeTypes();
    Q_FOREACH(const QString& mimeType, allMimeTypes) {
        collectInheritedServices(mimeType, visitedMimes);
    }
    // TODO do the same for all/all and all/allfiles, if (!KServiceTypeProfile::configurationMode())
}

void KBuildServiceFactory::collectInheritedServices(const QString& mimeTypeName, QSet<QString>& visitedMimes)
{
    if (visitedMimes.contains(mimeTypeName))
        return;
    visitedMimes.insert(mimeTypeName);

    // With multiple inheritance, the "mimeTypeInheritanceLevel" isn't exactly
    // correct (it should only be increased when going up a level, not when iterating
    // through the multiple parents at a given level). I don't think we care, though.
    int mimeTypeInheritanceLevel = 0;

    QMimeDatabase db;
    QMimeType qmime = db.mimeTypeForName(mimeTypeName);
    Q_FOREACH(const QString& parentMimeType, qmime.parentMimeTypes()) {

        collectInheritedServices(parentMimeType, visitedMimes);

        ++mimeTypeInheritanceLevel;
        const QList<KServiceOffer>& offers = m_offerHash.offersFor(parentMimeType);
        QList<KServiceOffer>::const_iterator itserv = offers.begin();
        const QList<KServiceOffer>::const_iterator endserv = offers.end();
        for ( ; itserv != endserv; ++itserv ) {
            if (!m_offerHash.hasRemovedOffer(mimeTypeName, (*itserv).service())) {
                KServiceOffer offer(*itserv);
                offer.setMimeTypeInheritanceLevel(mimeTypeInheritanceLevel);
                //qDebug() << "INHERITANCE: Adding service" << (*itserv).service()->entryPath() << "to" << mimeTypeName << "mimeTypeInheritanceLevel=" << mimeTypeInheritanceLevel;
                m_offerHash.addServiceOffer( mimeTypeName, offer );
            }
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
        KService::Ptr service = KService::Ptr(entry);

        if (!service->isDeleted()) {
            const QString parent = service->parentApp();
            if (!parent.isEmpty())
                m_serviceGroupFactory->addNewChild(parent, KSycocaEntry::Ptr(service));
        }

        const QString name = service->desktopEntryName();
        m_nameDict->add(name, entry);
        m_nameMemoryHash.insert(name, service);

        const QString relName = service->entryPath();
        //qDebug() << "adding service" << service.data() << "isApp=" << service->isApplication() << "menuId=" << service->menuId() << "name=" << name << "relName=" << relName;
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
    QMimeDatabase db;
    // For every service...
    KSycocaEntryDict::Iterator itserv = m_entryDict->begin();
    const KSycocaEntryDict::Iterator endserv = m_entryDict->end();
    for( ; itserv != endserv ; ++itserv ) {

        KService::Ptr service = KService::Ptr(*itserv);
        QVector<KService::ServiceTypeAndPreference> serviceTypeList = service->_k_accessServiceTypes();
        //bool hasAllAll = false;
        //bool hasAllFiles = false;

        // Add this service to all its servicetypes (and their parents) and to all its mimetypes
        for (int i = 0; i < serviceTypeList.count() /*don't cache it, it can change during iteration!*/; ++i) {
            const QString stName = serviceTypeList[i].serviceType;
            // It could be a servicetype or a mimetype.
            KServiceType::Ptr serviceType = KServiceType::serviceType(stName);
            if (serviceType) {
                const int preference = serviceTypeList[i].preference;
                const QString parent = serviceType->parentServiceType();
                if (!parent.isEmpty())
                    serviceTypeList.append(KService::ServiceTypeAndPreference(preference, parent));

                //qDebug() << "Adding service" << service->entryPath() << "to" << serviceType->name() << "pref=" << preference;
                m_offerHash.addServiceOffer(stName, KServiceOffer(service, preference, 0, service->allowAsDefault()) );
            } else {
                KServiceOffer offer(service, serviceTypeList[i].preference, 0, service->allowAsDefault());
                QMimeType mime = db.mimeTypeForName(stName);
                if (!mime.isValid()) {
                    if (stName.startsWith(QLatin1String("x-scheme-handler/"))) {
                        // Create those on demand
                        m_mimeTypeFactory->createFakeMimeType(stName);
                        m_offerHash.addServiceOffer(stName, offer);
                    } else {
                        //qDebug() << service->entryPath() << "specifies undefined mimetype/servicetype" << stName;
                        // technically we could call addServiceOffer here, 'mime' isn't used. But it
                        // would be useless, since the loops for writing out the offers iterate
                        // over all known servicetypes and mimetypes. Unknown -> never written out.
                        continue;
                    }
                } else {
                    //qDebug() << "Adding service" << service->entryPath() << "to mime" << mime->name();
                    m_offerHash.addServiceOffer(mime.name(), offer); // mime->name so that we resolve aliases
                }
            }
        }
    }

    // Read user preferences (added/removed associations) and add/remove serviceoffers to m_offerHash
    KMimeAssociations mimeAssociations(m_offerHash);
    mimeAssociations.parseAllMimeAppsList();

    // Now for each mimetype, collect services from parent mimetypes
    collectInheritedServices();

    // Now collect the offsets into the (future) offer list
    // The loops look very much like the ones in saveOfferList obviously.
    int offersOffset = 0;
    const int offerEntrySize = sizeof( qint32 ) * 4; // four qint32s, see saveOfferList.

    // TODO: idea: we could iterate over m_offerHash, and look up the servicetype or mimetype.
    // Would that be faster than iterating over all servicetypes and mimetypes?

    KSycocaEntryDict::const_iterator itstf = m_serviceTypeFactory->entryDict()->constBegin();
    const KSycocaEntryDict::const_iterator endstf = m_serviceTypeFactory->entryDict()->constEnd();
    for( ; itstf != endstf; ++itstf ) {
        KServiceType::Ptr entry = KServiceType::Ptr( *itstf );
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
        KMimeTypeFactory::MimeTypeEntry::Ptr entry = KMimeTypeFactory::MimeTypeEntry::Ptr( *itmtf );
        const int numOffers = m_offerHash.offersFor(entry->name()).count();
        if ( numOffers ) {
            //qDebug() << entry->name() << "offset=" << offersOffset;
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
        const KServiceType::Ptr entry = KServiceType::Ptr( *itstf );
        Q_ASSERT( entry );

        QList<KServiceOffer> offers = m_offerHash.offersFor(entry->name());
        qStableSort( offers ); // by initial preference

        for(QList<KServiceOffer>::const_iterator it2 = offers.constBegin();
            it2 != offers.constEnd(); ++it2) {
            //qDebug() << "servicetype offers list:" << entry->name() << "->" << (*it2).service()->entryPath();

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
        const KMimeTypeFactory::MimeTypeEntry::Ptr entry = KMimeTypeFactory::MimeTypeEntry::Ptr( *itmtf );
        Q_ASSERT( entry );
        QList<KServiceOffer> offers = m_offerHash.offersFor(entry->name());
        qStableSort( offers ); // by initial preference

        for(QList<KServiceOffer>::const_iterator it2 = offers.constBegin();
            it2 != offers.constEnd(); ++it2) {
            //qDebug() << "mimetype offers list:" << entry->name() << "->" << (*it2).service()->entryPath() << "pref" << (*it2).preference();
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

    const KService::Ptr service = KService::Ptr( newEntry );
    m_dupeDict.insert(newEntry);
    KSycocaFactory::addEntry(newEntry);
}
