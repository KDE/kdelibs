/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_PATH_H
#define PHONON_PATH_H

#include "phonon_export.h"
#include "objectdescription.h"

#include <QtCore/QExplicitlySharedDataPointer>

template<class T> class QList;

namespace Phonon
{

class PathPrivate;
class Effect;
class MediaNode;
class PHONON_EXPORT Path
{
    public:
        ~Path();
        Path(); // invalid
        Path(const Path &);
        bool isValid() const;
        //MediaStreamTypes mediaStreamTypes() const;

        /**
         * Creates and inserts an effect into the path.
         *
         * You may insert effects of the same class as often as you like,
         * but if you insert the same object, the call will fail.
         *
         * \param newEffect An Effect object.
         *
         * \param insertBefore If you already inserted an effect you can
         * tell with this parameter in which order the data gets
         * processed. If this is \c 0 the effect is appended at the end of
         * the processing list. If the effect has not been inserted before
         * the method will do nothing and return \c false.
         *
         * \return Returns whether the effect could be inserted at the
         * specified position. If \c false is returned the effect was not
         * inserted.
         *
         * \see removeEffect
         * \see effects
         */
        Effect *insertEffect(const EffectDescription &desc, Effect *insertBefore = 0);

        /**
         * Inserts an effect into the path.
         *
         * You may insert effects of the same class as often as you like,
         * but if you insert the same object, the call will fail.
         *
         * \param newEffect An Effect object.
         *
         * \param insertBefore If you already inserted an effect you can
         * tell with this parameter in which order the data gets
         * processed. If this is \c 0 the effect is appended at the end of
         * the processing list. If the effect has not been inserted before
         * the method will do nothing and return \c false.
         *
         * \return Returns whether the effect could be inserted at the
         * specified position. If \c false is returned the effect was not
         * inserted.
         *
         * \see removeEffect
         * \see effects
         */
        bool insertEffect(Effect *newEffect, Effect *insertBefore = 0);

        /**
         * Removes an effect from the path.
         *
         * If the effect gets deleted while it is still connected the effect
         * will be removed automatically.
         *
         * \param effect The effect to be removed.
         *
         * \return Returns whether the call was successful. If it returns
         * \c false the effect could not be found in the path, meaning it
         * has not been inserted before.
         *
         * \see insertEffect
         * \see effects
         */
        bool removeEffect(Effect *);

        /**
         * Returns a list of Effect objects that are currently
         * used as effects. The order in the list determines the order the
         * signal is sent through the effects.
         *
         * \return A list with all current effects.
         *
         * \see insertEffect
         * \see removeEffect
         */
        QList<Effect *> effects() const;

        /**
         * if reconnect fails the old connection is kept
         */
        bool reconnect(MediaNode *source, MediaNode *sink);
        /**
         * invalidates the Path (no connections to either source or sink are left
         */
        bool disconnect();

        Path &operator=(const Path &);
        bool operator==(const Path &) const;
        bool operator!=(const Path &) const;

    private:
        friend class PathPrivate;
        QExplicitlySharedDataPointer<PathPrivate> d;
};

/**
 * Creates a new Path connecting two MediaNodes.
 *
 * The implementation will automatically select the right format and media type. E.g. connecting a
 * MediaObject and AudioOutput will create a Path object connecting the audio. This might be
 * represented as PCM or perhaps even AC3 depending on the AudioOutput object.
 *
 * \param source The MediaNode to connect an output from
 * \param sink The MediaNode to connect to.
 */
PHONON_EXPORT Path createPath(MediaNode *source, MediaNode *sink);

} // namespace Phonon

#endif // PHONON_PATH_H
