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

#include <Phonon/MediaObject>
#include <Phonon/AudioPath>
#include <Phonon/AudioOutput>
#include <Phonon/Global>

#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtGui/QDirModel>
#include <QtGui/QColumnView>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        MainWindow();

    private slots:
        void play(const QModelIndex &index);

    private:
        void delayedInit();

        QColumnView m_fileView;
        QDirModel m_model;

        Phonon::MediaObject *m_media;
};

MainWindow::MainWindow()
    : m_fileView(this),
    m_media(0)
{
    setCentralWidget(&m_fileView);
    m_fileView.setModel(&m_model);
    m_fileView.setFrameStyle(QFrame::NoFrame);

    connect(&m_fileView, SIGNAL(updatePreviewWidget(const QModelIndex &)), SLOT(play(const QModelIndex &)));
}

void MainWindow::play(const QModelIndex &index)
{
    delayedInit();
    m_media->setCurrentSource(m_model.filePath(index));
    m_media->play();
}

void MainWindow::delayedInit()
{
    if (!m_media) {
        m_media = new Phonon::MediaObject(this);
        Phonon::AudioPath *audioPath = new Phonon::AudioPath(this);
        Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        m_media->addAudioPath(audioPath);
        audioPath->addOutput(audioOutput);
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Phonon Tutorial 2");
    MainWindow mw;
    mw.show();
    return app.exec();
}

#include "tutorial2.moc"
