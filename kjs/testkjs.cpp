/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <stdio.h>

#include "kjs.h"
#include "object.h"
#include "function.h"

int main(int argc, char **argv)
{
  // expecting a filename
  if (argc != 2) {
    fprintf(stderr, "You have to specify a filename\n");
    return -1;
  }

  // read code from file
  const char *file = argv[1];
  FILE *f = fopen(file, "r");
  if (!f) {
    fprintf(stderr, "Error opening file.\n");
    return -1;
  }
  const int BufferSize = 100000;
  char code[BufferSize];
  int num = fread(code, 1, BufferSize, f);
  code[num] = '\0';
  if(num >= BufferSize)
    fprintf(stderr, "Warning: File may have been too long.\n");

  // create interpreter
  KJScript *kjs = new KJScript();

  // add debug() function
  kjs->setCurrent(kjs);
  KJScript::global()->put("debug", zeroRef(new KJS::DebugPrint()));

  // run
  kjs->evaluate(code);

  delete kjs;

  fprintf(stderr, "OK.\n");
}
