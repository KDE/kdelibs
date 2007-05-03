/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    Permission to use, copy, modify, and distribute this software
    and its documentation for any purpose and without fee is hereby
    granted, provided that the above copyright notice appear in all
    copies and that both that the copyright notice and this
    permission notice and warranty disclaimer appear in supporting
    documentation, and that the name of the author not be used in
    advertising or publicity pertaining to distribution of the
    software without specific, written prior permission.

    The author disclaim all warranties with regard to this
    software, including all implied warranties of merchantability
    and fitness.  In no event shall the author be liable for any
    special, indirect or consequential damages or any damages
    whatsoever resulting from loss of use, data or profits, whether
    in an action of contract, negligence or other tortious action,
    arising out of or in connection with the use or performance of
    this software.

*/

#ifndef EXAMPLES_TUT1_H
#define EXAMPLES_TUT1_H

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtGui/QDirModel>
#include <QtGui/QColumnView>

namespace Phonon
{
    class MediaObject;
    class AudioPath;
    class AudioOutput;
} // namespace Phonon

class QModelIndex;

class PlayerWidget : public QWidget
{
    Q_OBJECT
    public:
        PlayerWidget();

        void play(const QString &);

    private:
        void delayedInit();

        Phonon::MediaObject *m_media;
        Phonon::AudioPath *m_audioPath;
        Phonon::AudioOutput *m_audioOutput;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        MainWindow();

    private slots:
        void providePlayer(const QModelIndex &index);

    private:
        QColumnView m_fileView;
        QDirModel m_model;
        PlayerWidget m_player;
};

#endif // EXAMPLES_TUT1_H
