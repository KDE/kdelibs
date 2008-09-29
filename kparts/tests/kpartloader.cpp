/*  This file is part of the KDE libraries
 *  Copyright 2008  David Faure  <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kpartloader.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kicon.h>

KPartLoaderWindow::KPartLoaderWindow(const QString& partLib)
{
    setXMLFile("kpartloaderui.rc");

    KAction * paQuit = new KAction( KIcon("application-exit"), "&Quit", this );
    actionCollection()->addAction( "file_quit", paQuit );
    connect(paQuit, SIGNAL(triggered()), this, SLOT(close()));

    KPluginLoader loader(partLib);
    KPluginFactory* factory = loader.factory();
    if (factory) {
        // Create the part
        m_part = factory->create<KParts::ReadOnlyPart>(this, this);
    } else {
        KMessageBox::error(this, i18n("No part named %1 found!", partLib));
    }

    if (m_part) {
        setCentralWidget( m_part->widget() );
        // Integrate its GUI
        createGUI( m_part );
    }

    // Set a reasonable size
    resize( 600, 350 );
}

KPartLoaderWindow::~KPartLoaderWindow()
{
}

#include <kapplication.h>
#include <kcmdlineargs.h>

int main( int argc, char **argv )
{
    KCmdLineOptions options;
    options.add("+part", ki18n("Name of the part to load, e.g. dolphinpart"));

    const char version[] = "v 1.0";
    KLocalizedString description = ki18n("This is a test application for KParts.");

    KCmdLineArgs::init(argc, argv, "kpartloader", 0, ki18n("kpartloader"), version, description);
    KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.
    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() == 1 )
    {
        KPartLoaderWindow *shell = new KPartLoaderWindow(args->arg(0));
        shell->show();
        return app.exec();
    }
    return -1;
}

#include "kpartloader.moc"
