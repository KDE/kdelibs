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
#ifndef Phonon_AUDIOEFFECT_H
#define Phonon_AUDIOEFFECT_H

#include "base.h"
#include "effect.h"
#include "phonondefs.h"
#include <QObject>
#include "objectdescription.h"

class QString;
template<class T> class QList;

namespace Phonon
{
    class EffectParameter;
    class AudioEffectPrivate;

    /**
     * \short Audio effects that can be inserted into an AudioPath.
     * An audio effect is a special object which can perform
     * transformations on the specified AudioPath. Examples may include simple
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
     * The AudioEffect will immediately begin applying it's transformations on
     * the AudioPath. To stop it, remove the Effect from the AudioPath.
     *
     * \warning This class is not finished.
     *
     * \author Matthias Kretz <kretz@kde.org>
     */
    class PHONONCORE_EXPORT AudioEffect : public QObject, public Effect, public Base
    {
        friend class AudioPath;
        friend class AudioPathPrivate;
        Q_OBJECT
        K_DECLARE_PRIVATE(AudioEffect)

        public:
            /**
             * QObject constructor.
             *
             * \param type An AudioEffectDescription object to determine the
             * type of effect. See \ref
             * BackendCapabilities::availableAudioEffects().
             * \param parent QObject parent
             */
            explicit AudioEffect(const AudioEffectDescription &type, QObject *parent = 0);

            /**
             * Returns the type of this effect. This is the same type as was
             * passed to the constructor.
             */
            AudioEffectDescription type() const;

            /**
             * Returns a list of parameters that this effect provides to control
             * its behaviour.
             *
             * \see EffectParameter
             * \see EffectWidget
             */
            virtual QList<EffectParameter> parameterList() const;

        protected:
            /**
             * \internal
             *
             * Constructs new audio effect with private data \p dd and a
             * \p parent.
             */
            AudioEffect(AudioEffectPrivate &dd, QObject *parent, const AudioEffectDescription &type = AudioEffectDescription());

        protected:
            virtual QVariant value(int parameterId) const;
            virtual void setValue(int parameterId, QVariant newValue);
    };
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_AUDIOEFFECT_H
