/*
*  Copyright (C) 2004, Richard J. Moore <rich@kde.org>
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
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

#include <QString>
#include <QApplication>

#include <kjs/interpreter.h>
#include <kjs/ustring.h>

#include "kjsembed.h"
#include "../kjsembed/global.h"

#include <QTime>

using namespace KJSEmbed;

int main( int argc, char **argv )
{
    QTime time;
    time.start();

#ifdef _WIN32
#   ifdef CONSOLEIO
    RedirectIOToConsole();
#   endif
#endif
    // Setup QApplication
    QApplication app( argc, argv );
    qDebug(" New QApplication %dms", time.elapsed() );
    app.connect( &app, SIGNAL( lastWindowClosed() ), SLOT(quit()) );

    // Setup Interpreter
    time.restart();
    Engine kernel;
    qDebug(" New engine %dms", time.elapsed() );
    time.restart();

    KJS::Interpreter *js = kernel.interpreter();
    KJS::ExecState *exec = js->globalExec();

    // Publish bindings
    KJS::JSObject *appobj = kernel.addObject( &app, "Application" );

    // Build args array
    KJS::List args;
    for ( int i = 1 ; i < argc ; i++ )
    {
        args.append( KJS::String( argv[i] ) );
    }

    KJS::JSObject *argobj( js->builtinArray()->construct( exec, args ) );
    appobj->put( exec, "args", argobj );

    Engine::ExitStatus result = Engine::Failure;

    if ( argc > 1 ) // Run script
    {
        result = kernel.runFile( argv[1] );
    }
    else // exec shell
    {
        result = kernel.runFile( ":/console.js" );
    }

    if ( result != Engine::Success )
    {
            KJS::Completion jsres = kernel.completion();
            (*KJSEmbed::conerr()) << jsres.value()->toString(exec).qstring() << endl;
    }
    return (int) result;
}

// Local Variables:
// c-basic-offset: 4
// End:
