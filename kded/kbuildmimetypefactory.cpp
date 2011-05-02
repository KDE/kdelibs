/*  This file is part of the KDE libraries
 *  Copyright 1999-2007 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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

#include "kbuildmimetypefactory.h"
#include "kmimetyperepository_p.h"
#include "ksycoca.h"
#include "ksycocadict_p.h"
#include "ksycocaresourcelist.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>
#include <QtCore/QHash>

KBuildMimeTypeFactory::KBuildMimeTypeFactory() :
    KMimeTypeFactory()
{
    m_resourceList = new KSycocaResourceList;
    // We want all xml files under xdgdata-mime - but not packages/*.xml
    m_resourceList->add( "xdgdata-mime", "*.xml" );
}

// return all resource types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildMimeTypeFactory::resourceTypes()
{
    return QStringList() << "xdgdata-mime";
}

KBuildMimeTypeFactory::~KBuildMimeTypeFactory()
{
    delete m_resourceList;
}

KMimeTypeFactory::MimeTypeEntry::Ptr KBuildMimeTypeFactory::findMimeTypeEntryByName(const QString &_name, KMimeType::FindByNameOption options)
{
    assert (KSycoca::self()->isBuilding());

    QString name = _name;
    if (options & KMimeType::ResolveAliases) {
        name = KMimeTypeRepository::self()->canonicalName(_name);
    }

    // We're building a database - the mime type must be in memory
    KSycocaEntry::Ptr servType = m_entryDict->value( name );
    return MimeTypeEntry::Ptr::staticCast( servType );
}

KSycocaEntry::List KBuildMimeTypeFactory::allEntries() const
{
    assert (KSycoca::self()->isBuilding());
    KSycocaEntry::List lst;
    KSycocaEntryDict::Iterator itmime = m_entryDict->begin();
    const KSycocaEntryDict::Iterator endmime = m_entryDict->end();
    for( ; itmime != endmime ; ++itmime )
        lst.append( *itmime );
    return lst;
}

KSycocaEntry* KBuildMimeTypeFactory::createEntry(const QString &file, const char *resource) const
{
    Q_UNUSED(resource);

    // file=text/plain.xml  ->  name=plain.xml dirName=text
    const int pos = file.lastIndexOf('/');
    if (pos == -1) // huh?
        return 0;
    const QString dirName = file.left(pos);
    if (dirName == "packages") // special subdir
        return 0;

    const int dot = file.lastIndexOf('.');
    if (dot == -1) // huh?
        return 0;
    const QString name = file.left(dot);

    //kDebug() << "Creating mimetype" << name << "from file" << file;

    MimeTypeEntry* e = new MimeTypeEntry(file, name);
    return e;
}

void KBuildMimeTypeFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);
}

void KBuildMimeTypeFactory::save(QDataStream &str)
{
    KSycocaFactory::save(str);

    str << (qint32) 0;

    const int endOfFactoryData = str.device()->pos();

    // Update header (pass #3)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
}

void KBuildMimeTypeFactory::createFakeMimeType(const QString& name)
{
   const QString file = name; // hack
   KSycocaEntry::Ptr entry = m_entryDict->value(file);
   if (!entry) {
       MimeTypeEntry* e = new MimeTypeEntry(file, name);
       entry = e;
   }

   Q_ASSERT(entry && entry->isValid());
   addEntry(entry);
}
