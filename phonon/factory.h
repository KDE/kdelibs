/*  This file is part of the KDE project
    Copyright (C) 2004-2007 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_FACTORY_H
#define PHONON_FACTORY_H

#include "phonon_export.h"
#include <kglobal.h>

#include <QObject>

namespace Phonon
{
    class BasePrivate;
/**
 * \internal
 * \brief Factory to access the preferred Backend.
 *
 * This class is used internally to get the backend's implementation.
 * It keeps track of the objects that were created. When a
 * request for a backend change comes, it asks all frontend objects to delete
 * their backend objects and then checks whether they were all deleted. Only
 * then the old backend is unloaded and the new backend is loaded.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
namespace Factory
{
    /**
     * Emits signals for Phonon::Factory.
     */
    class Sender : public QObject
    {
        Q_OBJECT
        Q_SIGNALS:
            /**
             * Emitted after the backend has successfully been changed.
             */
            void backendChanged();

            /**
             * Emitted immediately when the dtor is called.
             */
            void aboutToBeDestroyed();
    };

    /**
     * Returns a pointer to the object emitting the signals.
     *
     * \see Sender::backendChanged()
     * \see Sender::aboutToBeDestroyed()
     */
    PHONONCORE_EXPORT Sender *sender();

    /**
     * Create a new backend object for a MediaObject.
     *
     * \return a pointer to the MediaObject the backend provides.
     */
    PHONONCORE_EXPORT QObject* createMediaObject(QObject *parent = 0);
    /**
     * Create a new backend object for a MediaQueue.
     *
     * \return a pointer to the MediaQueue the backend provides.
     */
    PHONONCORE_EXPORT QObject* createMediaQueue(QObject *parent = 0);
    /**
     * Create a new backend object for a AvCapture.
     *
     * \return a pointer to the AvCapture the backend provides.
     */
    PHONONCORE_EXPORT QObject* createAvCapture(QObject *parent = 0);
    /**
     * Create a new backend object for a ByteStream.
     *
     * \return a pointer to the ByteStream the backend provides.
     */
    PHONONCORE_EXPORT QObject* createByteStream(QObject *parent = 0);

    /**
     * Create a new backend object for a AudioPath.
     *
     * \return a pointer to the AudioPath the backend provides.
     */
    PHONONCORE_EXPORT QObject* createAudioPath(QObject *parent = 0);
    /**
     * Create a new backend object for a AudioEffect.
     *
     * \return a pointer to the AudioEffect the backend provides.
     */
    PHONONCORE_EXPORT QObject* createAudioEffect(int effectId, QObject *parent = 0);
    /**
     * Create a new backend object for a VolumeFaderEffect.
     *
     * \return a pointer to the VolumeFaderEffect the backend provides.
     */
    PHONONCORE_EXPORT QObject* createVolumeFaderEffect(QObject *parent = 0);
    /**
     * Create a new backend object for a AudioOutput.
     *
     * \return a pointer to the AudioOutput the backend provides.
     */
    PHONONCORE_EXPORT QObject* createAudioOutput(QObject *parent = 0);
    /**
     * Create a new backend object for a AudioDataOutput.
     *
     * \return a pointer to the AudioDataOutput the backend provides.
     */
    PHONONCORE_EXPORT QObject* createAudioDataOutput(QObject *parent = 0);

    /**
     * Create a new backend object for a Visualization.
     *
     * \return a pointer to the Visualization the backend provides.
     */
    PHONONCORE_EXPORT QObject* createVisualization(QObject *parent = 0);

    /**
     * Create a new backend object for a VideoPath.
     *
     * \return a pointer to the VideoPath the backend provides.
     */
    PHONONCORE_EXPORT QObject* createVideoPath(QObject *parent = 0);
    /**
     * Create a new backend object for a VideoEffect.
     *
     * \return a pointer to the VideoEffect the backend provides.
     */
    PHONONCORE_EXPORT QObject* createVideoEffect(int effectId, QObject *parent = 0);
    /**
     * Create a new backend object for a BrightnessControl.
     *
     * \return a pointer to the BrightnessControl the backend provides.
     */
    PHONONCORE_EXPORT QObject* createBrightnessControl(QObject *parent = 0);
    /**
     * Create a new backend object for a BrightnessControl.
     *
     * \return a pointer to the BrightnessControl the backend provides.
     */
    PHONONCORE_EXPORT QObject *createDeinterlaceFilter(QObject *parent = 0);
    /**
     * Create a new backend object for a VideoDataOutput.
     *
     * \return a pointer to the VideoDataOutput the backend provides.
     */
    PHONONCORE_EXPORT QObject* createVideoDataOutput(QObject *parent = 0);

    /**
     * \return a pointer to the backend interface.
     */
    PHONONCORE_EXPORT QObject* backend( bool createWhenNull = true );

    /**
     * \returns The name of the library that needs to be loaded to get access to the
     * GUI dependent classes.
     *
     * \see \ref phonon_UiBackend
     */
    PHONONCORE_EXPORT const char* uiLibrary();

    /**
     * This method is useful if you want to put the GUI dependent and GUI indepentend
     * classes into the same DSO. Then \ref phonon_Backend_uiLibrary "uiLibrary()"
     * returns the same library as is specified in the .desktop file.
     * \returns The symbol in the library that needs to be called to get access to the
     * \ref phonon_UiBackend "UiBackend" instance.
     */
    PHONONCORE_EXPORT const char* uiSymbol();

    /**
     * Unique identifier for the Backend. Can be used in configuration files
     * for example.
     */
    QString identifier();

    /**
     * Get the name of the Backend. It's the name from the .desktop file.
     */
    PHONONCORE_EXPORT QString backendName();

    /**
     * Get the comment of the Backend. It's the comment from the .desktop file.
     */
    QString backendComment();

    /**
     * Get the version of the Backend. It's the version from the .desktop file.
     *
     * The version is especially interesting if there are several versions
     * available for binary incompatible versions of the backend's media
     * framework.
     */
    QString backendVersion();

    /**
     * Get the icon (name) of the Backend. It's the icon from the .desktop file.
     */
    QString backendIcon();

    /**
     * Get the website of the Backend. It's the website from the .desktop file.
     */
    QString backendWebsite();

    /**
     * registers the backend object
     */
    PHONONCORE_EXPORT QObject* registerQObject( QObject* o );
    PHONONCORE_EXPORT void registerFrontendObject(BasePrivate *);
    PHONONCORE_EXPORT void deregisterFrontendObject(BasePrivate *);
    PHONONCORE_EXPORT const KComponentData &componentData();

    PHONONCORE_EXPORT void createBackend(const QString &library, const QString &version = QString());

//X    It is probably better if we can get away with internal handling of
//X    freeing the soundcard device when it's not needed anymore and
//X    providing an IPC method to stop all MediaProducers -> free all
//X    devices
//X    /**
//X     * \internal
//X     * This is called when the application needs to free the soundcard
//X     * device(s).
//X     */
//X    void freeSoundcardDevices();
} // namespace Factory
} // namespace Phonon

#endif // PHONON_FACTORY_H
// vim: sw=4 ts=4
