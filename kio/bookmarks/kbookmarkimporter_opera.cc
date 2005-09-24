//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kfiledialog.h>
#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <qtextcodec.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>

#include "kbookmarkimporter.h"
#include "kbookmarkimporter_opera.h"

void KOperaBookmarkImporter::parseOperaBookmarks( )
{
   QFile file(m_fileName);
   if(!file.open(QIODevice::ReadOnly)) {
      return;
   }

   QTextCodec * codec = QTextCodec::codecForName("UTF-8");
   Q_ASSERT(codec);
   if (!codec)
      return;

   int lineno = 0;
   QString url, name, type;
   static const int g_lineLimit = 16*1024;
   QByteArray line(g_lineLimit);

   while ( file.readLine(line.data(), g_lineLimit) >=0 ) {
      lineno++;

      // skip lines that didn't fit in buffer and first two headers lines
      if ( line[line.length()-1] != '\n' || lineno <= 2 )
          continue;

      QString currentLine = codec->toUnicode(line).trimmed();

      if (currentLine.isEmpty()) {
         // end of data block
         if (type.isNull())
            continue;
         else if ( type == "URL")
            emit newBookmark( name, url, "" );
         else if (type == "FOLDER" )
            emit newFolder( name, false, "" );

         type = QString::null;
         name = QString::null;
         url = QString::null;

      } else if (currentLine == "-") {
         // end of folder
         emit endFolder();

      } else {
         // data block line
         QString tag;
         if ( tag = "#", currentLine.startsWith( tag ) )
            type = currentLine.remove( 0, tag.length() );
         else if ( tag = "NAME=", currentLine.startsWith( tag ) )
            name = currentLine.remove(0, tag.length());
         else if ( tag = "URL=", currentLine.startsWith( tag ) )
            url = currentLine.remove(0, tag.length());
      }
   }

}

QString KOperaBookmarkImporter::operaBookmarksFile()
{
   static KOperaBookmarkImporterImpl *p = 0;
   if (!p)
       p = new KOperaBookmarkImporterImpl;
   return p->findDefaultLocation();
}

void KOperaBookmarkImporterImpl::parse() {
   KOperaBookmarkImporter importer(m_fileName);
   setupSignalForwards(&importer, this);
   importer.parseOperaBookmarks();
}

QString KOperaBookmarkImporterImpl::findDefaultLocation(bool saving) const
{
   return saving ? KFileDialog::getSaveFileName(
                       QDir::homeDirPath() + "/.opera",
                       i18n("*.adr|Opera Bookmark Files (*.adr)") )
                 : KFileDialog::getOpenFileName(
                       QDir::homeDirPath() + "/.opera",
                       i18n("*.adr|Opera Bookmark Files (*.adr)") );
}

/////////////////////////////////////////////////

class OperaExporter : private KBookmarkGroupTraverser {
public:
    OperaExporter();
    QString generate( const KBookmarkGroup &grp ) { traverse(grp); return m_string; };
private:
    virtual void visit( const KBookmark & );
    virtual void visitEnter( const KBookmarkGroup & );
    virtual void visitLeave( const KBookmarkGroup & );
private:
    QString m_string;
    QTextStream m_out;
};

OperaExporter::OperaExporter() : m_out(&m_string, QIODevice::WriteOnly) {
    m_out << "Opera Hotlist version 2.0" << endl;
    m_out << "Options: encoding = utf8, version=3" << endl;
}

void OperaExporter::visit( const KBookmark &bk ) {
    // kdDebug() << "visit(" << bk.text() << ")" << endl;
    m_out << "#URL" << endl;
    m_out << "\tNAME=" << bk.fullText() << endl;
    m_out << "\tURL=" << bk.url().url().toUtf8() << endl;
    m_out << endl;
}

void OperaExporter::visitEnter( const KBookmarkGroup &grp ) {
    // kdDebug() << "visitEnter(" << grp.text() << ")" << endl;
    m_out << "#FOLDER" << endl;
    m_out << "\tNAME="<< grp.fullText() << endl;
    m_out << endl;
}

void OperaExporter::visitLeave( const KBookmarkGroup & ) {
    // kdDebug() << "visitLeave()" << endl;
    m_out << "-" << endl;
    m_out << endl;
}

void KOperaBookmarkExporterImpl::write(KBookmarkGroup parent) {
    OperaExporter exporter;
    QString content = exporter.generate( parent );
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly)) {
       kdError(7043) << "Can't write to file " << m_fileName << endl;
       return;
    }
    QTextStream fstream(&file);
    fstream.setEncoding(QTextStream::UnicodeUTF8);
    fstream << content;
}

#include "kbookmarkimporter_opera.moc"
