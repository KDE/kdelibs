/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2006 David Faure <faure@kde.org>
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
#include "kfoldermimetype.h"
#include <ksycoca.h>
#include <ksycocadict.h>
#include <kshell.h>
#include <kdebug.h>

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

        const int saveOffset = m_str->device()->pos();
        // Init index tables
        m_fastPatternDict = new KSycocaDict(m_str, m_fastPatternOffset);
        m_str->device()->seek(saveOffset);

    } else {
        // Build new database
        m_fastPatternDict = new KSycocaDict();
    }
}

KMimeTypeFactory::~KMimeTypeFactory()
{
    _self = 0;
    delete m_fastPatternDict;
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
     case KST_KDEDesktopMimeType: // old, compat only
        newEntry = new KMimeType(*str, offset);
        break;
     case KST_KFolderMimeType:
        newEntry = new KFolderMimeType(*str, offset);
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

QList<KMimeType::Ptr> KMimeTypeFactory::findFromFileName( const QString &filename, QString *matchingExtension )
{
    // Assume we're NOT building a database
    if (!m_str) return QList<KMimeType::Ptr>();

    // "Applications MUST first try a case-sensitive match, then try again with
    // the filename converted to lower-case if that fails. This is so that
    // main.C will be seen as a C++ file, but IMAGE.GIF will still use the
    // *.gif pattern."
    QList<KMimeType::Ptr> mimeList = findFromFileNameHelper(filename, matchingExtension);
    if (mimeList.isEmpty()) {
        const QString lowerCase = filename.toLower();
        if (lowerCase != filename)
            mimeList = findFromFileNameHelper(lowerCase, matchingExtension);
    }
    return mimeList;
}

QList<KMimeType::Ptr> KMimeTypeFactory::findFromFastPatternDict(const QString &extension)
{
    QList<KMimeType::Ptr> mimeList;
    if (!m_fastPatternDict) return mimeList; // Error!

    // Warning : this assumes we're NOT building a database

    const QList<int> offsetList = m_fastPatternDict->findMultiString(extension);
    foreach(int offset, offsetList) {
        KMimeType::Ptr newMimeType(createEntry(offset));
        // Check whether the dictionary was right.
        if (newMimeType && newMimeType->patterns().contains("*."+extension)) {
            mimeList.append(newMimeType);
        }
    }
    return mimeList;
}

static bool matchFileName( const QString &filename, const QString &pattern )
{
    int pattern_len = pattern.length();
    if (!pattern_len)
        return false;
    int len = filename.length();

    // Patterns like "*~", "*.extension"
    if (pattern[0] == '*'  && pattern.indexOf('[') == -1)
    {
        if ( len + 1 < pattern_len ) return false;

        const QChar *c1 = pattern.unicode() + pattern_len - 1;
        const QChar *c2 = filename.unicode() + len - 1;
        int cnt = 1;
        while (cnt < pattern_len && *c1-- == *c2--)
            ++cnt;
        return cnt == pattern_len;
    }

    // Patterns like "README*" (well this is currently the only one like that...)
    if (pattern[pattern_len - 1] == '*') {
        if ( len + 1 < pattern_len ) return false;
        if (pattern[0] == '*')
            return filename.indexOf(pattern.mid(1, pattern_len - 2)) != -1;

        const QChar *c1 = pattern.unicode();
        const QChar *c2 = filename.unicode();
        int cnt = 1;
        while (cnt < pattern_len && *c1++ == *c2++)
           ++cnt;
        return cnt == pattern_len;
    }

    // Names without any wildcards like "README"
    if (pattern.indexOf('[') == -1 && pattern.indexOf('*') == -1 && pattern.indexOf('?'))
        return (pattern == filename);

    // Other patterns, like "[Mm]akefile": use slow but correct method
    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(filename);
}

QList<KMimeType::Ptr> KMimeTypeFactory::findFromFileNameHelper( const QString &_filename, QString *matchingExtension )
{
    QList<KMimeType::Ptr> matchingMimeTypes;

    // Get stream to the header
    QDataStream *str = m_str;

    // Extract extension
    const int lastDot = _filename.lastIndexOf('.');
    int matchingPatternLength = 0;

    if (lastDot != -1) { // if no '.', skip the extension lookup
        const int ext_len = _filename.length() - lastDot - 1;
        const QString simpleExtension = _filename.right( ext_len );

        matchingMimeTypes = findFromFastPatternDict(simpleExtension);
        if (!matchingMimeTypes.isEmpty()) {
            matchingPatternLength = simpleExtension.length() + 2; // *.foo -> length=5
            if (matchingExtension)
                *matchingExtension = simpleExtension;
            // Keep going, there might be some matches from the 'other' list, like *.tar.bz2
        }
    }

    // Now try the "other" Pattern table
    if ( m_otherPatterns.isEmpty() ) {
        // Load it only once
        str->device()->seek( m_otherPatternOffset );

        QString pattern;
        qint32 mimetypeOffset;

        while (true)
        {
            KSycocaEntry::read(*str, pattern);
            if (pattern.isEmpty()) // end of list
                break;
            (*str) >> mimetypeOffset;
            m_otherPatterns.push_back( pattern );
            m_otherPatterns_offsets.push_back( mimetypeOffset );
        }
    }

    assert( m_otherPatterns.size() == m_otherPatterns_offsets.size() );

    QStringList::const_iterator it = m_otherPatterns.begin();
    const QStringList::const_iterator end = m_otherPatterns.end();
    QList<qint32>::const_iterator it_offset = m_otherPatterns_offsets.begin();

    for ( ; it != end; ++it, ++it_offset ) {
        const QString pattern = *it;
        if ( matchFileName( _filename, pattern ) ) {
            // Is this a shorter or a longer match than an existing one, or same length?
            if (pattern.length() < matchingPatternLength) {
                continue; // too short, ignore
            } else if (pattern.length() > matchingPatternLength) {
                // longer: clear any previous match (like *.bz2, when pattern is *.tar.bz2)
                matchingMimeTypes.clear();
                // remember the new "longer" length
                matchingPatternLength = pattern.length();
            }
            KMimeType *newMimeType = createEntry( *it_offset );
            assert (newMimeType && newMimeType->isType( KST_KMimeType ));
            matchingMimeTypes.append( KMimeType::Ptr( newMimeType ) );
            if (matchingExtension && pattern.startsWith("*."))
                *matchingExtension = pattern.mid(2);
        }
    }

    return matchingMimeTypes;
}

// TODO: remove unused whichPriority argument, once XDG shared-mime is updated
KMimeType::Ptr KMimeTypeFactory::findFromContent(QIODevice* device, WhichPriority whichPriority, int* accuracy, QByteArray& beginning)
{
    Q_ASSERT(device->isOpen());
    if (device->size() == 0) {
        if (accuracy)
            *accuracy = 100;
        return findMimeTypeByName("application/x-zerosize");
    }

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
        // Rules are sorted by decreasing priority, so we can abort when we're past high-prio rules
        if (whichPriority == HighPriorityRules && rule.priority() < 80)
            break;
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

// Sort them in descending order of priority
static bool mimeMagicRuleCompare(const KMimeMagicRule& lhs, const KMimeMagicRule& rhs) {
    return lhs.priority() > rhs.priority();
}


void KMimeTypeFactory::parseMagic()
{
    const QStringList magicFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "magic");
    //kDebug() << magicFiles;
    QListIterator<QString> magicIter( magicFiles );
    magicIter.toBack();
    while (magicIter.hasPrevious()) { // global first, then local. Turns out it doesn't matter though.
        const QString fileName = magicIter.previous();
        QFile magicFile(fileName);
        kDebug(7009) << "Now parsing " << fileName;
        if (magicFile.open(QIODevice::ReadOnly))
            m_magicRules += parseMagicFile(&magicFile, fileName);
    }
    qSort(m_magicRules.begin(), m_magicRules.end(), mimeMagicRuleCompare);
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
    if (header != QByteArray::fromRawData("MIME-Magic\0\n", 12)) {
        kWarning(7009) << "Invalid magic file " << fileName << " starts with " << header;
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
                    kWarning(7009) << "Invalid magic file " << fileName << " '>' not found, got " << ch << " at pos " << file->pos();
                    break;
                }
            }

            KMimeMagicMatch match;
            match.m_rangeStart = 0;
            ch = readNumber(match.m_rangeStart, file);
            if (ch != '=') {
                kWarning(7009) << "Invalid magic file " << fileName << " '=' not found";
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
                    //kDebug() << "wordSize=" << wordSize;
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
                    kDebug(7009) << "invalid line - garbage found - ch=" << ch;
                    break;
                }
                if (ch == '\n' || invalidLine)
                    break;
            }
            if (!invalidLine) {
                // Finish match, doing byte-swapping on little endian hosts
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
                if (wordSize > 1) {
                    //kDebug() << "data before swapping: " << match.m_data;;
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
                    //kDebug() << "data after swapping: " << match.m_data;
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
