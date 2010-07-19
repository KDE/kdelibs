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
#include <QXmlStreamReader>

KBuildMimeTypeFactory::KBuildMimeTypeFactory() :
    KMimeTypeFactory(), m_parser(this)
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

        QXmlStreamReader xml(&qfile);
        if (xml.readNextStartElement()) {
            if (xml.name() != "mime-type") {
                continue;
            }
            name = xml.attributes().value("type").toString();
            if (name.isEmpty())
                continue;

            while (xml.readNextStartElement()) {
                const QStringRef tag = xml.name();
                if (tag == "comment") {
                    QString lang = xml.attributes().value("xml:lang").toString();
                    const QString text = xml.readElementText();
                    if (lang.isEmpty()) {
                        comment = text;
                        lang = "en";
                    }
                    commentsByLanguage.insert(lang, text);
                    continue; // we called readElementText, so we're at the EndElement already.
                } else if (tag == "icon") { // as written out by shared-mime-info >= 0.40
                    userIcon = xml.attributes().value("name").toString();
                } else if (tag == "glob-deleteall") { // as written out by shared-mime-info > 0.60
                    mainPattern.clear();
                    m_parsedMimeTypes[name] = QString();
                } else if (tag == "glob" && mainPattern.isEmpty()) { // as written out by shared-mime-info > 0.60
                    const QString pattern = xml.attributes().value("pattern").toString();
                    if (pattern.startsWith('*')) {
                        mainPattern = pattern;
                    }
                }
                xml.skipCurrentElement();
            }
            if (xml.name() != "mime-type") {
                kFatal() << "Programming error in KBuildMimeTypeFactory::createEntry, please create a bug report on http://bugs.kde.org and attach the file" << fullPath;
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
    str << (qint32) 0; // old "other pattern offset"
    str << (qint32) 0;
    str << (qint32) m_highWeightPatternOffset;
    str << (qint32) m_lowWeightPatternOffset;
    str << (qint32) 0;
}

void KBuildMimeTypeFactory::save(QDataStream &str)
{
    m_parser.setParsedPatternMap(m_parsedMimeTypes);
    m_parser.parseGlobs();

    KSycocaFactory::save(str);

    savePatternLists(str);

    str << (qint32) 0;

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
            if (glob.weight == 50 && isFastPattern(pattern) && ((glob.flags & CaseSensitive) == 0)) {
                // The bulk of the patterns is *.foo with weight 50 --> those go into the fast
                // pattern dict.
                m_fastPatternDict->add(pattern.mid(2).toLower() /* extension only*/, KSycocaEntry::Ptr::staticCast(mimeType));
            } else {
                const QString adjustedPattern = (glob.flags & CaseSensitive) ? pattern : pattern.toLower();
                if (glob.weight > 50) {
                    highWeightPatternOffset.append(OtherPattern(adjustedPattern, mimeType->offset(), glob.weight, glob.flags));
                } else {
                    lowWeightPatternOffset.append(OtherPattern(adjustedPattern, mimeType->offset(), glob.weight, glob.flags));
                }
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
        str << (qint32)op.flags;
    }
    str << QString(""); // end of list marker (has to be a string !)

    m_lowWeightPatternOffset = str.device()->pos();
    Q_FOREACH(const OtherPattern& op, lowWeightPatternOffset) {
        str << op.pattern;
        str << (qint32)op.offset;
        str << (qint32)op.weight;
        str << (qint32)op.flags;
    }
    str << QString(""); // end of list marker (has to be a string !)
}
