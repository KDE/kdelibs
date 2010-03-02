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

#ifndef KDED_KMIMEASSOCIATIONS_H
#define KDED_KMIMEASSOCIATIONS_H

#include <QSet>
#include <QStringList>
#include <QHash>
#include <kserviceoffer.h>
class KMimeTypeFactory;
class KConfigGroup;

struct ServiceTypeOffersData {
    QList<KServiceOffer> offers; // service + initial preference + allow as default
    QSet<KService::Ptr> offerSet; // for quick contains() check
    QSet<KService::Ptr> removedOffers; // remember removed offers explicitly
};

class KOfferHash
{
public:
    KOfferHash() {}
    QList<KServiceOffer> offersFor(const QString& serviceType) const {
        QHash<QString, ServiceTypeOffersData>::const_iterator it = m_serviceTypeData.find(serviceType);
        if (it != m_serviceTypeData.end())
            return (*it).offers;
        return QList<KServiceOffer>();
    }
    void addServiceOffer(const QString& serviceType, const KServiceOffer& offer);
    void removeServiceOffer(const QString& serviceType, KService::Ptr service);
    bool hasRemovedOffer(const QString& serviceType, KService::Ptr service) const;

private:
    KOfferHash(const KOfferHash&); // forbidden
    QHash<QString, ServiceTypeOffersData> m_serviceTypeData;
};


/**
 * Parse mimeapps.list files and:
 * - modify mimetype associations in the relevant services (using KServiceFactory)
 * - remember preferrence order specified by user
 */
class KMimeAssociations
{
public:
    explicit KMimeAssociations(KOfferHash& offerHash, KMimeTypeFactory* mimeTypeFactory);

    // Read mimeapps.list files
    bool parseAllMimeAppsList();

    void parseMimeAppsList(const QString& file, int basePreference);

private:
    void parseAddedAssociations(const KConfigGroup& group, const QString& file, int basePreference);
    void parseRemovedAssociations(const KConfigGroup& group, const QString& file);

    KOfferHash& m_offerHash;
    KMimeTypeFactory* m_mimeTypeFactory;
};

#endif /* KMIMEASSOCIATIONS_H */
