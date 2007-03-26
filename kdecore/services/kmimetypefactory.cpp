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
#include <kshell.h>

////

KMimeTypeFactory::KMimeTypeFactory()
    : KSycocaFactory( KST_KMimeTypeFactory ),
      m_magicFilesParsed(false)
{
    _self = this;
    m_fastPatternOffset = 0;
    m_otherPatternOffset = 0;
    if (m_str) {
        // Read Header
        qint32 i;
        (*m_str) >> i;
        m_fastPatternOffset = i;
        (*m_str) >> i;
        m_otherPatternOffset = i;

        // alias map
        qint32 n;
        (*m_str) >> n;
        QString str1, str2;
        for(;n;n--) {
            KSycocaEntry::read(*m_str, str1);
            KSycocaEntry::read(*m_str, str2);
            m_aliases.insert(str1, str2);
        }
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

KMimeType::Ptr KMimeTypeFactory::findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options)
{
    if (!sycocaDict()) return KMimeType::Ptr(); // Error!
    assert (!KSycoca::self()->isBuilding());

    QString name = _name;
    if (options & KMimeType::ResolveAliases) {
        QMap<QString, QString>::const_iterator it = m_aliases.find(_name);
        if (it != m_aliases.end())
            name = *it;
    }

    int offset = sycocaDict()->find_string( name );
    if (!offset) return KMimeType::Ptr(); // Not found
    KMimeType::Ptr newMimeType(createEntry(offset));

    // Check whether the dictionary was right.
    if (newMimeType && (newMimeType->name() != name))
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
   return !isEmpty();
}

KMimeType * KMimeTypeFactory::createEntry(int offset) const
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


QString KMimeTypeFactory::resolveAlias(const QString& mime)
{
    return m_aliases.value(mime);
}

KMimeType::Ptr KMimeTypeFactory::findFromPattern( const QString &filename, QString *match )
{
    // Assume we're NOT building a database
    if (!m_str) return KMimeType::Ptr();

    // "Applications MUST first try a case-sensitive match, then try again with
    // the filename converted to lower-case if that fails. This is so that
    // main.C will be seen as a C++ file, but IMAGE.GIF will still use the
    // *.gif pattern."
    KMimeType::Ptr mime = findFromPatternHelper(filename, match);
    if (!mime) {
        const QString lowerCase = filename.toLower();
        if (lowerCase != filename)
            mime = findFromPatternHelper(lowerCase, match);
    }
    return mime;
}

KMimeType::Ptr KMimeTypeFactory::findFromPatternHelper( const QString &_filename, QString *match )
{
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
    // TODO: we could use a hash-table instead, for more performance and no extension-length limit.
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
        if ( KShell::matchFileName( _filename, *it ) )
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

KMimeType::Ptr KMimeTypeFactory::findFromContent(QIODevice* device, WhichPriority whichPriority, int* accuracy, const QByteArray& beginning)
{
    if (!device->isOpen()) {
        if (!device->open(QIODevice::ReadOnly))
            return KMimeType::Ptr();
    }
    if (device->size() == 0)
        return findMimeTypeByName("application/x-zerosize");

    if (!m_magicFilesParsed) {
        parseMagic();
        m_magicFilesParsed = true;
    }

    for ( QList<KMimeMagicRule>::const_iterator it = m_magicRules.begin(), end = m_magicRules.end();
          it != end; ++it ) {
        const KMimeMagicRule& rule = *it;
        // HighPriorityRules: select rules with priority >= 80
        // LowPriorityRules: select rules with priority < 80
        if ( ( whichPriority == AllRules ) ||
             ( (rule.priority() >= 80) == (whichPriority == HighPriorityRules) ) ) {
            if (rule.match(device, beginning)) {
                if (accuracy)
                    *accuracy = rule.priority();
                return findMimeTypeByName(rule.mimetype());
            }
        }
    }

    // Do fallback code so that we never return 0 - unless we were only looking for HighPriorityRules
    if (whichPriority != HighPriorityRules) {
        // Nothing worked, check if the file contents looks like binary or text
        if (!KMimeType::isBufferBinaryData(beginning)) {
            if (accuracy)
                *accuracy = 5;
            return findMimeTypeByName("text/plain");
        }
        if (accuracy)
            *accuracy = 0;
        return KMimeType::defaultMimeTypePtr();
    }

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

QMap<QString, QString>& KMimeTypeFactory::aliases()
{
    return m_aliases;
}

KMimeTypeFactory *KMimeTypeFactory::_self = 0;

void KMimeTypeFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }


#include <arpa/inet.h> // for ntohs
#include <kstandarddirs.h>
#include <QFile>

void KMimeTypeFactory::parseMagic()
{
    const QStringList magicFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "magic");
    //kDebug() << k_funcinfo << magicFiles << endl;
    QListIterator<QString> magicIter( magicFiles );
    magicIter.toBack();
    while (magicIter.hasPrevious()) { // global first, then local. Turns out it doesn't matter though.
        const QString fileName = magicIter.previous();
        QFile magicFile(fileName);
        kDebug() << k_funcinfo << "Now parsing " << fileName << endl;
        if (magicFile.open(QIODevice::ReadOnly))
            m_magicRules += parseMagicFile(&magicFile, fileName);
    }
}

static char readNumber(qint64& value, QIODevice* file)
{
    char ch;
    while (file->getChar(&ch)) {
        if (ch < '0' || ch > '9')
            return ch;
        value = 10 * value + ch - '0';
    }
    // eof
    return '\0';
}

#define MAKE_LITTLE_ENDIAN16(val) val = (quint16)(((quint16)(val) << 8)|((quint16)(val) >> 8))

#define MAKE_LITTLE_ENDIAN32(val) \
   val = (((quint32)(val) & 0xFF000000U) >> 24) | \
         (((quint32)(val) & 0x00FF0000U) >> 8) | \
         (((quint32)(val) & 0x0000FF00U) << 8) | \
         (((quint32)(val) & 0x000000FFU) << 24)

QList<KMimeMagicRule> KMimeTypeFactory::parseMagicFile(QIODevice* file, const QString& fileName) const
{
    QList<KMimeMagicRule> rules;
    QByteArray header = file->read(12);
    if (header != "MIME-Magic\0\n") {
        kWarning(7009) << "Invalid magic file " << fileName << " starts with " << header << endl;
        return rules;
    }
    QList<KMimeMagicMatch> matches; // toplevel matches (indent==0)
    int priority = 0; // to avoid warning
    QString mimeTypeName;

    Q_FOREVER {
        char ch = '\0';
        bool chOk = file->getChar(&ch);

        if (!chOk || ch == '[') {
            // Finish previous section
            if (!mimeTypeName.isEmpty()) {
                rules.append(KMimeMagicRule(mimeTypeName, priority, matches));
                matches.clear();
                mimeTypeName.clear();
            }
            if (file->atEnd())
                break; // done

            // Parse new section
            const QString line = file->readLine();
            const int pos = line.indexOf(':');
            if (pos == -1) { // syntax error
                kWarning(7009) << "Syntax error in " << mimeTypeName
                               << " ':' not present in section name" << endl;
                break;
            }
            priority = line.left(pos).toInt();
            mimeTypeName = line.mid(pos+1);
            mimeTypeName = mimeTypeName.left(mimeTypeName.length()-2); // remove ']\n'
            //kDebug(7009) << "New rule for " << mimeTypeName
            //             << " with priority " << priority << endl;
        } else {
            // Parse line in the section
            // [ indent ] ">" start-offset "=" value
            //   [ "&" mask ] [ "~" word-size ] [ "+" range-length ] "\n"
            qint64 indent = 0;
            if (ch != '>') {
                indent = ch - '0';
                ch = readNumber(indent, file);
                if (ch != '>') {
                    kWarning(7009) << "Invalid magic file " << fileName << " '>' not found, got " << ch << " at pos " << file->pos() << endl;
                    break;
                }
            }

            KMimeMagicMatch match;
            match.m_rangeStart = 0;
            ch = readNumber(match.m_rangeStart, file);
            if (ch != '=') {
                kWarning(7009) << "Invalid magic file " << fileName << " '=' not found" << endl;
                break;
            }

            char lengthBuffer[2];
            if (file->read(lengthBuffer, 2) != 2)
                break;
            const short valueLength = ntohs(*(short*)lengthBuffer);
            //kDebug() << "indent=" << indent << " rangeStart=" << match.m_rangeStart
            //         << " valueLength=" << valueLength << endl;

            match.m_data.resize(valueLength);
            if (file->read(match.m_data.data(), valueLength) != valueLength)
                break;

            match.m_rangeLength = 1;
            bool invalidLine = false;

            if (!file->getChar(&ch))
                break;
            qint64 wordSize = 1;

            Q_FOREVER {
                // We get 'ch' before coming here, or as part of the parsing in each case below.
                switch (ch) {
                case '\n':
                    break;
                case '&':
                    match.m_mask.resize(valueLength);
                    if (file->read(match.m_mask.data(), valueLength) != valueLength)
                        invalidLine = true;
                    if (!file->getChar(&ch))
                        invalidLine = true;
                    break;
                case '~': {
                    wordSize = 0;
                    ch = readNumber(wordSize, file);
                    //kDebug() << "wordSize=" << wordSize << endl;
                    break;
                }
                case '+':
                    // Parse range length
                    match.m_rangeLength = 0;
                    ch = readNumber(match.m_rangeLength, file);
                    if (ch == '\n')
                        break;
                    // fall-through intended
                default:
                    // "If an unknown character is found where a newline is expected
                    // then the whole line should be ignored (there will be no binary
                    // data after the new character, so the next line starts after the
                    // next "\n" character). This is for future extensions.", says spec
                    while (ch != '\n' && !file->atEnd()) {
                        file->getChar(&ch);
                    }
                    invalidLine = true;
                    kDebug(7009) << "invalid line - garbage found - ch=" << ch << endl;
                    break;
                }
                if (ch == '\n' || invalidLine)
                    break;
            }
            if (!invalidLine) {
                // Finish match, doing byte-swapping on little endian hosts
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
                if (wordSize > 1) {
                    //kDebug() << "data before swapping: " << match.m_data << endl;;
                    if ((wordSize != 2 && wordSize != 4) || (valueLength % wordSize != 0))
                        continue; // invalid word size
                    char* data = match.m_data.data();
                    char* mask = match.m_mask.data();
                    for (int i = 0; i < valueLength; i += wordSize) {
                        if (wordSize == 2)
                            MAKE_LITTLE_ENDIAN16( *((quint16 *) data + i) );
                        else if (wordSize == 4)
                            MAKE_LITTLE_ENDIAN32( *((quint32 *) data + i) );
                        if (!match.m_mask.isEmpty()) {
                            if (wordSize == 2)
                                MAKE_LITTLE_ENDIAN16( *((quint16 *) mask + i) );
                            else if (wordSize == 4)
                                MAKE_LITTLE_ENDIAN32( *((quint32 *) mask + i) );
                        }
                    }
                    //kDebug() << "data after swapping: " << match.m_data << endl;
                }
#endif
                // Append match at the right place depending on indent:
                if (indent == 0) {
                    matches.append(match);
                } else {
                    KMimeMagicMatch* m = &matches.last();
                    Q_ASSERT(m);
                    for (int i = 1 /* nothing to do for indent==1 */; i < indent; ++i) {
                        m = &m->m_subMatches.last();
                        Q_ASSERT(m);
                    }
                    m->m_subMatches.append(match);
                }
            }
        }
    }
    return rules;
}
