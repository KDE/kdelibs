// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef _KJS_FUNCTION_H_
#define _KJS_FUNCTION_H_

#include "object.h"

namespace KJS {

  class FunctionPrototypeImp;

  /**
   * Base class for all function objects.
   * It implements the hasInstance method (for instanceof, which only applies to function objects)
   * and allows to give the function a name, used in toString().
   *
   * Constructors and prototypes of internal objects (implemented in C++) directly inherit from this.
   * FunctionImp also does, for functions implemented in JS.
   */
  class KJS_EXPORT InternalFunctionImp : public ObjectImp {
  public:
    /**
     * Constructor. For C++-implemented functions, @p funcProto is usually
     * static_cast<FunctionPrototypeImp*>(exec->interpreter()->builtinFunctionPrototype().imp())
     */
    InternalFunctionImp(FunctionPrototypeImp *funcProto);
    InternalFunctionImp(ExecState *exec);

    bool implementsHasInstance() const;
    Boolean hasInstance(ExecState *exec, const Value &value);

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
    Identifier name() const { return ident; }
    /// You might want to use the helper function ObjectImp::setFunctionName for this
    void setName(Identifier _ident) { ident = _ident; }

  protected:
    Identifier ident;
  };

} // namespace

#endif
