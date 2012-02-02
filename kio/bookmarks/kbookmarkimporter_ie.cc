//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2002-2003  Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkimporter_ie.h"

#include <kfiledialog.h>
#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <qtextcodec.h>
#include <QApplication>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>

#include "kbookmarkimporter.h"

/**
 * A class for importing IE bookmarks
 * @deprecated
 */
class  KIEBookmarkImporter : public QObject
{
    Q_OBJECT
public:
    KIEBookmarkImporter( const QString & fileName ) : m_fileName(fileName) {}
    ~KIEBookmarkImporter() {}

    void parseIEBookmarks();

    // Usual place for IE bookmarks
    static QString IEBookmarksDir();

Q_SIGNALS:
    void newBookmark( const QString & text, const QString & url, const QString & additionalInfo );
    void newFolder( const QString & text, bool open, const QString & additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    void parseIEBookmarks_dir( const QString &dirname, const QString &name = QString() );
    void parseIEBookmarks_url_file( const QString &filename, const QString &name );

    QString m_fileName;
};

void KIEBookmarkImporter::parseIEBookmarks_url_file( const QString &filename, const QString &name ) {
    static const int g_lineLimit = 16*1024;

    QFile f(filename);

    if(f.open(QIODevice::ReadOnly)) {

        QByteArray s(g_lineLimit,0);

        while(f.readLine(s.data(), g_lineLimit)>=0) {
            if ( s[s.length()-1] != '\n' ) // Gosh, this line is longer than g_lineLimit. Skipping.
            {
               kWarning() << "IE bookmarks contain a line longer than " << g_lineLimit << ". Skipping.";
               continue;
            }
            QByteArray t = s.trimmed();
            QRegExp rx( "URL=(.*)" );
            if (rx.exactMatch(t)) {
               emit newBookmark( name, rx.cap(1), QString("") );
            }
        }

        f.close();
    }
}

void KIEBookmarkImporter::parseIEBookmarks_dir( const QString &dirname, const QString &foldername )
{

   QDir dir(dirname);
   dir.setFilter( QDir::Files | QDir::Dirs | QDir::AllDirs );
   dir.setSorting( QFlags<QDir::SortFlag>(QDir::Name | QDir::DirsFirst) );
   dir.setNameFilters(QStringList("*.url")); // AK - possibly add ";index.ini" ?

   const QFileInfoList list = dir.entryInfoList();
   if (list.isEmpty()) return;

   if (dirname != m_fileName)
      emit newFolder( foldername, false, "" );

   foreach (const QFileInfo &fi, list) {
      if (fi.fileName() == "." || fi.fileName() == "..") continue;

      if (fi.isDir()) {
         parseIEBookmarks_dir(fi.absoluteFilePath(), fi.fileName());

      } else if (fi.isFile()) {
         if (fi.fileName().endsWith(QLatin1String(".url"))) {
            QString name = fi.fileName();
            name.truncate(name.length() - 4); // .url
            parseIEBookmarks_url_file(fi.absoluteFilePath(), name);
         }
         // AK - add index.ini
      }
   }

   if (dirname != m_fileName)
      emit endFolder();
}


void KIEBookmarkImporter::parseIEBookmarks( )
{
    parseIEBookmarks_dir( m_fileName );
}

QString KIEBookmarkImporter::IEBookmarksDir()
{
   static KIEBookmarkImporterImpl* p = 0;
   if (!p)
       p = new KIEBookmarkImporterImpl;
   return p->findDefaultLocation();
}

void KIEBookmarkImporterImpl::parse() {
   KIEBookmarkImporter importer(m_fileName);
   setupSignalForwards(&importer, this);
   importer.parseIEBookmarks();
}

QString KIEBookmarkImporterImpl::findDefaultLocation(bool) const
{
    // notify user that they must give a new dir such
    // as "Favourites" as otherwise it'll just place
    // lots of .url files in the given dir and gui
    // stuff in the exporter is ugly so that exclues
    // the possibility of just writing to Favourites
    // and checking if overwriting...
    return KFileDialog::getExistingDirectory(KUrl(), QApplication::activeWindow());
}

/////////////////////////////////////////////////

class IEExporter : private KBookmarkGroupTraverser {
public:
    IEExporter( const QString & );
    void write( const KBookmarkGroup &grp ) { traverse(grp); }
private:
    virtual void visit( const KBookmark & );
    virtual void visitEnter( const KBookmarkGroup & );
    virtual void visitLeave( const KBookmarkGroup & );
private:
    QDir m_currentDir;
};

static QString ieStyleQuote( const QString &str ) {
    QString s(str);
    s.replace(QRegExp("[/\\:*?\"<>|]"), "_");
    return s;
}

IEExporter::IEExporter( const QString & dname ) {
    m_currentDir.setPath( dname );
}

void IEExporter::visit( const KBookmark &bk ) {
    QString fname = m_currentDir.path() + '/' + ieStyleQuote( bk.fullText() ) + ".url";
    // kDebug() << "visit(" << bk.text() << "), fname == " << fname;
    QFile file( fname );
    file.open( QIODevice::WriteOnly );
    QTextStream ts( &file );
    ts << "[InternetShortcut]\r\n";
    ts << "URL=" << bk.url().url().toUtf8() << "\r\n";
}

void IEExporter::visitEnter( const KBookmarkGroup &grp ) {
    QString dname = m_currentDir.path() + '/' + ieStyleQuote( grp.fullText() );
    // kDebug() << "visitEnter(" << grp.text() << "), dname == " << dname;
    m_currentDir.mkdir( dname );
    m_currentDir.cd( dname );
}

void IEExporter::visitLeave( const KBookmarkGroup & ) {
    // kDebug() << "visitLeave()";
    m_currentDir.cdUp();
}

void KIEBookmarkExporterImpl::write(const KBookmarkGroup& parent) {
    IEExporter exporter( m_fileName );
    exporter.write( parent );
}

#include "kbookmarkimporter_ie.moc"
