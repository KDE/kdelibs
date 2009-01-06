/***************************************************************************
 * main.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "testobject.h"

// Kross
#include "../core/action.h"
#include "../core/interpreter.h"
#include "../core/manager.h"
#include "../core/wrapperinterface.h"

// Qt

#include <QtCore/QFile>
#include <QtCore/QArgument>
#include <QtCore/QMetaEnum>

// KDE
#include <kdebug.h>
#include <kcomponentdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kurl.h>

// for std namespace
#include <string>
#include <iostream>

#define ERROR_OK 0
#define ERROR_HELP -1
#define ERROR_NOSUCHFILE -2
#define ERROR_OPENFAILED -3
#define ERROR_NOINTERPRETER -4
#define ERROR_EXCEPTION -6

KApplication *app = 0;

QString getInterpreterName(const QString& scriptfile)
{
    Kross::InterpreterInfo* interpreterinfo = Kross::Manager::self().interpreterInfo( Kross::Manager::self().interpreternameForFile(scriptfile) );
    return interpreterinfo ? interpreterinfo->interpreterName() : QString();
}

int readFile(const QString& scriptfile, QByteArray& content)
{
    QFile f(scriptfile);
    if(! f.exists()) {
        std::cerr << "No such scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_NOSUCHFILE;
    }
    if(! f.open(QIODevice::ReadOnly)) {
        std::cerr << "Failed to open scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_OPENFAILED;
    }
    content = f.readAll();
    f.close();
    return ERROR_OK;
}

int runScriptFile(const QString& scriptfile)
{
    // Read the scriptfile
    QByteArray scriptcode;
    int result = readFile(scriptfile, scriptcode);
    if(result != ERROR_OK)
        return result;

    // Determinate the matching interpreter
    QString interpretername = getInterpreterName(scriptfile);
    if( interpretername.isNull() ) {
        std::cerr << "No interpreter for scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_NOINTERPRETER;
    }

    // First we need a Action and fill it.
    Kross::Action* action = new Kross::Action(0 /*no parent*/, KUrl(scriptfile));
    action->setObjectName("MyAction");
    action->setInterpreter( interpretername );
    action->setCode( scriptcode );

    // Create the testobject instances.
    TestObject* testobj3 = new TestObject(action, "TestObject3");
    TestObject* testobj4 = new TestObject(action, "TestObject4");

    // Publish other both testobject instance to the script.
    action->addObject( testobj3, "TestObject3", Kross::ChildrenInterface::AutoConnectSignals );
    action->addObject( testobj4, "TestObject4" );

    // Now execute the Action.
    std::cout << "Execute scriptfile " << scriptfile.toLatin1().data() << " now" << std::endl;
    action->trigger();
    std::cout << "Execution of scriptfile " << scriptfile.toLatin1().data() << " done" << std::endl;

    /*
    if(action->hadException()) {
        // We had an exception.
        QString errormessage = action->getException()->getError();
        QString tracedetails = action->getException()->getTrace();
        std::cerr << QString("%2\n%1").arg(tracedetails).arg(errormessage).toLatin1().data() << std::endl;
        return ERROR_EXCEPTION;
    }
    */

    delete action;
    return ERROR_OK;
}

QVariant OtherObjectHandler(void* ptr)
{
    OtherObject* obj = static_cast<OtherObject*>(ptr);
    kDebug()<<"OtherObjectHandler objectName="<<(obj ? obj->objectName() : "NULL");
    OtherObjectWrapper* wrapper = new OtherObjectWrapper(obj);
    QVariant r;
    r.setValue( (QObject*) wrapper );
    return r;
}

int main(int argc, char **argv)
{
    int result = 0;

    KAboutData about("krosstest",
                     0,
                     ki18n("KrossTest"),
                     "0.1",
                     ki18n("KDE application to test the Kross framework."),
                     KAboutData::License_LGPL,
                     ki18n("(C) 2005-2007 Sebastian Sauer"),
                     ki18n("Test the Kross framework!"),
                     "http://kross.dipe.org",
                     "kross@dipe.org");
    about.addAuthor(ki18n("Sebastian Sauer"), ki18n("Author"), "mail@dipe.org");

    KCmdLineOptions options;
    options.add("+file", ki18n("Scriptfile"));
    //options.add("functionname <functioname>", ki18n("Execute the function in the defined script file."));
    //options.add("functionargs <functioarguments>", ki18n("List of arguments to pass to the function on execution."));

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QStringList scriptfiles;
    for(int i = 0; i < args->count(); i++)
        scriptfiles.append( args->arg(i) );

    if(scriptfiles.count() < 1) {
        std::cerr << "No scriptfile to execute defined. See --help" << std::endl;
        return ERROR_NOSUCHFILE;
    }

    // init
    app = new KApplication(true);
    TestObject* testobj1 = new TestObject(0, "TestObject1");
    TestObject* testobj2 = new TestObject(0, "TestObject2");
    Kross::Manager::self().addObject( testobj1 );
    Kross::Manager::self().addObject( testobj2 );

    Kross::Manager::self().registerMetaTypeHandler("OtherObject*", OtherObjectHandler);

    foreach(const QString &file, scriptfiles) {
        result = runScriptFile(file);
        if(result != ERROR_OK)
            break;
    }

    // finish
    delete testobj1;
    delete testobj2;
    delete app;

    kDebug() << "DONE!!!";
    return result;
}
