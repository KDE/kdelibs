/*  This file is part of the KDE libraries
 *  Copyright 2008  David Faure  <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kmimeassociations.h"
#include <kmimetype.h>
#include <kmimetypefactory.h>
#include <kservice.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

KMimeAssociations::KMimeAssociations(KOfferHash& offerHash, KMimeTypeFactory* mimeTypeFactory)
    : m_offerHash(offerHash), m_mimeTypeFactory(mimeTypeFactory)
{
}

/*

The goal of this class is to parse mimeapps.list files, which are used to
let users configure the application-mimetype associations.

Example file:

[Added Associations]
text/plain=kate.desktop;

[Removed Associations]
text/plain=gnome-gedit.desktop;gnu-emacs.desktop;



*/

bool KMimeAssociations::parseAllMimeAppsList()
{
    // Using the "merged view" from KConfig is not enough since we -add- at every level, we don't replace.
    const QStringList mimeappsFiles = KGlobal::dirs()->findAllResources("xdgdata-apps", "mimeapps.list");
    if (mimeappsFiles.isEmpty())
        return false;

    int basePreference = 1000; // start high :)
    QListIterator<QString> mimeappsIter( mimeappsFiles );
    mimeappsIter.toBack();
    while (mimeappsIter.hasPrevious()) { // global first, then local.
        const QString mimeappsFile = mimeappsIter.previous();
        kDebug(7021) << "Parsing" << mimeappsFile;
        parseMimeAppsList(mimeappsFile, basePreference);
        basePreference += 50;
    }
    return true;
}

void KMimeAssociations::parseMimeAppsList(const QString& file, int basePreference)
{
    KConfig profile(file, KConfig::SimpleConfig);
    parseAddedAssociations(KConfigGroup(&profile, "Added Associations"), file, basePreference);
    parseRemovedAssociations(KConfigGroup(&profile, "Removed Associations"), file);

    // KDE extension for parts and plugins, see settings/filetypes/mimetypedata.cpp
    parseAddedAssociations(KConfigGroup(&profile, "Added KDE Service Associations"), file, basePreference);
    parseRemovedAssociations(KConfigGroup(&profile, "Removed KDE Service Associations"), file);
}

void KMimeAssociations::parseAddedAssociations(const KConfigGroup& group, const QString& file, int basePreference)
{
    Q_FOREACH(const QString& mimeName, group.keyList()) {
        const QStringList services = group.readXdgListEntry(mimeName);
        KMimeType::Ptr mime = m_mimeTypeFactory->findMimeTypeByName(mimeName, KMimeType::ResolveAliases);
        if (!mime) {
            kDebug(7021) << file << "specifies unknown mimetype" << mimeName;
            continue;
        }
        const QString resolvedMimeName = mime->name();
        int pref = basePreference;
        Q_FOREACH(const QString &service, services) {
            KService::Ptr pService = KService::serviceByStorageId(service);
            if (!pService) {
                kDebug(7021) << file << "specifies unknown service" << service << "in" << group.name();
            } else {
                //kDebug(7021) << "adding mime" << resolvedMimeName << "to service" << pService->entryPath() << "pref=" << pref;
                m_offerHash.addServiceOffer(resolvedMimeName, KServiceOffer(pService, pref, 0, pService->allowAsDefault()));
                --pref;
            }
        }
    }
}

void KMimeAssociations::parseRemovedAssociations(const KConfigGroup& group, const QString& file)
{
    Q_FOREACH(const QString& mime, group.keyList()) {
        const QStringList services = group.readXdgListEntry(mime);
        Q_FOREACH(const QString& service, services) {
            KService::Ptr pService = KService::serviceByStorageId(service);
            if (!pService) {
                kDebug(7021) << file << "specifies unknown service" << service << "in" << group.name();
            } else {
                //kDebug(7021) << "removing mime" << mime << "from service" << pService.data() << pService->entryPath();
                m_offerHash.removeServiceOffer(mime, pService);
            }
        }
    }
}

void KOfferHash::addServiceOffer(const QString& serviceType, const KServiceOffer& offer)
{
    KService::Ptr service = offer.service();
    //kDebug(7021) << "Adding" << service->entryPath() << "to" << serviceType << offer.preference();
    ServiceTypeOffersData& data = m_serviceTypeData[serviceType]; // find or create
    QList<KServiceOffer>& offers = data.offers;
    QSet<KService::Ptr>& offerSet = data.offerSet;
    if ( !offerSet.contains( service ) ) {
        offers.append( offer );
        offerSet.insert( service );
    } else {
        //kDebug(7021) << service->entryPath() << "already in" << serviceType;
        // This happens when mimeapps.list mentions a service (to make it preferred)
        // Update initialPreference to qMax(existing offer, new offer)
        QMutableListIterator<KServiceOffer> sfit(data.offers);
        while (sfit.hasNext()) {
            if (sfit.next().service() == service) // we can compare KService::Ptrs because they are from the memory hash
                sfit.value().setPreference( qMax(sfit.value().preference(), offer.preference()) );
        }
    }
}

void KOfferHash::removeServiceOffer(const QString& serviceType, KService::Ptr service)
{
    ServiceTypeOffersData& data = m_serviceTypeData[serviceType]; // find or create
    data.removedOffers.insert(service);
    data.offerSet.remove(service);
    QMutableListIterator<KServiceOffer> sfit(data.offers);
    while (sfit.hasNext()) {
        if (sfit.next().service()->storageId() == service->storageId())
            sfit.remove();
    }
}

bool KOfferHash::hasRemovedOffer(const QString& serviceType, KService::Ptr service) const
{
    QHash<QString, ServiceTypeOffersData>::const_iterator it = m_serviceTypeData.find(serviceType);
    if (it != m_serviceTypeData.end()) {
        return (*it).removedOffers.contains(service);
    }
    return false;
}
