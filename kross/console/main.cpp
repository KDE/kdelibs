/***************************************************************************
 * main.cpp
 * This file is part of the KDE project
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
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

// for std namespace
#include <iostream>

// Qt

#include <QtCore/QFile>
#include <QApplication>
#include <QUrl>

// KDE
#include <klocalizedstring.h>
#include <qcommandlineparser.h>
#include <qcommandlineoption.h>

// Kross
#include "../core/manager.h"
#include "../core/action.h"
#include "../core/interpreter.h"

#define ERROR_OK 0
#define ERROR_HELP -1
#define ERROR_NOSUCHFILE -2
#define ERROR_OPENFAILED -3
#define ERROR_NOINTERPRETER -4
#define ERROR_EXCEPTION -6

QApplication* app = 0;

int runScriptFile(const QString& scriptfile)
{
    // Read the scriptfile
    QFile f(scriptfile);
    if(! f.exists()) {
        std::cerr << "No such scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_NOSUCHFILE;
    }
    if(! f.open(QIODevice::ReadOnly)) {
        std::cerr << "Failed to open scriptfile: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_OPENFAILED;
    }
    QByteArray scriptcode = f.readAll();
    f.close();

    // Determinate the matching interpreter
    Kross::InterpreterInfo* interpreterinfo = Kross::Manager::self().interpreterInfo( Kross::Manager::self().interpreternameForFile(scriptfile) );
    if(! interpreterinfo) {
        std::cerr << "No interpreter for file: " << scriptfile.toLatin1().data() << std::endl;
        return ERROR_NOINTERPRETER;
    }

    // First we need a Action and fill it.
    Kross::Action* action = new Kross::Action(0 /*no parent*/, QUrl::fromUserInput(scriptfile));
    action->setInterpreter( interpreterinfo->interpreterName() );
    action->setCode(scriptcode);

    // Now execute the Action.
    action->trigger();

    if(action->hadError()) {
        // We had an exception.
        std::cerr << QString("%2\n%1").arg(action->errorTrace()).arg(action->errorMessage()).toLatin1().data() << std::endl;
        delete action;
        return ERROR_EXCEPTION;
    }

    delete action;
    return ERROR_OK;
}

int main(int argc, char **argv)
{
    app = new QApplication(argc, argv);
    app->setApplicationName("kross");
    app->setApplicationVersion("0.1");
    app->setOrganizationDomain("dipe.org");

    int result = ERROR_OK;

    /*
    K4AboutData about("kross",
                     "kdelibs4",
                     ki18n("Kross"),
                     "0.1",
                     ki18n("KDE application to run Kross scripts."),
                     K4AboutData::License_LGPL,
                     ki18n("(C) 2006 Sebastian Sauer"),
                     ki18n("Run Kross scripts."),
                     "http://kross.dipe.org",
                     "kross@dipe.org");*/
    //about.addAuthor(ki18n("Sebastian Sauer"), ki18n("Author"), "mail@dipe.org");

    KLocalizedString::setApplicationCatalog("kdelibs4");

    // Initialize command line args
    // Tell which options are supported and parse them.
    QCommandLineParser parser;
    parser.addHelpOption(QCoreApplication::translate("main", "KDE application to run Kross scripts."));
    // TODO parser->addOption(QCommandLineOption(QStringList() << "+file", QCoreApplication::translate("main", "Scriptfile")));
    parser.process(*app);


    const QStringList args = parser.remainingArguments();
    // If no options are defined.
    if (args.count() < 1) {
        parser.showHelp();
        //std::cout << "Syntax: " << "kross" << " scriptfile1 [scriptfile2] [scriptfile3] ..." << std::endl;
        return ERROR_HELP;
    }

    // Each argument is a scriptfile to open
    for (int i = 0; i < args.count(); i++) {
        result = runScriptFile(args.at(i));
        if(result != ERROR_OK)
            break;
    }

    // Free the QApplication instance and exit the program.
    delete app;
    Kross::Manager::self().deleteModules();
    return result;
}
