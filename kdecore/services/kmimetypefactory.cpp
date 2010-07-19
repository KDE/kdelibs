/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2006-2009 David Faure <faure@kde.org>
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
#include <kstandarddirs.h>
#include <QFile>

extern int servicesDebugArea();

K_GLOBAL_STATIC(KSycocaFactorySingleton<KMimeTypeFactory>, kMimeTypeFactoryInstance)

KMimeTypeFactory::KMimeTypeFactory()
    : KSycocaFactory( KST_KMimeTypeFactory ),
      m_fastPatternOffset(0),
      m_highWeightPatternOffset(0),
      m_lowWeightPatternOffset(0),
      m_highWeightPatternsLoaded(false),
      m_lowWeightPatternsLoaded(false),
      m_parentsMapLoaded(false),
      m_magicFilesParsed(false),
      m_aliasFilesParsed(false)
{
    kMimeTypeFactoryInstance->instanceCreated(this);
    if (!KSycoca::self()->isBuilding()) {
        QDataStream* str = stream();
        Q_ASSERT(str);
        // Read Header
        qint32 i;
        (*str) >> i;
        m_fastPatternOffset = i;
        (*str) >> i;
        // that's the old m_otherPatternOffset, kept for compat but unused

        // alias map - old and unused now
        // KDE5: remove this block
        qint32 n;
        (*str) >> n;
        QString str1, str2;
        for(;n;n--) {
            KSycocaEntry::read(*str, str1);
            KSycocaEntry::read(*str, str2);
            // ignore str1 and str2
        }

        (*str) >> i;
        m_highWeightPatternOffset = i;
        (*str) >> i;
        m_lowWeightPatternOffset = i;
        (*str) >> i;
        //m_parentsMapOffset = i;

        const int saveOffset = str->device()->pos();
        // Init index tables
        m_fastPatternDict = new KSycocaDict(str, m_fastPatternOffset);
        str->device()->seek(saveOffset);
    } else {
        //m_parentsMapLoaded = true;
    }
}

KMimeTypeFactory::~KMimeTypeFactory()
{
    if (kMimeTypeFactoryInstance.exists())
        kMimeTypeFactoryInstance->instanceDestroyed(this);
    delete m_fastPatternDict;
}

KMimeTypeFactory * KMimeTypeFactory::self()
{
    return kMimeTypeFactoryInstance->self();
}

KMimeType::Ptr KMimeTypeFactory::findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options)
{
    if (!sycocaDict()) return KMimeType::Ptr(); // Error!
    assert (!KSycoca::self()->isBuilding());

    QString name = _name;
    if (options & KMimeType::ResolveAliases) {
        if (!m_aliasFilesParsed)
            aliases();
        AliasesMap::const_iterator it = m_aliases.constFind(_name);
        if (it != m_aliases.constEnd())
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
    return aliases().value(mime);
}

QList<KMimeType::Ptr> KMimeTypeFactory::findFromFastPatternDict(const QString &extension)
{
    QList<KMimeType::Ptr> mimeList;
    if (!m_fastPatternDict) return mimeList; // Error!

    // Warning : this assumes we're NOT building a database
    const QList<int> offsetList = m_fastPatternDict->findMultiString(extension);
    if (offsetList.isEmpty()) return mimeList;
    const QString expectedPattern = "*."+extension;
    foreach(int offset, offsetList) {
        KMimeType::Ptr newMimeType(createEntry(offset));
        // Check whether the dictionary was right.
        if (newMimeType && newMimeType->patterns().contains(expectedPattern, Qt::CaseInsensitive) ) {
            mimeList.append(newMimeType);
        }
    }
    return mimeList;
}

bool KMimeTypeFactory::matchFileName( const QString &filename, const QString &pattern )
{
    const int pattern_len = pattern.length();
    if (!pattern_len)
        return false;
    const int len = filename.length();

    const int starCount = pattern.count('*');

    // Patterns like "*~", "*.extension"
    if (pattern[0] == '*'  && pattern.indexOf('[') == -1 && starCount == 1)
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
    if (starCount == 1 && pattern[pattern_len - 1] == '*') {
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
    if (pattern.indexOf('[') == -1 && starCount == 0 && pattern.indexOf('?'))
        return (pattern == filename);

    // Other (quite rare) patterns, like "*.anim[1-9j]": use slow but correct method
    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(filename);
}

void KMimeTypeFactory::findFromOtherPatternList(QList<KMimeType::Ptr>& matchingMimeTypes,
                                                const QString &fileName,
                                                QString& foundExt,
                                                bool highWeight)
{
    OtherPatternList& patternList = highWeight ? m_highWeightPatterns : m_lowWeightPatterns;
    bool& loaded = highWeight ? m_highWeightPatternsLoaded : m_lowWeightPatternsLoaded;
    if ( !loaded ) {
        loaded = true;
        // Load it only once
        QDataStream* str = stream();
        str->device()->seek( highWeight ? m_highWeightPatternOffset : m_lowWeightPatternOffset );

        QString pattern;
        qint32 mimetypeOffset;
        qint32 weight;
        qint32 flags;
        Q_FOREVER {
            KSycocaEntry::read(*str, pattern);
            if (pattern.isEmpty()) // end of list
                break;
            (*str) >> mimetypeOffset;
            (*str) >> weight;
            (*str) >> flags;
            patternList.push_back(OtherPattern(pattern, mimetypeOffset, weight, flags));
        }
    }

    int matchingPatternLength = 0;
    qint32 lastMatchedWeight = 0;
    if (!highWeight && !matchingMimeTypes.isEmpty()) {
        // We found matches in the fast pattern dict already:
        matchingPatternLength = foundExt.length() + 2; // *.foo -> length=5
        lastMatchedWeight = 50;
    }

    // "Applications MUST match globs case-insensitively, except when the case-sensitive
    // attribute is set to true."
    // KMimeFileParser takes care of putting case-insensitive patterns in lowercase.
    const QString lowerCaseFileName = fileName.toLower();

    OtherPatternList::const_iterator it = patternList.constBegin();
    const OtherPatternList::const_iterator end = patternList.constEnd();
    for ( ; it != end; ++it ) {
        const OtherPattern op = *it;
        if ( matchFileName( (op.flags & CaseSensitive) ? fileName : lowerCaseFileName, op.pattern ) ) {
            // Is this a lower-weight pattern than the last match? Stop here then.
            if (op.weight < lastMatchedWeight)
                break;
            if (lastMatchedWeight > 0 && op.weight > lastMatchedWeight) // can't happen
                kWarning(servicesDebugArea()) << "Assumption failed; globs2 weights not sorted correctly"
                               << op.weight << ">" << lastMatchedWeight;
            // Is this a shorter or a longer match than an existing one, or same length?
            if (op.pattern.length() < matchingPatternLength) {
                continue; // too short, ignore
            } else if (op.pattern.length() > matchingPatternLength) {
                // longer: clear any previous match (like *.bz2, when pattern is *.tar.bz2)
                matchingMimeTypes.clear();
                // remember the new "longer" length
                matchingPatternLength = op.pattern.length();
            }
            KMimeType *newMimeType = createEntry( op.offset );
            assert (newMimeType && newMimeType->isType( KST_KMimeType ));
            matchingMimeTypes.push_back( KMimeType::Ptr( newMimeType ) );
            if (op.pattern.startsWith(QLatin1String("*.")))
                foundExt = op.pattern.mid(2);
        }
    }
}

QList<KMimeType::Ptr> KMimeTypeFactory::findFromFileName(const QString &fileName, QString *pMatchingExtension)
{
    // Assume we're NOT building a database
    if (!stream()) return QList<KMimeType::Ptr>();

    // First try the high weight matches (>50), if any.
    QList<KMimeType::Ptr> matchingMimeTypes;
    QString foundExt;
    findFromOtherPatternList(matchingMimeTypes, fileName, foundExt, true);
    if (matchingMimeTypes.isEmpty()) {

        // Now use the "fast patterns" dict, for simple *.foo patterns with weight 50
        // (which is most of them, so this optimization is definitely worth it)
        const int lastDot = fileName.lastIndexOf('.');
        if (lastDot != -1) { // if no '.', skip the extension lookup
            const int ext_len = fileName.length() - lastDot - 1;
            const QString simpleExtension = fileName.right( ext_len ).toLower();
            // (toLower because fast matterns are always case-insensitive and saved as lowercase)

            matchingMimeTypes = findFromFastPatternDict(simpleExtension);
            if (!matchingMimeTypes.isEmpty()) {
                foundExt = simpleExtension;
                // Can't return yet; *.tar.bz2 has to win over *.bz2, so we need the low-weight mimetypes anyway,
                // at least those with weight 50.
            }
        }

        // Finally, try the low weight matches (<=50)
        findFromOtherPatternList(matchingMimeTypes, fileName, foundExt, false);
    }
    if (pMatchingExtension)
        *pMatchingExtension = foundExt;
    return matchingMimeTypes;
}

// TODO: remove unused whichPriority argument
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

    Q_FOREACH ( const KMimeMagicRule& rule, m_magicRules ) {
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

QStringList KMimeTypeFactory::parents(const QString& mime)
{
    if (!m_parentsMapLoaded) {
        m_parentsMapLoaded = true;
        Q_ASSERT(m_parents.isEmpty());

        const QStringList subclassFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "subclasses");
        //kDebug() << subclassFiles;
        Q_FOREACH(const QString& fileName, subclassFiles) {

            QFile qfile( fileName );
            //kDebug(7021) << "Now parsing" << fileName;
            if (qfile.open(QIODevice::ReadOnly)) {
                QTextStream stream(&qfile);
                stream.setCodec("ISO 8859-1");
                while (!stream.atEnd()) {
                    const QString line = stream.readLine();
                    if (line.isEmpty() || line[0] == '#')
                        continue;
                    const int pos = line.indexOf(' ');
                    if (pos == -1) // syntax error
                        continue;
                    const QString derivedTypeName = line.left(pos);
                    KMimeType::Ptr derivedType = findMimeTypeByName(derivedTypeName, KMimeType::ResolveAliases);
                    if (!derivedType)
                        kWarning(7012) << fileName << " refers to unknown mimetype " << derivedTypeName;
                    else {
                        const QString parentTypeName = line.mid(pos+1);
                        Q_ASSERT(!parentTypeName.isEmpty());
                        //derivedType->setParentMimeType(parentTypeName);
                        m_parents[derivedTypeName].append(parentTypeName);
                    }
                }
            }
        }
    }
    return m_parents.value(mime);
}

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
        kDebug(servicesDebugArea()) << "Now parsing " << fileName;
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
        kWarning(servicesDebugArea()) << "Invalid magic file " << fileName << " starts with " << header;
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
                kWarning(servicesDebugArea()) << "Syntax error in " << mimeTypeName
                               << " ':' not present in section name" << endl;
                break;
            }
            priority = line.left(pos).toInt();
            mimeTypeName = line.mid(pos+1);
            mimeTypeName = mimeTypeName.left(mimeTypeName.length()-2); // remove ']\n'
            //kDebug(servicesDebugArea()) << "New rule for " << mimeTypeName
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
                    kWarning(servicesDebugArea()) << "Invalid magic file " << fileName << " '>' not found, got " << ch << " at pos " << file->pos();
                    break;
                }
            }

            KMimeMagicMatch match;
            match.m_rangeStart = 0;
            ch = readNumber(match.m_rangeStart, file);
            if (ch != '=') {
                kWarning(servicesDebugArea()) << "Invalid magic file " << fileName << " '=' not found";
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
                    kDebug(servicesDebugArea()) << "invalid line - garbage found - ch=" << ch;
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

const KMimeTypeFactory::AliasesMap& KMimeTypeFactory::aliases()
{
    if (!m_aliasFilesParsed) {
        m_aliasFilesParsed = true;

        const QStringList aliasFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "aliases");
        Q_FOREACH(const QString& fileName, aliasFiles) {
            QFile qfile(fileName);
            //kDebug(7021) << "Now parsing" << fileName;
            if (qfile.open(QIODevice::ReadOnly)) {
                QTextStream stream(&qfile);
                stream.setCodec("ISO 8859-1");
                while (!stream.atEnd()) {
                    const QString line = stream.readLine();
                    if (line.isEmpty() || line[0] == '#')
                        continue;
                    const int pos = line.indexOf(' ');
                    if (pos == -1) // syntax error
                        continue;
                    const QString aliasTypeName = line.left(pos);
                    const QString parentTypeName = line.mid(pos+1);
                    Q_ASSERT(!aliasTypeName.isEmpty());
                    Q_ASSERT(!parentTypeName.isEmpty());

                    const KMimeType::Ptr realMimeType =
                        findMimeTypeByName(aliasTypeName, KMimeType::DontResolveAlias);
                    if (realMimeType)
                        kDebug(servicesDebugArea()) << "Ignoring alias" << aliasTypeName << "because also defined as a real mimetype";
                    else
                        m_aliases.insert(aliasTypeName, parentTypeName);
                }
            }
        }
    }
    return m_aliases;
}
