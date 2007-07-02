/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <phonon/ui/videoplayer.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kurl.h>
#include <klocale.h>

using namespace Phonon;

int main(int argc, char ** argv)
{
    KCmdLineOptions options;
    options.add("+URL", ki18n("An URL to a video"));

    KAboutData about("videoplayandforget", 0, ki18n("Phonon VideoPlayer Example"),
            "1.0", KLocalizedString(),
            KAboutData::License_LGPL);
    about.addAuthor(ki18n("Matthias Kretz"), KLocalizedString(), "kretz@kde.org");
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;
    KUrl url;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() == 1)
    {
        url = args->url(0);
        if (url.isValid())
        {
            VideoPlayer player(Phonon::VideoCategory);
            QObject::connect(&player, SIGNAL(finished()), &app, SLOT(quit()));
            player.show();
            player.resize(640, 480);
            player.play(url);
            player.seek(player.totalTime() * 9 / 10);
            return app.exec();
        }
    }
    return 1;
}
