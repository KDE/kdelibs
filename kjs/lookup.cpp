/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>

#include "lookup.h"

using namespace KJS;

int Lookup::find(const struct HashTable *table,
		 const UChar *c, unsigned int len)
{
  // we only know about version 1 so far
  if (table->type != 1) {
    fprintf(stderr, "Unknown hash table version.\n");
    return -1;
  }

  int h = hash(c, len) % table->hashSize;
  const HashEntry *e = &table->entries[h];

  // empty bucket ?
  if (!e->c)
    return -1;

  do {
    // compare strings
    if ((len == e->len) && (memcmp(c, e->c, len * sizeof(UChar)) == 0))
	return e->value;
    // try next bucket
    e = e->next;
  } while (e);
  
  return -1;
}

int Lookup::find(const struct HashTable *table, const UString &s)
{
  return find(table, s.data(), s.size());
}

unsigned int Lookup::hash(const UChar *c, unsigned int len)
{
  unsigned int val = 0;
  for (unsigned int i = 0; i < len; i++, c++)
    val += c->low();

  return val;
}

unsigned int Lookup::hash(const UString &key)
{
  return hash(key.data(), key.size());
}
