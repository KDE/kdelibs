/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2008 Harri Porten (porten@kde.org)
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

#ifndef KJSINTERPRETER_H
#define KJSINTERPRETER_H

#include "kjsapi_export.h"
#include "kjsobject.h"
#include "kjscontext.h"

class KJSPrototype;
class KJSInterpreterHandle;

/**
 * A class representing a JavaScript interpreter
 *
 * @short JavaScript interpreter
 */
class KJSAPI_EXPORT KJSInterpreter
{
    friend class KJSPrototype;
public:
    /**
     * Constructs an interpreter with a default global object.
     */
    KJSInterpreter();
    /**
     * Constructs an interpreter with a custom global object.
     */
    KJSInterpreter(const KJSObject& global);
    /**
     * Destructs this interpreter and frees resources it has
     * allocated. This renders any still existing objects referencing
     * those invalid.
     */
    ~KJSInterpreter();

    /**
     * Returns a handle to the global execution context.
     */
    KJSContext* globalContext();
    /**
     * @overload
     */
    const KJSContext* globalContext() const;
    /**
     * Returns the object that is used as the global object during all
     * script execution performed by this interpreter,
     */
    KJSObject globalObject();

    KJSObject evaluate(const QString& sourceURL, int startingLineNumber,
                       const QString& code,
                       KJSObject* thisValue = 0);

    /**
     *  @overload
     */
    KJSObject evaluate(const QString& code,
                       KJSObject* thisValue = 0);

private:
    KJSInterpreter(const KJSInterpreter&); // undefined
    KJSInterpreter& operator=(const KJSInterpreter&); // undefined

    KJSInterpreterHandle* hnd;
    KJSContext globCtx;
};

#endif
