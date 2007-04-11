/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_AUDIOOUTPUT_H
#define Phonon_AUDIOOUTPUT_H

#include "phonon_export.h"
#include "abstractaudiooutput.h"
#include "phonondefs.h"
#include "phononnamespace.h"
#include "objectdescription.h"

class QString;

class AudioOutputAdaptor;
namespace Phonon
{
    class AudioOutputPrivate;

    /**
     * \short Class for audio output to the soundcard.
     *
     * Use this class to define the audio output. Example:
     * \code
     * m_audioOutput = new AudioOutput(Phonon::MusicCategory, this);
     * m_audioPath->addOutput(m_audioOutput);
     * \endcode
     *
     * \author Matthias Kretz <kretz@kde.org>
     * \see Phonon::Ui::VolumeSlider
     */
    class PHONONCORE_EXPORT AudioOutput : public AbstractAudioOutput
    {
        friend class FactoryPrivate;
        friend class ::AudioOutputAdaptor;
        Q_OBJECT
        K_DECLARE_PRIVATE(AudioOutput)
        /**
         * This is the name that appears in Mixer applications that control
         * the volume of this output.
         *
         * \see category
         */
        Q_PROPERTY(QString name READ name WRITE setName)
        /**
         * This is the current volume of the output as voltage factor.
         *
         * 1.0 means 100%, 0.5 means 50% voltage/25% power, 0.0 means 0%
         *
         * \see volumeDecibel
         */
        Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
        /**
         * This is the current volume of the output in decibel.
         *
         * 0 dB means no change in volume, -6dB means an attenuation of the
         * voltage to 50% and an attenuation of the power to 25%, -inf dB means
         * silence.
         *
         * \see volume
         */
        Q_PROPERTY(double volumeDecibel READ volumeDecibel WRITE setVolumeDecibel)
        /**
         * This property holds the (hardware) destination for the output.
         *
         * The default device is determined by the category and the global
         * configuration for that category of outputs. Normally you don't need
         * to override this setting - letting the user change the global
         * configuration is the right choice. You can still override the
         * device though, if you have good reasons to do so.
         *
         * \see outputDeviceChanged
         */
        Q_PROPERTY(AudioOutputDevice outputDevice READ outputDevice WRITE setOutputDevice)

        /**
         * This property tells whether the output is muted.
         *
         * Muting the output has the same effect as calling setVolume(0.0).
         */
        Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
        public:
            /**
             * Creates a new AudioOutput that defines output to a physical
             * device.
             *
             * \param category The category can be used by mixer applications to group volume
             * controls of applications into categories. That makes it easier for
             * the user to identify the programs.
             * The category is also used for the default output device that is
             * configured centrally. As an example: often users want to have the
             * audio signal of a VoIP application go to their USB headset while
             * all other sounds should go to the internal soundcard.
             *
             * \param parent QObject parent
             *
             * \see Phonon::categoryToString
             * \see outputDevice
             */
            explicit AudioOutput(Phonon::Category category, QObject *parent = 0);
            QString name() const;
            double volumeDecibel() const;

            /**
             * Returns the category of this output.
             *
             * \see AudioOutput(Phonon::Category, QObject *)
             */
            Phonon::Category category() const;
            AudioOutputDevice outputDevice() const;
            bool isMuted() const;

        protected:
            /**
             * \internal
             * After construction of the Iface object this method is called
             * throughout the complete class hierarchy in order to set up the
             * properties that were already set on the frontend objects.
             */
            void setupIface();

        public Q_SLOTS:
            void setName(const QString &newName);
            float volume() const;
            void setVolume(float newVolume);
            void setVolumeDecibel(double newVolumeDecibel);
            bool setOutputDevice(const AudioOutputDevice &newAudioOutputDevice);
            void setMuted(bool mute);

        Q_SIGNALS:
            /**
             * This signal is emitted whenever the volume has changed. As the
             * volume can change without a call to setVolume (calls over dbus)
             * this is important
             * to keep a widget showing the current volume up to date.
             */
            void volumeChanged(float newVolume);

            /**
             * This signal is emitted when the muted property has changed. As
             * this property can change by IPC (DBus) calls a UI element showing
             * the muted property should listen to this signal.
             */
            void mutedChanged(bool);

            /**
             * This signal is emitted when the (hardware) device for the output
             * has changed.
             *
             * The change can happen either through setOutputDevice or if the
             * global configuration for the used category has changed.
             *
             * \see outputDevice
             */
            void outputDeviceChanged(const AudioOutputDevice &newAudioOutputDevice);

        private:
            Q_PRIVATE_SLOT(k_func(), void _k_volumeChanged(float))
            Q_PRIVATE_SLOT(k_func(), void _k_revertFallback())
            Q_PRIVATE_SLOT(k_func(), void _k_audioDeviceFailed())
    };
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_AUDIOOUTPUT_H
