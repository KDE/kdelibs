// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
 *
 *  $Id$
 */

#include "value.h"
#include "property_map.h"
#include "ustring.h"

#include <stdio.h>

using namespace KJS;

int main() {


  Value val = Number(1);
  ValueImp *v = val.imp();

  PropertyMap2 map;

  /*
  map.put("0032",v);
  map.put("0016",v);
  map.put("0048",v);
  map.put("0008",v);
  map.put("0004",v);
  map.put("0020",v);
  map.put("0019",v);
  */

  /*
  map.put("0032",v);
  map.put("0016",v);
  map.put("0048",v);
  map.put("0056",v);
  map.put("0052",v);
  */

  for (int i = 12; i >= 0; i--) {
    char str[5];
    sprintf(str,"%04d",i);
    map.put(str,v);
  }

  //  map.put("0013",v);
  map.dump();

  printf("done\n");


  return 0;
}
