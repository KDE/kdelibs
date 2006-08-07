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
#include "phonondefs.h"
#include <QObject>
#include "effect.h"
#include "objectdescription.h"

class QString;

namespace Phonon
{
	class VideoEffectPrivate;

	class PHONONCORE_EXPORT VideoEffect : public QObject, public Effect, public Base
	{
		friend class VideoPath;
		friend class VideoPathPrivate;
		Q_OBJECT
		K_DECLARE_PRIVATE( VideoEffect )

		public:
			/**
			 * Standard QObject constructor.
			 *
			 * \param parent QObject parent
			 */
			VideoEffect( const VideoEffectDescription& type, QObject* parent = 0 );

			VideoEffectDescription type() const;
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

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_VIDEOEFFECT_H
