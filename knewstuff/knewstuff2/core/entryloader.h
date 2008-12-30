/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF2_ENTRY_LOADER_H
#define KNEWSTUFF2_ENTRY_LOADER_H

#include <knewstuff2/core/entry.h>

#include <QtXml/qdom.h>
#include <QtCore/QObject>
#include <QtCore/QString>

class KJob;

namespace KIO
{
class Job;
}

namespace KNS
{

class Feed;
class Provider;

/**
 * KNewStuff entry loader.
 * Loads any entries from a given file and notifies about when the
 * loading has completed.
 *
 * @internal
 */
class EntryLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    EntryLoader(QObject* parent);

    /**
     * Starts asynchronously loading the list of entries from the
     * given provider for the given feed.
     *
     * @param provider Provider to load the entries from
     * @param feed Feed to download
     */
    void load(const Provider *provider, Feed *feed);

    /**
     * Returns the provider which was used for download.
     *
     * @return Provider used by this loader
     */
    const Provider *provider() const;

    /**
     * Returns the feed which was used for download.
     *
     * @return Feed used by this loader
     */
    Feed *feed() const;

Q_SIGNALS:
    /**
     * Indicates that the list of entries has been successfully loaded.
     */
    void signalEntriesLoaded(KNS::Entry::List list);

    /**
     * Indicates that the list of entries could not be loaded.
     */
    void signalEntriesFailed();

    /**
     * report progress loading
     */
    void signalProgress(KJob*, unsigned long);

protected Q_SLOTS:
    void slotJobData(KIO::Job *, const QByteArray &);
    void slotJobResult(KJob *);

private:
    QByteArray m_jobdata;

    Entry::List m_entries;
    Feed *m_feed;
    const Provider *m_provider;
};

}

#endif
