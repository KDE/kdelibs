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

#ifndef PHONON_UI_MEDIACONTROLS_H
#define PHONON_UI_MEDIACONTROLS_H

#include <phonon/phononnamespace.h>
#include <QWidget>

namespace Phonon
{
class MediaObject;
class AudioOutput;
class MediaControlsPrivate;

/**
 * \short Simple widget showing buttons to control an MediaObject
 * object.
 *
 * This widget shows the standard player controls. There's at least the
 * play/pause and stop buttons. If the media is seekable it shows a seek-slider.
 * Optional controls include a volume control and a loop control button.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class MediaControls : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MediaControls)
    /**
     * This property holds whether the slider showing the progress of the
     * playback is visible.
     *
     * By default the slider is visible. It is enabled/disabled automatically
     * depending on whether the media can be seeked or not.
     */
    Q_PROPERTY(bool seekSliderVisible READ isSeekSliderVisible WRITE setSeekSliderVisible)

    /**
     * This property holds whether the slider controlling the volume is visible.
     *
     * By default the slider is visible if an AudioOutput has been set with
     * setAudioOutput.
     *
     * \see setAudioOutput
     */
    Q_PROPERTY(bool volumeControlVisible READ isVolumeControlVisible WRITE setVolumeControlVisible)

public:
    /**
     * Constructs a media control widget with a \p parent.
     */
    explicit MediaControls(QWidget *parent = 0);
    ~MediaControls();

    bool isSeekSliderVisible() const;
    bool isVolumeControlVisible() const;

public Q_SLOTS:
    void setSeekSliderVisible(bool isVisible);
    void setVolumeControlVisible(bool isVisible);

    /**
     * Sets the media object to be controlled by this widget.
     */
    void setMediaObject(MediaObject *mediaObject);

    /**
     * Sets the audio output object to be controlled by this widget.
     */
    void setAudioOutput(AudioOutput *audioOutput);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Q_PRIVATE_SLOT(d_func(), void _k_stateChanged(Phonon::State, Phonon::State))
    Q_PRIVATE_SLOT(d_func(), void _k_mediaDestroyed())

    MediaControlsPrivate *const d_ptr;
};

} // namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_UI_MEDIACONTROLS_H
