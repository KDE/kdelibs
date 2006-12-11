/* This file is part of the KDE libraries
    Copyright (C) 2004, 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

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

#include <QString>
#include <QApplication>
#include <QDebug>
#include <QStringList>

#ifndef QT_ONLY
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#endif // QT_ONLY

#include <kjs/interpreter.h>
#include <kjs/ustring.h>

#include <kjsembed/kjseglobal.h>
#include <kjsembed/kjsembed.h>

#include <QTime>

using namespace KJSEmbed;

void printUsage(QString appName)
{
    (*KJSEmbed::conerr()) << "Usage: " << appName << " [options] [file]" << endl
                          << "Options:" << endl
                          << "    -e, --exec            execute script without gui support." << endl 
                          << "    -i, --interactive     start interactive kjs interpreter." << endl
#ifndef QT_ONLY
                          << "    -n, --no-kde          start without KDE KApplication support." << endl
#endif
                          << endl;
}

#ifndef QT_ONLY

static KCmdLineOptions options[] =
{
    {"e", I18N_NOOP("Execute script without gui support"), 0 },
    {"exec", I18N_NOOP("Execute script without gui support"), 0 },
    {"i", I18N_NOOP("start interactive kjs interpreter"), 0 },
    {"interactive", I18N_NOOP("start interactive kjs interpreter"), 0 },
    {"n", I18N_NOOP("start without KDE KApplication support."), 0 },
    {"no-kde", I18N_NOOP("start without KDE KApplication support."), 0 },
    { "!+command", I18N_NOOP("Script to execute"), 0 },
    KCmdLineLastOption
};

#endif // QT_ONLY

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
#ifndef QT_ONLY
#ifdef __GNUC__
#warning "KDE Support enabled"
#endif
    bool kde = true;
#else
#ifdef __GNUC__
#warning "KDE Support disabled"
#endif
#endif

    if (argc > 1)
    {
        while (!args.isEmpty())
        {
            QString arg = args.takeFirst();
            if (arg.contains('-'))
            {
                if ((arg == "--exec") || (arg == "-e"))
		{
                    gui = false;
		}
                else if ((arg == "--interactive") || (arg == "-i"))
                    (*KJSEmbed::conout()) << "Interactive";
#ifndef QT_ONLY
                else if ((arg == "-n") || (arg == "--no-kde"))
		{
		    kde = false;
		}
#endif
		else
                {
                    printUsage(appName);
                    return 0;
                }
            }
            else
            {
                if (!script.isEmpty())
                    scriptArgs.append(KJS::String(arg));
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

#ifndef QT_ONLY
    if (kde)
    {
        KAboutData aboutData( "kjscmd", I18N_NOOP("KJSCmd"), "0.2",
            I18N_NOOP(""
            "Utility for running KJSEmbed scripts \n" ),
            KAboutData::License_LGPL,
            "(C) 2005-2006 The KJSEmbed Authors" );

        KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
        KCmdLineArgs::init( argc, argv, &aboutData );

        app = new KApplication(gui);
    }
    else
#endif
    if (gui)
    {
	qDebug("no KDE");
        app = new QApplication( argc, argv );
        dynamic_cast<QApplication*>(app)->connect( app, SIGNAL( lastWindowClosed() ), SLOT(quit()) );
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
        result = kernel.runFile(script);
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
    return (int)result;
}

