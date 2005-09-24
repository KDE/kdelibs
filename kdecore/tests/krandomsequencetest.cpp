/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qlist.h>
#include <qstring.h>

#include "krandomsequence.h"
#include "krandom.h"
#include "kcmdlineargs.h"
#include "kaboutdata.h"
#include "kapplication.h"

#include <stdio.h>

int
main(int argc, char *argv[])
{
   KAboutData about("krandomsequencetest", "krandomsequencetest", "version");
   KCmdLineArgs::init(argc, argv, &about);

   KApplication a;

   long seed;
   KRandomSequence seq;

   seed = 2;
   seq.setSeed(seed);printf("Seed = %4ld :", seed);
   for(int i = 0; i < 20; i++)
      printf("%3ld ", seq.getLong(100));
   printf("\n");

   seed = 0;
   seq.setSeed(seed);printf("Seed = %4ld :", seed);
   for(int i = 0; i < 20; i++)
      printf("%3ld ", seq.getLong(100));
   printf("\n");

   seed = 0;
   seq.setSeed(seed);printf("Seed = %4ld :", seed);
   for(int i = 0; i < 20; i++)
      printf("%3ld ", seq.getLong(100));
   printf("\n");

   seed = 2;
   seq.setSeed(seed);printf("Seed = %4ld :", seed);
   for(int i = 0; i < 20; i++)
      printf("%3ld ", seq.getLong(100));

   seq.setSeed(KRandom::random());

   QList<QString> list;
   list.append(QString("A"));
   list.append(QString("B"));
   list.append(QString("C"));
   list.append(QString("D"));
   list.append(QString("E"));
   list.append(QString("F"));
   list.append(QString("G"));

   for(QList<QString>::Iterator str = list.begin(); str != list.end(); str++)
      printf("%s", str->toLatin1().data());
   printf("\n\n");

   seq.randomize(list);

   for(QList<QString>::Iterator str = list.begin(); str != list.end(); str++)
      printf("%s", str->toLatin1().data());
   printf("\n\n");

   seq.randomize(list);

   for(QList<QString>::Iterator str = list.begin(); str != list.end(); str++)
      printf("%s", str->toLatin1().data());
   printf("\n\n");

   seq.randomize(list);

   for(QList<QString>::Iterator str = list.begin(); str != list.end(); str++)
      printf("%s", str->toLatin1().data());
   printf("\n\n");

   printf("\n");
}
