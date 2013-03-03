/* This file is part of the KDE libraries
    Copyright (C) 2004, 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2004, 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2004, 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2004, 2005, 2006 Erik L. Bunce <kde@bunce.us>

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


#include <QApplication>
#include <QtCore/QDebug>
#include <QtCore/QStringList>

#include <kjs/interpreter.h>
#include <kjs/ustring.h>

#include <kjsembed/kjseglobal.h>
#include <kjsembed/kjsembed.h>

#include <QtCore/QDate>

using namespace KJSEmbed;

void printUsage(QString appName)
{
    (*KJSEmbed::conerr()) << "Usage: " << appName << " [options] [file]" << endl
                          << "Options:" << endl
                          << "    -e, --exec            execute script without gui support." << endl
                          << "    -i, --interactive     start interactive kjs interpreter." << endl
                          << endl;
}

int main( int argc, char **argv )
{
    QTime time;
    time.start();

#ifdef _WIN32
#   ifdef CONSOLEIO
    RedirectIOToConsole();
#   endif
#endif

    // Handle arguments
    QString appName = argv[0];
    QStringList args;
    for (int i = 1; i < argc; i++ )
    {
        args << argv[i];
    }

    QString script;
    KJS::List scriptArgs;
    bool gui = true;

    if (argc > 1)
    {
        while (!args.isEmpty())
        {
            QString arg = args.takeFirst();
            if (arg.contains('-'))
            {
                if ((arg == "--version") || (arg == "-v"))
		{
		    printf("Qt: %s\n", qVersion());
		     return 0;
		}
                if ((arg == "--exec") || (arg == "-e"))
		{
                    gui = false;
		}
                else if ((arg == "--interactive") || (arg == "-i"))
                    (*KJSEmbed::conout()) << "Interactive";
		else
                {
                    printUsage(appName);
                    return 0;
                }
            }
            else
            {
                if (!script.isEmpty())
                    scriptArgs.append(KJS::jsString(arg));
                else
                    script = arg;
            }
        }
    }
    else
    {
        printUsage(appName);
        return 0;
    }

    // Setup QApplication
    QCoreApplication *app;

    if (gui)
    {
        app = new QApplication( argc, argv );
        QObject::connect( app, SIGNAL(lastWindowClosed()), app, SLOT(quit()) );
    }
    else
    {
	qDebug("no GUI");
        app = new QCoreApplication(argc, argv);
    }
    qDebug(" New %s %dms", app->metaObject()->className(), time.elapsed());

    app->setApplicationName( appName );

    // Setup Interpreter
    time.restart();
    Engine kernel;
    qDebug(" New engine %dms", time.elapsed());
    time.restart();

    KJS::Interpreter *js = kernel.interpreter();
    js->setShouldPrintExceptions(true);
    KJS::ExecState *exec = js->globalExec();

    // Publish bindings
    KJS::JSObject *appObject = kernel.addObject( app, "Application" );
    KJS::JSObject *argObject = js->builtinArray()->construct( exec, scriptArgs );
    appObject->put( exec, "args", argObject );
    Engine::ExitStatus result = Engine::Failure;

    if (!script.isEmpty())
    {
        result = kernel.runFile(toUString(script));
    }
    else // exec shell
    {
        result = kernel.runFile( ":/console.js" );
    }

    if ( result != Engine::Success )
    {
        KJS::Completion jsres = kernel.completion();
       (*KJSEmbed::conerr()) << toQString(jsres.value()->toString(exec)) << endl;
    }
    return (int)result;
}

