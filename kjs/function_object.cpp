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

#include "function_object.h"

using namespace KJS;

// ECMA 15.3.1 The Function Constructor Called as a Function
Completion FunctionObject::execute(const List &args)
{
  return Completion(ReturnValue, construct(args));
}

// ECMA 15.3.2 The Function Constructor
Object FunctionObject::construct(const List &args)
{
  UString p("");
  UString body;
  if (args.isEmpty()) {
    body = "";
  } if ( args.size() == 1 ) {
    body = args[0].toString().value();
  } else {
    p = args[0].toString().value();
    for (int k = 1; k < args.size() - 1; k++)
      p += "," + args[k].toString().value();
    body = args[args.size()-1].toString().value();
  }

  /* TODO parse parameter list, create function object */
  return Object::create(NumberClass, Boolean(true));
}
