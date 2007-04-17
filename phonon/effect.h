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

#ifndef PHONON_EFFECT_H
#define PHONON_EFFECT_H

#include "phonon_export.h"
#include <QVariant>

template<class T> class QList;

namespace Phonon
{

class EffectParameter;

/**
 * \internal
 *
 * \brief Common interface for audio and video effects.
 *
 * This class is an implementation detail: In order for the Ui::EffectWidget and
 * EffectParameter classes to work for both AudioEffect and VideoEffect objects
 * this common interface is used.
 *
 * \see AudioEffect
 * \see VideoEffect
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONCORE_EXPORT Effect
{
    friend class EffectParameter;
    public:
        virtual ~Effect();

        /**
         * Returns a list of all parameters of the effect.
         *
         * \see EffectParameter
         */
        virtual QList<EffectParameter> parameterList() const = 0;

    protected:
        /**
         * Gets the value of the parameter with the id \p parameterId.
         */
        KDE_NO_EXPORT virtual QVariant value(int parameterId) const = 0;

        /**
         * Sets the value of the parameter with the id \p parameterId.
         */
        KDE_NO_EXPORT virtual void setValue(int parameterId, QVariant newValue) = 0;
};

} // namespace Phonon

#endif // PHONON_EFFECT_H
// vim: sw=4 ts=4 tw=80
