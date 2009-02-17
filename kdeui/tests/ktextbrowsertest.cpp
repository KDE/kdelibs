/* This file is part of the KDE libraries
    Copyright (C) 2009 David Faure <faure@kde.org>

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
#include <kcmdlineargs.h>
#include <ktextbrowser.h>

#include <QFile>
#include <QAction>

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "ktextedittest", 0, ki18n("KTextEditTest"), "1.0", ki18n("ktextedit test app"));
    KApplication app;
    KTextBrowser *edit = new KTextBrowser();

    edit->setText("<html>This is a test program for KTextBrowser<br/>"
                  "<a href=\"whatsthis:This is a whatsthis\">What's this?</a><br/>"
                  "<a href=\"faure at kde.org\">faure at kde.org</a><br/>"
                  "<a href=\"http://www.kde.org\">http://www.kde.org</a><br/>"
                  "<a href=\"" KDESRCDIR "/ktextbrowsertest.cpp\">A local link</a><br/>"
                  "</html>");

    edit->resize( 600, 600 );
    edit->show();
    return app.exec();
}
