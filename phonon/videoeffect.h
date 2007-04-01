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
#ifndef Phonon_VIDEOEFFECT_H
#define Phonon_VIDEOEFFECT_H

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
	class VideoEffectPrivate;

	/**
	 * \short Video effects that can be inserted into a VideoPath.
	 * A video effect is a special object which can perform
	 * transformations on the specified VideoPath. Examples may include simple
	 * modifiers such as brightness or colour controls, or more complex mathematical
	 * transformations.
	 *
	 * In order to use an effect, insert it into the path as follows:
	 * \code
	 * VideoPath* path = new VideoPath( this );
	 * VideoEffect *effect = new VideoEffect( this );
	 * path->insertEffect( effect );
	 * \endcode
	 *
	 * The VideoEffect will immediately begin applying it's transformations on
	 * the VideoPath. To stop it, remove the Effect from the VideoPath.
	 *
	 * \warning This class is not finished.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONCORE_EXPORT VideoEffect : public QObject, public Effect, public Base
	{
		friend class VideoPath;
		friend class VideoPathPrivate;
		Q_OBJECT
		K_DECLARE_PRIVATE( VideoEffect )

		public:
			/**
             * QObject constructor.
             *
             * \param type A VideoEffectDescription object to determine the
             * type of effect. See \ref
             * BackendCapabilities::availableVideoEffects().
			 * \param parent QObject parent
			 */
            explicit VideoEffect(const VideoEffectDescription &type, QObject *parent = 0);

            /**
             * Returns the type of this effect. This is the same type as was
             * passed to the constructor.
             */
			VideoEffectDescription type() const;

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
			 * Constructs new video effect with private data \p dd and a
			 * \p parent.
			 */
			VideoEffect( VideoEffectPrivate& dd, QObject* parent, const VideoEffectDescription& type = VideoEffectDescription() );

			/**
			 * \internal
			 * After construction of the Iface object this method is called
			 * throughout the complete class hierarchy in order to set up the
			 * properties that were already set on the public interface.
			 *
			 * An example implementation could look like this:
			 * \code
			 * ParentClass::setupIface();
			 * m_iface->setPropertyA( d->propertyA );
			 * m_iface->setPropertyB( d->propertyB );
			 * \endcode
			 */
			void setupIface();

		protected:
			virtual QVariant value( int parameterId ) const;
			virtual void setValue( int parameterId, QVariant newValue );
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_VIDEOEFFECT_H
