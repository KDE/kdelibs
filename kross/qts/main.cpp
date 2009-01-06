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
#include <QScriptEngine>
#include <QLibraryInfo>
#include <QDebug>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
// #include <kurl.h>

KApplication* app = 0;

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
    KAboutData about("kross",0,ki18n("Kross"),"0.1",
                     ki18n("KDE application to run Kross scripts."),
                     KAboutData::License_LGPL,
                     ki18n("(C) 2006 Sebastian Sauer"),
                     ki18n("Run Kross scripts."),
                     "http://kross.dipe.org","kross@dipe.org");
    about.addAuthor(ki18n("Sebastian Sauer"), ki18n("Author"), "mail@dipe.org");

    // Initialize command line args
    KCmdLineArgs::init(argc, argv, &about);
    // Tell which options are supported and parse them.
    KCmdLineOptions options;
    options.add("+file", ki18n("Scriptfile"));
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QStringList files;
    for(int i = 0; i < args->count(); ++i)
        files << args->arg(i);

    // If no options are defined.
    if(files.count() < 1) {
        qWarning() << "Syntax:" << KCmdLineArgs::appName() << "scriptfile1 [scriptfile2] [scriptfile3] ...";
        return -1;
    }


    app = new KApplication( /* GUIenabled */ true );
    QScriptEngine* engine = new QScriptEngine();
    QScriptValue global = engine->globalObject();

    //qDebug()<<"QLibraryInfo::PluginsPath="<<QLibraryInfo::location(QLibraryInfo::PluginsPath);
    //app->addLibraryPath("/home/kde4/kde4/lib/kde4/");

    engine->importExtension("kross").toString();

    foreach(const QString &file, files)
        runScriptFile(engine, file);

    delete engine;
    delete app;
    return 0;
}
