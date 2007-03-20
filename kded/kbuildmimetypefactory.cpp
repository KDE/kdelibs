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
#include "kdedesktopmimetype.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>
#include <kdesktopfile.h>
#include <qhash.h>
#include <QFile>
#include <QDomDocument>
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
        QMap<QString, QString>::const_iterator it = aliases().find(_name);
        if (it != aliases().end())
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
        kWarning() << "Parse error in " << fullPath << endl;
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
    for ( QDomNode n = mimeTypeElement.firstChild();
          !n.isNull();
          n = n.nextSibling() ) {
        QDomElement e = n.toElement();
        if(!e.isNull() && e.tagName() == "comment") {
            const QString lang = e.attribute("xml:lang");
            if (lang.isEmpty())
                comment = e.text();
            else
                commentsByLanguage.insert(lang, e.text());
            break;
        }
    }
    if (comment.isEmpty()) {
        kWarning() << "Missing <comment> field in " << fullPath << endl;
    }
    foreach(const QString& lang, KGlobal::locale()->languageList()) {
        const QString comm = commentsByLanguage.value(lang);
        if (!comm.isEmpty()) {
            comment = comm;
            break;
        }
    }

    //kDebug() << "Creating mimetype " << name << " from file " << file << endl;

    KMimeType* e;
    if ( name == "inode/directory" )
        e = new KFolderType( file, name, comment );
    else if ( name == "application/x-desktop" )
        e = new KDEDesktopMimeType( file, name, comment );
    else
        e = new KMimeType( file, name, comment );

    if (e->isDeleted())
    {
        delete e;
        return 0;
    }

    if ( !(e->isValid()) )
    {
        kWarning(7012) << "Invalid MimeType : " << file << endl;
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
    kDebug() << k_funcinfo << "Now parsing " << fileName << endl;
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
                kWarning(7012) << fileName << " refers to unknown mimetype " << derivedTypeName << endl;
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
    kDebug() << k_funcinfo << "Now parsing " << fileName << endl;
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
    const QStringList subclassFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "subclasses");
    //kDebug() << k_funcinfo << subclassFiles << endl;
    Q_FOREACH(const QString& file, subclassFiles) {
        parseSubclassFile(file);
    }

    const QStringList aliasFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", "aliases");
    //kDebug() << k_funcinfo << aliasFiles << endl;
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
      // and is max 6 chars
      && pattern.length() <= 6
      // and contains no other special character
      && !pattern.contains('?')
      && !pattern.contains('[')
      ;
}

void
KBuildMimeTypeFactory::savePatternLists(QDataStream &str)
{
   // Store each patterns in one of the 2 string lists (for sorting)
   QStringList fastPatterns;  // for *.a to *.abcd
   QStringList otherPatterns; // for the rest (core.*, *.tar.bz2, *~) ...
   QHash<QString, const KMimeType*> dict; // KMimeType::Ptr not needed here, this is short term

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
         if (isFastPattern(pattern))
            fastPatterns.append( pattern );
         else if (!pattern.isEmpty()) // some stupid mimetype files have "Patterns=;"
            otherPatterns.append( pattern );
         // Assumption : there is only one mimetype for that pattern
         // It doesn't really make sense otherwise, anyway.
         dict.insert( pattern, mimeType.constData() );
      }
   }

   // Sort the list - the fast one, useless for the other one
   fastPatterns.sort();

   qint32 entrySize = 0;
   qint32 nrOfEntries = 0;

   m_fastPatternOffset = str.device()->pos();

   // Write out fastPatternHeader (Pass #1)
   str.device()->seek(m_fastPatternOffset);
   str << nrOfEntries;
   str << entrySize;

   // For each fast pattern
   for ( QStringList::ConstIterator it = fastPatterns.begin(); it != fastPatterns.end() ; ++it )
   {
     int start = str.device()->pos();
     // Justify to 6 chars with spaces, so that the size remains constant
     // in the database file. This is useful for doing a binary search in kmimetypefactory.cpp
     // TODO: we could use a hash-table instead, for more performance and no extension-length limit.
     QString paddedPattern = (*it).leftJustified(6).right(4); // remove leading "*."
     //kDebug(7021) << QString("FAST : '%1' '%2'").arg(paddedPattern).arg(dict[(*it)]->name()) << endl;
     str << paddedPattern;
     str << dict[(*it)]->offset();
     entrySize = str.device()->pos() - start;
     nrOfEntries++;
   }

   // store position
   m_otherPatternOffset = str.device()->pos();

   // Write out fastPatternHeader (Pass #2)
   str.device()->seek(m_fastPatternOffset);
   str << nrOfEntries;
   str << entrySize;

   // For the other patterns
   str.device()->seek(m_otherPatternOffset);

   for ( QStringList::ConstIterator it = otherPatterns.begin(); it != otherPatterns.end() ; ++it )
   {
     //kDebug(7021) << QString("OTHER : '%1' '%2'").arg(*it).arg(dict[(*it)]->name()) << endl;
     str << (*it);
     str << dict[(*it)]->offset();
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
