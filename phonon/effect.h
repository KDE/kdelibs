/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Matthias Kretz <kretz@kde.org>

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
#ifndef PHONON_EFFECT_H
#define PHONON_EFFECT_H

#include "phonondefs.h"
#include <QtCore/QObject>
#include "objectdescription.h"

class QString;
template<class T> class QList;

namespace Phonon
{
    class EffectParameter;
    class EffectPrivate;

    /**
     * \short Effects that can be inserted into an AudioPath or a VideoPath.
     * An effect is a special object which can perform
     * transformations on the specified path. Examples may include simple
     * modifiers such as fading or pitch shifting, or more complex mathematical
     * transformations.
     *
     * In order to use an effect, insert it into the path as follows:
     * \code
     * AudioPath *path = new AudioPath(this);
     * AudioEffect *effect = new AudioEffect(this);
     * path->insertEffect(effect);
     * \endcode
     *
     * The effect will immediately begin applying it's transformations on
     * the path. To stop it, remove the Effect from the path.
     *
     * \ingroup PhononEffects
     * \author Matthias Kretz <kretz@kde.org>
     */
    class PHONON_EXPORT Effect : public QObject
    {
        friend class AudioPath;
        friend class VideoPath;
        friend class AudioPathPrivate;
        friend class VideoPathPrivate;
        Q_OBJECT
        K_DECLARE_PRIVATE(Effect)

        public:
            ~Effect();

            enum Type {
                AudioEffect,
                VideoEffect
            };

            /**
             * QObject constructor.
             *
             * \param description An AudioEffectDescription object to determine the
             * type of effect. See \ref
             * BackendCapabilities::availableAudioEffects().
             * \param parent QObject parent
             */
            explicit Effect(const AudioEffectDescription &description, QObject *parent = 0);

            /**
             * QObject constructor.
             *
             * \param description A VideoEffectDescription object to determine the
             * type of effect. See \ref
             * BackendCapabilities::availableVideoEffects().
             * \param parent QObject parent
             */
            explicit Effect(const VideoEffectDescription &description, QObject *parent = 0);

            Type type() const;

            /**
             * Returns the audio description of this effect. This is the same type as was
             * passed to the constructor.
             */
            AudioEffectDescription audioDescription() const;

            /**
             * Returns the video description of this effect. This is the same type as was
             * passed to the constructor.
             */
            VideoEffectDescription videoDescription() const;

            /**
             * Returns a list of parameters that this effect provides to control
             * its behaviour.
             *
             * \see EffectParameter
             * \see EffectWidget
             */
            QList<EffectParameter> allDescriptions() const;
            EffectParameter description(int index) const;
            int parameterCount() const;

            QVariant parameterValue(int index) const;
            void setParameterValue(int index, const QVariant &value);

        protected:
            Effect(EffectPrivate &dd, QObject *parent);

            EffectPrivate *const k_ptr;
    };
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_EFFECT_H
