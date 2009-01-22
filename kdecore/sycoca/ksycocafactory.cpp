/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
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
 */

#include "ksycocafactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocaentry.h"
#include "ksycocadict.h"

#include <config.h>
#include <kdebug.h>

#include <QtCore/QMutableStringListIterator>
#include <QtCore/QHash>

class KSycocaFactory::Private
{
public:
    Private() {}
    ~Private()
    {
        delete m_sycocaDict;
    }

    int mOffset;
    int m_sycocaDictOffset;
    int m_beginEntryOffset;
    int m_endEntryOffset;
    KSycocaDict *m_sycocaDict;
};

KSycocaFactory::KSycocaFactory(KSycocaFactoryId factory_id)
    : m_resourceList(0), m_entryDict(0), d(new Private)
{
    if (!KSycoca::self()->isBuilding() && (m_str = KSycoca::self()->findFactory( factory_id )))
    {
        // Read position of index tables....
        qint32 i;
        (*m_str) >> i;
        d->m_sycocaDictOffset = i;
        (*m_str) >> i;
        d->m_beginEntryOffset = i;
        (*m_str) >> i;
        d->m_endEntryOffset = i;

        int saveOffset = m_str->device()->pos();
        // Init index tables
        d->m_sycocaDict = new KSycocaDict(m_str, d->m_sycocaDictOffset);
        saveOffset = m_str->device()->seek(saveOffset);
    }
    else
    {
        // Build new database!
        m_str = 0;
        m_entryDict = new KSycocaEntryDict;
        d->m_sycocaDict = new KSycocaDict;
        d->m_beginEntryOffset = 0;
        d->m_endEntryOffset = 0;

        // m_resourceList will be filled in by inherited constructors
    }
    KSycoca::self()->addFactory(this);
}

KSycocaFactory::~KSycocaFactory()
{
    delete m_entryDict;
    delete d;
}

void
KSycocaFactory::saveHeader(QDataStream &str)
{
    // Write header
    str.device()->seek(d->mOffset);
    str << (qint32) d->m_sycocaDictOffset;
    str << (qint32) d->m_beginEntryOffset;
    str << (qint32) d->m_endEntryOffset;
}

void
KSycocaFactory::save(QDataStream &str)
{
    if (!m_entryDict) return; // Error! Function should only be called when
    // building database
    if (!d->m_sycocaDict) return; // Error!

    d->mOffset = str.device()->pos(); // store position in member variable
    d->m_sycocaDictOffset = 0;

    // Write header (pass #1)
    saveHeader(str);

    d->m_beginEntryOffset = str.device()->pos();

    // Write all entries.
    int entryCount = 0;
    for(KSycocaEntryDict::Iterator it = m_entryDict->begin();
        it != m_entryDict->end(); ++it)
    {
        KSycocaEntry::Ptr entry = *it;
        entry->save(str);
        entryCount++;
    }

    d->m_endEntryOffset = str.device()->pos();

    // Write indices...
    // Linear index
    str << (qint32) entryCount;
    for(KSycocaEntryDict::Iterator it = m_entryDict->begin();
        it != m_entryDict->end(); ++it)
    {
        str << qint32(it->data()->offset());
    }

    // Dictionary index
    d->m_sycocaDictOffset = str.device()->pos();
    d->m_sycocaDict->save(str);

    int endOfFactoryData = str.device()->pos();

    // Update header (pass #2)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
}

void
KSycocaFactory::addEntry(const KSycocaEntry::Ptr& newEntry)
{
    if (!m_entryDict) return; // Error! Function should only be called when
    // building database

    if (!d->m_sycocaDict) return; // Error!

    const QString name = newEntry->storageId();
    m_entryDict->insert( name, newEntry );
    d->m_sycocaDict->add( name, newEntry );
}

void
KSycocaFactory::removeEntry(const QString& entryName)
{
    if (!m_entryDict) return; // Error! Function should only be called when
    // building database

    if (!d->m_sycocaDict) return; // Error!

    m_entryDict->remove( entryName );
    d->m_sycocaDict->remove( entryName ); // O(N)
}

KSycocaEntry::List KSycocaFactory::allEntries() const
{
    KSycocaEntry::List list;
    if (!m_str) return list;

    // Assume we're NOT building a database

    m_str->device()->seek(d->m_endEntryOffset);
    qint32 entryCount;
    (*m_str) >> entryCount;

    if (entryCount > 8192)
    {
        KSycoca::flagError();
        return list;
    }

    // offsetList is needed because createEntry() modifies the stream position
    qint32 *offsetList = new qint32[entryCount];
    for(int i = 0; i < entryCount; i++)
    {
        (*m_str) >> offsetList[i];
    }

    for(int i = 0; i < entryCount; i++)
    {
        KSycocaEntry *newEntry = createEntry(offsetList[i]);
        if (newEntry)
        {
            list.append( KSycocaEntry::Ptr( newEntry ) );
        }
    }
    delete [] offsetList;
    return list;
}

int KSycocaFactory::offset() const
{
    return d->mOffset;
}

const KSycocaResourceList * KSycocaFactory::resourceList() const
{
    return m_resourceList;
}

const KSycocaDict * KSycocaFactory::sycocaDict() const
{
    return d->m_sycocaDict;
}

bool KSycocaFactory::isEmpty() const
{
    return d->m_beginEntryOffset == d->m_endEntryOffset;
}

void KSycocaFactory::virtual_hook( int /*id*/, void* /*data*/)
{ /*BASE::virtual_hook( id, data );*/ }
