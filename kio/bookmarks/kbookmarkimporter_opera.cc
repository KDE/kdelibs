/* This file is part of the KDE libraries
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
   if(!file.open(IO_ReadOnly)) {
      return;
   }

   QTextCodec * codec = QTextCodec::codecForName("UTF-8");
   Q_ASSERT(codec);
   if (!codec)
      return;

   int lineno = 0;
   QString url, name, type;
   QCString line(4096);

   while ( file.readLine(line.data(), 4096) >=0 ) {
      lineno++;
    
      // skip lines that didn't fit in buffer and first two headers lines 
      if ( line[line.length()-1] != '\n' || lineno <= 2 )
          continue;
    
      QString currentLine = line.stripWhiteSpace();
      QString currentLine_utf8 = codec->toUnicode(line);
    
      // end of data block
      if (currentLine.isEmpty()) {
         if (type.isNull())
            continue;
         else if ( type == "URL")
            emit newBookmark( name, url.latin1(), "" );
         else if (type == "FOLDER" )
            emit newFolder( name, false, "" ); 

         type = QString::null;
         name = QString::null;
         url = QString::null;
         
      // end of folder
      } else if (currentLine == "-") {
         emit endFolder();
    
      // data block line
      } else {
         QString tag;
         if ( tag = "#", currentLine.startsWith( tag ) )
            type = currentLine.remove( 0, tag.length() );
         else if ( tag = "NAME=", currentLine.startsWith( tag ) )
            name = currentLine_utf8.remove(0, tag.length());
         else if ( tag = "URL=", currentLine.startsWith( tag ) )
            url = currentLine_utf8.remove(0, tag.length());
      }
   }

}

QString KOperaBookmarkImporter::operaBookmarksFile()
{
   static KOperaBookmarkImporterImpl importer;
   return importer.findDefaultLocation();
}

void KOperaBookmarkImporterImpl::parse() {
   KOperaBookmarkImporter importer(m_fileName);
   importer.parseOperaBookmarks();
}

QString KOperaBookmarkImporterImpl::findDefaultLocation(bool) const
{
   return KFileDialog::getOpenFileName( 
               QDir::homeDirPath() + "/.opera", 
               i18n("*.adr|Opera bookmark files (*.adr)") );
}

#include "kbookmarkimporter_opera.moc"
