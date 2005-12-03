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
   KAboutData about("kbugreporttest", "kbugreporttest", "version");
   KCmdLineArgs::init(argc, argv, &about);

   KApplication a;
   a.setQuitOnLastWindowClosed(false);
   KBugReport rep(0,true,&about);
   rep.exec();
   KAboutData about1("kbugreporttest", "kbugreporttest", "version",0,KAboutData::License_Unknown,0,0,0,"test@bugs.kde.org");
   KBugReport rep1(0,true,&about1);
   rep1.exec();
   return 0;
}
