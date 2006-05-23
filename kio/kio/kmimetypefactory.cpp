/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2006 David Faure <bastian@kde.org>
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

#include "kmimetypefactory.h"
#include "kmimetype.h"
#include "kdedesktopmimetype.h"
#include <ksycoca.h>
#include <ksycocadict.h>
#include <kstringhandler.h>

KMimeTypeFactory::KMimeTypeFactory()
    : KSycocaFactory( KST_KMimeTypeFactory )
{
    _self = this;
    m_fastPatternOffset = 0;
    m_otherPatternOffset = 0;
    if (m_str)
    {
        // Read Header
        qint32 i;
        (*m_str) >> i;
        m_fastPatternOffset = i;
        (*m_str) >> i;
        m_otherPatternOffset = i;
    }
}

KMimeTypeFactory::~KMimeTypeFactory()
{
    _self = 0;
}

KMimeTypeFactory * KMimeTypeFactory::self()
{
    if (!_self)
        _self = new KMimeTypeFactory();
    return _self;
}

KMimeType::Ptr KMimeTypeFactory::findMimeTypeByName(const QString &_name)
{
    if (!m_sycocaDict) return KMimeType::Ptr(); // Error!
    assert (!KSycoca::self()->isBuilding());
    int offset = m_sycocaDict->find_string( _name );
    if (!offset) return KMimeType::Ptr(); // Not found
    KMimeType::Ptr newMimeType(createEntry(offset));

    // Check whether the dictionary was right.
    if (newMimeType && (newMimeType->name() != _name))
    {
        // No it wasn't...
        newMimeType = 0; // Not found
    }
    return newMimeType;
}

bool KMimeTypeFactory::checkMimeTypes()
{
   QDataStream *str = KSycoca::self()->findFactory( factoryId() );
   if (!str) return false;

   // check if there are mimetypes
   return (m_beginEntryOffset != m_endEntryOffset);
}

KMimeType * KMimeTypeFactory::createEntry(int offset)
{
   KMimeType *newEntry = 0;
   KSycocaType type;
   QDataStream *str = KSycoca::self()->findEntry(offset, type);
   if (!str) return 0;

   switch(type)
   {
     case KST_KMimeType:
        newEntry = new KMimeType(*str, offset);
        break;
     case KST_KFolderType:
        newEntry = new KFolderType(*str, offset);
        break;
     case KST_KDEDesktopMimeType:
        newEntry = new KDEDesktopMimeType(*str, offset);
        break;

     default:
        kError(7011) << QString("KMimeTypeFactory: unexpected object entry in KSycoca database (type = %1)").arg((int)type) << endl;
        break;
   }
   if (newEntry && !newEntry->isValid())
   {
      kError(7011) << "KMimeTypeFactory: corrupt object in KSycoca database!\n" << endl;
      delete newEntry;
      newEntry = 0;
   }
   return newEntry;
}

KMimeType::Ptr KMimeTypeFactory::findFromPattern( const QString &_filename, QString *match )
{
    // Assume we're NOT building a database
    if (!m_str) return KMimeType::Ptr();

    // Get stream to the header
    QDataStream *str = m_str;

    str->device()->seek( m_fastPatternOffset );

    qint32 nrOfEntries;
    (*str) >> nrOfEntries;
    qint32 entrySize;
    (*str) >> entrySize;

    qint32 fastOffset =  str->device()->pos();

    qint32 matchingOffset = 0;

    // Let's go for a binary search in the "fast" pattern index
    qint32 left = 0;
    qint32 right = nrOfEntries - 1;
    qint32 middle;
    // Extract extension
    const int lastDot = _filename.lastIndexOf('.');
    const int ext_len = _filename.length() - lastDot - 1;
    if (lastDot != -1 && ext_len <= 4) // if no '.', skip the extension lookup
    {
        const QString extension = _filename.right( ext_len ).leftJustified(4);

        QString pattern;
        while (left <= right) {
            middle = (left + right) / 2;
            // read pattern at position "middle"
            str->device()->seek( middle * entrySize + fastOffset );
            KSycocaEntry::read(*str, pattern);
            int cmp = pattern.compare( extension );
            if (cmp < 0)
                left = middle + 1;
            else if (cmp == 0) // found
            {
                (*str) >> matchingOffset;
                // don't return newMimeType - there may be an "other" pattern that
                // matches best this file, like *.tar.bz
                if (match)
                    *match = "*."+pattern;
                break; // but get out of the fast patterns
            }
            else
                right = middle - 1;
        }
    }

    // Now try the "other" Pattern table
    if ( m_patterns.isEmpty() ) {
        str->device()->seek( m_otherPatternOffset );

        QString pattern;
        qint32 mimetypeOffset;

        while (true)
        {
            KSycocaEntry::read(*str, pattern);
            if (pattern.isEmpty()) // end of list
                break;
            (*str) >> mimetypeOffset;
            m_patterns.push_back( pattern );
            m_pattern_offsets.push_back( mimetypeOffset );
        }
    }

    assert( m_patterns.size() == m_pattern_offsets.size() );

    QStringList::const_iterator it = m_patterns.begin();
    const QStringList::const_iterator end = m_patterns.end();
    QList<qint32>::const_iterator it_offset = m_pattern_offsets.begin();

    for ( ; it != end; ++it, ++it_offset )
    {
        if ( KStringHandler::matchFileName( _filename, *it ) )
        {
            if ( !matchingOffset || !(*it).endsWith( "*" ) ) // *.html wins over Makefile.*
            {
                matchingOffset = *it_offset;
                if (match)
                    *match = *it;
                break;
            }
        }
    }

    if ( matchingOffset ) {
        KMimeType *newMimeType = createEntry( matchingOffset );
        assert (newMimeType && newMimeType->isType( KST_KMimeType ));
        return KMimeType::Ptr( newMimeType );
    }
    else
        return KMimeType::Ptr();
}

KMimeType::List KMimeTypeFactory::allMimeTypes()
{
    KMimeType::List result;
    const KSycocaEntry::List list = allEntries();
    for( KSycocaEntry::List::ConstIterator it = list.begin();
         it != list.end();
         ++it)
    {
        Q_ASSERT( (*it)->isType( KST_KMimeType ) );
        result.append( KMimeType::Ptr::staticCast( *it ) );
    }
    return result;
}

KMimeTypeFactory *KMimeTypeFactory::_self = 0;

void KMimeTypeFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }
