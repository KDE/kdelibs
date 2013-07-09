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

#include <QFileDialog>
#include <QMessageBox>

#include "kbookmarkimporter_ns.h"
#include "kbookmarkimporter.h"
#include "kbookmarkexporter.h"
#include "kbookmarkmanager.h"
#include <QtCore/QDebug>
#include <kcharsets.h>

#include <qtextcodec.h>
#include <QApplication>



void KNSBookmarkImporterImpl::parse()
{
    QFile f(m_fileName);
    QTextCodec * codec = m_utf8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForLocale();
    Q_ASSERT(codec);
    if (!codec)
        return;

    if(f.open(QIODevice::ReadOnly)) {

        static const int g_lineLimit = 16*1024;
        QByteArray s(g_lineLimit,0);
        // skip header
        while(f.readLine(s.data(), g_lineLimit) >= 1 && !s.contains("<DL>")) {
            ;
        }

        while( int size = f.readLine(s.data(), g_lineLimit)>=1) {
            if ( size == g_lineLimit ) // Gosh, this line is longer than g_lineLimit. Skipping.
            {
               qWarning() << "Netscape bookmarks contain a line longer than " << g_lineLimit << ". Skipping.";
               continue;
            }
            QByteArray t = s.trimmed();
            if(t.left(12).toUpper() == "<DT><A HREF=" ||
               t.left(16).toUpper() == "<DT><H3><A HREF=") {

              int firstQuotes = t.indexOf('"')+1;
              int secondQuotes = t.indexOf('"', firstQuotes);
              if (firstQuotes != -1 && secondQuotes != -1)
              {
                QByteArray link = t.mid(firstQuotes, secondQuotes-firstQuotes);
                int endTag = t.indexOf('>', secondQuotes+1);

                int closeTag = t.indexOf('<', endTag + 1);

                QByteArray name = t.mid(endTag + 1, closeTag - endTag - 1);
                QString qname = KCharsets::resolveEntities( codec->toUnicode( name ) );
                QByteArray additionalInfo = t.mid( secondQuotes+1, endTag-secondQuotes-1 );

                emit newBookmark( qname,
                                  codec->toUnicode(link),
                                  QByteArray() );
              }
            }
            else if(t.left(7).toUpper() == "<DT><H3") {
                int endTag = t.indexOf('>', 7);
                QByteArray name = t.mid(endTag+1);
                name = name.left(name.indexOf('<'));
                QString qname = KCharsets::resolveEntities( codec->toUnicode( name ) );
                QByteArray additionalInfo = t.mid( 8, endTag-8 );
                bool folded = (additionalInfo.left(6) == "FOLDED");
                if (folded) additionalInfo.remove(0,7);

                emit newFolder( qname,
                                !folded,
                                QByteArray() );
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
           return QFileDialog::getSaveFileName( QApplication::activeWindow(), QString(),
                                                QDir::homePath() + "/.mozilla",
                                                tr("HTML Files (*.html)"));
       else
           return QFileDialog::getOpenFileName( QApplication::activeWindow(), QString(),
                                                QDir::homePath() + "/.mozilla",
                                                tr("*.html|HTML Files (*.html)"));
    }
    else
    {
       return QDir::homePath() + "/.netscape/bookmarks.html";
    }
}

////////////////////////////////////////////////////////////////

void KNSBookmarkExporterImpl::setUtf8(bool utf8) {
   m_utf8 = utf8;
}

void KNSBookmarkExporterImpl::write(const KBookmarkGroup &parent)
{
   if (!QFile::exists(m_fileName)) {
      QString errorMsg = KNSBookmarkImporterImpl::tr("Could not find %1. Netscape is probably not installed. "
                       "Aborting the export.").arg(m_fileName);
      QMessageBox::critical(0, KNSBookmarkImporterImpl::tr("Netscape not found"), errorMsg);
      return;
   }
   if (QFile::exists(m_fileName)) {
       (void)QFile::rename(m_fileName, m_fileName + ".beforekde");
   }

   QFile file(m_fileName);

   if (!file.open(QIODevice::WriteOnly)) {
      qCritical() << "Can't write to file " << m_fileName << endl;
      return;
   }

   QTextStream fstream(&file);
   fstream.setCodec(m_utf8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForLocale());

   QString charset
      = m_utf8 ? "UTF-8" : QString::fromLatin1(QTextCodec::codecForLocale()->name()).toUpper();

   fstream << "<!DOCTYPE NETSCAPE-Bookmark-file-1>" << endl
           << KNSBookmarkImporterImpl::tr("<!-- This file was generated by Konqueror -->") << endl
           << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset="
              << charset << "\">" << endl
           << "<TITLE>" << KNSBookmarkImporterImpl::tr("Bookmarks") << "</TITLE>" << endl
           << "<H1>" << KNSBookmarkImporterImpl::tr("Bookmarks") << "</H1>" << endl
           << "<DL><p>" << endl
           << folderAsString(parent)
           << "</DL><P>" << endl;
}

QString KNSBookmarkExporterImpl::folderAsString(const KBookmarkGroup &parent) const {
   QString str;
   QTextStream fstream(&str, QIODevice::WriteOnly);

   for (KBookmark bk = parent.first(); !bk.isNull(); bk = parent.next(bk)) {
      if (bk.isSeparator()) {
         fstream << "<HR>" << endl;
         continue;
      }

      QString text = bk.fullText().toHtmlEscaped();

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
         fstream << "<DT><A HREF=\"" << bk.url().toString() << "\""
                    << bk.internalElement().attribute("netscapeinfo") << ">"
                 << text << "</A>" << endl;
         continue;
      }
   }

   return str;
}

////
