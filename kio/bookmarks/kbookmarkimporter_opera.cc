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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkimporter_opera.h"
#include "kbookmarkimporter_opera_p.h"

#include <kfiledialog.h>
#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <qtextcodec.h>
#include <QtGui/QApplication>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>

#include "kbookmarkimporter.h"

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

   QString url, name, type;
   int lineno = 0, version = 0;
   QTextStream stream(&file);
   stream.setCodec(codec);
   while(! stream.atEnd()) {
        lineno++;
        QString line = stream.readLine().trimmed();

        // first two headers lines contain details about the format
        if (lineno <= 2) {
            if (line.toLower().startsWith(QLatin1String("options:"))) {
                foreach(const QString &ba, line.mid(8).split(',')) {
                    const int pos = ba.indexOf('=');
                    if (pos < 1)
                        continue;
                    const QString key = ba.left(pos).trimmed().toLower();
                    const QString value = ba.mid(pos+1).trimmed();
                    if (key == "version")
                        version = value.toInt();
                }
            }
            continue;
        }

        // at least up till version<=3 the following is valid
        if (line.isEmpty()) {
            // end of data block
            if (type.isNull())
                continue;
            else if ( type == "URL")
                emit newBookmark( name, url, "" );
            else if (type == "FOLDER" )
                emit newFolder( name, false, "" );

            type.clear();
            name.clear();
            url.clear();
        } else if (line == "-") {
            // end of folder
            emit endFolder();
        } else {
            // data block line
            QString tag;
            if ( tag = '#', line.startsWith( tag ) )
                type = line.remove( 0, tag.length() );
            else if ( tag = "NAME=", line.startsWith( tag ) )
                name = line.remove(0, tag.length());
            else if ( tag = "URL=", line.startsWith( tag ) )
                url = line.remove(0, tag.length());
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
                       QString(QDir::homePath() + "/.opera"),
                       i18n("*.adr|Opera Bookmark Files (*.adr)"),
                       QApplication::activeWindow() )
                 : KFileDialog::getOpenFileName(
                       QString(QDir::homePath() + "/.opera"),
                       i18n("*.adr|Opera Bookmark Files (*.adr)"),
                       QApplication::activeWindow() );
}

/////////////////////////////////////////////////

class OperaExporter : private KBookmarkGroupTraverser {
public:
    OperaExporter();
    QString generate( const KBookmarkGroup &grp ) { traverse(grp); return m_string; }
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
    // kDebug() << "visit(" << bk.text() << ")";
    m_out << "#URL" << endl;
    m_out << "\tNAME=" << bk.fullText() << endl;
    m_out << "\tURL=" << bk.url().url().toUtf8() << endl;
    m_out << endl;
}

void OperaExporter::visitEnter( const KBookmarkGroup &grp ) {
    // kDebug() << "visitEnter(" << grp.text() << ")";
    m_out << "#FOLDER" << endl;
    m_out << "\tNAME="<< grp.fullText() << endl;
    m_out << endl;
}

void OperaExporter::visitLeave( const KBookmarkGroup & ) {
    // kDebug() << "visitLeave()";
    m_out << "-" << endl;
    m_out << endl;
}

void KOperaBookmarkExporterImpl::write(const KBookmarkGroup &parent) {
    OperaExporter exporter;
    QString content = exporter.generate( parent );
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly)) {
       kError(7043) << "Can't write to file " << m_fileName << endl;
       return;
    }
    QTextStream fstream(&file);
    fstream.setCodec(QTextCodec::codecForName("UTF-8"));
    fstream << content;
}

#include "kbookmarkimporter_opera_p.moc"
