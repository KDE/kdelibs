/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "knewstuff2_standard.h"

#include <knewstuff2/engine.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

KNewStuff2Standard::KNewStuff2Standard()
        : QObject()
{
    m_engine = NULL;
}

void KNewStuff2Standard::run(bool upload, bool modal, QString file)
{
    kDebug() << "-- test kns2 engine";

    m_engine = new KNS::Engine();
    bool success = m_engine->init("knewstuff2_test.knsrc");

    kDebug() << "-- engine test result: " << success;

    if (!success)
        return;

    if (upload) {
        if (modal) {
            kDebug() << "-- start upload (modal)";
            m_engine->uploadDialogModal(file);
            kDebug() << "-- upload (modal) finished";
        } else {
            kDebug() << "-- start upload (non-modal); will not block";
            m_engine->uploadDialog(file);
        }
    } else {
        if (modal) {
            kDebug() << "-- start download (modal)";
            m_engine->downloadDialogModal();
            kDebug() << "-- download (modal) finished";
        } else {
            kDebug() << "-- start download (non-modal); will not block";
            m_engine->downloadDialog();
        }
    }
}

int main(int argc, char **argv)
{
    KCmdLineOptions options;
    options.add("upload <file>", ki18n("Tests upload dialog"));
    options.add("download", ki18n("Tests download dialog"));
    options.add("modal", ki18n("Show modal dialogs"));

    KCmdLineArgs::init(argc, argv, "knewstuff2_standard", 0, ki18n("knewstuff2_standard"), 0);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    // Take source directory into account
    kDebug() << "-- adding source directory " << KNSSRCDIR;
    kDebug() << "-- adding build directory " << KNSBUILDDIR;
    KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);
    KGlobal::dirs()->addResourceDir("config", KNSBUILDDIR);

    KNewStuff2Standard *standard = new KNewStuff2Standard();
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool modal = false;
    if (args->isSet("modal")) {
        modal = true;
    }
    if (args->isSet("upload")) {
        standard->run(true, modal, args->getOption("upload"));
    } else if (args->isSet("download")) {
        standard->run(false, modal, QString());
    } else {
        return -1;
    }

    return app.exec();
}

#include "knewstuff2_standard.moc"
