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

#include "kfiledialog.h"
#include <qmsgbox.h>
#include <kconfig.h>
#include <kapp.h>
#include <kurl.h>

int main(int argc, char **argv)
{
    const char filter[] = "*";
    // "*.cpp|C++-Files (*.cpp)\t*.h|Header-Files (*.h)\t*.o *.a|Object-Files";

    KApplication a(argc, argv, "kfstest");
    QString name1;
    
    if (argc == 2 && QString(argv[1]) == "dirs")
	name1 = KFileDialog::getDirectory(0);
    else
	name1= KFileDialog::getOpenFileURL(0, filter);
    
    if (!(name1.isNull()))
	QMessageBox::message("Your Choice",
			     "You selected the file "+ name1);
    return 0;
}
