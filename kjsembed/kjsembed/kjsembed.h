/*
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
*  Copyright (C) 2006, Matt Broadstone <mbroadst@gmail.com>
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

#ifndef KJSEMBED_H
#define KJSEMBED_H

class QObject;

#include <kjs/object.h>
#include <kdemacros.h>

namespace KJS {
    class Interpreter;
    class Object;
    class Completion;
};

namespace KJSEmbed {

class KDE_EXPORT Engine
{
public:
    enum ExitStatus { Success = 0, Failure = 1 };

    Engine();
    virtual ~Engine();

    ExitStatus runFile( const KJS::UString &file );
    ExitStatus execute( const KJS::UString &code );

    KJS::Completion completion() const;

    KJS::Interpreter *interpreter() const;

    /**
    *  publishes a QObject to the global context of the javascript interpereter.
    */
    KJS::JSObject *addObject( QObject *obj, const KJS::UString &name = KJS::UString() ) const;

    /**
    * publishes a QObject to a parent object.
    */
    KJS::JSObject *addObject( QObject *obj, KJS::JSObject *parent, const KJS::UString &name = KJS::UString()) const;

    /**
    * Create a new instance of an object that the Javascript engine knows about.  If the object
    * doesn't exist a KJS::Null() is returned and an exception thrown.
    */
    KJS::JSObject *construct( const KJS::UString &className, const KJS::List &args = KJS::List() ) const;

    /**
    * Execute a method at the global scope of the javascript interperter.
    */
    KJS::JSObject *callMethod( const KJS::UString &methodName, const KJS::List &args = KJS::List() );

    /**
    * Execute a method on an object.
    */
    KJS::JSObject *callMethod( const KJS::JSObject *parent, const KJS::UString &methodName, const KJS::List &args = KJS::List() );

private:
    class EnginePrivate *dptr;
};

}

#endif
