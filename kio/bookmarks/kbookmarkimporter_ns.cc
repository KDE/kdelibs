/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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
#include <kfiledialog.h>
#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcharsets.h>
#include <qtextcodec.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>

void KNSBookmarkImporter::parseNSBookmarks( bool utf8 )
{
    QFile f(m_fileName);
    QTextCodec * codec = utf8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForLocale();
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

QString KNSBookmarkImporter::netscapeBookmarksFile( bool )
{
    return QDir::homeDirPath() + "/.netscape/bookmarks.html";
}

QString KNSBookmarkImporter::mozillaBookmarksFile( bool forSaving )
{
    //return QDir::homeDirPath() + "/.mozilla/default/bookmarks.html";
    if ( forSaving )
        return KFileDialog::getSaveFileName( QDir::homeDirPath() + "/.mozilla",
                                             i18n("*.html|HTML files (*.html)") );
    else
        return KFileDialog::getOpenFileName( QDir::homeDirPath() + "/.mozilla",
                                             i18n("*.html|HTML files (*.html)") );
}

////

#include "kbookmarkimporter_ns.moc"
