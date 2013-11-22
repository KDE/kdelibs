/* This file is part of the KDE libraries
   Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

   library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation, version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfileaudiopreview.h"

#include <QCheckBox>
#include <QLayout>
#include <QGroupBox>

#include <kconfig.h>
#define TRANSLATION_DOMAIN "kfileaudiopreview5"
#include <klocalizedstring.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <phonon/path.h>
#include <phonon/backendcapabilities.h>
#include <phonon/videowidget.h>
#include "mediacontrols.h"
#include <kconfiggroup.h>

K_PLUGIN_FACTORY(KFileAudioPreviewFactory, registerPlugin<KFileAudioPreview>();)

#define ConfigGroup "Audio Preview Settings"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

using namespace Phonon;

class KFileAudioPreview::Private
{
public:
    Private()
        : player(0)
        , audioOutput(0)
        , videoWidget(0) {
    }

    MediaObject* player;
    AudioOutput* audioOutput;
    VideoWidget* videoWidget;
    MediaControls* controls;
};


KFileAudioPreview::KFileAudioPreview(QWidget *parent, const QVariantList &)
    : KPreviewWidgetBase(parent)
    , d(new Private)
{
    setSupportedMimeTypes(BackendCapabilities::availableMimeTypes());

    d->audioOutput = new AudioOutput(Phonon::NoCategory, this);

    d->videoWidget = new VideoWidget(this);
    d->videoWidget->hide();

    d->controls = new MediaControls(this);
    d->controls->setEnabled(false);
    d->controls->setAudioOutput(d->audioOutput);

    m_autoPlay = new QCheckBox(i18n("Play &automatically"), this);
    KConfigGroup config(KSharedConfig::openConfig(), ConfigGroup);
    m_autoPlay->setChecked(config.readEntry("Autoplay", true));
    connect(m_autoPlay, SIGNAL(toggled(bool)), SLOT(toggleAuto(bool)));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(d->videoWidget);
    layout->addWidget(d->controls);
    layout->addWidget(m_autoPlay, 0, Qt::AlignHCenter);
    layout->addStretch();
}

KFileAudioPreview::~KFileAudioPreview()
{
    KConfigGroup config(KSharedConfig::openConfig(), ConfigGroup);
    config.writeEntry("Autoplay", m_autoPlay->isChecked());

    delete d;
}

void KFileAudioPreview::stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    if (oldstate == Phonon::LoadingState && newstate != Phonon::ErrorState)
        d->controls->setEnabled(true);
}

void KFileAudioPreview::showPreview(const QUrl &url)
{
    d->controls->setEnabled(false);
    if (!d->player) {
        d->player = new MediaObject(this);
        Phonon::createPath(d->player, d->videoWidget);
        Phonon::createPath(d->player, d->audioOutput);
        connect(d->player, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
                SLOT(stateChanged(Phonon::State, Phonon::State)));
        d->videoWidget->setVisible(d->player->hasVideo());
        connect(d->player, SIGNAL(hasVideoChanged(bool)), d->videoWidget, SLOT(setVisible(bool)));
        d->controls->setMediaObject(d->player);
    }
    d->player->setCurrentSource(url);

    if (m_autoPlay->isChecked())
        d->player->play();
}

void KFileAudioPreview::clearPreview()
{
    if (d->player) {
        delete d->player;
        d->player = 0;
        d->controls->setEnabled(false);
    }
}

void KFileAudioPreview::toggleAuto(bool on)
{
    if (!d->player)
        return;

    if (on && d->controls->isEnabled())
        d->player->play();
    else
        d->player->stop();
}

#include "kfileaudiopreview.moc"
