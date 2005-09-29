/* This file is part of the KDE libraries
    Copyright (C) 2002 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2003 David Faure <faure@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qdir.h>
#include <kopenwith.h>
#include <kurl.h>
#include <kdebug.h>

int main(int argc, char **argv)
{
    KApplication app(argc, argv, "kopenwithtest");
    KURL::List list;

    list += KURL("file:///tmp/testfile.txt");

    // Test with one URL
    KOpenWithDlg* dlg = new KOpenWithDlg(list, "OpenWith_Text", "OpenWith_Value", 0);
    if(dlg->exec()) {
        kdDebug() << "Dialog ended successfully\ntext: " << dlg->text() << endl;
    }
    else
        kdDebug() << "Dialog was canceled." << endl;
    delete dlg;

    // Test with two URLs
    list += KURL("http://www.kde.org/index.html");
    dlg = new KOpenWithDlg(list, "OpenWith_Text", "OpenWith_Value", 0);
    if(dlg->exec()) {
        kdDebug() << "Dialog ended successfully\ntext: " << dlg->text() << endl;
    }
    else
        kdDebug() << "Dialog was canceled." << endl;
    delete dlg;

    // Test with a mimetype
    QString mimetype = "text/plain";
    dlg = new KOpenWithDlg( mimetype, "kedit", 0);
    if(dlg->exec()) {
        kdDebug() << "Dialog ended successfully\ntext: " << dlg->text() << endl;
    }
    else
        kdDebug() << "Dialog was canceled." << endl;
    delete dlg;

    return 0;
}

