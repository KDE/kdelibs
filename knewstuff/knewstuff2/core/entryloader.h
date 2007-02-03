/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KNEWSTUFF2_ENTRY_LOADER_H
#define KNEWSTUFF2_ENTRY_LOADER_H

#include <knewstuff2/entry.h>

#include <qdom.h>
#include <qobject.h>
#include <qstring.h>

#include <kurl.h>

class KJob;

namespace KIO { class Job; }

namespace KNS {

/**
 * KNewStuff entry loader.
 * Loads any entries from a given file and notifies about when the
 * loading has completed.
 */
class KDE_EXPORT EntryLoader : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructor.
     */
    EntryLoader();

    /**
     * Starts asynchronously loading the list of entries from the
     * given URL.
     *
     * @param stuffurl location of the XML file containing the entries
     */
    void load(const QString &stuffurl);

  signals:
    /**
     * Indicates that the list of entries has been successfully loaded.
     */
    void signalEntriesLoaded(KNS::Entry::List *);
    void signalEntriesFailed();

  protected slots:
    void slotJobData(KIO::Job *, const QByteArray &);
    void slotJobResult(KJob *);

  private:
    QByteArray m_jobdata;

    Entry::List m_entries;
};

}

#endif
