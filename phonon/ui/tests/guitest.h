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

#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <QList>
#include <QFrame>
#include <QPoint>
#include <QLine>
#include <QPointer>

#include <phonon/phononnamespace.h>
#include <phonon/trackinterface.h>
#include <phonon/angleinterface.h>
#include <phonon/chapterinterface.h>
#include <phonon/navigationinterface.h>

class QLabel;
class QString;
class QAbstractButton;
class QFrame;
class QComboBox;
class QProgressBar;
namespace Phonon
{
    class MediaObject;
    class AudioPath;
    class AudioOutput;
    class VideoPath;
    class VideoOutput;
    class VideoWidget;
    class SeekSlider;
    class VolumeSlider;
    class VideoPath;
    class VideoWidget;
    class TrackInterface;
    class ChapterInterface;
    class NavigationInterface;
    class AngleInterface;
}

using namespace Phonon;

class PathWidget;
class OutputWidget;
class QSpinBox;
class QToolButton;

class NavigationWidget : public QWidget
{
    Q_OBJECT
    public:
        NavigationWidget(QWidget *parent = 0);
        ~NavigationWidget();
        void setInterface(NavigationInterface *i);

    private:
        NavigationInterface *m_iface;
};

class TrackWidget : public QWidget
{
    Q_OBJECT
    public:
        TrackWidget(QWidget *parent = 0);
        ~TrackWidget();
        void setInterface(TrackInterface *i);

    private slots:
        void availableTracksChanged(int);
    private:
        TrackInterface *m_iface;
        QSpinBox *m_currentTrack;
        QLabel *m_availableTracks;
        QToolButton *m_autoplay;
};

class ChapterWidget : public QWidget
{
    Q_OBJECT
    public:
        ChapterWidget(QWidget *parent = 0);
        ~ChapterWidget();
        void setInterface(ChapterInterface *i);

    private slots:
        void availableChaptersChanged(int);
    private:
        ChapterInterface *m_iface;
        QSpinBox *m_currentChapter;
        QLabel *m_availableChapters;
};

class AngleWidget : public QWidget
{
    Q_OBJECT
    public:
        AngleWidget(QWidget *parent = 0);
        ~AngleWidget();
        void setInterface(AngleInterface *i);

    private slots:
        void availableAnglesChanged(int);
    private:
        AngleInterface *m_iface;
        QSpinBox *m_currentAngle;
        QLabel *m_availableAngles;
};

class ProducerWidget : public QFrame
{
    Q_OBJECT
    public:
        ProducerWidget(QWidget *parent = 0);
        ~ProducerWidget();
        bool connectPath(PathWidget *w);

    private Q_SLOTS:
        void tick(qint64);
        void stateChanged(Phonon::State, Phonon::State);
        void length(qint64);
        void loadFile(const QString &);
        void slotFinished();
        void slotAboutToFinish(qint32 remaining);
        void updateMetaData();
        void checkVideoWidget();
        void openCD();
        void openDVD();
        void showTrackWidget(bool);
        void showChapterWidget(bool);
        void showAngleWidget(bool);
        void showNavigationWidget(bool);

    private:
        void ensureMedia();

        SeekSlider *m_seekslider;
        QLabel *m_statelabel, *m_totaltime, *m_currenttime, *m_remainingtime;
        QLabel *m_metaDataLabel;
        QAbstractButton *m_pause, *m_play, *m_stop;
        MediaObject *m_media;
        qint64 m_length;
        QList<AudioPath *> m_audioPaths;
        QProgressBar *m_bufferProgress;
        VideoPath *m_vpath;
        VideoWidget *m_vout;

        QAbstractButton *m_trackButton;
        QAbstractButton *m_chapterButton;
        QAbstractButton *m_angleButton;
        QAbstractButton *m_navigationButton;

        QPointer<TrackInterface> m_trackIface;
        QPointer<ChapterInterface> m_chapterIface;
        QPointer<AngleInterface> m_angleIface;
        QPointer<NavigationInterface> m_navigationIface;

        TrackWidget *m_trackWidget;
        ChapterWidget *m_chapterWidget;
        AngleWidget *m_angleWidget;
        NavigationWidget *m_navigationWidget;
};

class PathWidget : public QFrame
{
    Q_OBJECT
    public:
        PathWidget(QWidget *parent = 0);

        AudioPath *path() const { return m_path; }
        bool connectOutput(OutputWidget *w);

    private Q_SLOTS:
        void addVolumeFader();
        void addEffect();
        void effectToggled(bool checked);

    private:
        AudioPath *m_path;
        QComboBox *m_effectComboBox;
};

class OutputWidget : public QFrame
{
    Q_OBJECT
    public:
        OutputWidget(QWidget *parent = 0);

        AudioOutput *output() const { return m_output; }

    private Q_SLOTS:
        void deviceChange(int);

    private:
        VolumeSlider *m_volslider;
        AudioOutput *m_output;
};

class ConnectionWidget : public QFrame
{
    Q_OBJECT
    public:
        ConnectionWidget(QWidget *parent = 0);

        void addConnection(QWidget *, QWidget *);

    signals:
        void madeConnection(const QPoint &, const QPoint  &);

    protected:
        virtual void mousePressEvent(QMouseEvent *);
        virtual void mouseReleaseEvent(QMouseEvent *);
        virtual void mouseMoveEvent(QMouseEvent *);
        virtual void paintEvent(QPaintEvent *pe);

    private:
        QPoint m_pressPos;
        QLine m_currentLine;
        struct WidgetConnection
        {
            WidgetConnection(QWidget *_a, QWidget *_b) : a(_a), b(_b) {}
            QWidget *a;
            QWidget *b;
        };
        QList<WidgetConnection> m_lines;
};

class MainWidget : public QWidget
{
    Q_OBJECT
    public:
        MainWidget(QWidget *parent = 0);

    private slots:
        void addProducer();
        void addPath();
        void addOutput();
        void madeConnection1(const QPoint &, const QPoint  &);
        void madeConnection2(const QPoint &, const QPoint  &);

    private:
        QList<ProducerWidget *> m_producers;
        QList<PathWidget *> m_paths;
        QList<OutputWidget *> m_outputs;

        QFrame *m_producerFrame;
        QFrame *m_pathFrame;
        QFrame *m_outputFrame;
        ConnectionWidget *m_connectionFrame1;
        ConnectionWidget *m_connectionFrame2;
};

#endif // TESTWIDGET_H
// vim: sw=4 ts=4
