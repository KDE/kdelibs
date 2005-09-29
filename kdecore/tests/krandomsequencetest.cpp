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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qptrlist.h>

#include "krandomsequence.h"
#include "kapplication.h"

#include <stdio.h>

int
main(int argc, char *argv[])
{
   KApplication a(argc, argv, "krandomsequencetest");

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

   seq.setSeed(kapp->random());

   QPtrList<QString> list;
   list.append(new QString("A"));
   list.append(new QString("B"));
   list.append(new QString("C"));
   list.append(new QString("D"));
   list.append(new QString("E"));
   list.append(new QString("F"));
   list.append(new QString("G"));

   for(QString *str = list.first(); str; str = list.next())
      printf("%s", str->latin1());
   printf("\n\n");

   seq.randomize(&list);

   for(QString *str = list.first(); str; str = list.next())
      printf("%s", str->latin1());
   printf("\n\n");

   seq.randomize(&list);

   for(QString *str = list.first(); str; str = list.next())
      printf("%s", str->latin1());
   printf("\n\n");

   seq.randomize(&list);

   for(QString *str = list.first(); str; str = list.next())
      printf("%s", str->latin1());
   printf("\n\n");

   printf("\n");
}
