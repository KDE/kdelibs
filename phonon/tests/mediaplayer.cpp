/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "mediaplayer.h"
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <cstdlib>
#include <QtGui/QBoxLayout>
#include <QtGui/QPushButton>
#include "../backendcapabilities.h"
#include <QtGui/QSlider>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <kicon.h>
#include <QtGui/QAction>

using namespace Phonon;

MediaPlayer::MediaPlayer(QWidget *parent)
    : QWidget(parent)
    , m_effectWidget(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_vwidget = new VideoWidget(this);
    layout->addWidget(m_vwidget);
    //m_vwidget->hide();

    QAction *fullScreenAction = new QAction(m_vwidget);
    fullScreenAction->setShortcut(Qt::Key_F);
    fullScreenAction->setCheckable(true);
    fullScreenAction->setChecked(false);
    m_vwidget->addAction(fullScreenAction);
    connect(fullScreenAction, SIGNAL(toggled(bool)), m_vwidget, SLOT(setFullScreen(bool)));

    m_vpath = new VideoPath(this);

    m_aoutput = new AudioOutput(Phonon::VideoCategory, this);
    m_apath = new AudioPath(this);

    m_media = new MediaObject(this);
    m_media->addVideoPath(m_vpath);
    m_vpath->addOutput(m_vwidget);
    m_media->addAudioPath(m_apath);
    m_apath->addOutput(m_aoutput);

    m_controls = new MediaControls(this);
    layout->addWidget(m_controls);
    m_controls->setMediaObject(m_media);
    m_controls->setAudioOutput(m_aoutput);

    /*
    QList<AudioEffectDescription> effectList = BackendCapabilities::availableAudioEffects();
    if (!effectList.isEmpty())
    {
        m_effect = new AudioEffect(BackendCapabilities::availableAudioEffects().first(), m_apath);
        m_apath->insertEffect(m_effect);
        QPushButton *button = new QPushButton(this);
        layout->addWidget(button);
        button->setText("configure effect");
        connect(button, SIGNAL(clicked()), SLOT(openEffectWidget()));
    }
    */

    m_brightness = new BrightnessControl(this);
    QSlider *slider = new QSlider(this);
    layout->addWidget(slider);
    slider->setOrientation(Qt::Horizontal);
    slider->setRange(m_brightness->lowerBound(), m_brightness->upperBound());
    slider->setValue(m_brightness->brightness());
    connect(slider, SIGNAL(valueChanged(int)), m_brightness, SLOT(setBrightness(int)));
    m_vpath->insertEffect(m_brightness);

    QCheckBox *deinterlaceCheck = new QCheckBox(this);
    layout->addWidget(deinterlaceCheck);
    connect(deinterlaceCheck, SIGNAL(toggled(bool)), SLOT(toggleDeinterlacing(bool)));

    QCheckBox *scaleModeCheck = new QCheckBox(this);
    layout->addWidget(scaleModeCheck);
    connect(scaleModeCheck, SIGNAL(toggled(bool)), SLOT(toggleScaleMode(bool)));

    QComboBox *aspectRatioCombo = new QComboBox(this);
    layout->addWidget(aspectRatioCombo);
    connect(aspectRatioCombo, SIGNAL(currentIndexChanged(int)), SLOT(switchAspectRatio(int)));
    aspectRatioCombo->addItem("AspectRatioAuto");
    aspectRatioCombo->addItem("AspectRatioWidget");
    aspectRatioCombo->addItem("AspectRatio4_3");
    aspectRatioCombo->addItem("AspectRatio16_9");

    this->resize(width(), height() + 240 - m_vwidget->height());
}

void MediaPlayer::switchAspectRatio(int x)
{
    m_vwidget->setAspectRatio(static_cast<VideoWidget::AspectRatio>(x));
}

void MediaPlayer::toggleScaleMode(bool mode)
{
    if (mode) {
        m_vwidget->setScaleMode(VideoWidget::ScaleAndCrop);
    } else {
        m_vwidget->setScaleMode(VideoWidget::FitInView);
    }
}

void MediaPlayer::toggleDeinterlacing(bool deint)
{
    if (deint) {
        m_deinterlaceFilter = new DeinterlaceFilter(this);
        m_vpath->insertEffect(m_deinterlaceFilter);
    } else {
        delete m_deinterlaceFilter;
    }
}

void MediaPlayer::openEffectWidget()
{
    if (!m_effectWidget)
        m_effectWidget = new EffectWidget(m_effect);
    m_effectWidget->show();
    m_effectWidget->raise();
}

void MediaPlayer::setUrl(const KUrl &url)
{
    m_media->setCurrentSource(url);
    //m_vwidget->setVisible(m_media->hasVideo());
}

int main(int argc, char ** argv)
{
    KAboutData about("phononmediaplayer", 0, ki18n("Phonon Media Player"),
            "0.1", ki18n("Media Player"),
            KAboutData::License_GPL);
    about.addAuthor(ki18n("Matthias Kretz"), KLocalizedString(), "kretz@kde.org");
    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;
    MediaPlayer foo;
    foo.setWindowIcon(KIcon("phonon"));
    foo.show();
    KUrl url(getenv("PHONON_TESTURL"));
    if (url.isValid())
    {
        foo.setUrl(url);
        return app.exec();
    }
    return 1;
}

// vim: sw=4 ts=4
#include "mediaplayer.moc"
#include "mediacontrols.cpp"
#include "mediacontrols.moc"
