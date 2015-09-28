/*  This file is part of the KDE libraries
 *  Copyright (C) 2006-2007, 2010 David Faure <faure@kde.org>
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

#include "kmimetyperepository_p.h"
#include <kstandarddirs.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include "kmimetype.h"
#include <kdeversion.h> // KDE_MAKE_VERSION
#include <kmessage.h>
#include <klocale.h>
#include "kfoldermimetype.h"
#include <QFile>
#include <QProcess>
#include <QtEndian>

extern int servicesDebugArea();

KMimeTypeRepository * KMimeTypeRepository::self()
{
    K_GLOBAL_STATIC(KMimeTypeRepository, s_self)
    return s_self;
}

KMimeTypeRepository::KMimeTypeRepository()
    : m_parentsMapLoaded(false),
      m_magicFilesParsed(false),
      m_aliasFilesParsed(false),
      m_globsFilesParsed(false),
      m_patternsMapCalculated(false),
      m_mimeTypesChecked(false),
      m_useFavIcons(true),
      m_useFavIconsChecked(false),
      m_sharedMimeInfoVersion(0),
      m_mutex(QReadWriteLock::Recursive)
{
}

KMimeTypeRepository::~KMimeTypeRepository()
{
}

KMimeType::Ptr KMimeTypeRepository::findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options)
{
    QString name = _name;
    if (options & KMimeType::ResolveAliases) {
        name = canonicalName(name);
    }

    const QString filename = name + QLatin1String(".xml");

    if (KStandardDirs::locate("xdgdata-mime", filename).isEmpty()) {
        return KMimeType::Ptr(); // Not found
    }

    if (name == QLatin1String("inode/directory"))
        return KMimeType::Ptr(new KFolderMimeType(filename, name, QString() /*comment*/));
    else
        return KMimeType::Ptr(new KMimeType(filename, name, QString() /*comment*/));
}

bool KMimeTypeRepository::checkMimeTypes()
{
    // check if there are mimetypes
    const QStringList globFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", QLatin1String("globs"));
    return !globFiles.isEmpty();
}

QString KMimeTypeRepository::resolveAlias(const QString& mime)
{
    return aliases().value(mime);
}

QString KMimeTypeRepository::canonicalName(const QString& mime)
{
    QString c = resolveAlias(mime);
    if (c.isEmpty())
        return mime;
    return c;
}

bool KMimeTypeRepository::matchFileName( const QString &filename, const QString &pattern )
{
    const int pattern_len = pattern.length();
    if (!pattern_len)
        return false;
    const int len = filename.length();

    const int starCount = pattern.count(QLatin1Char('*'));

    // Patterns like "*~", "*.extension"
    if (pattern[0] == QLatin1Char('*')  && pattern.indexOf(QLatin1Char('[')) == -1 && starCount == 1)
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
    if (starCount == 1 && pattern[pattern_len - 1] == QLatin1Char('*')) {
        if ( len + 1 < pattern_len ) return false;
        if (pattern[0] == QLatin1Char('*'))
            return filename.indexOf(pattern.mid(1, pattern_len - 2)) != -1;

        const QChar *c1 = pattern.unicode();
        const QChar *c2 = filename.unicode();
        int cnt = 1;
        while (cnt < pattern_len && *c1++ == *c2++)
           ++cnt;
        return cnt == pattern_len;
    }

    // Names without any wildcards like "README"
    if (pattern.indexOf(QLatin1Char('[')) == -1 && starCount == 0 && pattern.indexOf(QLatin1Char('?')))
        return (pattern == filename);

    // Other (quite rare) patterns, like "*.anim[1-9j]": use slow but correct method
    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(filename);
}

// Helper for findFromFileName
void KMimeTypeRepository::findFromOtherPatternList(QStringList& matchingMimeTypes,
                                                   const QString &fileName,
                                                   QString& foundExt,
                                                   bool highWeight)
{
    KMimeGlobsFileParser::GlobList& patternList = highWeight ? m_globs.m_highWeightGlobs : m_globs.m_lowWeightGlobs;

    int matchingPatternLength = 0;
    qint32 lastMatchedWeight = 0;
    if (!highWeight && !matchingMimeTypes.isEmpty()) {
        // We found matches in the fast pattern dict already:
        matchingPatternLength = foundExt.length() + 2; // *.foo -> length=5
        lastMatchedWeight = 50;
    }

    // "Applications MUST match globs case-insensitively, except when the case-sensitive
    // attribute is set to true."
    // KMimeGlobsFileParser takes care of putting case-insensitive patterns in lowercase.
    const QString lowerCaseFileName = fileName.toLower();

    KMimeGlobsFileParser::GlobList::const_iterator it = patternList.constBegin();
    const KMimeGlobsFileParser::GlobList::const_iterator end = patternList.constEnd();
    for ( ; it != end; ++it ) {
        const KMimeGlobsFileParser::Glob& glob = *it;
        if ( matchFileName( (glob.flags & CaseSensitive) ? fileName : lowerCaseFileName, glob.pattern ) ) {
            // Is this a lower-weight pattern than the last match? Stop here then.
            if (glob.weight < lastMatchedWeight)
                break;
            if (lastMatchedWeight > 0 && glob.weight > lastMatchedWeight) // can't happen
                kWarning(servicesDebugArea()) << "Assumption failed; globs2 weights not sorted correctly"
                               << glob.weight << ">" << lastMatchedWeight;
            // Is this a shorter or a longer match than an existing one, or same length?
            if (glob.pattern.length() < matchingPatternLength) {
                continue; // too short, ignore
            } else if (glob.pattern.length() > matchingPatternLength) {
                // longer: clear any previous match (like *.bz2, when pattern is *.tar.bz2)
                matchingMimeTypes.clear();
                // remember the new "longer" length
                matchingPatternLength = glob.pattern.length();
            }
            matchingMimeTypes.push_back(glob.mimeType);
            if (glob.pattern.startsWith(QLatin1String("*.")))
                foundExt = glob.pattern.mid(2);
        }
    }
}

QStringList KMimeTypeRepository::findFromFileName(const QString &fileName, QString *pMatchingExtension)
{
    m_mutex.lockForWrite();
    parseGlobs();
    m_mutex.unlock();

    QReadLocker lock(&m_mutex);
    // First try the high weight matches (>50), if any.
    QStringList matchingMimeTypes;
    QString foundExt;
    findFromOtherPatternList(matchingMimeTypes, fileName, foundExt, true);
    if (matchingMimeTypes.isEmpty()) {

        // Now use the "fast patterns" dict, for simple *.foo patterns with weight 50
        // (which is most of them, so this optimization is definitely worth it)
        const int lastDot = fileName.lastIndexOf(QLatin1Char('.'));
        if (lastDot != -1) { // if no '.', skip the extension lookup
            const int ext_len = fileName.length() - lastDot - 1;
            const QString simpleExtension = fileName.right( ext_len ).toLower();
            // (toLower because fast matterns are always case-insensitive and saved as lowercase)

            matchingMimeTypes = m_globs.m_fastPatterns.value(simpleExtension);
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

KMimeType::Ptr KMimeTypeRepository::findFromContent(QIODevice* device, int* accuracy, QByteArray& beginning)
{
    Q_ASSERT(device->isOpen());
    const qint64 deviceSize = device->size();
    if (deviceSize == 0) {
        if (accuracy)
            *accuracy = 100;
        return findMimeTypeByName(QLatin1String("application/x-zerosize"));
    }
    if (beginning.isEmpty()) {
        // check if we can really read the data; also provide enough data for most rules
        const qint64 dataNeeded = qMin(deviceSize, (qint64) 16384);
        beginning.resize(dataNeeded);
        if (!device->seek(0) || device->read(beginning.data(), dataNeeded) == -1) {
            return defaultMimeTypePtr(); // don't bother detecting unreadable file
        }
    }

    m_mutex.lockForWrite();
    if (!m_magicFilesParsed) {
        parseMagic();
        m_magicFilesParsed = true;
    }
    m_mutex.unlock();

    // Apply magic rules
    {
        QReadLocker lock(&m_mutex);
        Q_FOREACH ( const KMimeMagicRule& rule, m_magicRules ) {
            if (rule.match(device, deviceSize, beginning)) {
                if (accuracy)
                    *accuracy = rule.priority();
                return findMimeTypeByName(rule.mimetype());
            }
        }
    }

    // Do fallback code so that we never return 0
    // Nothing worked, check if the file contents looks like binary or text
    if (!KMimeType::isBufferBinaryData(beginning)) {
        if (accuracy)
            *accuracy = 5;
        return findMimeTypeByName(QLatin1String("text/plain"));
    }
    if (accuracy)
        *accuracy = 0;
    return defaultMimeTypePtr();
}

static QString fallbackParent(const QString& mimeTypeName)
{
    const QString myGroup = mimeTypeName.left(mimeTypeName.indexOf(QLatin1Char('/')));
    // All text/* types are subclasses of text/plain.
    if (myGroup == QLatin1String("text") && mimeTypeName != QLatin1String("text/plain"))
        return QLatin1String("text/plain");
    // All real-file mimetypes implicitly derive from application/octet-stream
    if (myGroup != QLatin1String("inode") &&
        // kde extensions
        myGroup != QLatin1String("all") && myGroup != QLatin1String("fonts") && myGroup != QLatin1String("print") && myGroup != QLatin1String("uri")
        && mimeTypeName != QLatin1String("application/octet-stream")) {
        return QLatin1String("application/octet-stream");
    }
    return QString();
}

QStringList KMimeTypeRepository::parents(const QString& mime)
{
    QWriteLocker lock(&m_mutex);
    if (!m_parentsMapLoaded) {
        m_parentsMapLoaded = true;
        Q_ASSERT(m_parents.isEmpty());

        const QStringList subclassFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", QLatin1String("subclasses"));
        //kDebug() << subclassFiles;
        Q_FOREACH(const QString& fileName, subclassFiles) {

            QFile qfile( fileName );
            //kDebug(7021) << "Now parsing" << fileName;
            if (qfile.open(QIODevice::ReadOnly)) {
                QTextStream stream(&qfile);
                stream.setCodec("ISO 8859-1");
                while (!stream.atEnd()) {
                    const QString line = stream.readLine();
                    if (line.isEmpty() || line[0] == QLatin1Char('#'))
                        continue;
                    const int pos = line.indexOf(QLatin1Char(' '));
                    if (pos == -1) // syntax error
                        continue;
                    const QString derivedTypeName = line.left(pos);
                    KMimeType::Ptr derivedType = findMimeTypeByName(derivedTypeName, KMimeType::ResolveAliases);
                    if (!derivedType)
                        kDebug() << fileName << " refers to unknown mimetype " << derivedTypeName;
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
    QStringList parents = m_parents.value(mime);

    if (parents.isEmpty()) {
        const QString myParent = fallbackParent(mime);
        if (!myParent.isEmpty())
            parents.append(myParent);
    }

    return parents;
}

#include <arpa/inet.h> // for ntohs
#include <kstandarddirs.h>
#include <QFile>

// Sort them in descending order of priority
static bool mimeMagicRuleCompare(const KMimeMagicRule& lhs, const KMimeMagicRule& rhs) {
    return lhs.priority() > rhs.priority();
}

// Caller must hold m_mutex
void KMimeTypeRepository::parseMagic()
{
    const QStringList magicFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", QLatin1String("magic"));
    //kDebug() << magicFiles;
    QListIterator<QString> magicIter( magicFiles );
    magicIter.toBack();
    while (magicIter.hasPrevious()) { // global first, then local. Turns out it doesn't matter though.
        const QString fileName = magicIter.previous();
        QFile magicFile(fileName);
        //kDebug(servicesDebugArea()) << "Now parsing " << fileName;
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

QList<KMimeMagicRule> KMimeTypeRepository::parseMagicFile(QIODevice* file, const QString& fileName) const
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
            const QString line = QString::fromLatin1(file->readLine());
            const int pos = line.indexOf(QLatin1Char(':'));
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

            qint16 lengthBuffer;
            if (file->read(reinterpret_cast<char*>(&lengthBuffer), 2) != 2)
                break;
            const qint16 valueLength = qFromBigEndian(lengthBuffer);
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

const KMimeTypeRepository::AliasesMap& KMimeTypeRepository::aliases()
{
    QWriteLocker lock(&m_mutex);
    if (!m_aliasFilesParsed) {
        m_aliasFilesParsed = true;

        const QStringList aliasFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", QLatin1String("aliases"));
        Q_FOREACH(const QString& fileName, aliasFiles) {
            QFile qfile(fileName);
            //kDebug(7021) << "Now parsing" << fileName;
            if (qfile.open(QIODevice::ReadOnly)) {
                QTextStream stream(&qfile);
                stream.setCodec("ISO 8859-1");
                while (!stream.atEnd()) {
                    const QString line = stream.readLine();
                    if (line.isEmpty() || line[0] == QLatin1Char('#'))
                        continue;
                    const int pos = line.indexOf(QLatin1Char(' '));
                    if (pos == -1) // syntax error
                        continue;
                    const QString aliasTypeName = line.left(pos);
                    const QString parentTypeName = line.mid(pos+1);
                    Q_ASSERT(!aliasTypeName.isEmpty());
                    Q_ASSERT(!parentTypeName.isEmpty());

                    const KMimeType::Ptr realMimeType =
                        findMimeTypeByName(aliasTypeName, KMimeType::DontResolveAlias);
                    if (realMimeType) {
                        //kDebug(servicesDebugArea()) << "Ignoring alias" << aliasTypeName << "because also defined as a real mimetype";
                    } else {
                        m_aliases.insert(aliasTypeName, parentTypeName);
                    }
                }
            }
        }
    }
    return m_aliases;
}

// Caller must lock m_mutex for write
void KMimeTypeRepository::parseGlobs()
{
    if (!m_globsFilesParsed) {
        m_globsFilesParsed = true;
        KMimeGlobsFileParser parser;
        m_globs = parser.parseGlobs();
    }
}

QStringList KMimeTypeRepository::patternsForMimetype(const QString& mimeType)
{
    QWriteLocker lock(&m_mutex);
    if (!m_patternsMapCalculated) {
        m_patternsMapCalculated = true;
        parseGlobs();
        m_patterns = m_globs.patternsMap();
    }
    return m_patterns.value(mimeType);
}

static void errorMissingMimeTypes( const QStringList& _types )
{
    KMessage::message( KMessage::Error, i18np( "Could not find mime type <resource>%2</resource>",
                "Could not find mime types:\n<resource>%2</resource>", _types.count(), _types.join(QLatin1String("</resource>\n<resource>")) ) );
}

void KMimeTypeRepository::checkEssentialMimeTypes()
{
    QWriteLocker lock(&m_mutex);
    if (m_mimeTypesChecked) // already done
        return;

    m_mimeTypesChecked = true; // must be done before building mimetypes

    // No Mime-Types installed ?
    // Lets do some rescue here.
    if (!checkMimeTypes()) {
        // Note that this messagebox is queued, so it will only be shown once getting back to the event loop

        // No mimetypes installed? Are you setting XDG_DATA_DIRS without including /usr/share in it?
        KMessage::message(KMessage::Error, i18n("No mime types installed. "
            "Check that shared-mime-info is installed, and that XDG_DATA_DIRS is not set, or includes /usr/share."));
        return; // no point in going any further
    }

    QStringList missingMimeTypes;

    if (!KMimeType::mimeType(QLatin1String("inode/directory")))
        missingMimeTypes.append(QLatin1String("inode/directory"));
#ifndef Q_OS_WIN
    //if (!KMimeType::mimeType(QLatin1String("inode/directory-locked")))
    //  missingMimeTypes.append(QLatin1String("inode/directory-locked"));
    if (!KMimeType::mimeType(QLatin1String("inode/blockdevice")))
        missingMimeTypes.append(QLatin1String("inode/blockdevice"));
    if (!KMimeType::mimeType(QLatin1String("inode/chardevice")))
        missingMimeTypes.append(QLatin1String("inode/chardevice"));
    if (!KMimeType::mimeType(QLatin1String("inode/socket")))
        missingMimeTypes.append(QLatin1String("inode/socket"));
    if (!KMimeType::mimeType(QLatin1String("inode/fifo")))
        missingMimeTypes.append(QLatin1String("inode/fifo"));
#endif
    if (!KMimeType::mimeType(QLatin1String("application/x-shellscript")))
        missingMimeTypes.append(QLatin1String("application/x-shellscript"));
    if (!KMimeType::mimeType(QLatin1String("application/x-executable")))
        missingMimeTypes.append(QLatin1String("application/x-executable"));
    if (!KMimeType::mimeType(QLatin1String("application/x-desktop")))
        missingMimeTypes.append(QLatin1String("application/x-desktop"));

    if (!missingMimeTypes.isEmpty())
        errorMissingMimeTypes(missingMimeTypes);
}

KMimeType::Ptr KMimeTypeRepository::defaultMimeTypePtr()
{
    QWriteLocker lock(&m_mutex);
    if (!m_defaultMimeType) {
        // Try to find the default type
        KMimeType::Ptr mime = findMimeTypeByName(KMimeType::defaultMimeType());
        if (mime) {
            m_defaultMimeType = mime;
        } else {
            const QString defaultMimeType = KMimeType::defaultMimeType();
            errorMissingMimeTypes(QStringList(defaultMimeType));
            const QString pathDefaultMimeType = KGlobal::dirs()->locateLocal("xdgdata-mime", defaultMimeType+QLatin1String(".xml"));
            m_defaultMimeType = new KMimeType(pathDefaultMimeType, defaultMimeType, QLatin1String("mime"));
        }
    }
    return m_defaultMimeType;

}

bool KMimeTypeRepository::useFavIcons()
{
    // this method will be called quite often, so better not read the config
    // again and again.
    m_mutex.lockForWrite();
    if (!m_useFavIconsChecked) {
        m_useFavIconsChecked = true;
        KConfigGroup cg( KGlobal::config(), "HTML Settings" );
        m_useFavIcons = cg.readEntry("EnableFavicon", true);
    }
    m_mutex.unlock();
    return m_useFavIcons;
}

static void addPlatformSpecificPkgConfigPath(QStringList& paths)
{
#if defined (Q_OS_FREEBSD)
    paths << QLatin1String("/usr/local/libdata/pkgconfig"); // FreeBSD
#elif defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD) || defined(Q_OS_SOLARIS)
    paths << QLatin1String("/usr/local/lib/pkgconfig"); // {Net,Open}BSD/OpenSolaris
#elif defined (Q_OS_UNIX)
    paths << QLatin1String("/usr/share/pkgconfig"); // Linux and all other unix
#endif
}

static int mimeDataBaseVersion()
{
    // shared-mime-info installs a "version" file since 0.91
    const QStringList versionFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", QLatin1String("version"));
    if (!versionFiles.isEmpty()) {
        QFile file(versionFiles.first()); // Look at the global file, not at a possibly old local one
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray line = file.readLine().simplified();
            QRegExp versionRe(QString::fromLatin1("(\\d+)\\.(\\d+)(\\.(\\d+))?"));
            if (versionRe.indexIn(QString::fromLocal8Bit(line)) > -1) {
                return KDE_MAKE_VERSION(versionRe.cap(1).toInt(), versionRe.cap(2).toInt(), versionRe.cap(4).toInt());
            }
        }
    }

    // TODO: Remove the #idef'ed code below once the issue is fixed
    // in QProcess or when we require s-m-i >= 0.91
#ifdef Q_OS_UNIX
    // Try to read the version number from the shared-mime-info.pc file
    QStringList paths;
    const QByteArray pkgConfigPath = qgetenv("PKG_CONFIG_PATH");
    if (!pkgConfigPath.isEmpty()) {
        paths << QFile::decodeName(pkgConfigPath).split(QLatin1Char(':'), QString::SkipEmptyParts);
    }

    // Add platform specific hard-coded default paths to the list...
    addPlatformSpecificPkgConfigPath(paths);

    Q_FOREACH(const QString& path, paths) {
        const QString fileName = path + QLatin1String("/shared-mime-info.pc");
        if (!QFile::exists(fileName)) {
            continue;
        }

        QFile file (fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            break;
        }

        while (!file.atEnd()) {
            const QByteArray line = file.readLine().simplified();
            if (!line.startsWith("Version")) { // krazy:exclude=strings
                continue;
            }
            QRegExp versionRe(QString::fromLatin1("Version: (\\d+)\\.(\\d+)(\\.(\\d+))?"));
            if (versionRe.indexIn(QString::fromLocal8Bit(line)) > -1) {
                return KDE_MAKE_VERSION(versionRe.cap(1).toInt(), versionRe.cap(2).toInt(), versionRe.cap(4).toInt());
            }
        }
    }
#endif

    // Execute "update-mime-database -v" to determine version number.
    // NOTE: On *nix, the code below is known to cause freezes/hangs in apps
    // that block signals. See https://bugs.kde.org/show_bug.cgi?id=260719.
    const QString umd = KStandardDirs::findExe(QString::fromLatin1("update-mime-database"));
    if (umd.isEmpty()) {
        kWarning(servicesDebugArea()) << "update-mime-database not found!";
        return -1;
    }

    QProcess smi;
    smi.start(umd, QStringList() << QString::fromLatin1("-v"));
    if (smi.waitForStarted() && smi.waitForFinished()) {
        const QString out = QString::fromLocal8Bit(smi.readAllStandardError());
        QRegExp versionRe(QString::fromLatin1("update-mime-database \\(shared-mime-info\\) (\\d+)\\.(\\d+)(\\.(\\d+))?"));
        if (versionRe.indexIn(out) > -1) {
            return KDE_MAKE_VERSION(versionRe.cap(1).toInt(), versionRe.cap(2).toInt(), versionRe.cap(4).toInt());
        }
        kWarning(servicesDebugArea()) << "Unexpected version scheme from update-mime-database -v: got" << out;
    } else {
        kWarning(servicesDebugArea()) << "Error running update-mime-database -v";
    }

    return -1;
}

int KMimeTypeRepository::sharedMimeInfoVersion()
{
    m_mutex.lockForWrite();
    if (m_sharedMimeInfoVersion == 0)
        m_sharedMimeInfoVersion = mimeDataBaseVersion();
    m_mutex.unlock();
    return m_sharedMimeInfoVersion;
}
