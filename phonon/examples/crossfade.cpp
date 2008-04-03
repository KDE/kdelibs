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

#include "crossfade.h"
#include <phonon/mediaqueue.h>
#include <phonon/audiopath.h>
#include <phonon/audiooutput.h>
#include <phonon/ui/seekslider.h>
#include <kurl.h>
#include <QtCore/QTimer>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>

Crossfader::Crossfader(const KUrl &url1, const KUrl &url2, QObject *parent)
    : QObject(parent)
    , media(new MediaQueue(this))
    , path(new AudioPath(this))
    , output(new AudioOutput(Phonon::MusicCategory, this))
{
    media->addAudioPath(path);
    path->addOutput(output);
    media->setUrl(url1);
    media->setNextUrl(url2);
    connect(media, SIGNAL(finished()), SLOT(finished()));
    SeekSlider *slider = new SeekSlider();
    slider->setMediaProducer(media);
    slider->resize(1000, 50);
    slider->show();
    media->play();
    media->seek(media->totalTime() * 97 / 100);
    //QTimer::singleShot(0, media, SLOT(play()));
}

void Crossfader::finished()
{
    static int count = 0;
    ++count;
    if (count == 2)
    {
        qApp->quit();
    }
}

int main(int argc, char ** argv)
{
    KCmdLineOptions options;
    options.add("+URL1", ki18n("The first URL to play"));
    options.add("+URL2", ki18n("The second URL to play"));

    KAboutData about("crossfade", 0, ki18n("Phonon Crossfade Example"),
            "1.0", KLocalizedString(),
            KAboutData::License_LGPL);
    about.addAuthor(ki18n("Matthias Kretz"), KLocalizedString(), "kretz@kde.org");
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;
    KUrl url1;
    KUrl url2;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() == 2)
    {
        url1 = args->url(0);
        url2 = args->url(1);
        if (url1.isValid() && url2.isValid())
        {
            Crossfader xfader(url1, url2);
            return app.exec();
        }
    }
    return 1;
}

#include "crossfade.moc"
