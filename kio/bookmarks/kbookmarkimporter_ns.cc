//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
#include <qtextdocument.h> // Qt::escape

void KNSBookmarkImporterImpl::parse()
{
    QFile f(m_fileName);
    QTextCodec * codec = m_utf8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForLocale();
    Q_ASSERT(codec);
    if (!codec)
        return;

    if(f.open(QIODevice::ReadOnly)) {

        static const int g_lineLimit = 16*1024;
        QByteArray s(g_lineLimit);
        // skip header
        while(f.readLine(s.data(), g_lineLimit) >= 0 && !s.contains("<DL>"));

        while(f.readLine(s.data(), g_lineLimit)>=0) {
            if ( s[s.length()-1] != '\n' ) // Gosh, this line is longer than g_lineLimit. Skipping.
            {
               kWarning() << "Netscape bookmarks contain a line longer than " << g_lineLimit << ". Skipping." << endl;
               continue;
            }
            QByteArray t = s.trimmed();
            if(t.left(12).toUpper() == "<DT><A HREF=" ||
               t.left(16).toUpper() == "<DT><H3><A HREF=") {
              int firstQuotes = t.find('"')+1;
              int secondQuotes = t.find('"', firstQuotes);
              if (firstQuotes != -1 && secondQuotes != -1)
              {
                QByteArray link = t.mid(firstQuotes, secondQuotes-firstQuotes);
                int endTag = t.find('>', secondQuotes+1);
                QByteArray name = t.mid(endTag+1);
                name = name.left(name.lastIndexOf('<'));
                if ( name.endsWith("</A>" ) )
                    name = name.left( name.length() - 4 );
                QString qname = KCharsets::resolveEntities( codec->toUnicode( name ) );
                QByteArray additionalInfo = t.mid( secondQuotes+1, endTag-secondQuotes-1 );

                emit newBookmark( qname,
                                  codec->toUnicode(link),
                                  codec->toUnicode(additionalInfo) );
              }
            }
            else if(t.left(7).toUpper() == "<DT><H3") {
                int endTag = t.find('>', 7);
                QByteArray name = t.mid(endTag+1);
                name = name.left(name.lastIndexOf('<'));
                QString qname = KCharsets::resolveEntities( codec->toUnicode( name ) );
                QByteArray additionalInfo = t.mid( 8, endTag-8 );
                bool folded = (additionalInfo.left(6) == "FOLDED");
                if (folded) additionalInfo.remove(0,7);

                emit newFolder( qname,
                                !folded,
                                codec->toUnicode(additionalInfo) );
            }
            else if(t.left(4).toUpper() == "<HR>")
                emit newSeparator();
            else if(t.left(8).toUpper() == "</DL><P>")
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
           return KFileDialog::getSaveFileName( QDir::homePath() + "/.mozilla",
                                                i18n("*.html|HTML Files (*.html)") );
       else
           return KFileDialog::getOpenFileName( QDir::homePath() + "/.mozilla",
                                                i18n("*.html|HTML Files (*.html)") );
    }
    else
    {
       return QDir::homePath() + "/.netscape/bookmarks.html";
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
    static KNSBookmarkImporterImpl *p = 0;
    if (!p)
    {
        p = new KNSBookmarkImporterImpl;
        p->setUtf8(false);
    }
    return p->findDefaultLocation(forSaving);
}

QString KNSBookmarkImporter::mozillaBookmarksFile( bool forSaving )
{
    static KNSBookmarkImporterImpl *p = 0;
    if (!p)
    {
        p = new KNSBookmarkImporterImpl;
        p->setUtf8(true);
    }
    return p->findDefaultLocation(forSaving);
}


////////////////////////////////////////////////////////////////
//                   compat only
////////////////////////////////////////////////////////////////

void KNSBookmarkExporter::write(bool utf8) {
   KNSBookmarkExporterImpl exporter(m_pManager, m_fileName);
   exporter.setUtf8(utf8);
   exporter.write(m_pManager->root());
}

void KNSBookmarkExporter::writeFolder(QTextStream &/*stream*/, KBookmarkGroup /*gp*/) {
   // TODO - requires a d pointer workaround hack?
}

////////////////////////////////////////////////////////////////

void KNSBookmarkExporterImpl::setUtf8(bool utf8) {
   m_utf8 = utf8;
}

void KNSBookmarkExporterImpl::write(KBookmarkGroup parent) {
   if (QFile::exists(m_fileName)) {
      ::rename(
         QFile::encodeName(m_fileName),
         QFile::encodeName(m_fileName + ".beforekde"));
   }

   QFile file(m_fileName);

   if (!file.open(QIODevice::WriteOnly)) {
      kError(7043) << "Can't write to file " << m_fileName << endl;
      return;
   }

   QTextStream fstream(&file);
   fstream.setEncoding(m_utf8 ? QTextStream::UnicodeUTF8 : QTextStream::Locale);

   QString charset
      = m_utf8 ? "UTF-8" : QString::fromLatin1(QTextCodec::codecForLocale()->name()).toUpper();

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

QString KNSBookmarkExporterImpl::folderAsString(KBookmarkGroup parent) const {
   QString str;
   QTextStream fstream(&str, QIODevice::WriteOnly);

   for (KBookmark bk = parent.first(); !bk.isNull(); bk = parent.next(bk)) {
      if (bk.isSeparator()) {
         fstream << "<HR>" << endl;
         continue;
      }

      QString text = Qt::escape(bk.fullText());

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
