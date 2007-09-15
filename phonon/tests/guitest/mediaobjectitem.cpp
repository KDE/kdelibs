/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#include "mediaobjectitem.h"
#include <QtCore/QTime>
#include <QtCore/QUrl>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <kdebug.h>
#include <kurlcompletion.h>
#include <kmessagebox.h>
#include <kicon.h>
#include <klineedit.h>

MediaObjectItem::MediaObjectItem(const QPoint &pos, QGraphicsView *widget)
    : WidgetRectItem(pos, widget),
    m_length(-1),
    m_titleWidget(0),
    m_chapterWidget(0),
    m_angleWidget(0),
    m_navigationWidget(0)
{
    setBrush(QColor(255, 100, 100, 150));

    QVBoxLayout *topLayout = new QVBoxLayout(m_frame);
    topLayout->setMargin(0);

    KLineEdit *file = new KLineEdit(m_frame);
    file->setCompletionObject(new KUrlCompletion(KUrlCompletion::FileCompletion));
    connect(file, SIGNAL(returnPressed(const QString &)), SLOT(loadUrl(const QString &)));
    topLayout->addWidget(file);

    QHBoxLayout *mediaLayout = new QHBoxLayout(m_frame);
    topLayout->addLayout(mediaLayout);
    QPushButton *audiocdButton = new QPushButton("CD", m_frame);
    QPushButton *dvdButton = new QPushButton("DVD", m_frame);
    mediaLayout->addWidget(audiocdButton);
    mediaLayout->addWidget(dvdButton);
    connect(audiocdButton, SIGNAL(clicked()), SLOT(openCD()));
    connect(dvdButton,     SIGNAL(clicked()), SLOT(openDVD()));

    m_seekslider = new SeekSlider(m_frame);
    topLayout->addWidget(m_seekslider);

    QFrame *frame0 = new QFrame(m_frame);
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

    m_titleButton = new QToolButton(frame1);
    m_titleButton->setText("Title");
    m_titleButton->setCheckable(true);
    vlayout->addWidget(m_titleButton);
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
    connect(m_titleButton, SIGNAL(toggled(bool)), SLOT(showTitleWidget(bool)));
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
    m_metaDataLabel = new QLabel(m_frame);
    topLayout->addWidget(m_metaDataLabel);

    connect(&m_media, SIGNAL(metaDataChanged()), SLOT(updateMetaData()));
    m_seekslider->setMediaObject(&m_media);
    m_media.setTickInterval(100);
    m_media.setPrefinishMark(2000);

    stateChanged(m_media.state(), Phonon::LoadingState);
    length(m_media.totalTime());

    connect(m_pause, SIGNAL(clicked()), &m_media, SLOT(pause()));
    connect(m_play,  SIGNAL(clicked()), &m_media, SLOT(play()));
    connect(m_stop,  SIGNAL(clicked()), &m_media, SLOT(stop()));

    connect(&m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(&m_media, SIGNAL(tick(qint64)), SLOT(tick(qint64)));
    connect(&m_media, SIGNAL(totalTimeChanged(qint64)), SLOT(length(qint64)));
    connect(&m_media, SIGNAL(finished()), SLOT(slotFinished()));
    connect(&m_media, SIGNAL(prefinishMarkReached(qint32)), SLOT(slotPrefinishMarkReached(qint32)));
    connect(&m_media, SIGNAL(bufferStatus(int)), m_bufferProgress, SLOT(setValue(int)));
}

void MediaObjectItem::loadUrl(const QString &url)
{
    m_media.setCurrentSource(QUrl(url));
}

void MediaObjectItem::updateMetaData()
{
    QString metaData;
    const QMultiMap<QString, QString> map = m_media.metaData();
    const QStringList keys = map.keys();
    foreach (QString key, keys) {
        metaData += key + QLatin1String(": ") + QStringList(map.values(key)).join(QLatin1String("\n")) + "\n";
    }
    m_metaDataLabel->setText(metaData.left(metaData.length() - 1));
}

void MediaObjectItem::slotFinished()
{
    kDebug() << "finished signal from MediaObject with URL " << m_media.currentSource().url();
}

void MediaObjectItem::slotPrefinishMarkReached(qint32 remaining)
{
    kDebug() << "prefinishMarkReached(" << remaining << ") signal from MediaObject with URL " << m_media.currentSource().url();
}

void MediaObjectItem::tick(qint64 t)
{
    QTime x(0, 0);
    x = x.addMSecs(t);
    m_currenttime->setText(x.toString("m:ss.zzz"));
    x.setHMS(0, 0, 0, 0);
    qint64 r = m_media.remainingTime();
    if (r >= 0) {
        x = x.addMSecs(r);
    }
    m_remainingtime->setText(x.toString("m:ss.zzz"));
}

void MediaObjectItem::stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    Q_UNUSED(oldstate);
    switch(newstate)
    {
    case Phonon::ErrorState:
        m_statelabel->setText("Error");
        {
            QString text = m_media.errorString();
            if (text.isEmpty()) {
                KMessageBox::error(m_frame, "reached error state but the backend did not report an errorString");
            } else {
                KMessageBox::error(m_frame, text);
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

void MediaObjectItem::length(qint64 ms)
{
    m_length = ms;
    QTime x(0, 0);
    x = x.addMSecs(m_length);
    m_totaltime->setText(x.toString("m:ss.zzz"));
    tick(m_media.currentTime());
}

void MediaObjectItem::openCD()
{
    m_media.setCurrentSource(Phonon::Cd);
}

void MediaObjectItem::openDVD()
{
    m_media.setCurrentSource(Phonon::Dvd);
}

void MediaObjectItem::showTitleWidget(bool b)
{
    if (b) {
        if (!m_titleWidget) {
            m_titleWidget = new TitleWidget;
            connect(m_titleWidget, SIGNAL(destroyed()), SLOT(updateIfaceButtons()));
        }
        if (!m_mediaController) {
            m_mediaController = new MediaController(&m_media);
        }
        if (m_mediaController) {
            m_titleWidget->setInterface(m_mediaController);
        }
        m_titleWidget->show();
    } else {
        if (m_titleWidget) {
            m_titleWidget->hide();
        }
    }
}

void MediaObjectItem::showChapterWidget(bool b)
{
    if (b) {
        if (!m_chapterWidget) {
            m_chapterWidget = new ChapterWidget;
            connect(m_chapterWidget, SIGNAL(destroyed()), SLOT(updateIfaceButtons()));
        }
        if (!m_mediaController) {
            m_mediaController = new MediaController(&m_media);
        }
        if (m_mediaController) {
            m_chapterWidget->setInterface(m_mediaController);
        }
        m_chapterWidget->show();
    } else {
        if (m_chapterWidget) {
            m_chapterWidget->hide();
        }
    }
}

void MediaObjectItem::showAngleWidget(bool b)
{
    if (b) {
        if (!m_angleWidget) {
            m_angleWidget = new AngleWidget;
            connect(m_angleWidget, SIGNAL(destroyed()), SLOT(updateIfaceButtons()));
        }
        if (!m_mediaController) {
            m_mediaController = new MediaController(&m_media);
        }
        if (m_mediaController) {
            m_angleWidget->setInterface(m_mediaController);
        }
        m_angleWidget->show();
    } else {
        if (m_angleWidget) {
            m_angleWidget->hide();
        }
    }
}

void MediaObjectItem::showNavigationWidget(bool b)
{
    if (b) {
        if (!m_navigationWidget) {
            m_navigationWidget = new NavigationWidget;
            connect(m_navigationWidget, SIGNAL(destroyed()), SLOT(updateIfaceButtons()));
        }
        if (!m_mediaController) {
            m_mediaController = new MediaController(&m_media);
        }
        if (m_mediaController) {
            m_navigationWidget->setInterface(m_mediaController);
        }
        m_navigationWidget->show();
    } else {
        if (m_navigationWidget) {
            m_navigationWidget->hide();
        }
    }
}

