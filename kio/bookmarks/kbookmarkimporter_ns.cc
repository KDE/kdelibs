/* This file is part of the KDE libraries
   Copyright (C) 1996-1998 Martin R. Jones <mjones@kde.org>
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kbookmarkimporter.h"
#include "kbookmarkexporter.h"
#include "kbookmarkmanager.h"
#include <kfiledialog.h>
#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcharsets.h>
#include <qtextcodec.h>
#include <qstylesheet.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>

void KNSBookmarkImporterImpl::parse()
{
    QFile f(m_fileName);
    QTextCodec * codec = m_utf8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForLocale();
    Q_ASSERT(codec);
    if (!codec)
        return;

    if(f.open(IO_ReadOnly)) {

#define NSBKLINELIMIT 2048
        QCString s(NSBKLINELIMIT);
        // skip header
        while(f.readLine(s.data(), NSBKLINELIMIT) >= 0 && !s.contains("<DL>"));

        while(f.readLine(s.data(), NSBKLINELIMIT)>=0) {
            if ( s[s.length()-1] != '\n' ) // Gosh, this line is longer than NSBKLINELIMIT. Skipping.
            {
               kdWarning() << "Netscape bookmarks contain a line longer than " << NSBKLINELIMIT << ". Skipping." << endl;
               continue;
            }
            QCString t = s.stripWhiteSpace();
            if(t.left(12).upper() == "<DT><A HREF=" ||
               t.left(16).upper() == "<DT><H3><A HREF=") {
              int firstQuotes = t.find('"')+1;
              int secondQuotes = t.find('"', firstQuotes);
              if (firstQuotes != -1 && secondQuotes != -1)
              {
                QCString link = t.mid(firstQuotes, secondQuotes-firstQuotes);
                int endTag = t.find('>', secondQuotes+1);
                QCString name = t.mid(endTag+1);
                name = name.left(name.findRev('<'));
                if ( name.right(4) == "</A>" )
                    name = name.left( name.length() - 4 );
                QString qname = KCharsets::resolveEntities( codec->toUnicode( name ) );
                QCString additionnalInfo = t.mid( secondQuotes+1, endTag-secondQuotes-1 );

                emit newBookmark( KStringHandler::csqueeze(qname),
                                  link, codec->toUnicode(additionnalInfo) );
              }
            }
            else if(t.left(7).upper() == "<DT><H3") {
                int endTag = t.find('>', 7);
                QCString name = t.mid(endTag+1);
                name = name.left(name.findRev('<'));
                QString qname = KCharsets::resolveEntities( codec->toUnicode( name ) );
                QCString additionnalInfo = t.mid( 8, endTag-8 );
                bool folded = (additionnalInfo.left(6) == "FOLDED");
                if (folded) additionnalInfo.remove(0,7);

                emit newFolder( KStringHandler::csqueeze(qname),
                                !folded,
                                codec->toUnicode(additionnalInfo) );
            }
            else if(t.left(4).upper() == "<HR>")
                emit newSeparator();
            else if(t.left(8).upper() == "</DL><P>")
                emit endFolder();
        }

        f.close();
    }
}

QString KNSBookmarkImporterImpl::findDefaultLocation(bool forSaving) const
{
    if (m_utf8) 
    {
       if ( forSaving )
           return KFileDialog::getSaveFileName( QDir::homeDirPath() + "/.mozilla",
                                                i18n("*.html|HTML files (*.html)") );
       else
           return KFileDialog::getOpenFileName( QDir::homeDirPath() + "/.mozilla",
                                                i18n("*.html|HTML files (*.html)") );
    } 
    else 
    {
       return QDir::homeDirPath() + "/.netscape/bookmarks.html";
    }
}

////////////////////////////////////////////////////////////////


void KNSBookmarkImporter::parseNSBookmarks( bool utf8 )
{
    KNSBookmarkImporterImpl importer;
    importer.setFilename(m_fileName);
    importer.setUtf8(utf8);
    importer.setupSignalForwards(&importer, this);
    importer.parse();
}

QString KNSBookmarkImporter::netscapeBookmarksFile( bool forSaving )
{
    static KNSBookmarkImporterImpl importer;
    importer.setUtf8(false);
    return importer.findDefaultLocation(forSaving);
}

QString KNSBookmarkImporter::mozillaBookmarksFile( bool forSaving )
{
    static KNSBookmarkImporterImpl importer;
    importer.setUtf8(true);
    return importer.findDefaultLocation(forSaving);
}


////////////////////////////////////////////////////////////////
//                   compat only
////////////////////////////////////////////////////////////////

void KNSBookmarkExporter::write(bool utf8) {
   KNSBookmarkExporterImpl exporter(m_pManager, m_fileName);
   exporter.write(utf8, m_pManager->root());
}

void KNSBookmarkExporter::writeFolder(QTextStream &/*stream*/, KBookmarkGroup /*gp*/) {
   // TODO - requires a d pointer workaround hack?
}


////////////////////////////////////////////////////////////////


void KNSBookmarkExporterImpl::write(bool utf8, KBookmarkGroup parent) {
   if (QFile::exists(m_fileName)) {
      ::rename(
         QFile::encodeName(m_fileName), 
         QFile::encodeName(m_fileName + ".beforekde"));
   }

   QFile file(m_fileName);

   if (!file.open(IO_WriteOnly)) {
      kdError(7043) << "Can't write to file " << m_fileName << endl;
      return;
   }

   QTextStream fstream(&file);
   fstream.setEncoding(utf8 ? QTextStream::UnicodeUTF8 : QTextStream::Locale);

   QString charset 
      = utf8 ? "UTF-8" : QString::fromLatin1(QTextCodec::codecForLocale()->name()).upper();

   fstream << "<!DOCTYPE NETSCAPE-Bookmark-file-1>" << endl
           << i18n("<!-- This file was generated by Konqueror -->") << endl
           << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=" 
              << charset << "\">" << endl
           << "<TITLE>" << i18n("Bookmarks") << "</TITLE>" << endl
           << "<H1>" << i18n("Bookmarks") << "</H1>" << endl
           << "<DL><p>" << endl
           << folderAsString(parent)
           << "</DL><P>" << endl;
}

const QString KNSBookmarkExporterImpl::folderAsString(KBookmarkGroup parent) {
   QString str;
   QTextStream fstream(&str, IO_WriteOnly);

   for (KBookmark bk = parent.first(); !bk.isNull(); bk = parent.next(bk)) {
      if (bk.isSeparator()) {
         fstream << "<HR>" << endl;
         continue;
      }

      QString text = QStyleSheet::escape(bk.text());

      if (bk.isGroup() ) {
         fstream << "<DT><H3 " 
                    << (!bk.toGroup().isOpen() ? "FOLDED " : "")
                    << bk.internalElement().attribute("netscapeinfo") << ">" 
                 << text << "</H3>" << endl
                 << "<DL><P>" << endl
                 << folderAsString(bk.toGroup())
                 << "</DL><P>" << endl;
         continue;

      } else {
         // note - netscape seems to use local8bit for url...
         fstream << "<DT><A HREF=\"" << bk.url().url() << "\""
                    << bk.internalElement().attribute("netscapeinfo") << ">" 
                 << text << "</A>" << endl;
         continue;
      }
   }

   return str;
}

////

#include "kbookmarkimporter_ns.moc"
