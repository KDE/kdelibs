// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "value.h"
#include "property_map.h"
#include "internal.h"
#include "ustring.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

using namespace KJS;

bool testInsertDelete(int numInsert, int numDelete, int delOffset, int randSeed) {

  srand(randSeed);
  char str[20];
  bool result = true;

  assert(numDelete >= 0 && numDelete < numInsert);
  assert(delOffset >= 0 && delOffset+numDelete <= numInsert);
  PropertyMap map;

  // add some random numbers
  int *nums = (int*)malloc(numInsert*sizeof(int));
  int i;
  for (i = 0; i < numInsert; i++) {
    nums[i] = int(1000.0*rand()/RAND_MAX);

    Value val = Number(nums[i]);
    ValueImp *v = val.imp();
    v->ref();

    sprintf(str,"%05d-%05d",nums[i],i); // ensure uniqueness
    map.put(str,v,0);
    map.checkTree();
  }

  // check to ensure they're all there
  for (i = 0; i < numInsert; i++) {
    sprintf(str,"%05d-%05d",nums[i],i);
    ValueImp *v = map.get(str);
    if (v == 0 || v->type() != NumberType ||
        static_cast<NumberImp*>(v)->value() != nums[i]) {
      result = false;
    }
    map.checkTree();
  }

  // delete some
  for (i = delOffset; i < delOffset+numDelete; i++) {
    sprintf(str,"%05d-%05d",nums[i],i);
    map.remove(str);
    map.checkTree();
  }

  // make sure the deletes ones aren't there any more, and the others are
  for (i = 0; i < numInsert; i++) {
    sprintf(str,"%05d-%05d",nums[i],i);
    ValueImp *v = map.get(str);

    if (i >= delOffset && i < delOffset+numDelete) {
      // should have been deleted
      if (v)
        result = false;
    }
    else {
      // should not have been deleted
      if (v == 0 || v->type() != NumberType ||
          static_cast<NumberImp*>(v)->value() != nums[i]) {
        result = false;
      }
    }
    map.checkTree();
  }

  // check that first() and next() work
  PropertyMapNode *it = map.first();
  int itcount = 0;
  while (it) {
    itcount++;
    PropertyMapNode *prev = it;
    it = it->next();
    if (it) {
      if (uscompare(prev->name,it->name) >= 0)
        result = false;
    }
  }
  if (itcount != numInsert-numDelete)
    result = false;

  if (result)
    printf("PASS: Insert %d, delete %d-%d, seed %d\n",numInsert,delOffset,
           delOffset+numDelete-1,randSeed);
  else
    printf("FAIL: Insert %d, delete %d-%d, seed %d\n",numInsert,delOffset,
           delOffset+numDelete-1,randSeed);

  return result;
}

void testMisc() {
  PropertyMap *map;

  // test remove() doesn't crash with empty list
  map = new PropertyMap();
  map->remove("nonexistant");
  delete map;
  printf("PASS: remove() doesn't crash with empty list\n");

  // test get() doesn't crash with empty list
  map = new PropertyMap();
  map->get("nonexistant");
  delete map;
  printf("PASS: get() doesn't crash with empty list\n");

  // test get() returns 0 on an empty list
  map = new PropertyMap();
  if (map->get("nonexistant") == 0)
    printf("PASS: get() returns 0 on an empty list\n");
  else
    printf("FAIL: get() returns 0 on an empty list\n");
  delete map;
}

int main()
{
  PropertyMap map;

  testMisc();

  int randomSeed = 4;
  int numTests = 100;

  srand(randomSeed);

  int *numInserts = (int*)malloc(numTests*sizeof(int));
  int *numDeletes = (int*)malloc(numTests*sizeof(int));
  int *delOffsets = (int*)malloc(numTests*sizeof(int));
  int i;
  for (i = 0; i < numTests; i++) {
    numInserts[i] = int(1000.0*rand()/RAND_MAX);
    numDeletes[i] = int(float(numInserts[i])*rand()/RAND_MAX);
    delOffsets[i] = int(float(numInserts[i]-numDeletes[i])*rand()/RAND_MAX);
  }

  for (i = 0; i < numTests; i++) {
    testInsertDelete(numInserts[i],numDeletes[i],delOffsets[i],4);
  }



  return 0;
}
