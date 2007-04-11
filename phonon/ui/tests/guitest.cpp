/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#include "guitest.h"

#include <phonon/mediaobject.h>
#include <phonon/audiopath.h>
#include <phonon/audiooutput.h>
#include <phonon/videopath.h>
#include <phonon/backendcapabilities.h>
#include <phonon/volumefadereffect.h>
#include <phonon/ui/videowidget.h>
#include <phonon/ui/effectwidget.h>

#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klineedit.h>
#include <kurlcompletion.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>

#include <cstdlib>
#include <phonon/ui/seekslider.h>
#include <phonon/ui/volumeslider.h>
#include <QTime>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QComboBox>
#include <phonon/objectdescriptionmodel.h>
#include <kicon.h>
#include <QToolButton>
#include <QProgressBar>
#include <kmessagebox.h>
#include <QGroupBox>
#include <QSpinBox>

using namespace Phonon;

TrackWidget::TrackWidget(QWidget *parent)
    : QWidget(parent), m_iface(0)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    QLabel *l = new QLabel("current track:", this);
    m_currentTrack = new QSpinBox(this);
    hlayout->addWidget(l);
    hlayout->addWidget(m_currentTrack);
    l->setBuddy(m_currentTrack);
    m_currentTrack->setRange(1, 1);

    hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    l = new QLabel("available tracks:", this);
    m_availableTracks = new QLabel("0", this);
    hlayout->addWidget(l);
    hlayout->addWidget(m_availableTracks);

    hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    l = new QLabel("autoplay:", this);
    m_autoplay = new QToolButton(this);
    m_autoplay->setCheckable(true);
    hlayout->addWidget(l);
    hlayout->addWidget(m_autoplay);
}

TrackWidget::~TrackWidget()
{
}

void TrackWidget::setInterface(TrackInterface *i)
{
    if (m_iface) {
        disconnect(m_iface, 0, m_currentTrack, 0);
        disconnect(m_currentTrack, 0, m_iface, 0);
        disconnect(m_iface, 0, m_availableTracks, 0);
        disconnect(m_autoplay, 0, m_iface, 0);
    }
    m_iface = i;
    if (m_iface) {
        const int a = m_iface->availableTracks();
        m_currentTrack->setMaximum(a);
        m_availableTracks->setNum(a);
        m_currentTrack->setValue(m_iface->currentTrack());
        m_autoplay->setChecked(m_iface->autoplayTracks());

        connect(m_iface, SIGNAL(availableTracksChanged(int)), SLOT(availableTracksChanged(int)));
        connect(m_iface, SIGNAL(availableTracksChanged(int)), m_availableTracks, SLOT(setNum(int)));
        connect(m_iface, SIGNAL(trackChanged(int)), m_currentTrack, SLOT(setValue(int)));
        connect(m_currentTrack, SIGNAL(valueChanged(int)), m_iface, SLOT(setCurrentTrack(int)));
        connect(m_autoplay, SIGNAL(toggled(bool)), m_iface, SLOT(setAutoplayTracks(bool)));
    }
}

void TrackWidget::availableTracksChanged(int x) { m_currentTrack->setMaximum(x); }

ChapterWidget::ChapterWidget(QWidget *parent)
    : QWidget(parent), m_iface(0)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    QLabel *l = new QLabel("current chapter:", this);
    m_currentChapter = new QSpinBox(this);
    hlayout->addWidget(l);
    hlayout->addWidget(m_currentChapter);
    l->setBuddy(m_currentChapter);
    m_currentChapter->setRange(1, 1);

    hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    l = new QLabel("available chapters:", this);
    m_availableChapters = new QLabel("0", this);
    hlayout->addWidget(l);
    hlayout->addWidget(m_availableChapters);
}

ChapterWidget::~ChapterWidget()
{
}

void ChapterWidget::setInterface(ChapterInterface *i)
{
    if (m_iface) {
        disconnect(m_iface, 0, m_currentChapter, 0);
        disconnect(m_currentChapter, 0, m_iface, 0);
        disconnect(m_iface, 0, m_availableChapters, 0);
    }
    m_iface = i;
    if (m_iface) {
        connect(m_iface, SIGNAL(availableChaptersChanged(int)), SLOT(availableChaptersChanged(int)));
        connect(m_iface, SIGNAL(availableChaptersChanged(int)), m_availableChapters, SLOT(setNum(int)));
        connect(m_iface, SIGNAL(chapterChanged(int)), m_currentChapter, SLOT(setValue(int)));
        connect(m_currentChapter, SIGNAL(valueChanged(int)), m_iface, SLOT(setCurrentChapter(int)));
    }
}

void ChapterWidget::availableChaptersChanged(int x) { m_currentChapter->setMaximum(x); }

AngleWidget::AngleWidget(QWidget *parent)
    : QWidget(parent), m_iface(0)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    QLabel *l = new QLabel("current angle:", this);
    m_currentAngle = new QSpinBox(this);
    hlayout->addWidget(l);
    hlayout->addWidget(m_currentAngle);
    l->setBuddy(m_currentAngle);
    m_currentAngle->setRange(1, 1);

    hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    l = new QLabel("available angles:", this);
    m_availableAngles = new QLabel("0", this);
    hlayout->addWidget(l);
    hlayout->addWidget(m_availableAngles);
}

AngleWidget::~AngleWidget()
{
}

void AngleWidget::setInterface(AngleInterface *i)
{
    if (m_iface) {
        disconnect(m_iface, 0, m_currentAngle, 0);
        disconnect(m_currentAngle, 0, m_iface, 0);
        disconnect(m_iface, 0, m_availableAngles, 0);
    }
    m_iface = i;
    if (m_iface) {
        connect(m_iface, SIGNAL(availableAnglesChanged(int)), SLOT(availableAnglesChanged(int)));
        connect(m_iface, SIGNAL(availableAnglesChanged(int)), m_availableAngles, SLOT(setNum(int)));
        connect(m_iface, SIGNAL(angleChanged(int)), m_currentAngle, SLOT(setValue(int)));
        connect(m_currentAngle, SIGNAL(valueChanged(int)), m_iface, SLOT(setCurrentAngle(int)));
    }
}

void AngleWidget::availableAnglesChanged(int x) { m_currentAngle->setMaximum(x); }

NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent), m_iface(0)
{
    setAttribute(Qt::WA_QuitOnClose, false);
}

NavigationWidget::~NavigationWidget()
{
}

void NavigationWidget::setInterface(NavigationInterface *i)
{
    if (m_iface) {
    }
    m_iface = i;
    if (m_iface) {
    }
}

OutputWidget::OutputWidget(QWidget *parent)
    : QFrame(parent)
    , m_output(new AudioOutput(Phonon::MusicCategory, this))
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Sunken);

    m_output->setName("GUI-Test");

    QVBoxLayout *vlayout = new QVBoxLayout(this);

    QComboBox *deviceComboBox = new QComboBox(this);
    vlayout->addWidget(deviceComboBox);
    QList<AudioOutputDevice> deviceList = BackendCapabilities::availableAudioOutputDevices();
    deviceComboBox->setModel(new AudioOutputDeviceModel(deviceList, deviceComboBox));
    deviceComboBox->setCurrentIndex(deviceList.indexOf(m_output->outputDevice()));
    connect(deviceComboBox, SIGNAL(currentIndexChanged(int)), SLOT(deviceChange(int)));

    m_volslider = new VolumeSlider(this);
    m_volslider->setOrientation(Qt::Vertical);
    m_volslider->setAudioOutput(m_output);
    vlayout->addWidget(m_volslider);
}

void OutputWidget::deviceChange(int modelIndex)
{
    QList<AudioOutputDevice> deviceList = BackendCapabilities::availableAudioOutputDevices();
    if (modelIndex >= 0 && modelIndex < deviceList.size()) {
        m_output->setOutputDevice(deviceList[modelIndex]);
    }
}

PathWidget::PathWidget(QWidget *parent)
    : QFrame(parent)
    , m_path(new AudioPath(this))
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Raised);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_effectComboBox = new QComboBox(this);
    layout->addWidget(m_effectComboBox);
    QList<AudioEffectDescription> effectList = BackendCapabilities::availableAudioEffects();
    m_effectComboBox->setModel(new AudioEffectDescriptionModel(effectList, m_effectComboBox));

    QPushButton *addButton = new QPushButton(this);
    layout->addWidget(addButton);
    addButton->setText("add effect");
    connect(addButton, SIGNAL(clicked()), SLOT(addEffect()));

    QPushButton *button = new QPushButton(this);
    layout->addWidget(button);
    button->setText("add VolumeFader");
    connect(button, SIGNAL(clicked()), SLOT(addVolumeFader()));
}

void PathWidget::addEffect()
{
    int current = m_effectComboBox->currentIndex();
    QList<AudioEffectDescription> effectList = BackendCapabilities::availableAudioEffects();
    if (current < effectList.size()) {
        AudioEffect *effect = new AudioEffect(effectList[current], m_path);
        QGroupBox *gb = new QGroupBox(effectList[current].name(), this);
        layout()->addWidget(gb);
        gb->setFlat(true);
        gb->setCheckable(true);
        gb->setChecked(true);
        (new QHBoxLayout(gb))->addWidget(new EffectWidget(effect, gb));
        m_path->insertEffect(effect);
        gb->setProperty("AudioEffect", QVariant::fromValue(static_cast<QObject *>(effect)));
        connect(gb, SIGNAL(toggled(bool)), SLOT(effectToggled(bool)));
    }
}

void PathWidget::effectToggled(bool checked)
{
    if (checked) {
        return;
    }
    QVariant v = sender()->property("AudioEffect");
    if (!v.isValid()) {
        return;
    }
    QObject *effect = v.value<QObject *>();
    if (!effect) {
        return;
    }
    delete effect;
    sender()->deleteLater();
}

void PathWidget::addVolumeFader()
{
    VolumeFaderEffect *effect = new VolumeFaderEffect(m_path);
    QGroupBox *gb = new QGroupBox("VolumeFader", this);
    layout()->addWidget(gb);
    gb->setFlat(true);
    gb->setCheckable(true);
    gb->setChecked(true);
    (new QHBoxLayout(gb))->addWidget(new EffectWidget(effect, gb));
    m_path->insertEffect(effect);
    gb->setProperty("AudioEffect", QVariant::fromValue(static_cast<QObject *>(effect)));
    connect(gb, SIGNAL(toggled(bool)), SLOT(effectToggled(bool)));
}

bool PathWidget::connectOutput(OutputWidget *w)
{
    return m_path->addOutput(w->output());
}

bool ProducerWidget::connectPath(PathWidget *w)
{
    if (m_media)
    {
        if (m_media->addAudioPath(w->path())) {
            m_audioPaths.append(w->path());
            return true;
        }
    }
    if (m_audioPaths.contains(w->path()))
        return false;
    m_audioPaths.append(w->path());
    return true;
}

ProducerWidget::ProducerWidget(QWidget *parent)
    : QFrame(parent),
    m_media(0),
    m_length(-1),
    m_vpath(0),
    m_vout(0),
    m_trackWidget(0),
    m_chapterWidget(0),
    m_angleWidget(0),
    m_navigationWidget(0)

{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Sunken);

    QVBoxLayout *topLayout = new QVBoxLayout(this);

    KLineEdit *file = new KLineEdit(this);
    file->setCompletionObject(new KUrlCompletion(KUrlCompletion::FileCompletion));
    file->setText(getenv("PHONON_TESTURL"));
    connect(file, SIGNAL(returnPressed(const QString  &)), SLOT(loadFile(const QString  &)));
    topLayout->addWidget(file);

    QHBoxLayout *mediaLayout = new QHBoxLayout(this);
    topLayout->addLayout(mediaLayout);
    QPushButton *audiocdButton = new QPushButton("CD", this);
    QPushButton *dvdButton = new QPushButton("DVD", this);
    mediaLayout->addWidget(audiocdButton);
    mediaLayout->addWidget(dvdButton);
    connect(audiocdButton, SIGNAL(clicked()), SLOT(openCD()));
    connect(dvdButton,     SIGNAL(clicked()), SLOT(openDVD()));

    m_seekslider = new SeekSlider(this);
    topLayout->addWidget(m_seekslider);

    QFrame *frame0 = new QFrame(this);
    topLayout->addWidget(frame0);
    QHBoxLayout *hlayout = new QHBoxLayout(frame0);
    hlayout->setMargin(0);

    QFrame *frame1 = new QFrame(frame0);
    hlayout->addWidget(frame1);
    QHBoxLayout *vlayout = new QHBoxLayout(frame1);
    vlayout->setMargin(0);

    // playback controls
    m_play = new QToolButton(frame1);
    m_play->setIconSize(QSize(32, 32));
    m_play->setText("play");
    m_play->setIcon(KIcon("media-playback-start"));
    vlayout->addWidget(m_play);

    m_pause = new QToolButton(frame1);
    m_pause->setIconSize(QSize(32, 32));
    m_pause->setText("pause");
    m_pause->setIcon(KIcon("media-playback-pause"));
    vlayout->addWidget(m_pause);

    m_stop = new QToolButton(frame1);
    m_stop->setIconSize(QSize(32, 32));
    m_stop->setText("stop");
    m_stop->setIcon(KIcon("media-playback-stop"));
    vlayout->addWidget(m_stop);

    m_trackButton = new QToolButton(frame1);
    m_trackButton->setText("Track");
    m_trackButton->setCheckable(true);
    vlayout->addWidget(m_trackButton);
    m_chapterButton = new QToolButton(frame1);
    m_chapterButton->setText("Chapter");
    m_chapterButton->setCheckable(true);
    vlayout->addWidget(m_chapterButton);
    m_angleButton = new QToolButton(frame1);
    m_angleButton->setText("Angle");
    m_angleButton->setCheckable(true);
    vlayout->addWidget(m_angleButton);
    m_navigationButton = new QToolButton(frame1);
    m_navigationButton->setText("Navigation");
    m_navigationButton->setCheckable(true);
    vlayout->addWidget(m_navigationButton);
    connect(m_trackButton, SIGNAL(toggled(bool)), SLOT(showTrackWidget(bool)));
    connect(m_chapterButton, SIGNAL(toggled(bool)), SLOT(showChapterWidget(bool)));
    connect(m_angleButton, SIGNAL(toggled(bool)), SLOT(showAngleWidget(bool)));
    connect(m_navigationButton, SIGNAL(toggled(bool)), SLOT(showNavigationWidget(bool)));

    QFrame *frame2 = new QFrame(frame0);
    hlayout->addWidget(frame2);
    QVBoxLayout *vlayout2 = new QVBoxLayout(frame2);
    vlayout2->setMargin(0);

    // state label
    m_statelabel = new QLabel(frame2);
    vlayout2->addWidget(m_statelabel);

    // buffer progressbar
    m_bufferProgress = new QProgressBar(frame2);
    m_bufferProgress->setMaximumSize(100, 16);
    m_bufferProgress->setTextVisible(false);
    vlayout2->addWidget(m_bufferProgress);

    // time info
    m_totaltime = new QLabel(frame2);
    vlayout2->addWidget(m_totaltime);
    
    m_currenttime = new QLabel(frame2);
    vlayout2->addWidget(m_currenttime);
    
    m_remainingtime = new QLabel(frame2);
    vlayout2->addWidget(m_remainingtime);

    // meta data
    m_metaDataLabel = new QLabel(this);
    topLayout->addWidget(m_metaDataLabel);

    loadFile(getenv("PHONON_TESTURL"));
}

ProducerWidget::~ProducerWidget()
{
    delete m_media;
}

void ProducerWidget::tick(qint64 t)
{
    QTime x(0, 0);
    x = x.addMSecs(t);
    m_currenttime->setText(x.toString("m:ss.zzz"));
    x.setHMS(0, 0, 0, 0);
    qint64 r = m_media->remainingTime();
    if (r >= 0)
        x = x.addMSecs(r);
    m_remainingtime->setText(x.toString("m:ss.zzz"));
}

void ProducerWidget::checkVideoWidget()
{
    if (m_media->hasVideo() && BackendCapabilities::supportsVideo()) {
        VideoPath *m_vpath = new VideoPath(m_media);
        m_media->addVideoPath(m_vpath);
        VideoWidget *m_vout = new VideoWidget(0);
        m_vout->setAttribute(Qt::WA_QuitOnClose, false);
        connect(m_vpath, SIGNAL(destroyed()), m_vout, SLOT(deleteLater()));
        m_vpath->addOutput(m_vout);

        m_vout->setMinimumSize(160, 120);
        m_vout->resize(m_vout->sizeHint());
        m_vout->setFullScreen(false);
        m_vout->show();
    } else if (m_vpath) {
        delete m_vpath;
    }
}

void ProducerWidget::stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    Q_UNUSED(oldstate);
//X     if (oldstate == LoadingState) {
//X         m_trackButton       ->setEnabled(m_media->hasInterface<TrackInterface>());
//X         m_chapterButton     ->setEnabled(m_media->hasInterface<ChapterInterface>());
//X         m_angleButton       ->setEnabled(m_media->hasInterface<AngleInterface>());
//X         m_navigationButton  ->setEnabled(m_media->hasInterface<NavigationInterface>());
//X     }
    switch(newstate)
    {
    case Phonon::ErrorState:
        m_statelabel->setText("Error");
        {
            QString text = m_media->errorString();
            if (!text.isEmpty()) {
                KMessageBox::error(this, text);
            }
        }
        break;
    case Phonon::LoadingState:
        m_statelabel->setText("Loading");
        break;
    case Phonon::StoppedState:
        m_statelabel->setText("Stopped");
        break;
    case Phonon::PausedState:
        m_statelabel->setText("Paused");
        break;
    case Phonon::BufferingState:
        m_statelabel->setText("Buffering");
        m_bufferProgress->reset();
        m_bufferProgress->show();
        break;
    case Phonon::PlayingState:
        m_statelabel->setText("Playing");
        break;
    }
}

void ProducerWidget::length(qint64 ms)
{
    m_length = ms;
    QTime x(0, 0);
    x = x.addMSecs(m_length);
    m_totaltime->setText(x.toString("m:ss.zzz"));
    tick(m_media->currentTime());
}

void ProducerWidget::ensureMedia()
{
    if (m_media) {
        return;
    }
    m_media = new MediaObject(this);
    connect(m_media, SIGNAL(metaDataChanged()), SLOT(updateMetaData()));
    m_seekslider->setMediaProducer(m_media);
    m_media->setTickInterval(100);
    m_media->setAboutToFinishTime(2000);

    foreach (AudioPath *path, m_audioPaths) {
        m_media->addAudioPath(path);
    }

    stateChanged(m_media->state(), Phonon::LoadingState);
    length(m_media->totalTime());

    connect(m_pause, SIGNAL(clicked()), m_media, SLOT(pause()));
    connect(m_play,  SIGNAL(clicked()), m_media, SLOT(play()));
    connect(m_stop,  SIGNAL(clicked()), m_media, SLOT(stop()));

    connect(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(m_media, SIGNAL(tick(qint64)),          SLOT(tick(qint64)));
    connect(m_media, SIGNAL(length(qint64)),        SLOT(length(qint64)));
    connect(m_media, SIGNAL(finished()),            SLOT(slotFinished()));
    connect(m_media, SIGNAL(aboutToFinish(qint32)), SLOT(slotAboutToFinish(qint32)));
    connect(m_media, SIGNAL(hasVideoChanged(bool)), SLOT(checkVideoWidget()));
    connect(m_media, SIGNAL(bufferStatus(int)), m_bufferProgress, SLOT(setValue(int)));
}

void ProducerWidget::openCD()
{
    ensureMedia();
    m_media->openMedia(MediaObject::CD);

//X     m_trackButton       ->setEnabled(m_media->hasInterface<TrackInterface>());
//X     m_chapterButton     ->setEnabled(m_media->hasInterface<ChapterInterface>());
//X     m_angleButton       ->setEnabled(m_media->hasInterface<AngleInterface>());
//X     m_navigationButton  ->setEnabled(m_media->hasInterface<NavigationInterface>());
}

void ProducerWidget::openDVD()
{
    ensureMedia();
    m_media->openMedia(MediaObject::DVD);
//X     m_trackButton       ->setEnabled(m_media->hasInterface<TrackInterface>());
//X     m_chapterButton     ->setEnabled(m_media->hasInterface<ChapterInterface>());
//X     m_angleButton       ->setEnabled(m_media->hasInterface<AngleInterface>());
//X     m_navigationButton  ->setEnabled(m_media->hasInterface<NavigationInterface>());
}

void ProducerWidget::showTrackWidget(bool b)
{
    if (b) {
        if (!m_trackWidget) {
            m_trackWidget = new TrackWidget;
            connect(m_trackWidget, SIGNAL(destroyed()), SLOT(updateIfaceButtons()));
        }
        if (!m_trackIface && m_media) {
            m_trackIface = new TrackInterface(m_media);
        }
        if (m_trackIface) {
            m_trackWidget->setInterface(m_trackIface);
        }
        m_trackWidget->show();
    } else {
        if (m_trackWidget) {
            m_trackWidget->hide();
        }
    }
}

void ProducerWidget::showChapterWidget(bool b)
{
    if (b) {
        if (!m_chapterWidget) {
            m_chapterWidget = new ChapterWidget;
            connect(m_chapterWidget, SIGNAL(destroyed()), SLOT(updateIfaceButtons()));
        }
        if (!m_chapterIface && m_media) {
            m_chapterIface = new ChapterInterface(m_media);
        }
        if (m_chapterIface) {
            m_chapterWidget->setInterface(m_chapterIface);
        }
        m_chapterWidget->show();
    } else {
        if (m_chapterWidget) {
            m_chapterWidget->hide();
        }
    }
}

void ProducerWidget::showAngleWidget(bool b)
{
    if (b) {
        if (!m_angleWidget) {
            m_angleWidget = new AngleWidget;
            connect(m_angleWidget, SIGNAL(destroyed()), SLOT(updateIfaceButtons()));
        }
        if (!m_angleIface && m_media) {
            m_angleIface = new AngleInterface(m_media);
        }
        if (m_angleIface) {
            m_angleWidget->setInterface(m_angleIface);
        }
        m_angleWidget->show();
    } else {
        if (m_angleWidget) {
            m_angleWidget->hide();
        }
    }
}

void ProducerWidget::showNavigationWidget(bool b)
{
    if (b) {
        if (!m_navigationWidget) {
            m_navigationWidget = new NavigationWidget;
            connect(m_navigationWidget, SIGNAL(destroyed()), SLOT(updateIfaceButtons()));
        }
        if (!m_navigationIface && m_media) {
            m_navigationIface = new NavigationInterface(m_media);
        }
        if (m_navigationIface) {
            m_navigationWidget->setInterface(m_navigationIface);
        }
        m_navigationWidget->show();
    } else {
        if (m_navigationWidget) {
            m_navigationWidget->hide();
        }
    }
}

void ProducerWidget::loadFile(const QString  & file)
{
    delete m_media;
    m_media = 0;
    ensureMedia();
    Q_ASSERT(m_media);
    m_media->setUrl(KUrl(file));
//X     m_trackButton       ->setEnabled(m_media->hasInterface<TrackInterface>());
//X     m_chapterButton     ->setEnabled(m_media->hasInterface<ChapterInterface>());
//X     m_angleButton       ->setEnabled(m_media->hasInterface<AngleInterface>());
//X     m_navigationButton  ->setEnabled(m_media->hasInterface<NavigationInterface>());
}

void ProducerWidget::updateMetaData()
{
    QString metaData;
    QStringList keys = m_media->metaDataKeys();
    foreach (QString key, keys)
    {
        metaData += key + QLatin1String(": ") + m_media->metaDataItems(key).join(QLatin1String("\n")) + "\n";
    }
    m_metaDataLabel->setText(metaData.left(metaData.length() - 1));
}

void ProducerWidget::slotFinished()
{
    kDebug() << "finished signal from MediaObject with URL " << m_media->url() << endl;
}

void ProducerWidget::slotAboutToFinish(qint32 remaining)
{
    kDebug() << "aboutToFinish(" << remaining << ") signal from MediaObject with URL " << m_media->url() << endl;
}

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);
    setMinimumWidth(120);
}

void ConnectionWidget::mousePressEvent(QMouseEvent *me)
{
    m_pressPos = me->pos();
}

void ConnectionWidget::mouseReleaseEvent(QMouseEvent *me)
{
    emit madeConnection(m_pressPos, me->pos());
    m_pressPos = QPoint();
    m_currentLine = QLine();
    update();
}

void ConnectionWidget::mouseMoveEvent(QMouseEvent *me)
{
    if (m_pressPos.isNull())
        return;

    m_currentLine = QLine(m_pressPos, me->pos());
    update();
}

void ConnectionWidget::addConnection(QWidget *a, QWidget *b)
{
    m_lines.append(WidgetConnection(a, b));
    update();
}

void ConnectionWidget::paintEvent(QPaintEvent *pe)
{
    QFrame::paintEvent(pe);
    QPainter p(this);
    foreach (WidgetConnection x, m_lines)
    {
        QLine l(0, x.a->geometry().center().y(),
                width(), x.b->geometry().center().y());
        p.drawLine(l);
    }
    p.setPen(Qt::darkRed);
    p.drawLine(m_currentLine);
}

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *topLayout = new QHBoxLayout(this);

    m_producerFrame = new QFrame(this);
    topLayout->addWidget(m_producerFrame);
    new QVBoxLayout(m_producerFrame);
    m_producerFrame->layout()->setMargin(0);

    m_connectionFrame1 = new ConnectionWidget(this);
    topLayout->addWidget(m_connectionFrame1);
    connect(m_connectionFrame1, SIGNAL(madeConnection(const QPoint &, const QPoint &)),
            SLOT(madeConnection1(const QPoint &, const QPoint &)));

    m_pathFrame = new QFrame(this);
    topLayout->addWidget(m_pathFrame);
    new QVBoxLayout(m_pathFrame);
    m_pathFrame->layout()->setMargin(0);

    m_connectionFrame2 = new ConnectionWidget(this);
    topLayout->addWidget(m_connectionFrame2);
    connect(m_connectionFrame2, SIGNAL(madeConnection(const QPoint &, const QPoint &)),
            SLOT(madeConnection2(const QPoint &, const QPoint &)));

    m_outputFrame = new QFrame(this);
    topLayout->addWidget(m_outputFrame);
    new QVBoxLayout(m_outputFrame);
    m_outputFrame->layout()->setMargin(0);

    QPushButton *addProducerButton = new QPushButton("add Producer", m_producerFrame);
    m_producerFrame->layout()->addWidget(addProducerButton);
    connect(addProducerButton, SIGNAL(clicked()), SLOT(addProducer()));

    QPushButton *addPathButton = new QPushButton("add Path", m_pathFrame);
    m_pathFrame->layout()->addWidget(addPathButton);
    connect(addPathButton, SIGNAL(clicked()), SLOT(addPath()));

    QPushButton *addOutputButton = new QPushButton("add Output", m_outputFrame);
    m_outputFrame->layout()->addWidget(addOutputButton);
    connect(addOutputButton, SIGNAL(clicked()), SLOT(addOutput()));

    addProducer();
    addPath();
    addOutput();
    if (m_producers.first()->connectPath(m_paths.first()))
        m_connectionFrame1->addConnection(m_producers.first(), m_paths.first());
    if (m_paths.first()->connectOutput(m_outputs.first()))
        m_connectionFrame2->addConnection(m_paths.first(), m_outputs.first());
}

void MainWidget::addProducer()
{
    ProducerWidget *x = new ProducerWidget(m_producerFrame);
    m_producerFrame->layout()->addWidget(x);
    m_producers << x;
    m_connectionFrame1->update();
}

void MainWidget::addPath()
{
    PathWidget *x = new PathWidget(m_pathFrame);
    m_pathFrame->layout()->addWidget(x);
    m_paths << x;
    m_connectionFrame1->update();
    m_connectionFrame2->update();
}

void MainWidget::addOutput()
{
    OutputWidget *x = new OutputWidget(m_outputFrame);
    m_outputFrame->layout()->addWidget(x);
    m_outputs << x;
    m_connectionFrame2->update();
}

void MainWidget::madeConnection1(const QPoint &a, const QPoint &b)
{
    QPoint left;
    QPoint right;
    if (a.x() < m_connectionFrame1->width() / 3)
        left = a;
    else if (a.x() > m_connectionFrame1->width() * 2 / 3)
        right = a;
    if (b.x() < m_connectionFrame1->width() / 3)
        left = b;
    else if (b.x() > m_connectionFrame1->width() * 2 / 3)
        right = b;
    if (left.isNull() || right.isNull())
        return;

    kDebug() << k_funcinfo << left << right << endl;

    ProducerWidget *producer = 0;
    foreach (ProducerWidget *x, m_producers)
    {
        QRect rect = x->geometry();
        kDebug() << rect << endl;
        left.setX(rect.center().x());
        if (rect.contains(left))
            producer = x;
    }
    if (!producer)
        return;

    PathWidget *path = 0;
    foreach (PathWidget *x, m_paths)
    {
        QRect rect = x->geometry();
        kDebug() << rect << endl;
        right.setX(rect.center().x());
        if (rect.contains(right))
            path = x;
    }
    if (!path)
        return;

    if (producer->connectPath(path))
        m_connectionFrame1->addConnection(producer, path);
}

void MainWidget::madeConnection2(const QPoint &a, const QPoint &b)
{
    QPoint left;
    QPoint right;
    if (a.x() < m_connectionFrame2->width() / 3)
        left = a;
    else if (a.x() > m_connectionFrame2->width() * 2 / 3)
        right = a;
    if (b.x() < m_connectionFrame2->width() / 3)
        left = b;
    else if (b.x() > m_connectionFrame2->width() * 2 / 3)
        right = b;
    if (left.isNull() || right.isNull())
        return;

    kDebug() << k_funcinfo << left << right << endl;

    PathWidget *path = 0;
    foreach (PathWidget *x, m_paths)
    {
        QRect rect = x->geometry();
        kDebug() << rect << endl;
        left.setX(rect.center().x());
        if (rect.contains(left))
            path = x;
    }
    if (!path)
        return;

    OutputWidget *output = 0;
    foreach (OutputWidget *x, m_outputs)
    {
        QRect rect = x->geometry();
        kDebug() << rect << endl;
        right.setX(rect.center().x());
        if (rect.contains(right))
            output = x;
    }
    if (!output)
        return;

    if (path->connectOutput(output))
        m_connectionFrame2->addConnection(path, output);
}

int main(int argc, char **argv)
{
    KAboutData about("phonontest", "KDE Multimedia Test",
            "0.2", "Testprogram",
            KAboutData::License_LGPL, 0);
    about.addAuthor("Matthias Kretz", 0, "kretz@kde.org");
    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;
    MainWidget w;
    w.setWindowIcon(KIcon("phonon"));
    w.show();
    return app.exec();
}

#include "guitest.moc"

// vim: sw=4 ts=4
