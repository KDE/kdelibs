/* This file is part of the KDE libraries
    Copyright (c) 2011 David Faure <faure@kde.org>

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

#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klockfile.h>
#include <kdebug.h>

int
main(int argc, char *argv[])
{
   KAboutData about("klockfile_testlock", 0, ki18n("klockfile_testlock"), "version");
   KComponentData cData(&about);
   //KCmdLineArgs::init(argc, argv, &about);
   //KApplication a;

   if (argc <= 1) {
       return KLockFile::LockError;
   }

   const QString lockName = QString::fromLocal8Bit(argv[1]);
   
   KLockFile lockFile(lockName);
   if (lockFile.isLocked())
       return KLockFile::LockError;
   return lockFile.lock(KLockFile::NoBlockFlag);
}
