/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mediacontrols.h"

#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>

#include <phonon/audiooutput.h>
#include <phonon/backendcapabilities.h>
#include <phonon/effect.h>
#include <phonon/effectwidget.h>
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/videowidget.h>

#include <kaboutdata.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kicon.h>
#include <kurl.h>

#include <cstdlib>

using Phonon::MediaObject;
using Phonon::MediaSource;
using Phonon::AudioOutput;
using Phonon::VideoWidget;
using Phonon::MediaControls;
using Phonon::Effect;
using Phonon::EffectWidget;

class MediaPlayer : public QMainWindow
{
    Q_OBJECT
    public:
        MediaPlayer();
        void setUrl(const KUrl &url);

    private Q_SLOTS:
        void stateChanged(Phonon::State newstate);
        void workaroundQtBug();
        void getNextUrl();
        void startupReady();
        void openEffectWidget();
        void toggleScaleMode(bool);
        void switchAspectRatio(int x);
        void setBrightness(int b);

    private:
        bool setNextSource();

        QWidget *m_settingsWidget;
        Phonon::MediaObject *m_media;
        Phonon::Path m_apath;
        Phonon::AudioOutput *m_aoutput;
        Phonon::Path m_vpath;
        Phonon::Effect *m_effect;
        Phonon::VideoWidget *m_vwidget;
        Phonon::MediaControls *m_controls;
        Phonon::EffectWidget *m_effectWidget;
        QAction *m_fullScreenAction;
};

MediaPlayer::MediaPlayer()
    : QMainWindow(0), m_effectWidget(0)
{
    QDockWidget *dock = new QDockWidget(this);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea);

    m_settingsWidget = new QWidget(dock);
    dock->setWidget(m_settingsWidget);
    addDockWidget(Qt::BottomDockWidgetArea, dock);

    QVBoxLayout *layout = new QVBoxLayout(m_settingsWidget);

    m_vwidget = new VideoWidget(this);
    setCentralWidget(m_vwidget);

    m_fullScreenAction = new QAction(m_vwidget);
    m_fullScreenAction->setShortcut(Qt::Key_F);
    m_fullScreenAction->setCheckable(true);
    m_fullScreenAction->setChecked(false);
    this->addAction(m_fullScreenAction);
    connect(m_fullScreenAction, SIGNAL(toggled(bool)), m_vwidget, SLOT(setFullScreen(bool)));
    connect(m_fullScreenAction, SIGNAL(toggled(bool)), SLOT(workaroundQtBug()));

    m_aoutput = new AudioOutput(Phonon::VideoCategory, this);

    m_media = new MediaObject(this);
    connect(m_media, SIGNAL(finished()), SLOT(getNextUrl()));
    connect(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), SLOT(stateChanged(Phonon::State)));

    createPath(m_media, m_vwidget);
    m_apath = createPath(m_media, m_aoutput);

    m_controls = new MediaControls(m_settingsWidget);
    layout->addWidget(m_controls);
    m_controls->setMediaObject(m_media);
    m_controls->setAudioOutput(m_aoutput);
    m_controls->setMaximumHeight(28);

    /*
    QList<AudioEffectDescription> effectList = BackendCapabilities::availableAudioEffects();
    if (!effectList.isEmpty())
    {
        m_effect = new AudioEffect(BackendCapabilities::availableAudioEffects().first(), m_apath);
        m_apath->insertEffect(m_effect);
        QPushButton *button = new QPushButton(m_settingsWidget);
        layout->addWidget(button);
        button->setText("configure effect");
        connect(button, SIGNAL(clicked()), SLOT(openEffectWidget()));
    }
    */

    QSlider *slider = new QSlider(m_settingsWidget);
    layout->addWidget(slider);
    slider->setOrientation(Qt::Horizontal);
    slider->setRange(-100, 100);
    slider->setValue(static_cast<int>(m_vwidget->brightness() * 100));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setBrightness(int)));

    QCheckBox *scaleModeCheck = new QCheckBox(m_settingsWidget);
    layout->addWidget(scaleModeCheck);
    connect(scaleModeCheck, SIGNAL(toggled(bool)), SLOT(toggleScaleMode(bool)));

    QWidget *box = new QWidget(m_settingsWidget);
    layout->addWidget(box);

    QLabel *label = new QLabel("Aspect Ratio:", box);
    label->setAlignment(Qt::AlignRight);
    QComboBox *aspectRatioCombo = new QComboBox(box);
    QHBoxLayout *hbox = new QHBoxLayout(box);
    hbox->addWidget(label);
    hbox->addWidget(aspectRatioCombo);
    label->setBuddy(aspectRatioCombo);

    connect(aspectRatioCombo, SIGNAL(currentIndexChanged(int)), SLOT(switchAspectRatio(int)));
    aspectRatioCombo->addItem("auto");
    aspectRatioCombo->addItem("fit");
    aspectRatioCombo->addItem("4:3");
    aspectRatioCombo->addItem("16:9");

    this->resize(width(), height() + 240 - m_vwidget->height());

    QTimer::singleShot(0, this, SLOT(startupReady()));
}

void MediaPlayer::stateChanged(Phonon::State newstate)
{
    switch (newstate) {
    case Phonon::ErrorState:
    case Phonon::StoppedState:
        getNextUrl();
        break;
    default:
        break;
    }
}

void MediaPlayer::workaroundQtBug()
{
    kDebug();
    if (m_vwidget->actions().contains(m_fullScreenAction)) {
        m_vwidget->removeAction(m_fullScreenAction);
        this->addAction(m_fullScreenAction);
    } else {
        this->removeAction(m_fullScreenAction);
        m_vwidget->addAction(m_fullScreenAction);
    }
}

bool MediaPlayer::setNextSource()
{
    QWidget *extraWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(extraWidget);
    layout->setMargin(0);
    layout->addStretch();
    QPushButton *dvdButton = new QPushButton(i18n("DVD"), extraWidget);
    dvdButton->setCheckable(true);
    dvdButton->setChecked(false);
    layout->addWidget(dvdButton);
    QPushButton *cdButton = new QPushButton(i18n("Audio CD"), extraWidget);
    cdButton->setCheckable(true);
    cdButton->setChecked(false);
    layout->addWidget(cdButton);
    const QString dummyString;
    const KUrl dummyUrl;
    KFileDialog dlg(dummyUrl, dummyString, 0, extraWidget);
    connect(dvdButton, SIGNAL(toggled(bool)), &dlg, SLOT(accept()));
    connect(cdButton, SIGNAL(toggled(bool)), &dlg, SLOT(accept()));
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setWindowTitle(i18n("Open"));
    dlg.setMode(KFile::File);
    dlg.exec();
    KUrl url = dlg.selectedUrl();

    if (dvdButton->isChecked()) {
        if (url.isLocalFile()) {
            m_media->setCurrentSource(MediaSource(Phonon::Dvd, url.path()));
        } else {
            m_media->setCurrentSource(Phonon::Dvd);
        }
    } else if (cdButton->isChecked()) {
        m_media->setCurrentSource(Phonon::Cd);
    } else if (url.isValid()) {
        m_media->setCurrentSource(url);
    } else {
        QApplication::instance()->quit();
        return false;
    }
    return true;
}

void MediaPlayer::getNextUrl()
{
    static bool fileDialogAlreadyOpen = false;
    if (fileDialogAlreadyOpen) {
        return;
    }
    fileDialogAlreadyOpen = true;
    if (!setNextSource()) {
        return;
    }
    m_media->play();
    fileDialogAlreadyOpen = false;
}

void MediaPlayer::startupReady()
{
    if (m_media->currentSource().type() == MediaSource::Invalid) {
        if (!setNextSource()) {
            return;
        }
    }
    m_media->play();
}

void MediaPlayer::setBrightness(int b)
{
    m_vwidget->setBrightness(b * 0.01);
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
    KCmdLineOptions options;
    options.add("+[url]", ki18n("File to play"));
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;
    MediaPlayer foo;
    foo.setWindowIcon(KIcon("phonon"));
    foo.show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() == 1) {
        KUrl url = args->url(0);
        if (url.isValid()) {
            foo.setUrl(url);
        }
    }
    args->clear();
    return app.exec();
}

#include "mediaplayer.moc"
#include "mediacontrols.cpp"
#include "moc_mediacontrols.cpp"
