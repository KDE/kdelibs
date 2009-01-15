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
#include "kmimefileparser.h"

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

KMimeType::Ptr KBuildMimeTypeFactory::findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options)
{
    assert (KSycoca::self()->isBuilding());

    QString name = _name;
    if (options & KMimeType::ResolveAliases) {
        QMap<QString, QString>::const_iterator it = aliases().constFind(_name);
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
    //pos = dirName.lastIndexOf('/');
    //dirName = dirName.mid(pos+1);
    if (dirName == "packages") // special subdir
        return 0;

    const QString fullPath = KGlobal::dirs()->locate( resource, file );
    if (fullPath.isEmpty()) // can't happen
        return 0;
    QFile qfile(fullPath);
    if (!qfile.open(QFile::ReadOnly))
        return 0;
    QDomDocument doc;
    if (!doc.setContent(&qfile)) {
        kWarning() << "Parse error in " << fullPath;
        return 0;
    }
    const QDomElement mimeTypeElement = doc.documentElement();
    if (mimeTypeElement.tagName() != "mime-type")
        return 0;
    const QString name = mimeTypeElement.attribute("type");
    if (name.isEmpty())
        return 0;

    QString comment;
    QMap<QString, QString> commentsByLanguage;
    for ( QDomElement e = mimeTypeElement.firstChildElement();
          !e.isNull();
          e = e.nextSiblingElement() ) {
        if(e.tagName() == "comment") {
            const QString lang = e.attribute("xml:lang");
            if (lang.isEmpty())
                comment = e.text();
            else
                commentsByLanguage.insert(lang, e.text());
        } // TODO handle "icon" and "generic-icon"
    }
    if (comment.isEmpty()) {
        kWarning() << "Missing <comment> field in " << fullPath;
    }
    foreach(const QString& lang, KGlobal::locale()->languageList()) {
        const QString comm = commentsByLanguage.value(lang);
        if (!comm.isEmpty()) {
            comment = comm;
            break;
        }
    }

    //kDebug() << "Creating mimetype" << name << "from file" << file << "path" << fullPath;

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

    return e;
}

void KBuildMimeTypeFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);
    // This header is read by KMimeTypeFactory's constructor
    str << (qint32) m_fastPatternOffset;
    str << (qint32) m_otherPatternOffset;
    const QMap<QString, QString>& aliasMap = aliases();
    str << (qint32) aliasMap.count();
    for (QMap<QString, QString>::const_iterator it = aliasMap.begin(); it != aliasMap.end(); ++it) {
        str << it.key() << it.value();
    }
}

void KBuildMimeTypeFactory::parseSubclassFile(const QString& fileName)
{
    QFile qfile( fileName );
    kDebug(7021) << "Now parsing" << fileName;
    if (qfile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&qfile);
        stream.setCodec("UTF-8");
        while (!stream.atEnd()) {
            const QString line = stream.readLine();
            if (line.isEmpty() || line[0] == '#')
                continue;
            const int pos = line.indexOf(' ');
            if (pos == -1) // syntax error
                continue;
            const QString derivedTypeName = line.left(pos);
            KMimeType::Ptr derivedType = findMimeTypeByName(derivedTypeName);
            if (!derivedType)
                kWarning(7012) << fileName << " refers to unknown mimetype " << derivedTypeName;
            else {
                const QString parentTypeName = line.mid(pos+1);
                Q_ASSERT(!parentTypeName.isEmpty());
                derivedType->setParentMimeType(parentTypeName);
            }
        }
    }
}

void KBuildMimeTypeFactory::parseAliasFile(const QString& fileName)
{
    QFile qfile( fileName );
    kDebug(7021) << "Now parsing" << fileName;
    if (qfile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&qfile);
        stream.setCodec("UTF-8");
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
            aliases().insert(aliasTypeName, parentTypeName);
        }
    }
}

// Called by kbuildsycoca since it needs the subclasses and aliases for the trader index
void KBuildMimeTypeFactory::parseSubclasses()
{
    // First clear up any old data (loaded by the incremental mode) that we are going to reload anyway
    aliases().clear();

    KSycocaEntryDict::Iterator itmime = m_entryDict->begin();
    const KSycocaEntryDict::Iterator endmime = m_entryDict->end();
    for( ; itmime != endmime ; ++itmime ) {
        const KSycocaEntry::Ptr& entry = (*itmime);
        Q_ASSERT( entry->isType( KST_KMimeType ) );
        KMimeType::Ptr mimeType = KMimeType::Ptr::staticCast( entry );
        mimeType->internalClearData();
    }


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
    KMimeFileParser parser(this);
    parser.parseGlobs();

    KSycocaFactory::save(str);

    savePatternLists(str);

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

void
KBuildMimeTypeFactory::savePatternLists(QDataStream &str)
{
    // Store each patterns into either m_fastPatternDict (*.txt, *.html etc.)
    // or otherPatterns (for the rest, like core.*, *.tar.bz2, *~)

    // KMimeType::Ptr not needed here, this is short term
    typedef QMultiMap<QString, const KMimeType*> PatternMap;
    PatternMap otherPatterns;

    // For each mimetype in mimetypeFactory
    for(KSycocaEntryDict::Iterator it = m_entryDict->begin();
        it != m_entryDict->end();
        ++it)
    {
        const KSycocaEntry::Ptr& entry = (*it);
        Q_ASSERT( entry->isType( KST_KMimeType ) );

        const KMimeType::Ptr mimeType = KMimeType::Ptr::staticCast( entry );
        const QStringList pat = mimeType->patterns();
        QStringList::ConstIterator patit = pat.begin();
        for ( ; patit != pat.end() ; ++patit )
        {
            const QString &pattern = *patit;
            Q_ASSERT(!pattern.isEmpty());
            if (isFastPattern(pattern))
                m_fastPatternDict->add(pattern.mid(2) /* extension only*/, entry);
            else
                otherPatterns.insert(pattern, mimeType.constData());
        }
    }

    m_fastPatternOffset = str.device()->pos();
    m_fastPatternDict->save(str);

    // For the other patterns
    m_otherPatternOffset = str.device()->pos();
    str.device()->seek(m_otherPatternOffset);

    for ( PatternMap::ConstIterator it = otherPatterns.begin(); it != otherPatterns.end() ; ++it )
    {
        //kDebug(7021) << "OTHER:" << it.key() << it.value()->name();
        str << it.key();
        str << it.value()->offset();
    }

    str << QString(""); // end of list marker (has to be a string !)
}

void
KBuildMimeTypeFactory::addEntry(const KSycocaEntry::Ptr& newEntry)
{
   KMimeType::Ptr mimeType = KMimeType::Ptr::staticCast( newEntry );
   if ( m_entryDict->value( newEntry->name() ) )
   {
     // Already exists -> replace
     KSycocaFactory::removeEntry(newEntry->name());
   }
   KSycocaFactory::addEntry(newEntry);
}
