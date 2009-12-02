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
#include "ksycoca.h"
#include "kfoldermimetype.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>
#include <kdesktopfile.h>
#include <QtCore/QHash>
#include <QtCore/QFile>
#include <QtXml/QDomAttr>

KBuildMimeTypeFactory::KBuildMimeTypeFactory() :
    KMimeTypeFactory(), m_parser(this),
    m_oldOtherPatternOffset(0)
{
    m_resourceList = new KSycocaResourceList;
    // We want all xml files under xdgdata-mime - but not packages/*.xml
    m_resourceList->add( "xdgdata-mime", "*.xml" );

    m_fastPatternDict = new KSycocaDict();
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

KMimeType::Ptr KBuildMimeTypeFactory::findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options)
{
    assert (KSycoca::self()->isBuilding());

    QString name = _name;
    if (options & KMimeType::ResolveAliases) {
        AliasesMap::const_iterator it = aliases().constFind(_name);
        if (it != aliases().constEnd())
            name = *it;
    }

    // We're building a database - the mime type must be in memory
    KSycocaEntry::Ptr servType = m_entryDict->value( name );
    return KMimeType::Ptr::staticCast( servType );
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
    // file=text/plain.xml  ->  name=plain.xml dirName=text
    const int pos = file.lastIndexOf('/');
    if (pos == -1) // huh?
        return 0;
    const QString dirName = file.left(pos);
    if (dirName == "packages") // special subdir
        return 0;

    QString name;
    QString userIcon;
    QString comment;
    QString mainPattern;
    QMap<QString, QString> commentsByLanguage;

    const QStringList mimeFiles = KGlobal::dirs()->findAllResources(resource, file);
    if (mimeFiles.isEmpty()) {
        kWarning() << "No file found for" << file << ", even though the file appeared in a directory listing.";
        kWarning() << "Either it was just removed, or the directory doesn't have executable permission...";
        return 0;
    }
    QListIterator<QString> mimeFilesIter(mimeFiles);
    mimeFilesIter.toBack();
    while (mimeFilesIter.hasPrevious()) { // global first, then local.
        const QString fullPath = mimeFilesIter.previous();
        QFile qfile(fullPath);
        if (!qfile.open(QFile::ReadOnly))
            continue;
        QDomDocument doc;
        if (!doc.setContent(&qfile)) {
            kWarning() << "Parse error in " << fullPath;
            continue;
        }
        const QDomElement mimeTypeElement = doc.documentElement();
        if (mimeTypeElement.tagName() != "mime-type")
            continue;
        name = mimeTypeElement.attribute("type");
        if (name.isEmpty())
            continue;

        for ( QDomElement e = mimeTypeElement.firstChildElement();
              !e.isNull();
              e = e.nextSiblingElement() ) {
            const QString tag = e.tagName();
            if (tag == "comment") {
                QString lang = e.attribute("xml:lang");
                if (lang.isEmpty()) {
                    comment = e.text();
                    lang = "en";
                }
                commentsByLanguage.insert(lang, e.text());
            } else if (tag == "icon") { // as written out by shared-mime-info >= 0.40
                userIcon = e.attribute("name");
            } else if (tag == "glob-deleteall") { // as written out by shared-mime-info > 0.60
                mainPattern.clear();
                m_parsedMimeTypes[name] = QString();
            } else if (tag == "glob" && mainPattern.isEmpty()) { // as written out by shared-mime-info > 0.60
                const QString pattern = e.attribute("pattern");
                if (pattern.startsWith('*')) {
                    mainPattern = pattern;
                }
            }
        }
    }
    if (name.isEmpty()) {
        return 0;
    }
    Q_FOREACH(const QString& lang, KGlobal::locale()->languageList()) {
        const QString comm = commentsByLanguage.value(lang);
        if (!comm.isEmpty()) {
            comment = comm;
            break;
        }
        const int pos = lang.indexOf('_');
        if (pos != -1) {
            // "en_US" not found? try just "en"
            const QString shortLang = lang.left(pos);
            const QString comm = commentsByLanguage.value(shortLang);
            if (!comm.isEmpty()) {
                comment = comm;
                break;
            }
        }
    }
    if (comment.isEmpty()) {
        kWarning() << "Missing <comment> field in" << file;
    }

    //kDebug() << "Creating mimetype" << name << "from file" << file << mimeFiles;

    KMimeType* e;
    if ( name == "inode/directory" )
        e = new KFolderMimeType( file, name, comment );
    else
        e = new KMimeType( file, name, comment );

    if (e->isDeleted())
    {
        delete e;
        return 0;
    }

    if ( !(e->isValid()) )
    {
        kWarning(7012) << "Invalid MimeType : " << file;
        delete e;
        return 0;
    }

    if (!userIcon.isEmpty()) {
        e->setUserSpecifiedIcon(userIcon);
    }
    // mainPattern could be empty, but by doing this unconditionally
    // we also remember that we parsed this mimetype.
    m_parsedMimeTypes[name] = mainPattern;

    return e;
}

void KBuildMimeTypeFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);
    // This header is read by KMimeTypeFactory's constructor
    str << (qint32) m_fastPatternOffset;
    str << (qint32) m_oldOtherPatternOffset;
    const AliasesMap& aliasMap = aliases();
    str << (qint32) aliasMap.count();
    for (AliasesMap::const_iterator it = aliasMap.begin(); it != aliasMap.end(); ++it) {
        str << it.key() << it.value();
    }
    str << (qint32) m_highWeightPatternOffset;
    str << (qint32) m_lowWeightPatternOffset;
    str << (qint32) m_parentsMapOffset;
}

void KBuildMimeTypeFactory::parseSubclassFile(const QString& fileName)
{
    ParentsMap& parentsMap = this->parentsMap();
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
                parentsMap[derivedTypeName].append(parentTypeName);
            }
        }
    }
}

void KBuildMimeTypeFactory::parseAliasFile(const QString& fileName)
{
    AliasesMap& aliasMap = aliases();
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
            const QString aliasTypeName = line.left(pos);
            const QString parentTypeName = line.mid(pos+1);
            Q_ASSERT(!aliasTypeName.isEmpty());
            Q_ASSERT(!parentTypeName.isEmpty());
            aliasMap.insert(aliasTypeName, parentTypeName);
        }
    }
}

// Called by kbuildsycoca since it needs the subclasses and aliases for the trader index
void KBuildMimeTypeFactory::parseSubclasses()
{
    // First clear up any old data (loaded by the incremental mode) that we are going to reload anyway
    aliases().clear();

#if 0
    KSycocaEntryDict::Iterator itmime = m_entryDict->begin();
    const KSycocaEntryDict::Iterator endmime = m_entryDict->end();
    for( ; itmime != endmime ; ++itmime ) {
        const KSycocaEntry::Ptr& entry = (*itmime);
        Q_ASSERT( entry->isType( KST_KMimeType ) );
        KMimeType::Ptr mimeType = KMimeType::Ptr::staticCast( entry );
        mimeType->internalClearData();
    }
#endif

    const QStringList subclassFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "subclasses");
    //kDebug() << subclassFiles;
    Q_FOREACH(const QString& file, subclassFiles) {
        parseSubclassFile(file);
    }

    const QStringList aliasFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "aliases");
    //kDebug() << aliasFiles;
    Q_FOREACH(const QString& file, aliasFiles) {
        parseAliasFile(file);
    }
}

void KBuildMimeTypeFactory::save(QDataStream &str)
{
    m_parser.setParsedPatternMap(m_parsedMimeTypes);
    m_parser.parseGlobs();

    KSycocaFactory::save(str);

    savePatternLists(str);

    m_parentsMapOffset = str.device()->pos();
    ParentsMap& parentsMap = this->parentsMap();
    str << (qint32) parentsMap.count();
    for (ParentsMap::const_iterator it = parentsMap.constBegin(); it != parentsMap.constEnd(); ++it) {
        str << it.key() << it.value().join("|");
    }

    int endOfFactoryData = str.device()->pos();

    // Update header (pass #3)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
}

static bool isFastPattern(const QString& pattern)
{
   // starts with "*.", has no other '*' and no other '.'
   return pattern.lastIndexOf('*') == 0
      && pattern.lastIndexOf('.') == 1
      // and contains no other special character
      && !pattern.contains('?')
      && !pattern.contains('[')
      ;
}



void KBuildMimeTypeFactory::savePatternLists(QDataStream &str)
{
    // Store each patterns into either m_fastPatternDict (*.txt, *.html etc. with default weight 50)
    // or for the rest, like core.*, *.tar.bz2, *~, into highWeightPatternOffset (>50)
    // or lowWeightPatternOffset (<=50)

    OtherPatternList highWeightPatternOffset, lowWeightPatternOffset;

    // For each entry in the globs list
    const KMimeFileParser::AllGlobs& allGlobs = m_parser.mimeTypeGlobs();
    Q_FOREACH(const QString& mimeTypeName, m_parser.allMimeTypes()) {
        const KMimeType::Ptr mimeType = findMimeTypeByName(mimeTypeName, KMimeType::DontResolveAlias);
        if (!mimeType) {
            kDebug() << "globs file refers to unknown mimetype" << mimeTypeName;
            continue;
        }
        const KMimeFileParser::GlobList globs = allGlobs.value(mimeTypeName);
        Q_FOREACH(const KMimeFileParser::Glob& glob, globs) {
            const QString &pattern = glob.pattern;
            Q_ASSERT(!pattern.isEmpty());
            if (glob.weight == 50 && isFastPattern(pattern)) {
                // The bulk of the patterns is *.foo with weight 50 --> those go into the fast
                // pattern dict.
                m_fastPatternDict->add(pattern.mid(2) /* extension only*/, KSycocaEntry::Ptr::staticCast(mimeType));
            } else if (glob.weight > 50) {
                highWeightPatternOffset.append(OtherPattern(pattern, mimeType->offset(), glob.weight));
            } else {
                lowWeightPatternOffset.append(OtherPattern(pattern, mimeType->offset(), glob.weight));
            }
        }
    }

    m_fastPatternOffset = str.device()->pos();
    m_fastPatternDict->save(str);

    // The high and low weight pattern lists are already sorted by decreasing
    // weight, this is done by update-mime-database.

    m_highWeightPatternOffset = str.device()->pos();
    Q_FOREACH(const OtherPattern& op, highWeightPatternOffset) {
        str << op.pattern;
        str << (qint32)op.offset;
        str << (qint32)op.weight;
    }
    str << QString(""); // end of list marker (has to be a string !)

    m_lowWeightPatternOffset = str.device()->pos();
    Q_FOREACH(const OtherPattern& op, lowWeightPatternOffset) {
        str << op.pattern;
        str << (qint32)op.offset;
        str << (qint32)op.weight;
    }
    str << QString(""); // end of list marker (has to be a string !)

    // For compat with kde-4.1 kdecore: write the old "other patterns" thing, but empty
    m_oldOtherPatternOffset = str.device()->pos();
    str << QString(""); // end of list marker (has to be a string !)
}
