/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include <QtCore/QDebug>
#include <QtGui/QApplication>

#include <kjs/interpreter.h>
#include <kjs/ustring.h>

#include <kjsembed/kjsembed.h>
#include <kjsembed/object_binding.h>

#include "test_object.h"
#include "test_pointer.h"

int main( int argc, char **argv )
{
    QApplication app(argc, argv);

    Line obj;
    obj.name = "C++ Object";

    KJSEmbed::Engine *kernel = new KJSEmbed::Engine();
    KJS::Interpreter *js = kernel->interpreter();
    KJS::JSObject *globalObject = js->globalObject();
    KJS::ExecState *exec = js->globalExec();

    KJSEmbed::StaticConstructor::add( exec, globalObject, TestObject::constructor() ); // Ctor
    KJSEmbed::StaticConstructor::add( exec, globalObject, TestPointer::constructor() ); // Ctor

    KJS::JSValue *val = KJSEmbed::createObject<Line>(exec, "Line", &obj, KJSEmbed::ObjectBinding::CPPOwned);
    globalObject->put( exec, "Test", val ); // Static object

    /*KJS::JSObject *appobj =*/ kernel->addObject( &app, "Application" );

    KJSEmbed::Engine::ExitStatus result = kernel->runFile( ":/test.js" );
    if ( result != KJSEmbed::Engine::Success )
    {
        KJS::Completion jsres = kernel->completion();
        qDebug() << KJSEmbed::toQString(jsres.value()->toString(exec));
        delete kernel;
        return 0;
    }
    delete kernel;
    return result;
}

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;

