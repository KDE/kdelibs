/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
		  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <qdir.h>
#include <qfiledlg.h> 

#include "kfiledialog.h"
#include <qmsgbox.h>
#include <kconfig.h>
#include <kapp.h>
#include <kurl.h>


/*
 * just an example of how to make a new preview module
 *
bool previewTestFile( const KFileInfo *i, const QString inFilename,
                      QString &, QPixmap &outPixmap )
{
    bool loaded = false;
    QString iconName(inFilename);
    iconName.detach();
    int index = iconName.find( i->fileName() );
    iconName.insert(index,".xvpics/");
    QFile miniPic( iconName );
    if ( miniPic.exists() ) {
        outPixmap = QPixmap( iconName );
        loaded = true;
    }
    return loaded;
}
*/


int main(int argc, char **argv)
{

    KApplication a(argc, argv, "kfstest");
    QString name1;
    
    if (argc != 2) {
	warning("usage: %s {dirs, filter, preselect, normal, preview}", argv[0]);
	exit(0);
    }
    
    enum { Dirs, Filter, Preselect, Normal, Preview } mode;
    
    if (QString(argv[1]) == "dirs")
	mode = Dirs;
    else if (QString(argv[1]) == "filter")
	mode = Filter;
    else if (QString(argv[1]) == "preselect")
	mode = Preselect;
    else if (QString(argv[1]) == "preview")
        mode = Preview;
    else mode = Normal;

    switch (mode) {
    case Dirs:
	name1 = KFileDialog::getDirectory(0);
	break;
    case Filter:
	name1 = KFileDialog::getOpenFileURL(0, 
					   "*.cpp|C++-Files (*.cpp)\n"
					   "*.h|Header-Files (*.h)\n"
					   "*.o *.a *.lo *.la|Object-Files");
	break;
    case Preselect:
        name1 = KFileDialog::getOpenFileURL("/etc/inetd/inetd.conf");
	break;
    case Normal: {
      KFileDialog dlg(QDir::homeDirPath(), 0, 0, 0, true);
      // dlg.setSelection("../hello");
      dlg.exec();
      name1 = dlg.selectedFile();
    }
        break;
    case Preview:
        // this is how you activate the new preview module
        // 
        // KFilePreviewDialog::registerPreviewModule( "TEST", previewTestFile, PreviewPixmap );
        KFilePreviewDialog::getOpenFileURL(0,"*.cpp|C++-Files (*.cpp)\n""*|All Files");
        break;
    }
    
    if (!(name1.isNull()))
	QMessageBox::message("Your Choice",
			     "You selected the file "+ name1);
    return 0;
}
