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

#ifndef EXAMPLES_VIDEOPLAYBACK_H
#define EXAMPLES_VIDEOPLAYBACK_H

#include <QtGui/QWidget>

class KUrl;

namespace Phonon
{
    class VideoWidget;
    class VideoPath;
    class AudioOutput;
    class AudioPath;
    class MediaObject;
    class MediaControls;
} // namespace Phonon

using namespace Phonon;

class MediaPlayer : public QWidget
{
    Q_OBJECT
    public:
        MediaPlayer(QWidget *parent);
        void setUrl(const KUrl &url);

    private Q_SLOTS:
        void hasVideoChanged(bool hasVideo);

    private:
        VideoWidget *m_vwidget;
        VideoPath *m_vpath;
        AudioOutput *m_aoutput;
        AudioPath *m_apath;
        MediaObject *m_media;
        MediaControls *m_controls;
};

#endif // EXAMPLES_VIDEOPLAYBACK_H
