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
#include <QtGui/QWidget>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QDir>
#include <kopenwithdialog.h>
#include <kurl.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "kopenwithdialogtest", 0, qi18n("kopenwithdialogtest"), "0.1", qi18n("kopenwithdialogtest"));
    KApplication app;
    KUrl::List list;

    list += KUrl("file:///tmp/testfile.txt");

    // Test with one URL
    KOpenWithDialog* dlg = new KOpenWithDialog(list, "OpenWith_Text", "OpenWith_Value", 0);
    if(dlg->exec()) {
        kDebug() << "Dialog ended successfully\ntext: " << dlg->text();
    }
    else
        kDebug() << "Dialog was canceled.";
    delete dlg;

    // Test with two URLs
    list += KUrl("http://www.kde.org/index.html");
    dlg = new KOpenWithDialog(list, "OpenWith_Text", "OpenWith_Value", 0);
    if(dlg->exec()) {
        kDebug() << "Dialog ended successfully\ntext: " << dlg->text();
    }
    else
        kDebug() << "Dialog was canceled.";
    delete dlg;

    // Test with a mimetype
    QString mimetype = "text/plain";
    dlg = new KOpenWithDialog( mimetype, "kedit", 0);
    if(dlg->exec()) {
        kDebug() << "Dialog ended successfully\ntext: " << dlg->text();
    }
    else
        kDebug() << "Dialog was canceled.";
    delete dlg;

    return 0;
}

