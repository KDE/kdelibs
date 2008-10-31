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


#include <QtGui/QApplication>
#include <QtCore/QDebug>
#include <QtCore/QStringList>

#ifndef QT_ONLY
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdeversion.h>
#endif // QT_ONLY

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
#ifndef QT_ONLY
                          << "    -n, --no-kde          start without KDE KApplication support." << endl
#endif
                          << endl;
}

#ifndef QT_ONLY

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
    /*
    #ifdef __GNUC__
        #warning "KDE Support enabled"
    #endif
    */
    bool kde = true;
#else
    /*
    #ifdef __GNUC__
        #warning "KDE Support disabled"
    #endif
    */
#endif

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
#ifndef QT_ONLY
		    printf("KDE: %s\n", KDE_VERSION_STRING); 
#endif
		     return 0;
		}
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

#ifndef QT_ONLY
    if (kde)
    {
        KAboutData aboutData( "kjscmd", 0, ki18n("KJSCmd"), "0.2",
            ki18n(""
            "Utility for running KJSEmbed scripts \n" ),
            KAboutData::License_LGPL,
            ki18n("(C) 2005-2006 The KJSEmbed Authors") );

        KCmdLineOptions options;
        options.add("e", ki18n("Execute script without gui support"));
        options.add("exec", ki18n("Execute script without gui support"));
        options.add("i", ki18n("start interactive kjs interpreter"));
        options.add("interactive", ki18n("start interactive kjs interpreter"));
        options.add("n", ki18n("start without KDE KApplication support."));
        options.add("no-kde", ki18n("start without KDE KApplication support."));
        options.add("!+command", ki18n("Script to execute"));

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

