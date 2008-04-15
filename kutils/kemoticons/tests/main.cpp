/***************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include <kemoticons.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <qstring.h>

int main(int argc, char **argv)
{
    KAboutData aboutData("kemoticonstest", 0, ki18n("KEmoticons Test"), "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;
    
    KEmoticons e;
    QStringList tl = e.getThemeList();
    kDebug() << "ThemeList:" << tl;
    KEmoticonsTheme *t = e.getTheme(tl.at(0));
    kDebug() << "theme:" << t->themeName();
    
    QList<KService::Ptr> srv = e.loadedServices();
    foreach (KService::Ptr service, srv) {
        kDebug()<<"name:"<<service->name();
    }
    
    delete t;
    
    return 0;
}

