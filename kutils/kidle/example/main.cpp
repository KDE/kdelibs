/***************************************************************************
 *   Copyright (C) 2009 by Dario Freddi <drf@kde.org>                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
#include "KIdleTest.h"

int main(int argc, char **argv)
{
    KAboutData *data = new KAboutData("KIdleTest", "KIdleTest", ki18n("KIdleTest"), "1.0",
                                      ki18n("A small application to test KIdleTime functionality"),
                                      KAboutData::License_GPL , ki18n("2009 Dario Freddi"));
    data->addAuthor(ki18n("Dario Freddi"), ki18n("Maintainer"), "drf@kde.org", "http://drfav.wordpress.com");

    KCmdLineArgs::init(argc, argv, data);
    KApplication app;

    KIdleTest t;

    return app.exec();
}
