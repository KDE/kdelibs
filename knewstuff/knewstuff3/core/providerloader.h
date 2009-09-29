/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KNEWSTUFF2_PROVIDER_LOADER_H
#define KNEWSTUFF2_PROVIDER_LOADER_H

#include <knewstuff2/core/provider.h>

#include <QtXml/qdom.h>
#include <QtCore/QObject>
#include <QtCore/QString>

#include <kurl.h>

class KJob;

namespace KIO
{
class Job;
}

namespace KNS
{

/**
 * KNewStuff provider loader.
 * This class sets up a list of all possible providers by querying
 * the main provider database for this specific application.
 * It should probably not be used directly by the application.
 *
 * @internal
 */
class KNEWSTUFF_EXPORT ProviderLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    ProviderLoader(QObject* parent);

    /**
     * Starts asynchronously loading the list of providers from the
     * specified URL.
     *
     * @param providersurl location of the XML file containing the providers
     */
    void load(const QString &providersurl);

Q_SIGNALS:
    /**
     * Indicates that the list of providers has been successfully loaded.
     */
    void signalProvidersLoaded(KNS::Provider::List);
    void signalProvidersFailed();

protected Q_SLOTS:
    void slotJobData(KIO::Job *, const QByteArray &);
    void slotJobResult(KJob *);

private:
    QByteArray m_jobdata;

    Provider::List m_providers;
};

}

#endif
