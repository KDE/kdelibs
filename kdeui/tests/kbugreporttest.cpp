/* This file is part of the KDE libraries
    Copyright (C) 2005 Joseph Wenninger <jowenn@kde.org>

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

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kbugreport.h>

int main(int argc, char **argv) {

    // First a bug report to bugs.kde.org
    KAboutData about("kbugreporttest", 0, ki18n("kbugreporttest"), "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication a;
    a.setQuitOnLastWindowClosed(false);
    KBugReport rep(0,true,&about);
    rep.exec();

    // Then a bug report by email.
    // Change the email address to check if it worked :)
    KAboutData about1("kbugreporttest", 0, ki18n("kbugreporttest"), "version",
                      ki18n("description"), KAboutData::License_Unknown,
                      ki18n("copyright"), ki18n("bug report tool"),
                      QByteArray(), "null@bugs.kde.org");
    KBugReport rep1(0,true,&about1);
    rep1.exec();

    return 0;
}
