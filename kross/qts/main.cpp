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

#include <QFile>
#include <QStringList>
#include <QScriptEngine>
#include <QLibraryInfo>
#include <QDebug>
#include <QApplication>

#include <qcommandlineparser.h>
#include <qcommandlineoption.h>

bool runScriptFile(QScriptEngine* engine, const QString& scriptfile)
{
    // Read the scriptfile
    QFile f(scriptfile);
    if(! f.exists()) {
        qWarning() << "No such scriptfile:" << scriptfile;
        return false;
    }
    if(! f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open scriptfile:" << scriptfile;
        return false;
    }
    QByteArray scriptcode = f.readAll();
    f.close();

    // Execute the javascript code.
    qDebug() << "Execute scriptfile:" << scriptfile;
    QScriptValue v = engine->evaluate(scriptcode);
    qDebug() << "Execute done. Result:" << v.toString();

    return true;
}

int main(int argc, char **argv)
{
    QApplication *app = new QApplication(argc, argv);
    app->setApplicationName("kross");
    app->setApplicationVersion("0.1");
    app->setOrganizationDomain("dipe.org");

    QScriptEngine* engine = new QScriptEngine();

    /*K4AboutData about("kross",0,ki18n("Kross"),"0.1",
                     ki18n("KDE application to run Kross scripts."),
                     K4AboutData::License_LGPL,
                     ki18n("(C) 2006 Sebastian Sauer"),
                     ki18n("Run Kross scripts."),
                     "http://kross.dipe.org","kross@dipe.org");*/
    //about.addAuthor(ki18n("Sebastian Sauer"), ki18n("Author"), "mail@dipe.org");

    // Initialize command line args
    // Tell which options are supported and parse them.
    QCommandLineParser parser;
    parser.addHelpOption(QCoreApplication::translate("main", "KDE application to run Kross scripts."));
    parser.addOption(QCommandLineOption(QStringList() << "+file", QCoreApplication::translate("main", "Scriptfile")));
    parser.process(*app);

    const QStringList args = parser.remainingArguments();
    // If no options are defined.
    if (args.count() < 1) {
        parser.showHelp();
        //std::cout << "Syntax: " << "kross" << " scriptfile1 [scriptfile2] [scriptfile3] ..." << std::endl;
        return -1;
    }

    QScriptValue global = engine->globalObject();

    //qDebug()<<"QLibraryInfo::PluginsPath="<<QLibraryInfo::location(QLibraryInfo::PluginsPath);

    engine->importExtension("kross").toString();

    foreach(const QString &file, args)
        runScriptFile(engine, file);

    delete engine;
    delete app;
    return 0;
}
